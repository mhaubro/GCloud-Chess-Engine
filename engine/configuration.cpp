#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "configuration.h"
#include "shared.h"

engine_configuration engine_configuration_global;

using namespace std;

void engine_read_configuration() {
    string engine_config_path = file_get_parent_folder_path() + os_path_separator + "engine.yml";
    engine_configuration_global.instance_name = file_get_parent_folder_name();
    if (!filesystem::exists(engine_config_path)) {
        log_output("engine.yml missing. Expected at: " + engine_config_path + "\n");
        throw "";
    }
    // Read engine 
    YAML::Node config = YAML::LoadFile(engine_config_path);
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
}

// Requires the gcloud machine to be on, but analysis not running.
void engine_get_machine_data() {
    ssh_write("nproc --all\n");
    sleep_ms(200);
    engine_configuration_global.cpus = stoi(ssh_read());
    ssh_write("awk '/MemTotal/{print $2}' /proc/meminfo\n");
    sleep_ms(200);
    engine_configuration_global.hash = stoi(ssh_read()) / 1024;
    // Reserve 4gb or half of total memory to OS, depending on what is smaller
    if (engine_configuration_global.hash > 8192) {
        engine_configuration_global.hash -= 4096;
    } else {
        engine_configuration_global.hash /= 2;
    }
}
