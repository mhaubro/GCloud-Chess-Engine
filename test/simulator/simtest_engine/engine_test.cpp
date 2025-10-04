#include "config.h"
#include "shared.h"
#include "configuration.h"

#include <iostream>
using namespace std;
static int errors = 0;

static void checkError(string expectedvalue, string actualvalue, string error) {
    if (expectedvalue.compare(actualvalue) != 0) {
        cout << "Error: " << error << ", Actual: " << expectedvalue << ", Expected: " << actualvalue << endl;
        errors++;
    }
}

void output_to_terminal(string output) {
    cout << output << endl;
}

int main() {
    // Test file settings loads
    engine_read_configuration();
    checkError(engine_configuration_global.instance_name, "simtest_engine", "Instance name wrong");
    checkError(engine_configuration_global.executable_path, "engine_path", "Engine path wrong");
    checkError(engine_configuration_global.uci_output, "uci-test-output", "Uci test output read wrong");

    if (errors > 0) {
        cout << to_string(errors) + " Errors caught in testing" << endl;
        return -1;
    }

    return 0;
}
