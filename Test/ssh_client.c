#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>  

int main() {
    ssh_session my_ssh_session;
    int rc;
    time_t start_time, end_time;

    my_ssh_session = ssh_new();
    if (my_ssh_session == NULL) {
        fprintf(stderr, "Error creating SSH session\n");
        exit(-1);
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "10.107.2.224");
    int port = 22;
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "user");

    rc = ssh_connect(my_ssh_session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting: %s\n", ssh_get_error(my_ssh_session));
        ssh_free(my_ssh_session);
        exit(-1);
    }

    rc = ssh_userauth_publickey_auto(my_ssh_session, "your_username", NULL);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating: %s\n", ssh_get_error(my_ssh_session));
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        exit(-1);
    }

    printf("Successfully connected and authenticated!\n");


    time(&start_time);
    printf("Start time: %ld\n", start_time);
    time(&end_time);
    printf("End time: %ld\n", end_time);

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);

    return 0;
}
