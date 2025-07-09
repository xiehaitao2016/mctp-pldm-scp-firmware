#ifndef MOD_MCTP_PCC_H
#define MOD_MCTP_PCC_H

#include <mod_mctp.h>

#define PCC_SUBSPACE_0      0x50434300
#define PCC_SUBSPACE_1      0x50434301
#define PCC_MCTP_COMMAND    0x5054434D // PTCM

#define PCC_TX_BUFFER_SIZE  128

struct mod_mctp_pcc_config {
  fwk_id_t driver_id;
  fwk_id_t driver_api_id;
};

/* mctp_pcc supports the follwoing bind requests */
enum {
    MCTP_PCC_BIND_REQ_TRANSPORT_API_IDX,
    MCTP_PCC_BIND_REQ_SERVICE_API_IDX,
    MCTP_PCC_BIND_REQ_API_IDX_COUNT,
};

/* mctp pcc binds to the following modules*/
enum {
    MCTP_PCC_BIND_TRANSPORT_API_IDX,
    MCTP_PCC_BIND_SERVICE_API_IDX,
    MCTP_PCC_BIND_API_IDX_COUNT,
};

#pragma pack(1)
struct mctp_pcc_pkt {
    uint32_t signature;
    uint32_t flags;
    uint32_t length;
    uint32_t command;
    uint8_t  mctp_payload[];
};
#pragma pack()

/* type for mctp_pcc apis exposed by the module */
typedef struct mctp_pcc_api {
    mctp_binding_pcc_t *(*mctp_pcc_register_tx)(void);
} mctp_pcc_api_t;

#endif
