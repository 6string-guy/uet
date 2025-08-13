#include "uet_utils.h"
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

void init_endpoint(SES_Endpoint_type *ep, char *ip, int ipv6, UET_profile_type profile)
{
    memset(ep, 0, sizeof(*ep));
    ep->endpoint_type = FI_EP_RDM;
    ep->JobID = DEFAULT_JOB_ID;
    ep->UET_Address.ver = DEFAULT_UEC_VERSION;
    ep->UET_Address.flags = UET_ADDR_FLAG_FA_V | (ipv6 ? UET_ADDR_FLAG_IPV6 : UET_ADDR_FLAG_IPV4);
    ep->UET_Address.fep_cap = (profile == AI_BASIC_PROFILE) ? UET_ADDR_FEP_AI_MIN : (profile == AI_FULL_PROFILE) ? UET_ADDR_FEP_AI_FULL
                                                                                                                 : UET_ADDR_FEP_HPC;
    if (ipv6)
        memcpy(ep->UET_Address.fa.addr.v6, ip, UET_ADDR_IPV6_ADDR_OCTETS);
    else
        ep->UET_Address.fa.addr.v4 = inet_addr(ip);
    ep->max_payload_size = SES_DEFAULT_PAYLOAD_MTU;
}

void prepare_semantic_header(semantic_header_struct_type *hdr, SES_Endpoint_type *src_ep,
                             SES_Endpoint_type *dst_ep, char *payload, int payload_len)
{
    memset(hdr, 0, sizeof(*hdr));
    hdr->semantic_header_type = SEMANTIC_STANDARD_HEADER_SOM1;
    hdr->hdr.som1_hdr.common.opcode = UET_SEND;
    hdr->hdr.som1_hdr.common.job_id = src_ep->JobID;
    hdr->hdr.som1_hdr.common.message_id = src_ep->next_message_id_to_send++;
    hdr->hdr.som1_hdr.common.request_length = payload_len;
}

int uet_send_message(char *ip, int port, char *data, size_t len)
{
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return -2;
    send(sockfd, data, len, 0);
    close(sockfd);
    return 0;
}

int uet_receive_message(int listen_port, char *buffer, size_t buf_size)
{
    int sockfd, connfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(listen_port);
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);
    connfd = accept(sockfd, NULL, NULL);
    int n = recv(connfd, buffer, buf_size, 0);
    close(connfd);
    close(sockfd);
    return n;
}
