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
    
    // 1. PDS Header (Packet Delivery Service) - WITH MEANINGFUL FLAGS
    RUD_ROD_request_type pds_header;
    memset(&pds_header, 0, sizeof(pds_header));
    pds_header.prologue_hdr.type = PDS_RUD_REQUEST;
    pds_header.prologue_hdr.next_hdr = UET_HDR_REQUEST_STD;
    pds_header.prologue_hdr.flags = 0x42;  // SET TO 0x42 for verification
    pds_header.clear_psn_offset = 100;     // SET TO 100 for verification
    pds_header.psn = UET_INITIATOR_START_PSN + 555;  // Add offset for verification
    pds_header.spdcid = UET_INITIATOR_PDCID + 10;    // Add offset for verification
    pds_header.dpdcid = UET_TARGET_PDCID + 20;       // Add offset for verification
    
    memcpy(packet_buffer + packet_offset, &pds_header, sizeof(pds_header));
    packet_offset += sizeof(pds_header);
    
    // 2. SES Header (Semantic Header) - WITH ALL FLAGS SET
    semantic_header_struct_type ses_header;
    memset(&ses_header, 0, sizeof(ses_header));
    ses_header.semantic_header_type = SEMANTIC_STANDARD_HEADER_SOM1;
    
    // Fill SOM1 header with meaningful values
    ses_header.hdr.som1_hdr.common.opcode = UET_SEND;
    ses_header.hdr.som1_hdr.common.version = DEFAULT_UEC_VERSION;
    ses_header.hdr.som1_hdr.common.delivery_complete = 1;    // SET FLAG
    ses_header.hdr.som1_hdr.common.initiator_error = 0;
    ses_header.hdr.som1_hdr.common.relative = 1;            // SET FLAG  
    ses_header.hdr.som1_hdr.common.header_data_present = 1; // SET FLAG
    ses_header.hdr.som1_hdr.common.end_of_msg = 1;          // SET FLAG
    ses_header.hdr.som1_hdr.common.start_of_msg = 1;        // SET FLAG
    ses_header.hdr.som1_hdr.common.message_id = 0xABCD;     // Easy to verify
    ses_header.hdr.som1_hdr.common.ri_generation = 5;       // SET TO 5
    ses_header.hdr.som1_hdr.common.job_id = 0x12345;        // Easy to verify
    ses_header.hdr.som1_hdr.common.pid_on_fep = 0x9999;     // Easy to verify
    ses_header.hdr.som1_hdr.common.resource_index = 77;     // SET TO 77
    ses_header.hdr.som1_hdr.common.initiator = 0xDEAD;      // Easy to verify
    ses_header.hdr.som1_hdr.common.request_length = 7;      // "sanskar" length
    
    // Set buffer offset with meaningful values
    ses_header.hdr.som1_hdr.buffer_offset[0] = 0x1111;      // Easy to verify
    ses_header.hdr.som1_hdr.buffer_offset[1] = 0x2222;      // Easy to verify
    
    // Set memory key/match bits with meaningful values
    //ses_header.hdr.som1_hdr.memory_key_match_bits = 0xBEEF;  // Easy to verify
    ses_header.hdr.som1_hdr.memory_key_match_bits[1] = 0xCAFE;  // Easy to verify
    
    // Set header data with meaningful values
    //ses_header.hdr.som1_hdr.header_data = 0xAAAA;        // Easy to verify
    ses_header.hdr.som1_hdr.header_data[1] = 0xBBBB;        // Easy to verify
    
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
    
    printf("UET packet sent with verification values!\n");
    printf("===========================================\n");
    printf("PDS Header Values (for verification):\n");
    printf("- Flags: 0x%02X (should be 0x42)\n", pds_header.prologue_hdr.flags);
    printf("- Clear PSN Offset: %d (should be 100)\n", pds_header.clear_psn_offset);
    printf("- PSN: %u (should be %u)\n", pds_header.psn, UET_INITIATOR_START_PSN + 555);
    printf("- Source PDC ID: %u (should be %u)\n", pds_header.spdcid, UET_INITIATOR_PDCID + 10);
    printf("- Dest PDC ID: %u (should be %u)\n", pds_header.dpdcid, UET_TARGET_PDCID + 20);
    
    printf("\nSES Header Values (for verification):\n");
    printf("- Message ID: 0x%X (should be 0xABCD)\n", ses_header.hdr.som1_hdr.common.message_id);
    printf("- Job ID: 0x%X (should be 0x12345)\n", ses_header.hdr.som1_hdr.common.job_id);
    printf("- PID on FEP: 0x%X (should be 0x9999)\n", ses_header.hdr.som1_hdr.common.pid_on_fep);
    printf("- Resource Index: %u (should be 77)\n", ses_header.hdr.som1_hdr.common.resource_index);
    printf("- Initiator: 0x%X (should be 0xDEAD)\n", ses_header.hdr.som1_hdr.common.initiator);
    printf("- Buffer Offset[0]: 0x%X (should be 0x1111)\n", ses_header.hdr.som1_hdr.buffer_offset);
    printf("- Buffer Offset[1]: 0x%X (should be 0x2222)\n", ses_header.hdr.som1_hdr.buffer_offset[1]);
    printf("- Match Bits: 0x%X (should be 0xBEEF)\n", ses_header.hdr.som1_hdr.memory_key_match_bits);
    printf("- Match Bits[1]: 0x%X (should be 0xCAFE)\n", ses_header.hdr.som1_hdr.memory_key_match_bits[1]);
    printf("- Header Data: 0x%X (should be 0xAAAA)\n", ses_header.hdr.som1_hdr.header_data);
    printf("- Header Data[1]: 0x%X (should be 0xBBBB)\n", ses_header.hdr.som1_hdr.header_data[1]);
    
    printf("\nFlag Values (for verification):\n");
    printf("- delivery_complete: %d (should be 1)\n", ses_header.hdr.som1_hdr.common.delivery_complete);
    printf("- relative: %d (should be 1)\n", ses_header.hdr.som1_hdr.common.relative);
    printf("- header_data_present: %d (should be 1)\n", ses_header.hdr.som1_hdr.common.header_data_present);
    printf("- end_of_msg: %d (should be 1)\n", ses_header.hdr.som1_hdr.common.end_of_msg);
    printf("- start_of_msg: %d (should be 1)\n", ses_header.hdr.som1_hdr.common.start_of_msg);
    printf("- ri_generation: %d (should be 5)\n", ses_header.hdr.som1_hdr.common.ri_generation);
    
    printf("\n- Total packet size: %d bytes\n", bytes_sent);
    printf("- Transport: UDP (like real UET)\n");
    
    close(sockfd);
    return 0;
}
