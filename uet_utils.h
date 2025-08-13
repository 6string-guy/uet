#ifndef UET_UTILS_H
#define UET_UTILS_H

#include "ses.h"
#include "pds.h"

#define DEFAULT_JOB_ID 1
#define DEFAULT_PID 100
#define DEFAULT_RI 1
#define DEFAULT_SDI 0

// Initialize an SES endpoint
void init_endpoint(SES_Endpoint_type *ep, char *ip, int ipv6, UET_profile_type profile);

// Prepare a semantic header for sending a message
void prepare_semantic_header(semantic_header_struct_type *hdr, SES_Endpoint_type *src_ep,
                             SES_Endpoint_type *dst_ep, char *payload, int payload_len);

// Simulated send over socket
int uet_send_message(char *ip, int port, char *data, size_t len);

// Simulated receive over socket
int uet_receive_message(int listen_port, char *buffer, size_t buf_size);

#endif
