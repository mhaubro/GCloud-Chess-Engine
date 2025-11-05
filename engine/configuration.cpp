#include <string>
#include <filesystem>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "configuration.h"
#include "shared.h"

engine_configuration engine_configuration_global;
extern bool logging_enabled;
using namespace std;

void engine_read_configuration() {
    string engine_config_path = file_get_parent_folder_path() + os_path_separator + "engine.yml";
    engine_configuration_global.instance_name = file_get_parent_folder_name();
    if (!filesystem::exists(engine_config_path)) {
        cout << "engine.yml missing. Expected at: " << engine_config_path << endl;
        throw "";
    }
    // Read engine 
    YAML::Node config = YAML::LoadFile(engine_config_path);
    if (config["logging"]) {
        logging_enabled = (config["logging"].as<string>().compare(string("enabled")) == 0);
        log_output("logging enabled: " + to_string(logging_enabled) + "\n");
    }

    if (config["executable_path"]) {
        engine_configuration_global.executable_path = config["executable_path"].as<string>();
        log_output("executable_path: " + engine_configuration_global.executable_path + "\n");
    } else {
        log_output("executable_path not found\n");
        throw "";
    }

    if (config["uci_output"]) {
        engine_configuration_global.uci_output = config["uci_output"].as<string>();
        log_output("uci_output: " + engine_configuration_global.executable_path + "\n");
    } else {
        log_output("uci_output not found\n");
        throw "";
    }
    if (config["zone"]) {
        engine_configuration_global.zone = config["zone"].as<string>();
        log_output("zone: " + engine_configuration_global.executable_path + "\n");
    } else {
        log_output("zone not found\n");
        throw "";
    }
    // Non-mandatory fields
    if (config["instance_name"]) {
        engine_configuration_global.uci_output = config["uci_output"].as<string>();
        log_output("instance name set in setting: " + engine_configuration_global.instance_name + "\n");
    } else {
        log_output("instance name: " + engine_configuration_global.instance_name + "\n");
    }
    if (config["hash"]) {
        engine_configuration_global.hash = stoi(config["hash"].as<string>());
        log_output("Hash, Mb: " + to_string(engine_configuration_global.hash) + "\n");
    } else {
        engine_configuration_global.hash = 0;
    }
    if (config["cpus"]) {
        engine_configuration_global.cpus = stoi(config["cpus"].as<string>());
        log_output("CPU threads: " + to_string(engine_configuration_global.cpus) + "\n");
    } else {
        engine_configuration_global.cpus = 0;
    }
}

// Requires the gcloud machine to be on, but analysis not running.
void engine_get_machine_data() {
    if (engine_configuration_global.cpus == 0) {
        ssh_write("nproc --all\n");
        string output = ssh_return_first_data_and_empty_buffer();

        // Reserve 2 CPU cores for OS, ssh etc to not bog down the remote engine
        engine_configuration_global.cpus = stoi(output);
        if (engine_configuration_global.cpus > 2) {
            engine_configuration_global.cpus -= 2;
        } else {
            engine_configuration_global.cpus = 1;
        }
    }
    
    if (engine_configuration_global.hash == 0) {
        ssh_write("awk '/MemTotal/{print $2}' /proc/meminfo\n");
        string output = ssh_return_first_data_and_empty_buffer();

        engine_configuration_global.hash = stoi(output) / 1024;
        // Reserve 8gb or half of total memory to OS, depending on what is smaller
        if (engine_configuration_global.hash > 16384) {
            engine_configuration_global.hash -= 8192;
        } else {
            engine_configuration_global.hash /= 2;
        }
    }
}
