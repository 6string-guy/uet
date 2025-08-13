#include "uet_utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <listen_port>\n", argv[0]);
        return 1;
    }
    int listen_port = atoi(argv[1]);
    char buffer[1024];
    int n = uet_receive_message(listen_port, buffer, sizeof(buffer));
    if (n > 0) {
        buffer[n] = '\0';
        printf("Message received: %s\n", buffer);
    }
    return 0;
}
