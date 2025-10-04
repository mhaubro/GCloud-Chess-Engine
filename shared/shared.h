#ifndef SHARED_H
#define SHARED_H

#include <string>
using namespace std;

string file_get_parent_folder_path();
string file_get_parent_folder_name();
string ssh_get_private_key_filename();
int ssh_connection_start(string instance);
void ssh_connection_terminate();
int gcloud_instance_start(string instance);
void gcloud_execute_dummy_command(string instance);
string gcloud_get_ip_address(string instance);
void gcloud_terminate_instance(string instance);
void ssh_write(string data);
string ssh_read();
void log_output(string data);
void sleep_ms(int ms);

extern string os_path_separator;

#endif // SHARED_H
