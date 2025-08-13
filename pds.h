#ifndef  PDS_H
#define  PDS_H
#include <stdio.h>
#include <stdlib.h>
#include "ses.h"
#include <netinet/in.h>

#define UET_INITIATOR_START_PSN   123456
#define UET_TARGET_START_PSN      456789
#define UET_INITIATOR_PDCID       40000
#define UET_TARGET_PDCID          50000
#define UET_PDS_ROD_MODE          1
#define UET_PDS_RUD_MODE          2

//extern int ses_header_size[SEMANTIC_HEADER_MAX];

typedef enum {
    UET_HDR_NONE = 0,
    UET_HDR_REQUEST_SMALL = 1,
    UET_HDR_REQUEST_MEDIUM = 2,
    UET_HDR_REQUEST_STD = 3,
    UET_HDR_RESPONSE = 4,
    UET_HDR_RESPONSE_DATA = 5,
    UET_HDR_RESPONSE_DATA_SMALL = 6
} pds_next_hdr_type;

// Question: Some UET opcodes map to both UET_HDR_REQUEST_SMALL and UET_HDR_REQUEST_MEDIUM
// In such cases, which pds.next_hdr value to use?
extern pds_next_hdr_type uet_request_opcode_to_pds_next_hdr[UET_REQUEST_OPCODE_MAX];

typedef struct packet_delivery_context
{
    unsigned int source_fa;
    unsigned int dest_fa;
    unsigned int ipv4_or_ipv6;
    unsigned int tc;
    unsigned char pds_mode;
    unsigned int job_id;
    unsigned int source_PIDonFEP;
    unsigned int dest_PIDonFEP;
    unsigned int resource_index;
    unsigned int sdi;
    unsigned int spdc_id;
    unsigned int dpdc_id;
    unsigned int start_PSN;
    unsigned int tx_handle;
    
    // TX state on the initiator for forward direction
    unsigned int TxDir_next_sequence_number_Initiator; // PSN sequence number to be transmitted next at initiator
    unsigned int TxDir_clear_PSN_Initiator; //  MUST be supported when using RUD/ROD. CLEAR_PSN indicates that all PSNs
             // up to and including this PSN have been cleared. Clearing a PSN involves confirming the
             // receipt of the PDS ACK for a packet by initiator. A CLEAR_PSN of X indicates that acknowledgements
             // for the packet with PSN = X and all packets with earlier PSNs have been received at target.
             // The purpose of CLEAR_PSN is to enable guaranteed delivery of certain SES Responses by target. When SES
             // at initiator indicates that an SES Response by target requires reliable delivery, PDS at target
             // MUST store the response until a clear is received at target. This is done to support retransmitting
             // the associated ACK from target to initiator, if necessary.
             // When PDS at target receives a clear, any SES Response state associated with a PSN equal to or
             // lower than CLEAR_PSN is deleted.
             // TxDir_clear_PSN_Initiator = TxDir_CACK_PSN_Target from RUD/ROD ACK packet received
             // pds.clear_psn_offset = clear_PSN - pds.psn at initiator
             // When SES at target indicates that an SES Response requires guaranteed delivery, PDS at target
             // MUST request a clear by setting pds.flags.req to REQ_CLEAR in the associated PDC ACK.
    unsigned int TxDirACK_bitmap_Initiator;
    
    // Rx state at the target for forward direction
    unsigned int RxDir_received_sequence_number_state_Target; // PSN sequence number received at the target
    unsigned int RxDir_ACK_PSN_Target;  // The acknowledgement PSN, ACK_PSN, identifies the initiator sent packet that
             // triggered the generation of an ACK by target. ACK_PSN is indirectly carried in the RUD/ROD ACK
             // header, sent by target, using the pds.ack_psn_offset field
             // pds.ack_psn_offset = ACK_PSN - CACK_PSN;
    unsigned int RxDir_CACK_PSN_Target; // cumulative acknowledgement PSN sent by target
             // The cumulative ACK PSN (CACK_PSN) is carried in full 32-bit format in the PDS ACK
             // header field pds.cack_psn, sent by target. CACK_PSN is required for RUD/ROD PDCs. CACK_PSN is
             // defined such that all earlier PSNs up to and including CACK_PSN have been successfully received
             // and all corresponding guaranteed delivery SES responses have been cleared by the peer PDS.
    unsigned int RxDir_ACK_state_Target; // including guaranteed delivery indication


    // TX state on the target for reverse direction
    unsigned int TxDir_next_sequence_number_Target; // PSN sequence number to be transmitted next at target
    unsigned int TxDir_clear_PSN_Target; //  MUST be supported when using RUD/ROD. CLEAR_PSN indicates that all PSNs
             // up to and including this PSN have been cleared. Clearing a PSN involves confirming the
             // receipt of the PDS ACK for a packet by target. A CLEAR_PSN of X indicates that acknowledgements
             // for the packet with PSN = X and all packets with earlier PSNs have been received at initiator.
             // The purpose of CLEAR_PSN is to enable guaranteed delivery of certain SES Responses by initiator. When SES
             // at target indicates that an SES Response by initiator requires reliable delivery, PDS at initiator
             // MUST store the response until a clear is received at initiator. This is done to support retransmitting
             // the associated ACK from initiator to target, if necessary.
             // When PDS at initiator receives a clear, any SES Response state associated with a PSN equal to or
             // lower than CLEAR_PSN is deleted.
             // TxDir_clear_PSN_Target = TxDir_CACK_PSN_Initiator from RUD/ROD ACK packet received
             // pds.clear_psn_offset = clear_PSN - pds.psn
             // When SES indicates that an SES Response requires guaranteed delivery, PDS MUST request a clear by
             // setting pds.flags.req to REQ_CLEAR in the associated PDC ACK.
             // TxDir_clear_PSN_Target = TxDir_CACK_PSN_Initiator from RUD/ROD ACK packet received
             // pds.clear_psn_offset = clear_PSN - pds.psn at target
             // When SES at initiator indicates that an SES Response requires guaranteed delivery, PDS at initiator
             // MUST request a clear by setting pds.flags.req to REQ_CLEAR in the associated PDC ACK.
    unsigned int TxDirACK_bitmap_Target;

    // Rx state at the initiator for reverse direction
    unsigned int RxDir_received_sequence_number_state_Initiator;  // PSN sequence number received at the initiator
    unsigned int RxDir_ACK_PSN_Initiator;  // The acknowledgement PSN, ACK_PSN, identifies the target sent packet that
             // triggered the generation of an ACK by initiator. ACK_PSN is indirectly carried in the RUD/ROD ACK
             // header, sent by target, using the pds.ack_psn_offset field
             // pds.ack_psn_offset = ACK_PSN - CACK_PSN;
    unsigned int RxDir_CACK_PSN_Initiator; // cumulative acknowledgement PSN sent by initiator
             // The cumulative ACK PSN (CACK_PSN) is carried in full 32-bit format in the PDS ACK
             // header field pds.cack_psn, sent by initiator. CACK_PSN is required for RUD/ROD PDCs. CACK_PSN is
             // defined such that all earlier PSNs up to and including CACK_PSN have been successfully received
             // and all corresponding guaranteed delivery SES responses have been cleared by the peer PDS.
    unsigned int RxDir_ACK_state_Initiator; // including guaranteed delivery indication
    struct packet_delivery_context  *next_pdc_ptr;
} packet_delivery_context_type;
// A single PDC MUST be limited to a single mode – RUD or ROD – and a single traffic class for PDS Requests
// A PDC is used to send requests from an initiator to a target with PSNs that are returned in ACK
// responses by the target
// ROD uses GoBackN loss recovery. GoBackN drops all packets that arrive out of order, requiring the
// source to retransmit all packets starting from the first missing PSN. That is, the missing PSN is
// referred to as ‘N’ and the source goes back to ‘N’ and retransmits.
// RUD mode uses selective retransmission capabilities and enables semantic processing and direct data
// placement out of order. Direct data placement refers to writing of data arriving at the Ethernet network
// port directly into system memory without CPU intervention. RUD relies on sequence numbering to
// identify lost and duplicate packets
// PDS ACKs carrying SES Responses that are marked for guaranteed delivery MUST be delivered. Specifically,
// when an SES Response on a RUD or ROD PDC is marked as requiring guaranteed delivery, PDS is responsible
// for assuring that response is delivered and cleared

// Once a PDC is established (i.e., the pds.flags.syn field is cleared), the locally assigned PDCID can be
// used directly to identify the specific PDC. The same pds.dpdcid is used for all packets arriving to this
// PDC over the network.
// ROD traffic has a restriction that packets from the same {JobID, source FA, destination FA, source
// PIDonFEP, destination PIDonFEP, Resource Index, TC} MUST use the same PDC

typedef enum {
    RESERVED = 0,
    PDS_RUD_REQUEST = 2,
    PDS_ROD_REQUEST = 3,
    PDS_ACK = 7
} pds_packet_type;

#define PDS_RUD_ROD_REQUEST_SIZE   12
#define PDS_RUD_ROD_REQUEST_WITH_CC_STATE_SIZE   16
#define PDS_RUD_ROD_ACK_SIZE       12
#define PDS_RUD_ROD_ACK_CC_SIZE    28
#define PDS_RUD_ROD_ACK_CCX_SIZE   36
#define PDS_RUD_ROD_CONTROL_PKT_SIZE    16
#define PDS_NACK_HEADER_SIZE       16
#define PDS_NACK_CCX_HEADER_SIZE   28

typedef struct {
    pds_packet_type type;
    unsigned char next_hdr;
    unsigned char flags;
} pds_prologue_hdr_type; 

typedef struct {
    pds_prologue_hdr_type prologue_hdr; 
    int                   clear_psn_offset;
    // Encoding of CLEAR_PSN relative to PSN This field is a sequence number used to acknowledge reception of
    // PDS ACKs
    unsigned int          psn;
    // Packet sequence number assigned to the PDS Request 
    // The pds.psn field refers to the packet sequence number assigned to RUD/ROD PDS Request packets
    // and certain CPs. The pds.psn field MUST monotonically increase by one for each packet on each PDC
    // direction with the exception of specific CPs. The PSN increases independently on the forward
    // and reverse directions
    unsigned int          spdcid;
    unsigned int          dpdcid;
    // The pds.psn_offset field is present during PDC creation, when the pds.flags.syn bit is set. This field
    // allows the destination to determine the Start_PSN of the PDC
} RUD_ROD_request_type;

typedef struct {
    pds_prologue_hdr_type prologue_hdr; 
    int                   ack_psn_offset;
    unsigned int          cack_psn;
    unsigned int          spdcid;
    unsigned int          dpdcid;
} PDS_ACK_type;

void
ses_create_initiator_pdc(unsigned int source_fa, unsigned int dest_fa, char tc, unsigned int tx_handle,
                         unsigned char pds_mode);
void
ses_create_target_pdc(unsigned int source_fa, unsigned int dest_fa, char tc, unsigned int tx_handle,
                         unsigned char pds_mode);

void
ses_create_target_pdc(unsigned int source_fa, unsigned int dest_fa, char tc, unsigned int tx_handle,
                      unsigned char pds_mode);

char *
create_pds_req_header_at_initiator(RUD_ROD_request_type *pds_hdr_ptr, char **pds_ptr_ptr,
                                   unsigned int *pds_hdr_length_ptr);

packet_delivery_context_type *
parse_pds_header(char *pds_ptr, unsigned int initiator_or_target, unsigned int *pds_header_length);
#endif
