#include <string>
#include "configuration.h"
#include "shared.h"

using namespace std;

string os_execute_local_shell_command(string cmd);
extern string gcloud_command_name;

int gcloud_instance_start(string instance) {
    os_execute_local_shell_command(gcloud_command_name + " compute instances start " + instance);
    return 0; // TODO: Add better debug options
}

void gcloud_terminate_instance(string instance) {
    // os_execute_local_shell_command(gcloud_command_name + " compute instances stop " + instance + " --async");
}

string gcloud_get_ip_address(string instance) {
	return os_execute_local_shell_command(gcloud_command_name + " compute instances describe " + instance + " --format=get(networkInterfaces[0].accessConfigs.natIP)");
}

// This will ensure that an up-to-date ssh-key is present in the users .ssh-directory, eliminating needs to rely on user keys
void gcloud_execute_dummy_command(string instance) {
    os_execute_local_shell_command(gcloud_command_name + " compute ssh " + instance + " --command=\"ls\"");
}
