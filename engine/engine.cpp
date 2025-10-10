#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <semaphore>

#include "shared.h"
#include "configuration.h"

using namespace std;
mutex uci_commands_mutex;
list<string> uci_commands_incoming;
string setoption_hash_string = "setoption name Hash value ";
string setoption_threads_string = "setoption name Threads value ";
extern bool quit_received;
extern binary_semaphore command_received;
bool main_thread_signaled = false;

// Used as the main thread does not want to start a gcloud instance when
// a user is only checking the parameters. So a 'uci' or 'quit' command
// doesn't cause the VM to start, creating for a better user experience.
void signal_main_thread() {
    if (!main_thread_signaled) {
        command_received.release();
    }
}

// CB sometimes sends erratic quits. If the previous command has not been a
// 'stop' and the remote engine has been started, ignore the quit and stay alive.
static bool previous_command_stop = false;

// Thread to be started on program boot. Will always listen to cin, and
// Reply to the commands 'uci' and 'readyok' immediately.
// Will put all commands in a list, which can then be consumed by the engine
// once it's up and running. Since cin is a buffer, this thread is necessary.
void engine_command_listener() {
    for (string line; getline(cin, line);) {
        log_output("Incoming on cin: " + string(line) + "\n");
        uci_commands_mutex.lock(); // Controls both the list and access to cout.

        // If it's a "uci" or "isready", it is not added to command list, instead reply is immediate
        if (line.compare(0, string("uci").length(), string("uci")) == 0) {
            previous_command_stop = false;
            log_output("Outputting on cout: " + engine_configuration_global.uci_output + "\n");
            cout << engine_configuration_global.uci_output << endl;
        } else if (line.compare(0, string("isready").length(), string("isready")) == 0) {
            previous_command_stop = false;
            log_output("Outputting on cout: readyok\n");
            cout << "readyok" << endl;
            signal_main_thread();
        } else if (line.compare(0, string("stop").length(), string("stop")) == 0) {
            previous_command_stop = true;
            uci_commands_mutex.unlock();
            signal_main_thread();
        } else if (line.compare(0, string("quit").length(), string("quit")) == 0) {
            if (previous_command_stop || !main_thread_signaled) {
                quit_received = true;
                uci_commands_incoming.push_back(line);
                uci_commands_mutex.unlock();
                signal_main_thread();
                return;
            }
        } else {
            previous_command_stop = false;
            // Add command to command list
            signal_main_thread();
            uci_commands_incoming.push_back(line);
        }
        uci_commands_mutex.unlock();
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
        if (command.find("RamLimitMb") != string::npos) { // Ram limit mb is written automatically during startup
            continue;
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
    // Get the machine data before starting stockfish.
    log_output("Getting machine data.\n");
    engine_get_machine_data();

    log_output("Starting engine program.\n");
    string command = engine_configuration_global.executable_path + "\n";
    ssh_write(command);
    if (engine_configuration_global.uci_output.find("RamLimitMb") != string::npos) {
        ssh_write("setoption name RamLimitMb value " + to_string(engine_configuration_global.hash) + "\n");
    }
    // Starting engine
    if (!send_all_commands()) {
        return;
    }
    sleep_ms(1000);

    // Swallow all output from the commands
    string output = ssh_read();

    // Engine should be running now. No duplicate startup output to trip up chessbase should be in the cache either.
    // Forever: Pipe forward data from engine to cout, and send commands the other way
    log_output("Starting run of main-loop.\n");
    while(true) {
        output = ssh_read();
        if (!output.empty()) {
            log_output("Transmitting to CB: " + string(output));
            cout << output << endl;
        }
        if (!send_all_commands()) {
            return;
        }
        sleep_ms(200);
    }
}
