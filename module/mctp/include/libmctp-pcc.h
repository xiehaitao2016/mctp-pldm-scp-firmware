/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#ifndef _LIBMCTP_PCC_H
#define _LIBMCTP_PCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libmctp.h>

struct mctp_binding_pcc;

struct mctp_binding_pcc *mctp_pcc_init(void);
void mctp_pcc_destroy(struct mctp_binding_pcc *pcc);

struct mctp_binding *mctp_binding_pcc_core(struct mctp_binding_pcc *b);

/* direct function call IO */
typedef int (*mctp_pcc_tx_fn)(void *data, void *buf, size_t len)
	__attribute__((warn_unused_result));
void mctp_pcc_set_tx_fn(struct mctp_binding_pcc *pcc,
			   mctp_pcc_tx_fn fn, void *data);
int mctp_pcc_rx(struct mctp_binding_pcc *pcc, const void *buf,
		   size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _LIBMCTP_PCC_H */
