#include "../include/uet_utils.h"
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <target_ip> <target_port>\n", argv[0]);
        return 1;
    }
    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    SES_Endpoint_type sender_ep, receiver_ep;
    init_endpoint(&sender_ep, "192.168.0.2", 0, AI_BASIC_PROFILE);
    init_endpoint(&receiver_ep, target_ip, 0, AI_BASIC_PROFILE);

    char message[] = "sanskar";
    semantic_header_struct_type hdr;
    prepare_semantic_header(&hdr, &sender_ep, &receiver_ep, message, sizeof(message));

    if (uet_send_message(target_ip, target_port, message, sizeof(message)) == 0)
        printf("Message sent: %s\n", message);
    else
        printf("Send failed!\n");
    return 0;
}
