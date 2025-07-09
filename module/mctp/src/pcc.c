/* SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "libmctp.h"
#include "libmctp-alloc.h"
#include "libmctp-log.h"
#include "libmctp-pcc.h"
#include "container_of.h"

#include "fwk_log.h"

struct mctp_binding_pcc {
	struct mctp_binding binding;

	mctp_pcc_tx_fn tx_fn;
	void *tx_fn_data;

	struct mctp_pktbuf *rx_pkt;
};

#define binding_to_pcc(b)                                                   \
	container_of(b, struct mctp_binding_pcc, binding)

static int mctp_binding_pcc_tx(struct mctp_binding *b,
				  struct mctp_pktbuf *pkt)
{
	struct mctp_binding_pcc *pcc = binding_to_pcc(b);
	size_t len;

	len = mctp_pktbuf_size(pkt);

	if (pcc->tx_fn) {
		return pcc->tx_fn(pcc->tx_fn_data, pkt->data, len);
	}

	return -EBUSY;
}

void mctp_pcc_set_tx_fn(struct mctp_binding_pcc *pcc,
			   mctp_pcc_tx_fn fn, void *data)
{
	pcc->tx_fn = fn;
	pcc->tx_fn_data = data;
}

int mctp_pcc_rx(struct mctp_binding_pcc *pcc, const void *buf,
		   size_t len)
{
	pcc->rx_pkt = mctp_pktbuf_alloc(&pcc->binding, 0);

	mctp_pktbuf_push(pcc->rx_pkt, (void *) buf, len);

	struct mctp_pktbuf *pkt = pcc->rx_pkt;
	assert(pkt);

  FWK_LOG_ERR("john ===============");

	mctp_bus_rx(&pcc->binding, pkt);

	pcc->rx_pkt = NULL;

	return 0;
}

static int mctp_pcc_core_start(struct mctp_binding *binding)
{
	mctp_binding_set_tx_enabled(binding, true);
	return 0;
}

struct mctp_binding *mctp_binding_pcc_core(struct mctp_binding_pcc *b)
{
	return &b->binding;
}

struct mctp_binding_pcc *mctp_pcc_init(void)
{
	struct mctp_binding_pcc *pcc;

	pcc = __mctp_alloc(sizeof(*pcc));
	memset(pcc, 0, sizeof(*pcc));
	pcc->rx_pkt = NULL;
	pcc->binding.name = "pcc";
	pcc->binding.version = 1;
	pcc->binding.pkt_size = 0x1000;
	pcc->binding.pkt_header = 0;
	pcc->binding.pkt_trailer = 0;

	pcc->binding.start = mctp_pcc_core_start;
	pcc->binding.tx = mctp_binding_pcc_tx;

	return pcc;
}

void mctp_pcc_destroy(struct mctp_binding_pcc *pcc)
{
	__mctp_free(pcc);
}
