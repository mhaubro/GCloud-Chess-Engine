#include <string>
#include "configuration.h"
#include "shared.h"

using namespace std;

extern string gcloud_command_name;
string instance;
string zone;

void gcloud_cache_settings(string gcloud_instance, string gcloud_zone) {
    instance = gcloud_instance;
    zone = gcloud_zone;
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
	return os_execute_local_shell_command(gcloud_command_name + " compute instances describe " + instance + " --zone=" + zone + " --format=get(networkInterfaces[0].accessConfigs.natIP)");
}

string gcloud_execute_command(string command) {
    return os_execute_local_shell_command(gcloud_command_name + " compute ssh " + instance + " --zone=" + zone + " --command=\"" + command + "\"");
}

// This will ensure that an up-to-date ssh-key is present in the users .ssh-directory, eliminating needs to rely on user keys
string gcloud_execute_dummy_command() {
    return gcloud_execute_command("ls");
}
