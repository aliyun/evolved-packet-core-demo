#ifndef __PGW_CONTEXT_H__
#define __PGW_CONTEXT_H__

#include "core_list.h"
#include "core_index.h"
#include "core_errno.h"
#include "core_hash.h"
#include "core_network.h"
#include "core_msgq.h"
#include "core_timer.h"
#include "core_zmq.h"

#include "gtp/gtp_types.h"
#include "gtp/gtp_message.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#pragma pack(4)

#define MAX_NUM_OF_DEV          16
#define MAX_NUM_OF_SUBNET       16

#define	SPGW_UE_FLOW_STATS_FILE            "/epc/log/ue_flow_stats"
#define	SPGW_UE_FLOW_STATS_MAX_NUM         65536

typedef struct {
    c_uint32_t      ipv4;
    c_uint32_t      uplink_bytes;
    c_uint32_t      downlink_bytes;
} spgw_ue_flow_stats_t;

typedef struct {
    spgw_ue_flow_stats_t    ue_flow_stats[SPGW_UE_FLOW_STATS_MAX_NUM];
} spgw_ue_flow_stats_ctx_t;

typedef struct _gtp_node_t gtp_node_t;
typedef struct _fd_config_t fd_config_t;

typedef struct _spgw_context_t {
    const char*     fd_conf_path;   /* PGW freeDiameter conf path */
    fd_config_t     *fd_config;     /* PGW freeDiameter config */

    c_uint32_t      gtpc_port;      /* Default: PGW GTP-C local port */
    c_uint32_t      gtpu_port;      /* Default: PGW GTP-U local port */
    const char      *tun_ifname;    /* Default:: pgwtun */

    list_t          gtpc_list;      /* PGW GTPC IPv4 Server List */
    list_t          gtpc_list6;     /* PGW GTPC IPv6 Server List */
    sock_id         gtpc_sock;      /* PGW GTPC IPv4 Socket */
    sock_id         gtpc_sock6;     /* PGW GTPC IPv6 Socket */
    c_sockaddr_t    *gtpc_addr;     /* PGW GTPC IPv4 Address */
    c_sockaddr_t    *gtpc_addr6;    /* PGW GTPC IPv6 Address */

    list_t          gtpu_list;      /* PGW GTPU IPv4 Server List */
    list_t          gtpu_list6;     /* PGW GTPU IPv6 Server List */
    sock_id         gtpu_sock;      /* PGW GTPU IPv4 Socket */
    sock_id         gtpu_sock6;     /* PGW GTPU IPv6 Socket */
    c_sockaddr_t    *gtpu_addr;     /* PGW GTPU IPv4 Address */
    c_sockaddr_t    *gtpu_addr6;    /* PGW GTPU IPv6 Address */

    list_t          dev_list;       /* PGW Tun Device List */
    list_t          subnet_list;    /* PGW UE Subnet List */

    msgq_id         queue_id;       /* Qsesssess for processing PGW control plane */
    tm_service_t    tm_service;     /* Timer Service */

    /* zmq related params */
    char            event_addr[MAX_ZMQ_ADDR_LEN];
    void            *zmq_context;
    void            *zmq_event_sender;      /* event sender */
    void            *zmq_event_sender_zmq_main;      /* event sender */
    void            *zmq_event_receiver;    /* event receiver */    

#define MAX_NUM_OF_DNS              2
    const char      *dns[MAX_NUM_OF_DNS];
    const char      *dns6[MAX_NUM_OF_DNS];

#define MAX_NUM_OF_P_CSCF           16
    const char      *p_cscf[MAX_NUM_OF_P_CSCF];
    int             num_of_p_cscf;
    int             p_cscf_index;
    const char      *p_cscf6[MAX_NUM_OF_P_CSCF];
    int             num_of_p_cscf6;
    int             p_cscf6_index;

    list_t          sgw_s5c_list;  /* SGW GTPC Node List */
    list_t          sgw_s5u_list;  /* SGW GTPU Node List */
    list_t          ip_pool_list;

    // SGW
    list_t          mme_s11_list;  /* MME GTPC Node List */
    list_t          pgw_s5c_list;  /* PGW GTPC Node List */
    list_t          enb_s1u_list;  /* eNB GTPU Node List */
    list_t          pgw_s5u_list;  /* PGW GTPU Node List */
    hash_t          *imsi_ue_hash;  /* hash table (IMSI : SGW_UE) */

    list_t          static_ip_list;

    hash_t          *sess_hash; /* hash table (IMSI+APN) */
    
    spgw_ue_flow_stats_ctx_t    *ue_flow_stats_ctx;
} spgw_context_t;


typedef struct _pgw_subnet_t pgw_subnet_t;
typedef struct _pgw_ue_ip_t {
    c_uint32_t      addr[4];
    int             is_static;

    /* Related Context */
    pgw_subnet_t    *subnet;
} pgw_ue_ip_t;

typedef struct _pgw_dev_t {
    lnode_t     node;

    c_int8_t    ifname[IFNAMSIZ];
    sock_id     sock;

    c_sockaddr_t *link_local_addr;
} pgw_dev_t;

typedef struct _pgw_subnet_t {
    lnode_t     node;

    ipsubnet_t  sub;                    /* Subnet : cafe::0/64 */
    ipsubnet_t  gw;                     /* Gateway : cafe::1 */
    c_int8_t    apn[MAX_APN_LEN];       /* APN : "internet", "volte", .. */

    int         family;                 /* AF_INET or AF_INET6 */
    c_uint8_t   prefixlen;              /* prefixlen */

    struct {
        int head, tail;
        int size, avail;
        mutex_id mut;
        pgw_ue_ip_t *free[MAX_POOL_OF_UE], pool[MAX_POOL_OF_UE];
    } pool;

    /* Related Context */
    pgw_dev_t   *dev;
} pgw_subnet_t;

typedef struct _sgw_ue_t {
    index_t         index;  /* An index of this node */

    c_uint32_t      sgw_s11_teid;   /* SGW-S11-TEID is derived from INDEX */
    c_uint32_t      mme_s11_teid;   /* MME-S11-TEID is received from MME */

    /* UE identity */
    c_uint8_t       imsi[MAX_IMSI_LEN];
    int             imsi_len;
    c_int8_t        imsi_bcd[MAX_IMSI_BCD_LEN+1];

#define SGW_S1U_INACTIVE  0x0001
#define SGW_DL_NOTI_SENT  0x0002

#define SGW_GET_UE_STATE(__uE)  ((__uE)->state)
#define SGW_SET_UE_STATE(__uE,__sTATE)  ((__uE)->state |= (__sTATE))
#define SGW_RESET_UE_STATE(__uE, __sTATE)  ((__uE)->state &= ~(__sTATE))

    c_uint32_t      state;

    list_t          sess_list;

    gtp_node_t       *gnode;
} sgw_ue_t;


typedef struct _spgw_sess_t {
    lnode_t         node;       /* A node of list_t */
    index_t         index;          /**< An index of this node */

#define SGW_S5C_TEID(__tEID) (__tEID & 0x80000000)
#define SGW_S5C_TEID_TO_INDEX(__iNDEX) (__iNDEX & ~0x80000000)
#define SGW_S5C_INDEX_TO_TEID(__iNDEX) (__iNDEX | 0x80000000)
    c_uint32_t      pgw_s5c_teid;   /* PGW-S5C-TEID is derived from INDEX */
    c_uint32_t      sgw_s5c_teid;   /* SGW-S5C-TEID is received from SGW */

    c_int8_t        *gx_sid;        /* Gx Session ID */

    /* IMSI */
    c_uint8_t       imsi[MAX_IMSI_LEN];
    int             imsi_len;
    c_int8_t        imsi_bcd[MAX_IMSI_BCD_LEN+1];

    /* APN Configuration */
    pdn_t           pdn;
    pgw_ue_ip_t*    ipv4;
    pgw_ue_ip_t*    ipv6;

    /* User-Lication-Info */
    tai_t           tai;
    e_cgi_t         e_cgi;

    /* Hash Key : IMSI+APN */
    c_uint8_t       hash_keybuf[MAX_IMSI_LEN+MAX_APN_LEN+1];
    int             hash_keylen;

    list_t          bearer_list;

    /* Related Context */
    gtp_node_t      *gnode;
    sgw_ue_t        *sgw_ue;
} spgw_sess_t;

typedef struct _pgw_rule_t {
    c_uint8_t proto;
ED5(c_uint8_t ipv4_local:1;,
    c_uint8_t ipv4_remote:1;,
    c_uint8_t ipv6_local:1;,
    c_uint8_t ipv6_remote:1;,
    c_uint8_t reserved:4;)
    struct {
        struct {
            c_uint32_t addr[4];
            c_uint32_t mask[4];
        } local;
        struct {
            c_uint32_t addr[4];
            c_uint32_t mask[4];
        } remote;
    } ip;
    struct {
        struct {
            c_uint16_t low;
            c_uint16_t high;
        } local;
        struct {
            c_uint16_t low;
            c_uint16_t high;
        } remote;
    } port;
} pgw_rule_t;

typedef struct _spgw_bearer_t {
    lnode_t         node; /**< A node of list_t */
    index_t         index;

    c_uint8_t       ebi;

    /* User-Lication-Info */
    tai_t           tai;
    e_cgi_t         e_cgi;

    /* Pkts which will be buffered in case of UE-IDLE */
    c_uint32_t      num_buffered_pkt;

    /* FIXME: The value should be depdendant on the clbuf number. */
#define MAX_NUM_BUFFER_PKT      10 
    pkbuf_t*        buffered_pkts[MAX_NUM_BUFFER_PKT];

    list_t          tunnel_list;

    c_uint32_t      pgw_s5u_teid;   /* PGW_S5U is derived from INDEX */
    c_uint32_t      sgw_s5u_teid;   /* SGW_S5U is received from SGW */

    c_int8_t        *name;          /* PCC Rule Name */
    qos_t           qos;            /* QoS Infomration */

    /* Packet Filter Identifier Generator(1~15) */
    c_uint8_t       pf_identifier;
    /* Packet Filter List */
    list_t          pf_list;

    spgw_sess_t      *sess;
    sgw_ue_t        *sgw_ue;
    gtp_node_t      *gnode;
} spgw_bearer_t;

typedef struct _pgw_pf_t {
    lnode_t         node;

ED3(c_uint8_t spare:2;,
    c_uint8_t direction:2;,
    c_uint8_t identifier:4;)
    pgw_rule_t      rule;

    spgw_bearer_t    *bearer;
} pgw_pf_t;


/* ********************************************************
 * SGW Start
 * ********************************************************/
#if 0
typedef struct _sgw_sess_t {
    lnode_t         node;       /* A node of list_t */
    index_t         index;      /* An index of this node */

    /* 
     * SGW-S5C-TEID     = INDEX         | 0x80000000 
     * INDEX            = SGW-S5C-TEID  & ~0x80000000
     */
#define SGW_S5C_TEID(__tEID) (__tEID & 0x80000000)
#define SGW_S5C_TEID_TO_INDEX(__iNDEX) (__iNDEX & ~0x80000000)
#define SGW_S5C_INDEX_TO_TEID(__iNDEX) (__iNDEX | 0x80000000)
    c_uint32_t      sgw_s5c_teid;   /* SGW-S5C-TEID is derived from INDEX */    
    c_uint32_t      pgw_s5c_teid;   /* PGW-S5C-TEID is received from PGW */

    /* APN Configuration */
    pdn_t           pdn;

    list_t          bearer_list;

    /* Related Context */
    gtp_node_t      *gnode;
    sgw_ue_t        *sgw_ue;
} sgw_sess_t;
#endif

typedef struct _sgw_tunnel_t {
    lnode_t         node; /**< A node of list_t */
    index_t         index;

    c_uint8_t       interface_type;

    c_uint32_t      local_teid;
    c_uint32_t      remote_teid;

    /* Related Context */
    spgw_bearer_t    *bearer;
    gtp_node_t      *gnode;
} sgw_tunnel_t;

typedef struct _spgw_static_ip_t {
    lnode_t         node;
    c_uint8_t       imsi[MAX_IMSI_LEN];
    int             imsi_len;
    pgw_ue_ip_t     ue_ip;
} spgw_static_ip_t;

CORE_DECLARE(gtp_node_t *)  spgw_mme_add_by_message(gtp_message_t *message);

CORE_DECLARE(sgw_ue_t *)    spgw_ue_add_by_message(gtp_message_t *message);
CORE_DECLARE(sgw_ue_t*)     sgw_ue_add(c_uint8_t *imsi, int imsi_len);
CORE_DECLARE(sgw_ue_t*)     sgw_ue_find_by_imsi(c_uint8_t *imsi, int imsi_len);
CORE_DECLARE(sgw_ue_t*)     sgw_ue_find_by_teid(c_uint32_t teid);

CORE_DECLARE(spgw_sess_t*)   sgw_sess_add(sgw_ue_t *sgw_ue, c_int8_t *apn, c_uint8_t ebi);
CORE_DECLARE(status_t )      sgw_sess_remove(spgw_sess_t *sess);
CORE_DECLARE(status_t )      sgw_sess_remove_all(sgw_ue_t *sgw_ue);
CORE_DECLARE(spgw_sess_t*)   sgw_sess_find_by_apn(sgw_ue_t *sgw_ue, c_int8_t *apn);
CORE_DECLARE(spgw_sess_t*)   sgw_sess_find_by_ebi(sgw_ue_t *sgw_ue, c_uint8_t ebi);
CORE_DECLARE(spgw_sess_t*)   sgw_sess_find_by_teid(c_uint32_t teid);
CORE_DECLARE(spgw_sess_t*)   sgw_sess_first(sgw_ue_t *sgw_ue);
CORE_DECLARE(spgw_sess_t*)   sgw_sess_next(spgw_sess_t *sess);

CORE_DECLARE(status_t)       sgw_bearer_remove(spgw_bearer_t *bearer);
CORE_DECLARE(status_t)       sgw_bearer_remove_all(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) sgw_bearer_find_by_ue_ebi(sgw_ue_t *sgw_ue, c_uint8_t ebi);
CORE_DECLARE(spgw_bearer_t*) sgw_bearer_find_by_sess_ebi( spgw_sess_t *sess, c_uint8_t ebi);
CORE_DECLARE(spgw_bearer_t*) spgw_bearer_find_by_s1u_teid(c_uint32_t sgw_s1u_teid);
CORE_DECLARE(spgw_bearer_t*) sgw_default_bearer_in_sess(spgw_sess_t *sess);

CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_add(spgw_bearer_t *bearer, c_uint8_t interface_type);
CORE_DECLARE(status_t)      sgw_tunnel_remove(sgw_tunnel_t *tunnel);
CORE_DECLARE(status_t)      sgw_tunnel_remove_all(spgw_bearer_t *bearer);
CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_find(index_t index);
CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_find_by_teid(c_uint32_t teid);
CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_find_by_interface_type(spgw_bearer_t *bearer, c_uint8_t interface_type);
CORE_DECLARE(sgw_tunnel_t*) sgw_s1u_tunnel_in_bearer(spgw_bearer_t *bearer);
CORE_DECLARE(sgw_tunnel_t*) sgw_s5u_tunnel_in_bearer(spgw_bearer_t *bearer);
CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_first(spgw_bearer_t *bearer);
CORE_DECLARE(sgw_tunnel_t*) sgw_tunnel_next(sgw_tunnel_t *tunnel);
/* ********************************************************
 * SGW End
 * ********************************************************/
CORE_DECLARE(status_t)      spgw_context_init(void);
CORE_DECLARE(status_t)      spgw_context_final(void);
CORE_DECLARE(spgw_context_t*) spgw_self(void);

CORE_DECLARE(status_t)      spgw_context_parse_config(void);
CORE_DECLARE(status_t)      spgw_context_setup_trace_module(void);

CORE_DECLARE(gtp_node_t *)  pgw_sgw_add_by_message(gtp_message_t *message);
//CORE_DECLARE(spgw_sess_t *) pgw_sess_add_by_message(gtp_message_t *message);
//CORE_DECLARE(status_t) spgw_sess_update_by_message(spgw_sess_t *sess, gtp_message_t *message);

CORE_DECLARE(spgw_sess_t *) spgw_sess_add(sgw_ue_t *sgw_ue,
                                         c_int8_t *apn, 
                                         c_uint8_t pdn_type,
                                         c_uint8_t ebi);
#if 0
CORE_DECLARE(spgw_sess_t*)   pgw_sess_add(c_uint8_t *imsi,
                                          int imsi_len, 
                                          c_int8_t *apn,
                                          c_uint8_t pdn_type,
                                          c_uint8_t ebi);
#endif
CORE_DECLARE(status_t )     pgw_sess_remove(spgw_sess_t *sess);
CORE_DECLARE(status_t )     spgw_sess_remove(spgw_sess_t *sess);
CORE_DECLARE(status_t )     pgw_sess_remove_all();
CORE_DECLARE(spgw_sess_t*)   pgw_sess_find(index_t index);
CORE_DECLARE(spgw_sess_t*)   pgw_sess_find_by_teid(c_uint32_t teid);
CORE_DECLARE(spgw_sess_t*)   pgw_sess_find_by_imsi_apn(
        c_uint8_t *imsi, int imsi_len, c_int8_t *apn);
CORE_DECLARE(hash_index_t *)  pgw_sess_first();
CORE_DECLARE(hash_index_t *)  pgw_sess_next(hash_index_t *hi);
CORE_DECLARE(spgw_sess_t *)  spgw_sess_this(hash_index_t *hi);

CORE_DECLARE(spgw_bearer_t*) spgw_bearer_add(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_add(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) sgw_bearer_add(spgw_sess_t *sess);
CORE_DECLARE(status_t)       pgw_bearer_remove(spgw_bearer_t *bearer);
CORE_DECLARE(status_t)       pgw_bearer_remove_all(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_find(index_t index);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_find_by_pgw_s5u_teid(c_uint32_t pgw_s5u_teid);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_find_by_ebi(spgw_sess_t *sess, c_uint8_t ebi);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_find_by_name(spgw_sess_t *sess, c_int8_t *name);
CORE_DECLARE(spgw_bearer_t*) pgw_bearer_find_by_qci_arp(spgw_sess_t *sess, 
                                c_uint8_t qci,
                                c_uint8_t priority_level,
                                c_uint8_t pre_emption_capability,
                                c_uint8_t pre_emption_vulnerability);
CORE_DECLARE(spgw_bearer_t*) pgw_default_bearer_in_sess(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) spgw_bearer_first(spgw_sess_t *sess);
CORE_DECLARE(spgw_bearer_t*) spgw_bearer_next(spgw_bearer_t *bearer);
CORE_DECLARE(spgw_bearer_t*) spgw_bearer_find(index_t index);

CORE_DECLARE(pgw_pf_t*)     pgw_pf_add(
                                spgw_bearer_t *bearer, c_uint32_t precedence);
CORE_DECLARE(status_t )     pgw_pf_remove(pgw_pf_t *pf);
CORE_DECLARE(status_t )     pgw_pf_remove_all(spgw_bearer_t *bearer);
CORE_DECLARE(pgw_pf_t*)     pgw_pf_find_by_id(
                                spgw_bearer_t *pgw_bearer, c_uint8_t id);
CORE_DECLARE(pgw_pf_t*)     pgw_pf_first(spgw_bearer_t *bearer);
CORE_DECLARE(pgw_pf_t*)     pgw_pf_next(pgw_pf_t *pf);

CORE_DECLARE(status_t )     pgw_ue_pool_generate();
CORE_DECLARE(pgw_ue_ip_t *) pgw_ue_ip_alloc(int family, const char *apn);
CORE_DECLARE(status_t)      pgw_ue_ip_free(pgw_ue_ip_t *ip);

CORE_DECLARE(pgw_dev_t*)    pgw_dev_add(const char *ifname);
CORE_DECLARE(status_t )     pgw_dev_remove(pgw_dev_t *dev);
CORE_DECLARE(status_t )     pgw_dev_remove_all();
CORE_DECLARE(pgw_dev_t*)    pgw_dev_find_by_ifname(const char *ifname);
CORE_DECLARE(pgw_dev_t*)    pgw_dev_first();
CORE_DECLARE(pgw_dev_t*)    pgw_dev_next(pgw_dev_t *dev);

CORE_DECLARE(pgw_subnet_t*) pgw_subnet_add(
        const char *ipstr, const char *mask_or_numbits,
        const char *apn, const char *ifname);
CORE_DECLARE(status_t )     pgw_subnet_remove(pgw_subnet_t *subnet);
CORE_DECLARE(status_t )     pgw_subnet_remove_all();
CORE_DECLARE(pgw_subnet_t*) pgw_subnet_first();
CORE_DECLARE(pgw_subnet_t*) pgw_subnet_next(pgw_subnet_t *subnet);

CORE_DECLARE(spgw_static_ip_t*) static_ip_add(c_uint8_t *imsi, int imsi_len, c_uint32_t ipv4);
CORE_DECLARE(pgw_ue_ip_t*) get_static_ip_by_imsi(c_uint8_t *imsi, int imsi_len);
CORE_DECLARE(void) remove_all_static_ip();
CORE_DECLARE(void) spgw_load_static_ip_config();

CORE_DECLARE(spgw_ue_flow_stats_ctx_t*)         spgw_ue_flow_stats_memory_init();
CORE_DECLARE(void)                              spgw_ue_flow_stat_update(c_uint32_t ipv4, c_uint32_t rx_bytes, c_uint32_t tx_bytes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PGW_CONTEXT_H__ */
