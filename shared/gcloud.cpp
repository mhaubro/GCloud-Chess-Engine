#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include "configuration.h"
#include "shared.h"

using namespace std;

extern string gcloud_command_name;
string instance;
string zone;
string ssh_username = "";

void gcloud_cache_settings(string gcloud_instance, string gcloud_zone) {
    instance = gcloud_instance;
    zone = gcloud_zone;
}

void gcloud_set_project_id(string project_id) {
    os_execute_local_shell_command(gcloud_command_name + " config set project " + project_id);
}

string gcloud_instance_start() {
    string output = os_execute_local_shell_command(gcloud_command_name + " compute instances start " + instance + " --zone=" + zone);
    if (output.find("ERROR: (gcloud.compute.instances.start) You do not currently have an active account selected") != string::npos) {
        // Do a login before starting. Should be a one-time thing, but catching it here is reasonable.
        os_execute_local_shell_command(gcloud_command_name + " auth login");
        output = os_execute_local_shell_command(gcloud_command_name + " compute instances start " + instance + " --zone=" + zone);
    }
    return output;
}

string gcloud_terminate_instance() {
    return os_execute_local_shell_command(gcloud_command_name + " compute instances stop " + instance + " --zone=" + zone + " --async");
}

string gcloud_get_ip_address() {
    #ifdef _WIN32
    string ip_format = " --format=get(networkInterfaces[0].accessConfigs.natIP)";
    #else
    string ip_format = " --format='get(networkInterfaces[0].accessConfigs.natIP)'";
    #endif
	return os_execute_local_shell_command(gcloud_command_name + " compute instances describe " + instance + " --zone=" + zone + ip_format);
}

std::string load_file(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open " + path);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string last_word(std::string s) {
    std::string ws = " \t\n\r\f\v";
    auto end = s.find_last_not_of(ws);
    if (end == std::string::npos) return {};            // all whitespace or empty
    auto start = s.find_last_of(ws, end);
    if (start == std::string::npos) return s.substr(0, end + 1);
    return s.substr(start + 1, end - start);
}

string gcloud_execute_command(string command) {
    create_folder_if_missing(ssh_get_private_key_folder());
    if (ssh_username != "") {
        return os_execute_local_shell_command(gcloud_command_name + " compute ssh " + ssh_username + "@" + instance + " --zone=" + zone + " --command=\"" + command + "\"");
    } else {
        string output = os_execute_local_shell_command(gcloud_command_name + " compute ssh " + instance + " --zone=" + zone + " --command=\"" + command + "\"");
        if (output.find("FATAL ERROR: No supported authentication methods available (server sent: publickey)") != string::npos) {
            log_output("Initial SSH Key authentication failed.");
            string pub_file = ssh_get_private_key_filename() + ".pub";
            string content = load_file(pub_file);
            string username_server = last_word(content);
            unsigned int separator = username_server.find('@');
            ssh_username = username_server.substr(0, separator);
            if (ssh_username.find("\\") != string::npos) {
                // SSH-key has had "<machinename>\" prependend to username
                // This has been uploaded to gcloud, so this is now our
                // username.
                output = os_execute_local_shell_command(gcloud_command_name + " compute ssh " + ssh_username + "@" + instance + " --zone=" + zone + " --command=\"" + command + "\"");
            } else {
                throw runtime_error("SSH Login failed manually.");
            }
        }
        return output;
    }
}

// This will ensure that an up-to-date ssh-key is present in the users .ssh-directory, eliminating needs to rely on user keys
string gcloud_execute_dummy_command() {
    return gcloud_execute_command("ls");
}
