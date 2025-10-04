#ifndef CONFIGURATION_H
#define CONFIGURATION_H

using namespace std;
#include <string>

void engine_read_configuration();
void engine_get_machine_data();

class engine_configuration {
    protected:
    public:
        string instance_name; // Instance name in gCloud. Default: the folder name containing the executable if not set.
        string executable_path; // Executable path in remote engine
        int hash; // Mb, Default: Machine memory size subtracted 4GB for OS if not set.
        int cpus; // Cores, Default: Machine CPUs if not set
        string uci_output; // Full output for uci command
};

extern engine_configuration engine_configuration_global;

#endif // CONFIGURATION_H
