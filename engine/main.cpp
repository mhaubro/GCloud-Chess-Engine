#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <semaphore>
#include <chrono>

#include "engine.h"
#include "shared.h"
#include "configuration.h"

using namespace std;
bool quit_received;
binary_semaphore command_received(0);
    
int main()
{
    try
    {
        int status;
        engine_read_configuration();
        gcloud_cache_settings(engine_configuration_global.instance_name, engine_configuration_global.zone);

        log_output("Starting command listener\n");
        thread command_thread(engine_command_listener);
        command_thread.detach();
        
        // Wait for go-signal from command listener - used to determine if
        // it's a full engine load or just setting up parameters.
        command_received.acquire();
        if (quit_received) {
            log_output("Terminating without starting gcloud instance\n");
            return 0;
        }

        // Start the gcloud instances
        log_output("Starting gcloud\n");
        gcloud_instance_start();
        if (quit_received) {
            log_output("Terminating early\n");
            gcloud_terminate_instance();
            return 0;
        }

        // The machine might need a bit more time to boot.
        // This might be redundant
        // Todo: Replace with a clever polling function that calls the
        sleep_ms(20 * 1000);
        string output = gcloud_execute_dummy_command();
        double timeout_seconds = 30;
        auto start = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
        while (output.find("ERROR") != string::npos && elapsed_seconds.count() < timeout_seconds) {
            output = gcloud_execute_dummy_command();
            elapsed_seconds = chrono::system_clock::now() - start;
        }

        if (output.find("ERROR") != string::npos) {
            log_output("Error: GCloud machine not up and running within " + to_string(timeout_seconds) + " seconds. Terminating.\n");
            gcloud_terminate_instance();
            return 0;
        }

        if (quit_received) {
            log_output("Terminating early\n");
            gcloud_terminate_instance();
            return 0;
        }
        log_output("Starting ssh\n");
        status = ssh_connection_start();
        if (status != 0) {
            cerr << "Error occurred starting ssh connection." << endl;
            gcloud_terminate_instance();
            return -1;
        }

        if (quit_received) {
            log_output("Terminating early\n");
            gcloud_terminate_instance();
            return 0;
        }

        log_output("Starting engine\n");
        engine_run();

        ssh_connection_terminate();
        log_output("Terminating gcloud instance\n");
        gcloud_terminate_instance();
    }
    catch (const std::exception &exc)
    {
        // catch anything thrown within try block that derives from std::exception
        log_output(exc.what());
        cerr << exc.what();
    }
    return 0;
}
