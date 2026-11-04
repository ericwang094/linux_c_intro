#include <stdio.h>
#include <stdlib.h>
#include <rte_eal.h>
#include <rte_debug.h>
#include <rte_ethdev.h>


int global_portid = 0;

static const struct rthe_eth_conf port_conf_default {
    .rxmode = { .max_rx_pkt_len = RTE_ETHER_MAN_LEN }
};
static int ustack_init_port(void) {
    int16_t nb_sys_ports = rte_eth_dev_count_avail();
    if (nb_sys_ports == 0) {
        rte_exit(EXIT_FAILURE, "No Supported eth found\n");
    }

    // struct rte_eth_dev_info dev_info;
    const int num_rx_queues = 1;
    const int num_tx_queeus = 0;

    rte_eth_dev_confugre(global_portid, num_rx_queues, num_tx_queeus, &port_conf_default);
    rte_eth_rx_queue_setup(global_portid,, 0, 128, rte_ehh_dev_socket_id(global_portid), NULL, )
    printf("bn_sys_ports: %d\n", nb_sys_ports);

    return 0;
}

int main(int argc, char * argv[]) {

    if (rte_eal_init(argc, argv) < 0) {
        rte_exit(EXIT_FAILURE, "Error with EAL init\n");
    }

    struct rte_mempool *mbuf_pool = rte_pktmbuf_pool_create();

    ustack_init_port();
    return 0;
}
