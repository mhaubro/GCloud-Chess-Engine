#include <iostream>
#include <string>
#include <libssh/libssh.h> // Cannot coexist with windows.h
#include "configuration.h"
#include "shared.h"

using namespace std;

ssh_session session;
ssh_channel channel;
ssh_key privatekey;
int verbosity = SSH_LOG_PROTOCOL;
int port = 22;
char ssh_incoming_buffer[512*1024];

// Setting up session according to https://api.libssh.org/stable/libssh_tutor_guided_tour.html
// Assumes that gcloud machine is online -> that gcloud_instance_start has been called beforehand
int ssh_connection_start() {
    ssh_init();
    int status;
    session = ssh_new();
    if (session == NULL) {
        log_output("SSH session is null");
        return -1;
    }
    // Get the IP Address of the remote machine.
    string ip_address = gcloud_get_ip_address();
    // Pop-back CR and newline for SSHLIB to be able to handle the string
    ip_address.pop_back();
    ip_address.pop_back();
    status = ssh_options_set(session, SSH_OPTIONS_HOST, ip_address.c_str());
    if (status != SSH_OK) {
        log_output("Setting Host failed with: " + status + string("\n"));
        return status;
    }
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_PUBLICKEY_ACCEPTED_TYPES, "rsa-sha2-256,rsa-sha2-512,ecdh-sha2-nistp256,ssh-rsa");

    // This will attempt authentication using all private keys in ~/.ssh
    string private_key_file = ssh_get_private_key_filename();
    string public_key_file = private_key_file + ".pub";
    // Connect to SSH
    status = ssh_connect(session);
    if (status != SSH_OK) {
        log_output("SSH Connect failed with: " + status + string("\n"));
        return status;
    }
    status = ssh_pki_import_pubkey_file(public_key_file.c_str(), &privatekey);
    if (status != SSH_OK) {
        log_output("Public key import failed with: " + status + string("\n"));
        return status;
    }
    status = ssh_userauth_try_publickey(session, NULL, privatekey);
    if (status != SSH_OK) {
        log_output("Public key verification failed with: " + status + string("\n"));
        return status;
    }
    status = ssh_pki_import_privkey_file(private_key_file.c_str(), NULL, NULL, NULL, &privatekey);
    if (status != SSH_OK) {
        log_output("Private key import failed with: " + status + string("\n"));
        return status;
    }
    status = ssh_userauth_publickey(session, NULL, privatekey);
    if (status != SSH_OK) {
        log_output("Authentication failed with code: " + status + string("\n"));
        return status;
    }

    // Set up the channel to get a shell for the engine
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        log_output("SSH Channel is null");
        return status;
    }
 
    status = ssh_channel_open_session(channel);
    if (status != SSH_OK)
    {
        ssh_channel_free(channel);
        log_output("SSH channel open failed. Status: " + status + string("\n"));
        return status;
    }
    status = ssh_channel_request_shell(channel);
    if (status != SSH_OK)
    {
        ssh_channel_free(channel);
        log_output("SSH shell request failed. Status: " + status + string("\n"));
        return status;
    }
    // Discard the boot-text
    ssh_read();
    return SSH_OK;
}

void ssh_connection_terminate() {
    ssh_channel_close(channel);
    ssh_channel_send_eof(channel);
    ssh_channel_free(channel);
    ssh_free(session);
    ssh_key_free(privatekey);
}

void ssh_write(string data) {
    int bytes = ssh_channel_write(channel, data.c_str(), strlen(data.c_str()));
    log_output("Writing to remote, data: " + data);
}

string ssh_read_blocking() {
    int bytes;
    bytes = ssh_channel_read(channel, ssh_incoming_buffer, sizeof(ssh_incoming_buffer), false);
    if (bytes > 0) {
        ssh_incoming_buffer[bytes] = '\0';
        log_output("Read from remote: " + string(ssh_incoming_buffer));
        return string(ssh_incoming_buffer);
    } else {
        log_output("SSH READ OUTPUT: " + to_string(bytes));
        return string("");
    }
}

string ssh_read() {
    int bytes;
    bytes = ssh_channel_read_nonblocking(channel, ssh_incoming_buffer, sizeof(ssh_incoming_buffer), false);
    if (bytes > 0) {
        ssh_incoming_buffer[bytes] = '\0';
        log_output("Read from remote: " + string(ssh_incoming_buffer));
        return string(ssh_incoming_buffer);
    } else {
        return string("");
    }
}
