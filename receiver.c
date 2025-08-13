#include "ses.h"
#include "pds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <listen_port>\n", argv[0]);
        return 1;
    }
    
    int listen_port = atoi(argv[1]);
    
    // Create UDP socket (like real UET)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Bind to listen port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);
    
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }
    
    printf("UET receiver listening on UDP port %d...\n", listen_port);
    printf("Waiting for UET packets...\n");
    
    // Receive UET packet
    char packet_buffer[2048];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    
    int bytes_received = recvfrom(sockfd, packet_buffer, sizeof(packet_buffer), 0,
                                 (struct sockaddr*)&sender_addr, &sender_len);
    
    if (bytes_received < 0) {
        perror("Receive failed");
        close(sockfd);
        return 1;
    }
    
    printf("UET packet received from %s!\n", inet_ntoa(sender_addr.sin_addr));
    printf("Total packet size: %d bytes\n", bytes_received);
    
    // Parse UET packet
    int packet_offset = 0;
    
    // 1. Parse PDS Header
    RUD_ROD_request_type *pds_header = (RUD_ROD_request_type*)(packet_buffer + packet_offset);
    packet_offset += sizeof(RUD_ROD_request_type);
    
    printf("\nPDS Header:\n");
    printf("- Type: %s\n", (pds_header->prologue_hdr.type == PDS_RUD_REQUEST) ? "RUD_REQUEST" : "Other");
    printf("- Next Header: %d\n", pds_header->prologue_hdr.next_hdr);
    printf("- PSN: %u\n", pds_header->psn);
    printf("- Source PDC ID: %u\n", pds_header->spdcid);
    printf("- Dest PDC ID: %u\n", pds_header->dpdcid);
    
    // 2. Parse SES Header
    semantic_header_struct_type *ses_header = (semantic_header_struct_type*)(packet_buffer + packet_offset);
    packet_offset += sizeof(semantic_header_struct_type);
    
    printf("\nSES Header:\n");
    printf("- Opcode: %s\n", (ses_header->hdr.som1_hdr.common.opcode == UET_SEND) ? "UET_SEND" : "Other");
    printf("- Message ID: %u\n", ses_header->hdr.som1_hdr.common.message_id);
    printf("- Job ID: %u\n", ses_header->hdr.som1_hdr.common.job_id);
    printf("- Request Length: %u bytes\n", ses_header->hdr.som1_hdr.common.request_length);
    
    // 3. Extract Payload
    int payload_length = ses_header->hdr.som1_hdr.common.request_length;
    char *payload = packet_buffer + packet_offset;
    
    printf("\nPayload:\n");
    printf("- Message: %.*s\n", payload_length, payload);
    
    // Verify it's "sanskar"
    if (strncmp(payload, "sanskar", 7) == 0) {
        printf("✓ SUCCESS: Received 'sanskar' via UET protocol!\n");
    }
    
    close(sockfd);
    return 0;
}
