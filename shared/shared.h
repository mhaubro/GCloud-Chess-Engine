#ifndef SHARED_H
#define SHARED_H

#include <string>
using namespace std;

string file_get_parent_folder_path();
string file_get_parent_folder_name();
string ssh_get_private_key_filename();
int ssh_connection_start();
void ssh_connection_terminate();
void gcloud_cache_settings(string gcloud_instance, string gcloud_zone);
string gcloud_instance_start();
string gcloud_execute_dummy_command();
string gcloud_execute_command(string command);
string gcloud_get_ip_address();
string gcloud_terminate_instance();
void ssh_write(string data);
string ssh_read();
string ssh_read_blocking();
void log_output(string data);
string os_execute_local_shell_command(string cmd);
void sleep_ms(int ms);

extern string os_path_separator;

#endif // SHARED_H
