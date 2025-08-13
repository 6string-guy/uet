#ifndef SES_H
#define SES_H
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define DEFAULT_UEC_VERSION          0
#define UET_ADDR_FLAG_FEP_CAP_V      (1 << 0) /* FEP capabilities valid flag */
#define UET_ADDR_FLAG_FA_V           (1 << 1) /* fabric address valid flag */
#define UET_ADDR_FLAG_PID_V          (1 << 2) /* PIDonFEP valid flag */
#define UET_ADDR_FLAG_RI_V           (1 << 3) /* resource index valid flag */
#define UET_ADDR_FLAG_INI_V          (1 << 4) /* initiator id valid flag */
#define UET_ADDR_FLAG_ABS_MODE       (1 << 5) /* absolute address mode */
#define UET_ADDR_FLAG_REL_MODE       (0 << 5) /* relative address mode */
#define UET_ADDR_FLAG_IPV6           (1 << 6) /* IPv6 fabric address type */
#define UET_ADDR_FLAG_IPV4           (0 << 6) /* IPv4 fabric address type */
#define UET_ADDR_FLAG_MTU_MSG_SIZE   (1 << 7) /* max message size is MTU */
#define UET_ADDR_FEP_AI_MIN          (1 << 0) /* AI base profile supported */
#define UET_ADDR_FEP_AI_FULL         (1 << 1) /* AI full profile supported */
#define UET_ADDR_FEP_HPC             (1 << 2) /* HPC profile supported */
#define UET_ADDR_FEP_OPT_NM_SEM      (1 << 7) /* non-matching hdr supported */
#define UET_ADDR_IPV6_ADDR_OCTETS    16

#define SES_PAYLOAD_MTU_1024         1024
#define SES_PAYLOAD_MTU_2048         2048
#define SES_PAYLOAD_MTU_4096         4096
#define SES_PAYLOAD_MTU_8192         8192
#define SES_DEFAULT_PAYLOAD_MTU      4096

typedef enum {
    SEMANTIC_STATE_NULL=0,
    SEMANTIC_STANDARD_HEADER_SOM1_SENT=1,
    SEMANTIC_STANDARD_HEADER_SOM1_RCVD,
    SEMANTIC_STANDARD_HEADER_SOM0_SENT,
    SEMANTIC_STANDARD_HEADER_SOM0_RCVD,
    SEMANTIC_RESPONSE_SENT,
    SEMANTIC_RESPONSE_RCVD,
    SEMANTIC_RESPONSE_WITH_HEADER_SENT,
    SEMANTIC_RESPONSE_WITH_HEADER_RCVD,
} semantic_state_type;

struct semantic_header_struct;
struct SES_Endpoint;
typedef struct semantic_transaction {
    unsigned int message_id;
    semantic_state_type  semantic_transaction_state;
    struct SES_Endpoint  *dest_ep_ptr;
    struct semantic_header_struct  *semantic_hdr_ptr;
    struct     semantic_transaction *next_transaction_ptr;
} semantic_transaction_state_type;

// Addressing describes the entire process of selecting a target and identifying a data buffer at the target.
// This may be the destination of data (e.g., for a Send (3.4.1.5.1)– or a Write (3.4.1.5.2), the source of
// the data (e.g., for a Read (3.4.1.5.3), or both a source and destination of data (e.g., for atomic
// operations).
// Relative addressing is intended to enable scalable addressing for parallel communication within a
// distributed application, which is called a job. Absolute Addressing is intended to enable scalable
// addressing for client/server operations where the server is not required to be part of the application.
// In both cases, the initiator of a transaction is assigned a JobID (3.4.1.3.1) that is inserted in each
// packet in a trusted way (3.4.7.1).
// In absolute addressing, JobID is used only as part of authorization to access a buffer (3.4.1.4.1). Buffer
// authorization in this scenario is expected to occur on a per-buffer basis (e.g., per memory region or per
// receive buffer).
struct uet_fa { /* fabric address */
     union {
          unsigned int v4;
          char v6[UET_ADDR_IPV6_ADDR_OCTETS];
     } addr;
};

struct uet_addr { /* UET address */
     char      ver;        // Version number that identifies the format of the address
                           // • The fields in this table are associated with version 0
     char      reserved; /* for alignment */
     unsigned int  flags;      // bit 0 - Indicates validity of fabric endpoint capabilities field
                           // bit 1 - Indicates validity of fabric address fields (fabric address
                           //         type and fabric address)
                           // bit 2 - Indicates validity of PIDonFEP field
                           // bit 3 - Indicates validity of index fields (start resource index and
                           //         num resource indices)
                           // bit 4 - Indicates validity of Initiator ID field
                           // bit 5 - Indicates whether relative or absolute address mode is used
                           // bit 6 - Fabric address type, IPv4 or IPv6
                           // bit 7 - Indicates if maximum message size is limited to MTU
     unsigned int  fep_cap;    // Bits 0,1,2 indicate support for AI Base, AI Full, HPC Profiles
                           // Bit 7 indicates support for optimized non-matching SES header
     unsigned int  pid_on_fep; // Process ID in context of fabric endpoint,
                           // Meaning depends on the address mode
     struct    uet_fa fa;
     unsigned int  start_resource_index;  // Resources can include constructs such as buffers, completion queues,
                           // and completion counters. A “Resource Index” is an addressing construct used to
                           // select an addressable set of resources within a service.
                           // • Index 0 is reserved for a UET provider-to-provider control channel
                           //   o No standardized provider-to-provider control channel operations have
                           //     been defined
                           //   o The provider-to-provider control channel MAY be used for vendor-specific
                           //     operations
     unsigned int  num_resource_indices;  // Three Resource Index spaces exist, corresponding to three operation types:
                           //     RMA, SEND, and TAGGED. That is, Resource Index 0 for an RMA opcode (e.g., UET_WRITE) has a
                           //     different meaning from Resource Index 0 for a SEND opcode (e.g., UET_DEFERRABLE_SEND). Similarly,
                           //     TAGGED opcodes (e.g., UET_RENDEZVOUS_SEND) have a third meaning for Resource Index 0
                           // The libfabric endpoint has a variety of resources that can include such things
                           // as a receive queue for send/receive operation, a set of matching buffers, RMA
                           // resources, and completion delivery – both completion queues and counters. Some of these
                           // resources are directly addressable from the network (e.g., a memory region) and
                           // others which are not (e.g., a completion queue).
     unsigned int  initiator_id;  // Initiator identifier
};

typedef enum {
     FI_EP_DGRAM = 0,
     FI_EP_RDM
}  FI_Endpoint_type;

typedef struct SES_Endpoint {
     unsigned char   endpoint_type; /* FI_EP_DGRAM or FI_EP_RDM */ 
     unsigned int    JobID; /* 24 bits. The JobID is an identifying property of the initiator only,
                               and then is used as part of addressing and authorization (3.4.1.4) at the
                               target. A libfabric endpoint is part of one or more JobIDs. A libfabric
                               endpoint is likely to utilize one or more {JobID, PIDonFEP, Resource Index}
                               tuples. */
     struct uet_addr UET_Address;  /* IP Address is a part of it */ 
     char            addressing_type; /* relative or absolute */
     char            FI_Tclass;
     char            DSCP;
     unsigned int    message_ordering_mode;
     unsigned int    ssi; /* One JobID MAY be allowed to use more than one SDI.
                             The number of SDIs that are usable by a single JobID is implementation-defined
                             (a number greater than or equal to 1). */
     unsigned int    sdi;
     unsigned int    max_payload_size;
     unsigned int    next_message_id_to_send;
     semantic_transaction_state_type *transaction_ptr;
} SES_Endpoint_type;

typedef enum {
    AI_BASIC_PROFILE,
    AI_FULL_PROFILE,
    HPC_PROFILE
} UET_profile_type;
 
typedef enum {
    LIBFABRIC_CCL_SERVICE,
    LIBFABRIC_MPI_SERVICE,
    LIBFABRIC_SHMEM_SERVICE,
    LIBFABRIC_PGAS_SERVICE,
} libfabric_service_type; // A “service” is a libfabric-level construct that encapsulates all of
    // the resources associated with a higherlevel library (e.g., *CCL, MPI).
    // In relative addressing, the FEP uses JobID to define the scope of the PIDonFEP. Within the PIDonFEP,
    // a Resource Index (RI) is associated with a “service”. A service can correspond to a specific use case
    // or library. Within a service, each Resource Index creates a unique addressing space where a send
    // operation, tagged send operation, or RMA operation can select a buffer.
typedef enum {
    SEMANTIC_STANDARD_HEADER_SOM1 = 0,
    SEMANTIC_STANDARD_HEADER_SOM0,
    SEMANTIC_DEFERRABLE_HEADER,
    SEMANTIC_READY_TO_RESTART_HEADER,
    SEMANTIC_ATOMIC_OPERATION_HEADER,
    SEMANTIC_COMPARE_AND_SWAP_ATOMIC_HEADER,
    SEMANTIC_RESPONSE_HEADER,
    SEMANTIC_RESPONSE_WITH_DATA_HEADER,
    SEMANTIC_HEADER_MAX
} semantic_header_type;

#define  SEMANTIC_STANDARD_HEADER_SIZE               11*4
#define  SEMANTIC_DEFERRABLE_HEADER_SIZE             11*4
#define  SEMANTIC_READY_TO_RESTART_HEADER_SIZE       11*4
#define  SEMANTIC_ATOMIC_OPERATION_HEADER_SIZE        1*4
#define  SEMANTIC_COMPARE_AND_SWAP_ATOMIC_HEADER_SIZE 9*4
#define  SEMANTIC_RESPONSE_HEADER_SIZE                3*4
#define  SEMANTIC_RESPONSE_WITH_DATA_HEADER_SIZE      5*4

//extern int ses_header_size[SEMANTIC_HEADER_MAX];

typedef struct {
    char opcode;
    char version;  // Semantic protocol version – set to 0 in the initial version.
    char delivery_complete; // Defer the semantic response for this until the packet has
         // been made globally observable (3.4.8.3). This matches the FI_DELIVERY_COMPLETE
         // option in libfabric.
         // If the DC (delivery complete) bit is set in a message, an implementation MAY
         // indicate with a semantic failure that the transaction is not supported. If an
         // implementation does not indicate that the transaction is not supported, then the
         // implementation MUST defer the semantic response until it can guarantee that the
         // data is globally observable at the target
         // The default for global observability is that global observability is not guaranteed.
         // A FEP MUST NOT indicate that a message has completed until all of the packets have
         // been acknowledged by PDS. In addition, the FEP MUST wait until a semantic response
         // is received before indicating that the message is complete. This is sufficient for
         // implementing the “transmit complete” semantic in libfabric.
    char initiator_error;  // The initiator error (ses.ie) bit in the semantic header is used
         // to indicate that the header was properly constructed, but that the payload cannot
         // be used. Implementations MAY use the ses.ie bit to indicate errors encountered at
         // the initiator, but implementations are not required to have any class of error that
         // causes the ses.ie bit to be set
         // Packets with the ses.ie bit set MUST NOT access memory at the target. Messages
         // containing packets with the ses.ie bit set MUST use a non-RC_OK ses.return_code. If
         // the initiator error is the first error, the return code MUST be RC_INITIATOR_ERROR.
    char relative;
    char header_data_present; // Set when the associated libfabric API provided remote CQ data
         // Libfabric defines completion data that can have a size up to 64 bits. UET supports
         // this with a field called ses.header_data that is 64 bits in size. Header data is
         // provided only in the start of message packet, and only if the ses.hd bit indicates
         // that it was provided by software
    char end_of_msg;
    char start_of_msg;
    unsigned int  message_id;  // Message identifier – assists in associating different packets
         // to one message at the target. Also assists in reversemapping responses to message.
         // The value of 0 is reserved to indicate that the message ID is not valid
    char ri_generation; //UET introduces the concept of a “generation” for a Resource Index as a
         // separate field carried in the header when using tagged or untagged messaging. At the
         // target, a Resource Index MAY be configured to use a generation. If it is not configured
         // to use a generation, the generation MUST be zero. A message that arrives at the target
         // MUST contain the correct generation, or the message MUST receive a semantic response
         // indicating a generation mismatch (RC_BAD_GENERATION). This response MUST contain the
         // current generation. 3.4.6.3.2
         // A tagged or untagged message using a ROD PDC that encounters resource exhaustion at
         // the semantic level that prevents accepting the message MUST disable the target Resource
         // Index. The message MUST receive a semantic response indicating that the message
         // encountered a disabled index (RC_DISABLED_GEN). The index MUST remain disabled until
         // additional resources are available and the generation number is incremented.
    unsigned int  job_id;
    unsigned int  pid_on_fep;  // The PIDonFEP value to be used at the target.
    unsigned int  resource_index;
    unsigned int  initiator;   // Initiator ID used as part of matching criteria.
    unsigned int  request_length; // Length of the payload to be transferred (in bytes). 0 is a
         // legal transfer size (0 byte write/read). Maximum size is 2^32-1. The request
         // length field MUST be populated both when ses.som=1 and ses.som=0.
} standard_semantic_header_common_fields_type;

typedef struct {
    standard_semantic_header_common_fields_type common;
    unsigned int  buffer_offset[2]; // Offset within the target buffer used for 0 based addressing.
         // The first memory access of the first packet in a message begins at Buffer Offset
         // bytes from the base of the memory region selected
         // In multi-packet messages, Buffer Offset is the same across packets, so that the
         // Buffer Offset and Request Length can be used together to determine if the message
         // fits in the target buffer. 
         // The access address of a given packet is Buffer Base Address (from the memory
         // region) + Buffer Offset + the offset within the message (taken from the header
         // data field for packets on which ses.som=0).
    unsigned int  memory_key_match_bits[2]; // Used for tagged matching or as a memory key, depending
         // on the opcode being used. In ready-to-restart requests (UET_DEFERRABLE_RTR),
         // this field carries the upper 32 bits of the restart token that was part of the
         // deferrable send request as well as 32 bits allocated by the target in the lower
         // 32 bits. 3.4.1.3.5
         // The UET header defines a set of fields to enable either matching or nonmatching
         // operations. When the message indicates it is a tagged send (e.g., UET_TAGGED_SEND,
         // UET_RENDEZVOUS_TSEND), the “matching criteria” (the 64 ses.match_bits field and the
         // ses.initiator_id field) are used for buffer selection. When matching is not requested
         // (e.g., UET_SEND, UET_RENDEZVOUS_SEND), a network message selects the next buffer
         // provided by the Resource Index. The matching (if present) and nonmatching resources
         // associated with a Resource Index are logically separated.
         // 3.4.1.3.8 For an fi_tsend(), the {JobID, PIDonFEP, Resource Index} tuple selects a
         // construct similar to a shared receive queue in a libfabric endpoint. Within the resources
         // selected by that tuple, the matching criteria (ses.match_bits plus ses.initiator) select
         // a buffer based on the corresponding matching criteria provided in the fi_trecv(). It is
         // expected that libfabric will be extended to allow an endpoint to specify that it supports
         // only exact matching (i.e., that all of the ignore bits must be 0). Exact matching is a
         // property of the libfabric endpoint – particularly the target libfabric endpoint – and not
         // the wire protocol itself. 
    unsigned int  header_data[2]; // This is the completion data to deliver at the target when
         // this operation completes when ses.som=1. If ses.hd=0, this field is ignored.
} standard_semantic_header_som1_type;

typedef struct {
    standard_semantic_header_common_fields_type common;
    unsigned int  buffer_offset[2]; 
    unsigned int  memory_key_match_bits[2]; 
    unsigned int  payload_length;
    unsigned int  message_offset;
} standard_semantic_header_som0_type;

typedef struct {
    standard_semantic_header_common_fields_type common;
    unsigned int initiator_restart_token;
    unsigned int target_restart_token; // must be zero on first send
    unsigned int  match_bits[2]; 
    unsigned int  header_data[2]; 
} standard_semantic_header_deferred_send_type;

typedef struct {
    standard_semantic_header_common_fields_type common;
    unsigned int  buffer_offset[2]; 
    unsigned int  initiator_restart_token;
    unsigned int  target_restart_token;
    unsigned int  header_data[2]; 
} standard_semantic_header_RTR_type;

#define UET_EXPECTED_LIST_RESPONSE   0x0
#define UET_OVERFLOW_LIST_RESPONSE   0x1

typedef struct {
    char          list;   // Indicates if the payload was delivered to the
                          // expected or unexpected list. Common values are
                          // (1) UET_EXPECTED (0x0) - Message matched the expected list.
                          // (2) UET_OVERFLOW (0x1) - An unexpected header was tracked
                          // for this message (3.4.3.6.2.1 or 3.4.3.6.2.2). Message payload
                          // may have been (3.4.3.6.2.2)
    char          opcode; // Indicates type of response (e.g., default response,
                          // response with payload, etc.). Table 3-18
    char          version;// Semantic protocol version – set to 0 in the
                          // initial version.
    char          return_code;
    unsigned int  JobID;
    unsigned int  modified_length; // Indicates the number of bytes of the target
                  // buffer that will be modified by this transaction. For example,
                  // some message may be truncated because no buffer is available.
} semantic_response_common_header_type;

typedef struct {
    semantic_response_common_header_type  common;
    char          ri_generation; // Contains the new index generation on a
                                 // generation mismatch response.
    unsigned int  request_message_id;
} semantic_response_header_type;

typedef struct {
    semantic_response_common_header_type  common;
    unsigned int  response_message_id;     // Message ID of the response
    unsigned int  read_request_message_id; // Message ID used in the original read
                  // request (or of the original fetching atomic operation request).
    unsigned int  message_offset;  // Indicates the relative position in the message
                                   // that this payload corresponds to
} semantic_response_with_data_header_type;

typedef struct semantic_header_struct {
    int  semantic_header_type;
    union {
        standard_semantic_header_som1_type som1_hdr;
        standard_semantic_header_som0_type som0_hdr;
        standard_semantic_header_deferred_send_type def_send_hdr;
        standard_semantic_header_RTR_type rtr_hdr;
        semantic_response_header_type      resp_hdr;
        semantic_response_with_data_header_type  resp_with_data_hdr;
    } hdr;
} semantic_header_struct_type;

typedef enum {
    LIBFABRIC_NO_OP_TRANSACTION = 0,
    LIBFABRIC_SEND_TRANSACTION,
    LIBFABRIC_DATAGRAM_SEND_TRANSACTION,
    LIBFABRIC_TAGGED_SEND_EXACT_MATCH_TRANSACTION,
    LIBFABRIC_WRITE_TRANSACTION,
    LIBFABRIC_WRITE_IMMEDIATE_TRANSACTION,
    LIBFABRIC_READ_TRANSACTION,
    LIBFABRIC_NON_FETCHING_ATOMIC_TRANSACTION,
    LIBFABRIC_FETCHING_ATOMIC_TRANSACTION,
    LIBFABRIC_DEFERRABLE_SEND_TRANSACTION,
    LIBFABRIC_DEFERRABLE_TSEND_TRANSACTION,
    LIBFABRIC_DEFERRABLE_SEND_AS_SEND_TRANSACTION,
    LIBFABRIC_READY_TO_RESTART_TRANSACTION,
    LIBFABRIC_RESPONSE_TRANSACTION,
    LIBFABRIC_RESPONSE_WITH_DATA_TRANSACTION,
    LIBFABRIC_TRANSACTION_MAX
} libfabric_transaction_type;
// A “transaction” is composed of all of the packets needed to eventually deliver the payload
// desired by the user and implement the libfabric request. A “message” consists of a set of packets
// sharing a single message ID. A transaction consists of one or more messages and supporting packets. 
// As an example, a 32 KB libfabric fi_send() would carry the payload in 8 packets with the same
// message ID that would be the send. A send transaction would include the acknowledgments and
// semantic responses to the send message packets

typedef enum {
    LIBFABRIC_NO_OP_TRANSACTION_TYPE = 0,
    LIBFABRIC_UNTAGGED_MSG_TRANSACTION_TYPE,
    LIBFABRIC_TAGGED_MSG_TRANSACTION_TYPE,
    LIBFABRIC_RMA_TRANSACTION_TYPE,
    LIBFABRIC_ATOMIC_TRANSACTION_TYPE
} libfabric_transaction_category;
// RMA operations (e.g., fi_read() and fi_write()) use a Resource Index space independent from
// send/receive (and tagged send/receive) operations. At the libfabric level, a target memory region is
// accessed using a memory key. A Resource Index defines a context within which a memory key has
// meaning. A given Resource Index (e.g., RI 0) selects a service (e.g.,SHMEM). Within that service,
// a memory key is mapped to a descriptor of a memory region. Each combination of FA, JobID (in the
// case of relative addressing), PIDonFEP, Resource Index, and Memory Key selects a unique buffer.

// An implementation MUST support the standard header format for RMA operations. In this format,
// an RMA opcode used with a {JobID, PIDonFEP, Resource Index, Memory Key} tuple MUST map to exactly
// one buffer
// Some profiles (e.g.HPC) support the optimized header format for RMA operations.
extern libfabric_transaction_category 
    libfabric_transaction_mapping_to_category[LIBFABRIC_TRANSACTION_MAX];

typedef enum {
    LIBFABRIC_API_RECV,
    LIBFABRIC_API_RECVVECTOR,
    LIBFABRIC_API_RECVMSG,
    LIBFABRIC_API_SEND,
    LIBFABRIC_API_SENDVECTOR,
    LIBFABRIC_API_SENDMSG,
    LIBFABRIC_API_INJECT,
    LIBFABRIC_API_SENDDATA,
    LIBFABRIC_API_INJECTDATA,
} libfabric_api_type;

typedef enum {
    UET_NO_OP = 0,  // Payload size must be 0. Single-packet messages only
    UET_WRITE = 1,
    UET_READ = 2,
    UET_ATOMIC = 3,
    UET_FETCHING_ATOMIC = 4,
    UET_SEND = 5,       // Payload size must be less than or equal to one Payload MTU.
    UET_RENDEZVOUS_SEND = 6, // Not required to be supported by AI Basic and AI Full profiles
    UET_DATAGRAM_SEND = 7,  // 
    UET_DEFERRABLE_SEND = 8,
    UET_TAGGED_SEND = 9,// Payload size must be less than or equal to one Payload MTU.
    UET_RENDEZVOUS_TSEND = 10, // Not required to be supported by AI Basic and AI Full profiles
    UET_DEFERRABLE_TSEND = 11,
    UET_DEFERRABLE_RTR = 12,
    UET_TSEND_ATOMIC = 13,
    UET_TSEND_FETCH_ATOMIC = 14,
    UET_MSG_ERROR = 15,
    UET_REQUEST_OPCODE_MAX
} uet_request_opcode_type;
    // The term “operation” is reserved for the behavior implemented at the endpoint.
    // The operation is encoded in the opcode (3.4.6.2) and specifies operations such as reads and
    // writes to memory, sending of messages, and atomic operations on memory

typedef enum {
    UET_DEFAULT_RESPONSE = 0,
    UET_RESPONSE = 1,
    UET_RESPONSE_W_DATA = 2,
    UET_NO_RESPONSE = 3,
    UET_RESPONSE_OPCODE_MAX
} uet_response_opcode_type;

extern unsigned int
    uet_opcode_for_libfabric_transaction[LIBFABRIC_TRANSACTION_MAX]; 

extern semantic_header_type
ses_hdr_format_for_libfabric_transaction[LIBFABRIC_TRANSACTION_MAX];

typedef enum
{
    SES_RC_NULL = 0,       // The RC status of this transaction is unknown.
    SES_RC_OK   = 1,       // The transaction completed successfully at the target.
    SES_RC_BAD_GENERATION = 2, // The generation in the request did not match the
                               // generation at the target index.
    SES_RC_DISABLED = 3,   // The targeted resource is disabled. Disabled resource
                           // has precedence over RC_NO_MATCH.
    SES_RC_DISABLED_GEN = 4,  // The targeted resource is disabled and supports the
                           // index generation. Disabled resource has precedence over RC_NO_MATCH.
    SES_RC_NO_MATCH = 5,   // The message could not be matched at the target and was dropped.
                           // This is returned for matching, nonmatching, and RMA transactions
                           // that fail to find a buffer.
    SES_RC_UNSUPPORTED_OP = 6,  // Unsupported network message type.
    SES_RC_UNSUPPORTED_SIZE = 7, // The message was larger than the supported size.
    SES_RC_AT_INVALID = 8, // Invalid address translation context.
    SES_RC_AT_PERM = 9,    // Address translation permission failure
    SES_RC_AT_ATS_ERROR = 10,
    SES_RC_AT_NO_TRANS = 11,
    SES_RC_AT_OUT_OF_RANGE = 12,
    SES_RC_HOST_POISONED = 13,
    SES_RC_HOST_UNSUCCESS_CMPL = 14,
    SES_RC_AMO_UNSUPPORTED_OP = 15,
    SES_RC_AMO_UNSUPPORTED_DT = 16,
    SES_RC_AMO_UNSUPPRTED_SIZE = 17,
    
    SES_RC_UNDELIVERABLE = 0x1F,  // Message could not be delivered. 
    SES_RC_UNCOR = 0x20,  // An uncorrectable error was detected. The error is
                          // not likely to be rectified without corrective action
    SES_RC_UNCOR_TRNSNT = 0x21,  // An uncorrectable error was detected. The error is
                          // likely to be transient.
    SES_RC_TOO_LONG = 0x22, // The message was longer than the buffer it addressed.
                          // The target was configured to reject a message that was too
                          // long rather than truncate it.
    SES_RC_INITIATOR_ERROR = 0x23, // This RC echoes back the initiator error field from
                          // the incoming packet. 
    SES_RC_DROPPED = 0x24 // Message dropped at the target for reasons other
                          // than those enumerated elsewhere.
} ses_return_code_type;

typedef struct {
    int buffer_id;
    int buffer_length;
    int operation_type;
} buffer_type;

// A memory buffer MUST be registered with a resource domain before it can be used as the target of a
// remote RMA or atomic data transfer. The fi_mr_reg() API is used to register a memory region on the domain.
//
// Additionally, a fabric provider MAY require that data buffers be registered before being used in
// local transfers (e.g., the buffer is the source for a write operation or the destination for a read operation).
// A memory region is bound to an endpoint using the fi_mr_bind() API.
typedef struct {
    int memory_region_id; 
    int optimized_key;
    union {
        unsigned int optimized_rkey;  // 12 bits
        unsigned long unoptimized_rkey; // 48 bits
    } key;
} memory_region_type;

char *create_semantic_header(semantic_header_struct_type *ses_hdr, char **ses_ptr, unsigned int *semantic_hdr_length);
void
parse_semantic_header(SES_Endpoint_type *src_ep, SES_Endpoint_type *dest_ep,
                      char *ses_ptr, unsigned int initiator_or_target);

#endif
