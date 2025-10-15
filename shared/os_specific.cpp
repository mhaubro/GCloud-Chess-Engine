#include <array>
#include <string>
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <iostream>
#include "shared.h"

#ifdef _WIN32
#include <windows.h>
#include <atlstr.h>
    // Necessary to invoke .bat-file, as MS createProcess self-appends .exe to the executable with no file ending.
    string gcloud_command_name = "gcloud.cmd";
    string os_path_separator = "\\";
#else // _WIN32
    string gcloud_command_name = "gcloud";
    string os_path_separator = "/";
#endif //_WIN32

using namespace std;

string ssh_get_private_key_folder() {
    #ifdef _WIN32
        return "C:\\Users\\" + getenv("username") + "\\.ssh";
    #else // Linux
        return "~/.ssh";
    #endif
}

string ssh_get_private_key_filename() {
    return ssh_get_private_key_folder() + os_path_separator + "google_compute_engine";
}


void sleep_ms(int ms) {
    #ifdef _WIN32
        Sleep(ms);
    #else
        sleep(ms);
    #endif
}

// Get the full path to the location where the executable is
string os_get_path_to_executable() {
    #ifdef _WIN32
        // Get full path using Windows API
        wchar_t buffer[MAX_PATH + 1];
        GetModuleFileNameW(NULL, &buffer[0], MAX_PATH);
        wstring ws(buffer);
        return string(ws.begin(), ws.end());
    #else
        return string("");
    #endif
}

#ifdef _WIN32
// Converts a string to widestring, format used for executing shell commands on windows
// Found on Stack Exchange
static wstring to_wide (const string &multi) {
    wstring wide; wchar_t w; mbstate_t mb {};
    size_t n = 0, len = multi.length () + 1;
    while (auto res = mbrtowc (&w, multi.c_str () + n, len - n, &mb)) {
        if (res == size_t (-1) || res == size_t (-2))
            throw "invalid encoding";

        n += res;
        wide += w;
    }
    return wide;
}

// Gets the most recent windows error as a string.
// Used for executing gcloud shell commands and writing the process creation error to the logs.
static string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return string(); //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a string.
    string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
}

//
// Execute a command and get the results. (Only standard output)
// Source: https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
//
string os_execute_local_shell_command(
    string cmdstr              // [in] command to execute
)
{
    wstring wstr = to_wide(cmdstr);
    const wchar_t* cmd = wstr.c_str();
    log_output("Executing from os_execute_local_shell_command " + cmdstr + "\n");
    CStringA strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        log_output("Returning, failed createpipe, data: " + string(strResult));
        return string(strResult);
    }

    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput  = hPipeWrite;
    si.hStdError   = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
                              // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (! fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        log_output("Returning, failed create process, data: " + string(strResult) + ", error: " + GetLastErrorAsString());
        return string(strResult);
    }

    bool bProcessEnded = false;
    for (; !bProcessEnded ;)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject( pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if
        // there is some data available over pipe.
        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // No data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // Error, the child process might ended
                break;

            buf[dwRead] = 0;
            strResult += buf;
        }
    } //for

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    log_output("Returning, data: " + string(strResult));
    return string(strResult);
} //os_execute_local_shell_command

void os_copy_binaries(string new_folder_path) {
    os_execute_local_shell_command("powershell Copy-Item -path \"" + file_get_parent_folder_path() + "\\*\" -include \"*.dll\" -Destination \"" + new_folder_path + "\"");
    filesystem::copy_file(file_get_parent_folder_path() + "\\gcloud_engine.exe", new_folder_path + "\\gcloud_engine.exe");
}

#else //_WIN32

string os_execute_local_shell_command(string cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw runtime_error("Popen failed");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

void os_copy_binaries(string new_folder_path) {
    // os_execute_local_shell_command("powershell Copy-Item -path \"" + file_get_parent_folder_path() + "\\*\" -include \"*.dll\" -Destination \"" + new_folder_path + "\"");
    // filesystem::copy_file(file_get_parent_folder_path() + "\\gcloud_engine.exe", new_folder_path + "\\gcloud_engine.exe");
}

#endif
