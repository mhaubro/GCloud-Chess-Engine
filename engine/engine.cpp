#include <iostream>
#include <list>
#include <thread>
#include <mutex>

#include "shared.h"
#include "configuration.h"

using namespace std;
mutex uci_commands_mutex;
list<string> uci_commands_incoming;
string setoption_hash_string = "setoption name Hash value ";
string setoption_threads_string = "setoption name Threads value ";


// Thread to be started on program boot. Will always listen to cin, and
// Reply to the commands 'uci' and 'readyok' immediately.
// Will put all commands in a list, which can then be consumed by the engine
// once it's up and running. Since cin is a buffer, this thread is necessary.
void engine_command_listener() {
    // Initial chessbase trickery, making it think all is ok
    for (string line; getline(cin, line);) {
        log_output("Incoming on cin: " + string(line) + "\n");
        uci_commands_mutex.lock(); // Controls both the list and access to cout.

        // If it's a "uci" or "isready", it is not added to command list, instead reply is immediate
        if (line.compare(0, string("uci").length(), string("uci")) == 0) {
            log_output("Outputting on cin: " + engine_configuration_global.uci_output + "\n");
            cout << engine_configuration_global.uci_output << endl;
        } else if (line.compare(0, string("isready").length(), string("isready")) == 0) {
            log_output("Outputting on cin: readyok\n");
            cout << "readyok" << endl;
        } else {
            // Add command to command list
            uci_commands_incoming.push_back(line);
        }
        uci_commands_mutex.unlock();
        if (line.compare("quit") == 0) {
            return;
        }
    }
}

static bool send_all_commands() {
    while(!uci_commands_incoming.empty()) {
        // Get the commands from the list top-down
        uci_commands_mutex.lock();
        string command = uci_commands_incoming.front();
        uci_commands_incoming.pop_front();
        uci_commands_mutex.unlock();

        if (command.compare(0, setoption_hash_string.length(), setoption_hash_string) == 0) { // Necessary to overwrite the hash value from CB
            command = setoption_hash_string + to_string(engine_configuration_global.hash);
        }
        if (command.compare(0, setoption_threads_string.length(), setoption_threads_string) == 0) { // Necessary to overwrite the hash value from CB
            command = setoption_threads_string + to_string(engine_configuration_global.cpus);
        }

        // Incoming data on cin does not contain newlines that are needed for terminal activation on remote
        command.push_back('\n');

        ssh_write(command);
        // Persist in log
        if (command.compare("quit\n") == 0) { // A quit-command arrived, so we return false indicating the program shouldn't continue
            return false;
        }
    }
    return true;
}

// Will consistently read and write from the ssh.
// Will read every 100ms nonblocking.
// Will write whenever a new command has been picked up by that listener.
// Will react to a 'quit' by writing it and then returning.
void engine_run() {
    if (!send_all_commands()) {
        return;
    }
    sleep_ms(1000);

    // Swallow all output from the commands
    string output = ssh_read();

    // Engine should be running now. No duplicate startup output to trip up chessbase should be in the cache either.
    // Forever: Pipe forward data from engine to cout, and send commands the other way
    while(true) {
        output = ssh_read();
        if (!output.empty()) {
            uci_commands_mutex.lock();
            log_output("Transmitting to CB: " + string(output));
            cout << output << endl;
            uci_commands_mutex.unlock();
        }
        if (!send_all_commands()) {
            return;
        }
        sleep_ms(200);
    }
}
