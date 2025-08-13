#include "uet_utils.h"
#include <stdio.h>

void debug_print_endpoint(SES_Endpoint_type *ep) {
    printf("Endpoint JobID=%u, IPv4=%u, MTU=%u\n",
           ep->JobID, ep->UET_Address.fa.addr.v4, ep->max_payload_size);
}
