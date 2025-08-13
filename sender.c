#include "ses.h"
#include "pds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <target_ip> <target_port>\n", argv[0]);
        return 1;
    }
    
    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    
    // Create UDP socket (like real UET)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Setup target address
    struct sockaddr_in target_addr;
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip, &target_addr.sin_addr);
    
    // Create UET packet with PDS + SES headers + payload
    char packet_buffer[2048];
    int packet_offset = 0;
    
    // 1. PDS Header (Packet Delivery Service)
    RUD_ROD_request_type pds_header;
    memset(&pds_header, 0, sizeof(pds_header));
    pds_header.prologue_hdr.type = PDS_RUD_REQUEST;
    pds_header.prologue_hdr.next_hdr = UET_HDR_REQUEST_STD;
    pds_header.prologue_hdr.flags = 0;
    pds_header.clear_psn_offset = 0;
    pds_header.psn = UET_INITIATOR_START_PSN;
    pds_header.spdcid = UET_INITIATOR_PDCID;
    pds_header.dpdcid = UET_TARGET_PDCID;
    
    memcpy(packet_buffer + packet_offset, &pds_header, sizeof(pds_header));
    packet_offset += sizeof(pds_header);
    
    // 2. SES Header (Semantic Header)
    semantic_header_struct_type ses_header;
    memset(&ses_header, 0, sizeof(ses_header));
    ses_header.semantic_header_type = SEMANTIC_STANDARD_HEADER_SOM1;
    
    // Fill SOM1 header
    ses_header.hdr.som1_hdr.common.opcode = UET_SEND;
    ses_header.hdr.som1_hdr.common.version = DEFAULT_UEC_VERSION;
    ses_header.hdr.som1_hdr.common.start_of_msg = 1;
    ses_header.hdr.som1_hdr.common.end_of_msg = 1;
    ses_header.hdr.som1_hdr.common.message_id = 1001;
    ses_header.hdr.som1_hdr.common.job_id = 12345;
    ses_header.hdr.som1_hdr.common.pid_on_fep = 1000;
    ses_header.hdr.som1_hdr.common.resource_index = 1;
    ses_header.hdr.som1_hdr.common.initiator = 1001;
    ses_header.hdr.som1_hdr.common.request_length = 7; // "sanskar" length
    
    memcpy(packet_buffer + packet_offset, &ses_header, sizeof(ses_header));
    packet_offset += sizeof(ses_header);
    
    // 3. Payload
    char message[] = "sanskar";
    memcpy(packet_buffer + packet_offset, message, strlen(message));
    packet_offset += strlen(message);
    
    // Send UET packet over UDP
    int bytes_sent = sendto(sockfd, packet_buffer, packet_offset, 0, 
                           (struct sockaddr*)&target_addr, sizeof(target_addr));
    
    if (bytes_sent < 0) {
        perror("Send failed");
        close(sockfd);
        return 1;
    }
    
    printf("UET packet sent successfully!\n");
    printf("- PDS header size: %zu bytes\n", sizeof(pds_header));
    printf("- SES header size: %zu bytes\n", sizeof(ses_header));
    printf("- Payload: %s (%zu bytes)\n", message, strlen(message));
    printf("- Total packet size: %d bytes\n", bytes_sent);
    printf("- Transport: UDP (like real UET)\n");
    
    close(sockfd);
    return 0;
}
