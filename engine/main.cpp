#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "engine.h"
#include "shared.h"
#include "configuration.h"

using namespace std;
    
int main()
{
    int status;
    engine_read_configuration();

    log_output("Starting command listener\n");
    thread command_thread(engine_command_listener);
    // Start the gcloud instances
    log_output("Starting gcloud\n");
    status = gcloud_instance_start(engine_configuration_global.instance_name);
    if (status != 0) {
        cerr << "Error occurred starting gcloud instance." << endl;
        return -1;
    }
    log_output("Starting ssh\n");
    status = ssh_connection_start(engine_configuration_global.instance_name);
    if (status != 0) {
        cerr << "Error occurred starting ssh connection." << endl;
        gcloud_terminate_instance(engine_configuration_global.instance_name);
        return -1;
    }
    log_output("Starting engine\n");
    engine_run();
    command_thread.join();
    ssh_connection_terminate();
    log_output("Terminating gcloud instance\n");
    gcloud_terminate_instance(engine_configuration_global.instance_name);
    return 0;
}
