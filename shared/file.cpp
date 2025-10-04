#include <fstream>
#include <string>
#include <filesystem>
#include "shared.h"
#include <mutex>

mutex logfile_mutex;

string os_get_path_to_executable();

// Useful for file loads
string file_get_parent_folder_path() {
    // Shave off the executable part of full path
    return filesystem::path(os_get_path_to_executable()).parent_path().string();
}

string file_get_parent_folder_name() {
    return filesystem::path(file_get_parent_folder_path()).filename().string();
}

void log_output(string data) {
    ofstream logfile;
    logfile_mutex.lock();
    logfile.open (file_get_parent_folder_path() + os_path_separator + "log.txt", ios_base::app);
    logfile << data;
    logfile.close();
    logfile_mutex.unlock();
}
