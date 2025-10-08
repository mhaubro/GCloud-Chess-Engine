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
    gcloud_cache_settings(engine_configuration_global.instance_name, engine_configuration_global.zone);

    log_output("Starting command listener\n");
    thread command_thread(engine_command_listener);
    // Start the gcloud instances
    log_output("Starting gcloud\n");
    gcloud_instance_start();
    // The machine might need a bit more time to boot.
    sleep_ms(10 * 1000);
    log_output("Starting ssh\n");
    status = ssh_connection_start();
    if (status != 0) {
        cerr << "Error occurred starting ssh connection." << endl;
        gcloud_terminate_instance();
        return -1;
    }
    log_output("Starting engine\n");
    engine_run();
    command_thread.join();
    ssh_connection_terminate();
    log_output("Terminating gcloud instance\n");
    gcloud_terminate_instance();
    return 0;
}
