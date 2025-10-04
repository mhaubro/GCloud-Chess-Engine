#ifndef CONFIGURATION_H
#define CONFIGURATION_H

using namespace std;
#include <string>

void engine_read_configuration();
void engine_get_machine_data();

class engine_configuration {
    protected:
    public:
        string instance_name; // Instance name in gCloud, same name as folder executable is run from
        string executable_path; // Executable path in remote engine
        int hash; // Hash size
        int cpus; // Cpus
        string uci_output; // Full output for uci command
};

extern engine_configuration engine_configuration_global;

#endif // CONFIGURATION_H
