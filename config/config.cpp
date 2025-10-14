#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <vector>

#include "shared.h"

class Engineconfig {
public:
    string engine_name;
    string executable_path;
    vector<string> setup_commands;
    vector<string> neural_nets;
    string uci_output;
    string instance_name;
    string zone;

    string toString();
    void setup_remote_engine();
    void get_uci_output();
    bool check_gcloud_engine_subdirectory();
    void emit_gcloud_config();
};

void Engineconfig::setup_remote_engine() {
    gcloud_cache_settings(instance_name, zone);
    string output = gcloud_instance_start();
    if (output.find("ERROR") != string::npos && output.find("gcloud auth login") != string::npos) {
        // Run gcloud auth login
        cout << "A browser window for login to will open. Log in and close the browser window to successfully complete the login." << endl;
        os_execute_local_shell_command(gcloud_command_name + " auth login");
        output = gcloud_instance_start();
    }

    if (output.find("ERROR: (gcloud.compute.instances.start) The required property [project] is not currently set") != string::npos) {
        // Ask user to set project.
        cout << "Set the project ID for your gcloud instance" << endl;
        string project_id;
        getline(cin, project_id);
        gcloud_set_project_id(project_id);
        gcloud_instance_start();
    }

    gcloud_execute_dummy_command();
    ssh_connection_start();
    // Will block until done
    cout << "Executing " << to_string(setup_commands.size()) << " setup commands" << endl;
    for (unsigned int i = 0; i < setup_commands.size(); i++) {
        cout << "Executing setup command " << to_string(i + 1) << endl;
        string output = gcloud_execute_command(setup_commands[i]);
        if (output.find("reboot") != string::npos) {
            cout << "Compute engine machine is rebooting. Waiting " << endl;
            // A reboot is imminent, so sleep for 60s and reconnect over ssh.
            ssh_connection_terminate();
            sleep_ms(90 * 1000);
            ssh_connection_start();
        }
    }
    if (neural_nets.size() > 0) {
        gcloud_execute_command("cd ~"); // Ensure location is home directory
        for (unsigned int i = 0; i < neural_nets.size(); i++) {
            cout << "Downloading neural network " << to_string(i) << " out of " << to_string(neural_nets.size()) << " in total" << endl;
            gcloud_execute_command("wget " + neural_nets[i]);
        }
    }
}

void Engineconfig::get_uci_output() {
    ssh_write(executable_path + "\n");
    // Swallow all output from the commands
    sleep_ms(1500);
    // A blocking read does not work here, as Lc0 does not output data,
    // but stockfish does, and we only want the output of the uci command
    ssh_read(); 

    ssh_write("uci\n");
    uci_output = "";
    string data_read = ssh_read();
    do {
        // cout << "data_read: " << data_read << endl;
        uci_output += data_read;
        sleep_ms(100);
        data_read = ssh_read();
    } while (uci_output.find("uciok") == string::npos);
}

bool Engineconfig::check_gcloud_engine_subdirectory() {
    string new_folder_path = file_get_parent_folder_path() + os_path_separator + instance_name;

    bool new_folder_exists_already = filesystem::exists(new_folder_path);
    if (new_folder_exists_already) {
        cout << "New folder for instance: " + instance_name + " with full path '" + new_folder_path + "' already exists. Terminating.\n";
        return false;
    }
    return true;
}


void Engineconfig::emit_gcloud_config() {
    string new_folder_path = file_get_parent_folder_path() + os_path_separator + instance_name;

    bool status = filesystem::create_directory(new_folder_path);
    if (!status) {
        cout << "Creating folder failed\n";
        return;
    }

    // For windows: Copy dlls and exe. This should possibly live as OS Specific.
    log_output("Copying dll's and .exe.\n");
    // This is a bit of powershell magic
    os_execute_local_shell_command("powershell Copy-Item -path \"" + file_get_parent_folder_path() + "\\*\" -include \"*.dll\" -Destination \"" + new_folder_path + "\"");
    filesystem::copy_file(file_get_parent_folder_path() + "\\gcloud_engine.exe", new_folder_path + "\\gcloud_engine.exe");
    log_output("Emitting engine.yml\n");
    // Emit yml configuration file
    YAML::Node node;
    node["executable_path"] = executable_path;
    node["uci_output"] = uci_output;
    node["instance_name"] = instance_name;
    node["zone"] = zone;
    std::ofstream fout(file_get_parent_folder_path() + os_path_separator + instance_name + os_path_separator + "engine.yml");
    fout << node;
}

string Engineconfig::toString() {
    string s = "Name: " + engine_name + "\n";
    s += "Executable: " + executable_path + "\n";
    s += "Setup commands:\n";
    for (unsigned int i = 0; i < setup_commands.size(); i++) {
        s += setup_commands[i] + "\n";
    }
    s += "neural nets:\n";
    for (unsigned int i = 0; i < neural_nets.size(); i++) {
        s += neural_nets[i] + "\n";
    }
    return s;
}

using namespace std;
extern bool logging_enabled;
vector<Engineconfig> engines;

void create_engine() {
    logging_enabled = true;

    string foss_list_path = file_get_parent_folder_path() + os_path_separator + "foss_engines.yml";
    if (!filesystem::exists(foss_list_path)) {
        cout << "foss_engines.yml missing. Expected at: " << foss_list_path << endl;
        throw "";
    }
    // Read engine 
    YAML::Node config = YAML::LoadFile(foss_list_path);
    for (YAML::Node node : config["engines"]) {
        Engineconfig engine;
        engine.engine_name = node["name"].as<string>();
        engine.executable_path = node["executable_path"].as<string>();
        for (YAML::Node child : node["setup"]) {
            engine.setup_commands.push_back(child.as<string>());
        }
        for (YAML::Node child : node["neural_nets"]) {
            engine.neural_nets.push_back(child.as<string>());
        }
        engines.push_back(engine);
        log_output("Engine read from YML file.\n" + engine.toString());
    }
    cout << "The following options are possible to enter for setup:" << endl;
    cout << "Press [q] followed by enter for quitting" << endl;

    Engineconfig engine_for_setup;
    for (unsigned int i = 0; i < engines.size(); i++) {
        cout << "[" + to_string(i) + "] for setting up " + engines[i].engine_name << endl;
    }
    string line;
    getline(cin, line);
    unsigned int index;
    try {
        index = stoi(line);
    }
    catch (...) {
        cout << "input was '" + line + "'. Terminating.\n";
        return;
    }
    if (index > engines.size()) {
        cout << "input was '" + line + "'. There is no engine with that number. Terminating.\n";
        return;
    }
    engine_for_setup = engines[index];
    // Here it's assumed that the action is correctly taken. Next step: Instance name
    cout << "Please enter the google cloud compute engine instance name followed by enter:\n";
    getline(cin, line);
    engine_for_setup.instance_name = line; // There's no way to validate this
    cout << "Please enter the zone of the machine:\n";
    getline(cin, line);
    engine_for_setup.zone = line; // There's no way to validate this
    gcloud_cache_settings(engine_for_setup.instance_name, engine_for_setup.zone);
    if (!engine_for_setup.check_gcloud_engine_subdirectory()) {
        return;
    }
    engine_for_setup.setup_remote_engine();
    engine_for_setup.get_uci_output();
    engine_for_setup.emit_gcloud_config();
    cout << "Setup of engine completed. Enter any input to terminate.\n";
    gcloud_terminate_instance();
    getline(cin, line);
}
