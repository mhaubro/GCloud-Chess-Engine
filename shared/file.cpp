#include <fstream>
#include <string>
#include <filesystem>
#include "shared.h"
#include <mutex>

mutex logfile_mutex;

string os_get_path_to_executable();
bool logging_enabled = false; // Set when engine configuration is read

// Useful for file loads
string file_get_parent_folder_path() {
    // Shave off the executable part of full path
    return filesystem::path(os_get_path_to_executable()).parent_path().string();
}

void create_folder_if_missing(string path) {
    if (!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
}

string file_get_parent_folder_name() {
    return filesystem::path(file_get_parent_folder_path()).filename().string();
}

void log_output(string data) {
    if (!logging_enabled) {
        return;
    }
    ofstream logfile;
    logfile_mutex.lock();
    logfile.open (file_get_parent_folder_path() + os_path_separator + "log.txt", ios_base::app);
    logfile << data;
    logfile.close();
    logfile_mutex.unlock();
}
