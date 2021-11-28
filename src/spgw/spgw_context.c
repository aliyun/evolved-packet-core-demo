#define TRACE_MODULE _spgw_context
#include "core_debug.h"
#include "core_pool.h"
#include "core_index.h"
#include "core_lib.h"
#include "core_network.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <mongoc.h>
#include <yaml.h>
#include "common/yaml_helper.h"

#include "gtp/gtp_types.h"
#include "gtp/gtp_conv.h"
#include "gtp/gtp_node.h"
#include "gtp/gtp_path.h"
#include "gtp/gtp_xact.h"

#include "fd/fd_lib.h"

#include "common/context.h"
#include "spgw_context.h"

static spgw_context_t self;
static fd_config_t g_fd_conf;
static void spgw_context_init_zmq_paths();

pool_declare(pgw_dev_pool, pgw_dev_t, MAX_NUM_OF_DEV);
pool_declare(pgw_subnet_pool, pgw_subnet_t, MAX_NUM_OF_SUBNET);

index_declare(spgw_sess_pool, spgw_sess_t, MAX_POOL_OF_SESS);
index_declare(spgw_bearer_pool, spgw_bearer_t, MAX_POOL_OF_BEARER);

pool_declare(pgw_pf_pool, pgw_pf_t, MAX_POOL_OF_PF);

// SGW
index_declare(sgw_ue_pool, sgw_ue_t, MAX_POOL_OF_UE);
index_declare(sgw_tunnel_pool, sgw_tunnel_t, MAX_POOL_OF_TUNNEL);

pgw_subnet_t *default_subnet = NULL;

static int context_initiaized = 0;

status_t spgw_context_init()
{
    status_t rv;

    d_assert(context_initiaized == 0, return CORE_ERROR, "SPGW context already has been initialized");

    /* Initial FreeDiameter Config */
    memset(&g_fd_conf, 0, sizeof(fd_config_t));

    /* Initialize PGW context */
    memset(&self, 0, sizeof(spgw_context_t));
    self.ue_flow_stats_ctx = spgw_ue_flow_stats_memory_init();
    d_assert(self.ue_flow_stats_ctx, return CORE_ERROR, "spgw_ue_flow_stats_memory_init failed!");

    /* ZMQ */
    rv = zmq_context_init(&self.zmq_context);
    d_assert(rv == CORE_OK, return CORE_ERROR, "zmq ctx init failed!");    
    
    self.fd_config = &g_fd_conf;

    list_init(&self.gtpc_list);
    list_init(&self.gtpc_list6);
    list_init(&self.gtpu_list);
    list_init(&self.gtpu_list6);

    gtp_node_init();
    list_init(&self.sgw_s5c_list);
    list_init(&self.sgw_s5u_list);

    list_init(&self.dev_list);
    pool_init(&pgw_dev_pool, MAX_NUM_OF_DEV);
    list_init(&self.subnet_list);
    pool_init(&pgw_subnet_pool, MAX_NUM_OF_SUBNET);

    index_init(&spgw_sess_pool, MAX_POOL_OF_SESS);
    index_init(&spgw_bearer_pool, MAX_POOL_OF_BEARER);

    pool_init(&pgw_pf_pool, MAX_POOL_OF_PF);

    // SGW
    list_init(&self.mme_s11_list);
    list_init(&self.enb_s1u_list);
    index_init(&sgw_ue_pool, MAX_POOL_OF_UE);
    index_init(&sgw_tunnel_pool, MAX_POOL_OF_TUNNEL);
    self.imsi_ue_hash = hash_make();

    list_init(&self.static_ip_list);

    self.sess_hash = hash_make();

    context_initiaized = 1;

    return CORE_OK;
}

status_t spgw_context_final()
{
    d_assert(context_initiaized == 1, return CORE_ERROR,
            "SPGW context already has been finalized");

    zmq_context_final(self.zmq_context);    

    pgw_sess_remove_all();

    // SGW
    d_assert(self.imsi_ue_hash, , "Null param");
    hash_destroy(self.imsi_ue_hash);

    d_assert(self.sess_hash, , "Null param");
    hash_destroy(self.sess_hash);

    if (index_used(&spgw_sess_pool))
        d_error("%d not freed in spgw_sess_pool[%d] in PGW-Context",
                index_used(&spgw_sess_pool), index_size(&spgw_sess_pool));
    d_trace(9, "%d not freed in spgw_sess_pool[%d] in PGW-Context\n",
            index_used(&spgw_sess_pool), index_size(&spgw_sess_pool));

    pgw_dev_remove_all();
    pgw_subnet_remove_all();

    index_final(&spgw_bearer_pool);
    index_final(&spgw_sess_pool);
    pool_final(&pgw_pf_pool);

    pool_final(&pgw_dev_pool);
    pool_final(&pgw_subnet_pool);

    gtp_remove_all_nodes(&self.sgw_s5c_list);
    gtp_remove_all_nodes(&self.sgw_s5u_list);
    // SGW
    gtp_remove_all_nodes(&self.mme_s11_list);
    gtp_remove_all_nodes(&self.enb_s1u_list);
    gtp_node_final();
    
    sock_remove_all_nodes(&self.gtpc_list);
    sock_remove_all_nodes(&self.gtpc_list6);
    sock_remove_all_nodes(&self.gtpu_list);
    sock_remove_all_nodes(&self.gtpu_list6);

    remove_all_static_ip();

    context_initiaized = 0;
    
    return CORE_OK;
}

spgw_context_t* spgw_self()
{
    return &self;
}

static status_t spgw_context_prepare()
{
    self.gtpc_port = GTPV2_C_UDP_PORT;
    self.gtpu_port = GTPV1_U_UDP_PORT;
    self.fd_config->cnf_port = DIAMETER_PORT;
    self.fd_config->cnf_port_tls = DIAMETER_SECURE_PORT;

    self.tun_ifname = "pgwtun";
    
    spgw_context_init_zmq_paths();

    return CORE_OK;
}

static status_t spgw_context_validation()
{
    if (self.fd_conf_path == NULL && (self.fd_config->cnf_diamid == NULL || self.fd_config->cnf_diamrlm == NULL || self.fd_config->cnf_addr == NULL))
    {
        d_error("No pgw.freeDiameter in '%s'", context_self()->config.path);
        return CORE_ERROR;
    }
    if (list_first(&self.gtpc_list) == NULL && list_first(&self.gtpc_list6) == NULL)
    {
        d_error("No pgw.gtpc in '%s'", context_self()->config.path);
        return CORE_ERROR;
    }
    if (list_first(&self.gtpu_list) == NULL && list_first(&self.gtpu_list6) == NULL)
    {
        d_error("No pgw.gtpu in '%s'", context_self()->config.path);
        return CORE_ERROR;
    }
    if (self.dns[0] == NULL && self.dns6[0] == NULL)
    {
        d_error("No pgw.dns in '%s'", context_self()->config.path);
        return CORE_ERROR;
    }
    return CORE_OK;
}

status_t spgw_context_parse_config()
{
    status_t rv;
    config_t *config = &context_self()->config;
    yaml_document_t *document = NULL;
    yaml_iter_t root_iter;

    d_assert(config, return CORE_ERROR,);
    document = config->document;
    d_assert(document, return CORE_ERROR,);

    rv = spgw_context_prepare();
    if (rv != CORE_OK) return rv;

    yaml_iter_init(&root_iter, document);
    while(yaml_iter_next(&root_iter))
    {
        const char *root_key = yaml_iter_key(&root_iter);
        d_assert(root_key, return CORE_ERROR,);
        if (!strcmp(root_key, "spgw"))
        {
            yaml_iter_t pgw_iter;
            yaml_iter_recurse(&root_iter, &pgw_iter);
            while(yaml_iter_next(&pgw_iter))
            {
                const char *pgw_key = yaml_iter_key(&pgw_iter);
                d_assert(pgw_key, return CORE_ERROR,);
                if (!strcmp(pgw_key, "freeDiameter"))
                {
                    yaml_node_t *node = 
                        yaml_document_get_node(document, pgw_iter.pair->value);
                    d_assert(node, return CORE_ERROR,);
                    if (node->type == YAML_SCALAR_NODE)
                    {
                        self.fd_conf_path = yaml_iter_value(&pgw_iter);
                    }
                    else if (node->type == YAML_MAPPING_NODE)
                    {
                        yaml_iter_t fd_iter;
                        yaml_iter_recurse(&pgw_iter, &fd_iter);

                        while(yaml_iter_next(&fd_iter))
                        {
                            const char *fd_key = yaml_iter_key(&fd_iter);
                            d_assert(fd_key, return CORE_ERROR,);
                            if (!strcmp(fd_key, "identity"))
                            {
                                self.fd_config->cnf_diamid = 
                                    yaml_iter_value(&fd_iter);
                            }
                            else if (!strcmp(fd_key, "realm"))
                            {
                                self.fd_config->cnf_diamrlm = 
                                    yaml_iter_value(&fd_iter);
                            }
                            else if (!strcmp(fd_key, "port"))
                            {
                                const char *v = yaml_iter_value(&fd_iter);
                                if (v) self.fd_config->cnf_port = atoi(v);
                            }
                            else if (!strcmp(fd_key, "sec_port"))
                            {
                                const char *v = yaml_iter_value(&fd_iter);
                                if (v) self.fd_config->cnf_port_tls = atoi(v);
                            }
                            else if (!strcmp(fd_key, "no_sctp"))
                            {
                                self.fd_config->cnf_flags.no_sctp =
                                    yaml_iter_bool(&fd_iter);
                            }
                            else if (!strcmp(fd_key, "listen_on"))
                            {
                                self.fd_config->cnf_addr = 
                                    yaml_iter_value(&fd_iter);
                            }
                            else if (!strcmp(fd_key, "load_extension"))
                            {
                                yaml_iter_t ext_array, ext_iter;
                                yaml_iter_recurse(&fd_iter, &ext_array);
                                do
                                {
                                    const char *module = NULL;
                                    const char *conf = NULL;

                                    if (yaml_iter_type(&ext_array) ==
                                        YAML_MAPPING_NODE)
                                    {
                                        memcpy(&ext_iter, &ext_array,
                                                sizeof(yaml_iter_t));
                                    }
                                    else if (yaml_iter_type(&ext_array) ==
                                        YAML_SEQUENCE_NODE)
                                    {
                                        if (!yaml_iter_next(&ext_array))
                                            break;
                                        yaml_iter_recurse(
                                                &ext_array, &ext_iter);
                                    }
                                    else if (yaml_iter_type(&ext_array) ==
                                        YAML_SCALAR_NODE)
                                    {
                                        break;
                                    }
                                    else
                                        d_assert(0, return CORE_ERROR,);

                                    while(yaml_iter_next(&ext_iter))
                                    {
                                        const char *ext_key =
                                            yaml_iter_key(&ext_iter);
                                        d_assert(ext_key,
                                                return CORE_ERROR,);
                                        if (!strcmp(ext_key, "module"))
                                        {
                                            module = yaml_iter_value(&ext_iter);
                                        }
                                        else if (!strcmp(ext_key, "conf"))
                                        {
                                            conf = yaml_iter_value(&ext_iter);
                                        }
                                        else
                                            d_warn("unknown key `%s`", ext_key);
                                    }

                                    if (module)
                                    {
                                        self.fd_config->
                                            ext[self.fd_config->num_of_ext].
                                                module = module;
                                        self.fd_config->
                                            ext[self.fd_config->num_of_ext].
                                                conf = conf;
                                        self.fd_config->num_of_ext++;
                                    }
                                } while(yaml_iter_type(&ext_array) ==
                                        YAML_SEQUENCE_NODE);
                            }
                            else if (!strcmp(fd_key, "connect"))
                            {
                                yaml_iter_t conn_array, conn_iter;
                                yaml_iter_recurse(&fd_iter, &conn_array);
                                do
                                {
                                    const char *identity = NULL;
                                    const char *addr = NULL;
                                    c_uint16_t port = 0;

                                    if (yaml_iter_type(&conn_array) ==
                                        YAML_MAPPING_NODE)
                                    {
                                        memcpy(&conn_iter, &conn_array,
                                                sizeof(yaml_iter_t));
                                    }
                                    else if (yaml_iter_type(&conn_array) ==
                                        YAML_SEQUENCE_NODE)
                                    {
                                        if (!yaml_iter_next(&conn_array))
                                            break;
                                        yaml_iter_recurse(&conn_array, &conn_iter);
                                    }
                                    else if (yaml_iter_type(&conn_array) ==
                                        YAML_SCALAR_NODE)
                                    {
                                        break;
                                    }
                                    else
                                        d_assert(0, return CORE_ERROR,);

                                    while(yaml_iter_next(&conn_iter))
                                    {
                                        const char *conn_key =
                                            yaml_iter_key(&conn_iter);
                                        d_assert(conn_key,
                                                return CORE_ERROR,);
                                        if (!strcmp(conn_key, "identity"))
                                        {
                                            identity = yaml_iter_value(&conn_iter);
                                        }
                                        else if (!strcmp(conn_key, "addr"))
                                        {
                                            addr = yaml_iter_value(&conn_iter);
                                        }
                                        else if (!strcmp(conn_key, "port"))
                                        {
                                            const char *v =
                                                yaml_iter_value(&conn_iter);
                                            if (v) port = atoi(v);
                                        }
                                        else
                                            d_warn("unknown key `%s`", conn_key);
                                    }

                                    if (identity && addr)
                                    {
                                        self.fd_config->
                                            conn[self.fd_config->num_of_conn].
                                                identity = identity;
                                        self.fd_config->
                                            conn[self.fd_config->num_of_conn].
                                                addr = addr;
                                        self.fd_config->
                                            conn[self.fd_config->num_of_conn].
                                                port = port;
                                        self.fd_config->num_of_conn++;
                                    }
                                } while(yaml_iter_type(&conn_array) ==
                                        YAML_SEQUENCE_NODE);
                            }
                            else
                                d_warn("unknown key `%s`", fd_key);
                        }
                    }
                }
                else if (!strcmp(pgw_key, "gtpc"))
                {
                    yaml_iter_t gtpc_array, gtpc_iter;
                    yaml_iter_recurse(&pgw_iter, &gtpc_array);
                    do
                    {
                        int family = AF_UNSPEC;
                        int i, num = 0;
                        const char *hostname[MAX_NUM_OF_HOSTNAME];
                        c_uint16_t port = self.gtpc_port;
                        const char *dev = NULL;
                        c_sockaddr_t *list = NULL;
                        sock_node_t *node = NULL;

                        if (yaml_iter_type(&gtpc_array) == YAML_MAPPING_NODE)
                        {
                            memcpy(&gtpc_iter, &gtpc_array,
                                    sizeof(yaml_iter_t));
                        }
                        else if (yaml_iter_type(&gtpc_array) ==
                            YAML_SEQUENCE_NODE)
                        {
                            if (!yaml_iter_next(&gtpc_array))
                                break;
                            yaml_iter_recurse(&gtpc_array, &gtpc_iter);
                        }
                        else if (yaml_iter_type(&gtpc_array) ==
                            YAML_SCALAR_NODE)
                        {
                            break;
                        }
                        else
                            d_assert(0, return CORE_ERROR,);

                        while(yaml_iter_next(&gtpc_iter))
                        {
                            const char *gtpc_key =
                                yaml_iter_key(&gtpc_iter);
                            d_assert(gtpc_key,
                                    return CORE_ERROR,);
                            if (!strcmp(gtpc_key, "family"))
                            {
                                const char *v = yaml_iter_value(&gtpc_iter);
                                if (v) family = atoi(v);
                                if (family != AF_UNSPEC &&
                                    family != AF_INET && family != AF_INET6)
                                {
                                    d_warn("Ignore family(%d) : AF_UNSPEC(%d), "
                                        "AF_INET(%d), AF_INET6(%d) ", 
                                        family, AF_UNSPEC, AF_INET, AF_INET6);
                                    family = AF_UNSPEC;
                                }
                            }
                            else if (!strcmp(gtpc_key, "addr") ||
                                    !strcmp(gtpc_key, "name"))
                            {
                                yaml_iter_t hostname_iter;
                                yaml_iter_recurse(&gtpc_iter, &hostname_iter);
                                d_assert(yaml_iter_type(&hostname_iter) !=
                                    YAML_MAPPING_NODE, return CORE_ERROR,);

                                do
                                {
                                    if (yaml_iter_type(&hostname_iter) ==
                                            YAML_SEQUENCE_NODE)
                                    {
                                        if (!yaml_iter_next(&hostname_iter))
                                            break;
                                    }

                                    d_assert(num <= MAX_NUM_OF_HOSTNAME,
                                            return CORE_ERROR,);
                                    hostname[num++] = 
                                        yaml_iter_value(&hostname_iter);
                                } while(
                                    yaml_iter_type(&hostname_iter) ==
                                        YAML_SEQUENCE_NODE);
                            }
                            else if (!strcmp(gtpc_key, "port"))
                            {
                                const char *v = yaml_iter_value(&gtpc_iter);
                                if (v)
                                {
                                    port = atoi(v);
                                    self.gtpc_port = port;
                                }
                            }
                            else if (!strcmp(gtpc_key, "dev"))
                            {
                                dev = yaml_iter_value(&gtpc_iter);
                            }
                            else
                                d_warn("unknown key `%s`", gtpc_key);
                        }

                        list = NULL;
                        for (i = 0; i < num; i++)
                        {
                            rv = core_addaddrinfo(&list,
                                    family, hostname[i], port, 0);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);
                        }

                        if (list)
                        {
                            if (context_self()->parameter.no_ipv4 == 0)
                            {
                                rv = sock_add_node(&self.gtpc_list,
                                        &node, list, AF_INET);
                                d_assert(rv == CORE_OK, return CORE_ERROR,);
                            }

                            if (context_self()->parameter.no_ipv6 == 0)
                            {
                                rv = sock_add_node(&self.gtpc_list6,
                                        &node, list, AF_INET6);
                                d_assert(rv == CORE_OK, return CORE_ERROR,);
                            }

                            core_freeaddrinfo(list);
                        }

                        if (dev)
                        {
                            rv = sock_probe_node(
                                    context_self()->parameter.no_ipv4 ?
                                        NULL : &self.gtpc_list,
                                    context_self()->parameter.no_ipv6 ?
                                        NULL : &self.gtpc_list6,
                                    dev, self.gtpc_port);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);
                        }

                    } while(yaml_iter_type(&gtpc_array) == YAML_SEQUENCE_NODE);

                    if (list_first(&self.gtpc_list) == NULL &&
                        list_first(&self.gtpc_list6) == NULL)
                    {
                        rv = sock_probe_node(
                                context_self()->parameter.no_ipv4 ?
                                    NULL : &self.gtpc_list,
                                context_self()->parameter.no_ipv6 ?
                                    NULL : &self.gtpc_list6,
                                NULL, self.gtpc_port);
                        d_assert(rv == CORE_OK, return CORE_ERROR,);
                    }
                }
                else if (!strcmp(pgw_key, "gtpu"))
                {
                    yaml_iter_t gtpu_array, gtpu_iter;
                    yaml_iter_recurse(&pgw_iter, &gtpu_array);
                    do
                    {
                        int family = AF_UNSPEC;
                        int i, num = 0;
                        const char *hostname[MAX_NUM_OF_HOSTNAME];
                        c_uint16_t port = self.gtpu_port;
                        const char *dev = NULL;
                        c_sockaddr_t *list = NULL;
                        sock_node_t *node = NULL;
                        sock_bind_cell_t bind_cell;
                        c_uint8_t num_of_cell = 0;

                        memset(&bind_cell, 0, sizeof(sock_bind_cell_t));

                        if (yaml_iter_type(&gtpu_array) == YAML_MAPPING_NODE)
                        {
                            memcpy(&gtpu_iter, &gtpu_array,
                                    sizeof(yaml_iter_t));
                        }
                        else if (yaml_iter_type(&gtpu_array) ==
                            YAML_SEQUENCE_NODE)
                        {
                            if (!yaml_iter_next(&gtpu_array))
                                break;
                            yaml_iter_recurse(&gtpu_array, &gtpu_iter);
                        }
                        else if (yaml_iter_type(&gtpu_array) ==
                            YAML_SCALAR_NODE)
                        {
                            break;
                        }
                        else
                            d_assert(0, return CORE_ERROR,);

                        while(yaml_iter_next(&gtpu_iter))
                        {
                            const char *gtpu_key =
                                yaml_iter_key(&gtpu_iter);
                            d_assert(gtpu_key,
                                    return CORE_ERROR,);
                            if (!strcmp(gtpu_key, "family"))
                            {
                                const char *v = yaml_iter_value(&gtpu_iter);
                                if (v) family = atoi(v);
                                if (family != AF_UNSPEC &&
                                    family != AF_INET && family != AF_INET6)
                                {
                                    d_warn("Ignore family(%d) : AF_UNSPEC(%d), "
                                        "AF_INET(%d), AF_INET6(%d) ", 
                                        family, AF_UNSPEC, AF_INET, AF_INET6);
                                    family = AF_UNSPEC;
                                }
                            }
                            else if (!strcmp(gtpu_key, "addr") ||
                                    !strcmp(gtpu_key, "name"))
                            {
                                yaml_iter_t hostname_iter;
                                yaml_iter_recurse(&gtpu_iter, &hostname_iter);
                                d_assert(yaml_iter_type(&hostname_iter) !=
                                    YAML_MAPPING_NODE, return CORE_ERROR,);

                                do
                                {
                                    if (yaml_iter_type(&hostname_iter) ==
                                            YAML_SEQUENCE_NODE)
                                    {
                                        if (!yaml_iter_next(&hostname_iter))
                                            break;
                                    }

                                    d_assert(num <= MAX_NUM_OF_HOSTNAME,
                                            return CORE_ERROR,);
                                    hostname[num++] = 
                                        yaml_iter_value(&hostname_iter);
                                } while(
                                    yaml_iter_type(&hostname_iter) ==
                                        YAML_SEQUENCE_NODE);
                            }
                            else if (!strcmp(gtpu_key, "port"))
                            {
                                const char *v = yaml_iter_value(&gtpu_iter);
                                if (v)
                                {
                                    port = atoi(v);
                                    self.gtpu_port = port;
                                }
                            }
                            else if (!strcmp(gtpu_key, "cell_id"))
                            {
                                yaml_iter_t cell_id_iter;
                                yaml_iter_recurse(&gtpu_iter, &cell_id_iter);
                                d_assert(yaml_iter_type(&cell_id_iter) != YAML_MAPPING_NODE, return CORE_ERROR,);

                                do {
                                    const char *v = NULL;

                                    d_assert(num_of_cell <= MAX_NUM_OF_BIND_CELL, return CORE_ERROR,);
                                    if (yaml_iter_type(&cell_id_iter) == YAML_SEQUENCE_NODE) {
                                        if (!yaml_iter_next(&cell_id_iter))
                                            break;
                                    }

                                    v = yaml_iter_value(&cell_id_iter);
                                    if (v) {
                                        bind_cell.cells[num_of_cell] = atoi(v);
                                        num_of_cell++;
                                    }
                                } while(yaml_iter_type(&cell_id_iter) == YAML_SEQUENCE_NODE);
                            }
                            else if (!strcmp(gtpu_key, "dev"))
                            {
                                dev = yaml_iter_value(&gtpu_iter);
                            }
                            else
                                d_warn("unknown key `%s`", gtpu_key);
                        }

                        list = NULL;
                        for (i = 0; i < num; i++)
                        {
                            rv = core_addaddrinfo(&list,
                                    family, hostname[i], port, 0);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);
                        }

                        if (list)
                        {
                            if (context_self()->parameter.no_ipv4 == 0)
                            {
                                rv = sock_add_node(&self.gtpu_list,
                                        &node, list, AF_INET);
                                d_assert(rv == CORE_OK, return CORE_ERROR,);

                                /*printf("====num: %d, cells: %d %d %d %d\r\n", num_of_cell,
                                        bind_cell.cells[0],
                                        bind_cell.cells[1],
                                        bind_cell.cells[2],
                                        bind_cell.cells[3]); */
                                if (num_of_cell > 0) {
                                    rv = sock_set_bind_cell(node, &bind_cell);
                                    d_assert(rv == CORE_OK, return CORE_ERROR,);
                                }
                            }

                            if (context_self()->parameter.no_ipv6 == 0)
                            {
                                rv = sock_add_node(&self.gtpu_list6,
                                        &node, list, AF_INET6);
                                d_assert(rv == CORE_OK, return CORE_ERROR,);
                            }

                            core_freeaddrinfo(list);
                        }

                        if (dev)
                        {
                            rv = sock_probe_node(
                                    context_self()->parameter.no_ipv4 ?
                                        NULL : &self.gtpu_list,
                                    context_self()->parameter.no_ipv6 ?
                                        NULL : &self.gtpu_list6,
                                    dev, self.gtpu_port);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);
                        }

                    } while(yaml_iter_type(&gtpu_array) == YAML_SEQUENCE_NODE);

                    if (list_first(&self.gtpu_list) == NULL &&
                        list_first(&self.gtpu_list6) == NULL)
                    {
                        rv = sock_probe_node(
                                context_self()->parameter.no_ipv4 ?
                                    NULL : &self.gtpu_list,
                                context_self()->parameter.no_ipv6 ?
                                    NULL : &self.gtpu_list6,
                                NULL, self.gtpu_port);
                        d_assert(rv == CORE_OK, return CORE_ERROR,);
                    }
                }
                else if (!strcmp(pgw_key, "ue_pool"))
                {
                    yaml_iter_t ue_pool_array, ue_pool_iter;
                    yaml_iter_recurse(&pgw_iter, &ue_pool_array);
                    do
                    {
                        pgw_subnet_t *subnet = NULL;
                        const char *ipstr = NULL;
                        const char *mask_or_numbits = NULL;
                        const char *apn = NULL;
                        const char *dev = self.tun_ifname;

                        if (yaml_iter_type(&ue_pool_array) ==
                                YAML_MAPPING_NODE)
                        {
                            memcpy(&ue_pool_iter, &ue_pool_array,
                                    sizeof(yaml_iter_t));
                        }
                        else if (yaml_iter_type(&ue_pool_array) ==
                            YAML_SEQUENCE_NODE)
                        {
                            if (!yaml_iter_next(&ue_pool_array))
                                break;
                            yaml_iter_recurse(&ue_pool_array,
                                    &ue_pool_iter);
                        }
                        else if (yaml_iter_type(&ue_pool_array) ==
                                YAML_SCALAR_NODE)
                        {
                            break;
                        }
                        else
                            d_assert(0, return CORE_ERROR,);

                        while(yaml_iter_next(&ue_pool_iter))
                        {
                            const char *ue_pool_key =
                                yaml_iter_key(&ue_pool_iter);
                            d_assert(ue_pool_key,
                                    return CORE_ERROR,);
                            if (!strcmp(ue_pool_key, "addr"))
                            {
                                char *v =
                                    (char *)yaml_iter_value(&ue_pool_iter);
                                if (v)
                                {
                                    ipstr = (const char *)strsep(&v, "/");
                                    if (ipstr)
                                    {
                                        mask_or_numbits = (const char *)v;
                                    }
                                }
                            }
                            else if (!strcmp(ue_pool_key, "apn"))
                            {
                                apn = yaml_iter_value(&ue_pool_iter);
                            }
                            else if (!strcmp(ue_pool_key, "dev"))
                            {
                                dev = yaml_iter_value(&ue_pool_iter);
                            }
                            else
                                d_warn("unknown key `%s`", ue_pool_key);
                        }

                        if (ipstr && mask_or_numbits)
                        {
                            subnet = pgw_subnet_add(
                                    ipstr, mask_or_numbits, apn, dev);
                            d_assert(subnet, return CORE_ERROR,);
                        }
                        else
                        {
                            d_warn("Ignore : addr(%s/%s), apn(%s)",
                                    ipstr, mask_or_numbits, apn);
                        }
                    } while(yaml_iter_type(&ue_pool_array) ==
                            YAML_SEQUENCE_NODE);
                }
                else if (!strcmp(pgw_key, "dns"))
                {
                    yaml_iter_t dns_iter;
                    yaml_iter_recurse(&pgw_iter, &dns_iter);
                    d_assert(yaml_iter_type(&dns_iter) !=
                        YAML_MAPPING_NODE, return CORE_ERROR,);

                    do
                    {
                        const char *v = NULL;

                        if (yaml_iter_type(&dns_iter) ==
                                YAML_SEQUENCE_NODE)
                        {
                            if (!yaml_iter_next(&dns_iter))
                                break;
                        }

                        v = yaml_iter_value(&dns_iter);
                        if (v)
                        {
                            ipsubnet_t ipsub;
                            rv = core_ipsubnet(&ipsub, v, NULL);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);

                            if (ipsub.family == AF_INET)
                            {
                                if (self.dns[0] && self.dns[1])
                                    d_warn("Ignore DNS : %s", v);
                                else if (self.dns[0]) self.dns[1] = v;
                                else self.dns[0] = v;
                            }
                            else if (ipsub.family == AF_INET6)
                            {
                                if (self.dns6[0] && self.dns6[1])
                                    d_warn("Ignore DNS : %s", v);
                                else if (self.dns6[0]) self.dns6[1] = v;
                                else self.dns6[0] = v;
                            }
                            else
                                d_warn("Ignore DNS : %s", v);
                        }

                    } while(
                        yaml_iter_type(&dns_iter) ==
                            YAML_SEQUENCE_NODE);
                }
                else if (!strcmp(pgw_key, "p-cscf"))
                {
                    yaml_iter_t dns_iter;
                    yaml_iter_recurse(&pgw_iter, &dns_iter);
                    d_assert(yaml_iter_type(&dns_iter) !=
                        YAML_MAPPING_NODE, return CORE_ERROR,);

                    self.num_of_p_cscf = 0;
                    self.num_of_p_cscf6 = 0;
                    do
                    {
                        const char *v = NULL;

                        if (yaml_iter_type(&dns_iter) ==
                                YAML_SEQUENCE_NODE)
                        {
                            if (!yaml_iter_next(&dns_iter))
                                break;
                        }

                        v = yaml_iter_value(&dns_iter);
                        if (v)
                        {
                            ipsubnet_t ipsub;
                            rv = core_ipsubnet(&ipsub, v, NULL);
                            d_assert(rv == CORE_OK, return CORE_ERROR,);

                            if (ipsub.family == AF_INET)
                            {
                                if (self.num_of_p_cscf >= MAX_NUM_OF_P_CSCF)
                                    d_warn("Ignore P-CSCF : %s", v);
                                else self.p_cscf[self.num_of_p_cscf++] = v;
                            }
                            else if (ipsub.family == AF_INET6)
                            {
                                if (self.num_of_p_cscf6 >= MAX_NUM_OF_P_CSCF)
                                    d_warn("Ignore P-CSCF : %s", v);
                                else self.p_cscf6[self.num_of_p_cscf6++] = v;
                            }
                            else
                                d_warn("Ignore P-CSCF : %s", v);
                        }

                    } while(
                        yaml_iter_type(&dns_iter) ==
                            YAML_SEQUENCE_NODE);
                }
                else
                    d_warn("unknown key `%s`", pgw_key);
            }
        }
    }
    
    spgw_load_static_ip_config();

    rv = spgw_context_validation();
    if (rv != CORE_OK) return rv;

    return CORE_OK;
}

static void spgw_context_init_zmq_paths()
{
    //int instance_id = context_self()->instance_id;
    int instance_id = 1;

    snprintf(self.event_addr, sizeof(self.event_addr), "%s%u", ZMQ_EVENT_AMF_PATH, instance_id);
}


status_t spgw_context_setup_trace_module()
{
    int app = context_self()->logger.trace.app;
    int diameter = context_self()->logger.trace.diameter;
    int gtpv2 = context_self()->logger.trace.gtpv2;
    int gtp = context_self()->logger.trace.gtp;

    gtpv2 = 6;

    if (app)
    {
        extern int _spgw_context;
        d_trace_level(&_spgw_context, app);
    }

    if (diameter)
    {
        extern int _pgw_fd_path;
        d_trace_level(&_pgw_fd_path, diameter);
        extern int _fd_init;
        d_trace_level(&_fd_init, diameter);
        extern int _fd_logger;
        d_trace_level(&_fd_logger, diameter);
    }

    if (gtpv2)
    {
        extern int _spgw_sm;
        d_trace_level(&_spgw_sm, gtpv2);
        extern int _spgw_s5c_build;
        d_trace_level(&_spgw_s5c_build, gtpv2);
        extern int _spgw_s5c_handler;
        d_trace_level(&_spgw_s5c_handler, gtpv2);

        extern int _gtp_node;
        d_trace_level(&_gtp_node, gtpv2);
        extern int _gtp_message;
        d_trace_level(&_gtp_message, gtpv2);
        extern int _gtp_path;
        d_trace_level(&_gtp_path, gtpv2);
        extern int _gtp_xact;
        //d_trace_level(&_gtp_xact, gtpv2);
        d_trace_level(&_gtp_xact, 16);

        extern int _tlv_msg;
        d_trace_level(&_tlv_msg, gtpv2);
    }

    if (gtp)
    {
        extern int _spgw_gtp_path;
        d_trace_level(&_spgw_gtp_path, gtp);
        extern int _pgw_ipfw;
        d_trace_level(&_pgw_ipfw, gtp);
    }

    {
        extern int _spgw_s11_handler;
        d_trace_level(&_spgw_s11_handler, 6);
    }

    return CORE_OK;
}

static void *sess_hash_keygen(c_uint8_t *out, int *out_len,
        c_uint8_t *imsi, int imsi_len, c_int8_t *apn)
{
    memcpy(out, imsi, imsi_len);
    core_cpystrn((char*)(out+imsi_len), apn, MAX_APN_LEN+1);
    *out_len = imsi_len+strlen((char*)(out+imsi_len));

    return out;
}

spgw_sess_t *spgw_sess_add(sgw_ue_t *sgw_ue,
                           c_int8_t *apn, 
                           c_uint8_t pdn_type,
                           c_uint8_t ebi)
{
    char buf1[CORE_ADDRSTRLEN];
    char buf2[CORE_ADDRSTRLEN];
    spgw_sess_t   *sess = NULL;
    spgw_bearer_t *bearer = NULL;
    pgw_subnet_t  *subnet6 = NULL;

    d_assert(sgw_ue, return NULL, "Null param");
    d_assert(ebi, return NULL, "Invalid EBI(%d)", ebi);

    index_alloc(&spgw_sess_pool, &sess);
    d_assert(sess, return NULL, "Null param");

    sess->sgw_s5c_teid = SGW_S5C_INDEX_TO_TEID(sess->index);
    sess->pgw_s5c_teid = sess->index;  /* derived from an index */

    /* Set IMSI */
    sess->imsi_len = sgw_ue->imsi_len;
    memcpy(sess->imsi, sgw_ue->imsi, sess->imsi_len);
    core_buffer_to_bcd(sess->imsi, sess->imsi_len, sess->imsi_bcd);

    /* Set APN */
    core_cpystrn(sess->pdn.apn, apn, MAX_APN_LEN+1);

    sess->sgw_ue = sgw_ue;
    sess->gnode = NULL;

    list_init(&sess->bearer_list);

    bearer = spgw_bearer_add(sess);
    d_assert(bearer, sgw_sess_remove(sess); return NULL, "Can't add default bearer context");
    bearer->ebi = ebi;

    sess->pdn.paa.pdn_type = pdn_type;
    if (pdn_type == GTP_PDN_TYPE_IPV4 || pdn_type == GTP_PDN_TYPE_IPV4V6)
    {
        sess->pdn.paa.pdn_type = GTP_PDN_TYPE_IPV4;
        sess->ipv4 = get_static_ip_by_imsi(sess->imsi, sess->imsi_len);
        if (sess->ipv4 == NULL) {
            sess->ipv4 = pgw_ue_ip_alloc(AF_INET, apn);
        }
        d_assert(sess->ipv4, pgw_sess_remove(sess); return NULL, "Can't allocate IPv4 Pool");
        sess->pdn.paa.addr = sess->ipv4->addr[0];
    }
    else if (pdn_type == GTP_PDN_TYPE_IPV6)
    {
        sess->ipv6 = pgw_ue_ip_alloc(AF_INET6, apn);
        d_assert(sess->ipv6, pgw_sess_remove(sess); return NULL, "Can't allocate IPv6 Pool");

        subnet6 = sess->ipv6->subnet;
        d_assert(subnet6, pgw_sess_remove(sess); return NULL, "No IPv6 subnet");

        sess->pdn.paa.len = subnet6->prefixlen;
        memcpy(sess->pdn.paa.addr6, sess->ipv6->addr, IPV6_LEN);
    }
    else if (pdn_type == GTP_PDN_TYPE_IPV4V6)
    {
        sess->ipv4 = get_static_ip_by_imsi(sess->imsi, sess->imsi_len);
        if (sess->ipv4 == NULL) {
            sess->ipv4 = pgw_ue_ip_alloc(AF_INET, apn);
        }
        d_assert(sess->ipv4, pgw_sess_remove(sess); return NULL, "Can't allocate IPv4 Pool");
        sess->ipv6 = pgw_ue_ip_alloc(AF_INET6, apn);
        d_assert(sess->ipv6, pgw_sess_remove(sess); return NULL, "Can't allocate IPv6 Pool");

        subnet6 = sess->ipv6->subnet;
        d_assert(subnet6, pgw_sess_remove(sess); return NULL, "No IPv6 subnet");

        sess->pdn.paa.both.addr = sess->ipv4->addr[0];
        sess->pdn.paa.both.len = subnet6->prefixlen;
        memcpy(sess->pdn.paa.both.addr6, sess->ipv6->addr, IPV6_LEN);
    }
    else
    {
        d_trace(0, "Unsupported PDN Type(%d)", pdn_type);
    }

    d_trace(1, "UE IPv4:[%s] IPv6:[%s]\n",
            sess->ipv4 ?  INET_NTOP(&sess->ipv4->addr, buf1) : "",
            sess->ipv6 ?  INET6_NTOP(&sess->ipv6->addr, buf2) : "");

    /* Generate Hash Key : IMSI + APN */
    sess_hash_keygen(sess->hash_keybuf, &sess->hash_keylen,
            sgw_ue->imsi, sgw_ue->imsi_len, apn);
    hash_set(self.sess_hash, sess->hash_keybuf, sess->hash_keylen, sess);

    list_append(&sgw_ue->sess_list, sess);

    return sess;
}

status_t pgw_sess_remove(spgw_sess_t *sess)
{
    d_assert(self.sess_hash, return CORE_ERROR, "Null param");
    d_assert(sess, return CORE_ERROR, "Null param");

    hash_set(self.sess_hash, sess->hash_keybuf, sess->hash_keylen, NULL);

    if (sess->ipv4)
        pgw_ue_ip_free(sess->ipv4);
    if (sess->ipv6)
        pgw_ue_ip_free(sess->ipv6);

    pgw_bearer_remove_all(sess);

    index_free(&spgw_sess_pool, sess);

    return CORE_OK;
}

status_t spgw_sess_remove(spgw_sess_t *sess)
{
    d_assert(self.sess_hash, return CORE_ERROR, "Null param");
    d_assert(sess, return CORE_ERROR, "Null param");

    hash_set(self.sess_hash, sess->hash_keybuf, sess->hash_keylen, NULL);

    if (sess->ipv4)
        pgw_ue_ip_free(sess->ipv4);
    if (sess->ipv6)
        pgw_ue_ip_free(sess->ipv6);
    
    list_remove(&sess->sgw_ue->sess_list, sess);

    sgw_bearer_remove_all(sess);

    pgw_bearer_remove_all(sess);

    index_free(&spgw_sess_pool, sess);

    return CORE_OK;
}

status_t pgw_sess_remove_all()
{
    hash_index_t *hi = NULL;
    spgw_sess_t *sess = NULL;

    for (hi = pgw_sess_first(); hi; hi = pgw_sess_next(hi))
    {
        sess = spgw_sess_this(hi);
        pgw_sess_remove(sess);
    }

    return CORE_OK;
}

spgw_sess_t* pgw_sess_find(index_t index)
{
    d_assert(index, return NULL, "Invalid Index");
    return index_find(&spgw_sess_pool, index);
}

spgw_sess_t* pgw_sess_find_by_teid(c_uint32_t teid)
{
    return pgw_sess_find(teid);
}

spgw_sess_t* pgw_sess_find_by_imsi_apn(
    c_uint8_t *imsi, int imsi_len, c_int8_t *apn)
{
    c_uint8_t keybuf[MAX_IMSI_LEN+MAX_APN_LEN+1];
    int keylen = 0;

    d_assert(self.sess_hash, return NULL, "Null param");

    sess_hash_keygen(keybuf, &keylen, imsi, imsi_len, apn);
    return (spgw_sess_t *)hash_get(self.sess_hash, keybuf, keylen);
}

status_t spgw_check_sess_by_imsi_apn_ebi(spgw_sess_t *sess,
                                         c_uint8_t *imsi,
                                         int imsi_len,
                                         c_int8_t *apn)
{
    sgw_ue_t     *sgw_ue;

    d_assert(sess, return CORE_ERROR, "Null param");

    sgw_ue = sess->sgw_ue;
    d_assert(sgw_ue, return CORE_ERROR, "Null param");

    d_assert(sgw_ue->imsi_len == imsi_len, return CORE_ERROR, "imsi not equal!");

    if (memcmp(sgw_ue->imsi, imsi, sgw_ue->imsi_len))
    {
        d_error("imsi not equal!\n");
        return CORE_ERROR;
    }

    if (strcmp(sess->pdn.apn, apn))
    {
        d_error("apn not equal!\n");
        return CORE_ERROR;
    }

    return CORE_OK;
}

gtp_node_t *pgw_sgw_add_by_message(gtp_message_t *message)
{
    status_t rv;
    gtp_node_t *sgw = NULL;
    gtp_f_teid_t *sgw_s5c_teid = NULL;

    gtp_create_session_request_t *req = &message->create_session_request;

    if (req->sender_f_teid_for_control_plane.presence == 0)
    {
        d_error("No Sender F-TEID");
        return NULL;
    }

    sgw_s5c_teid = req->sender_f_teid_for_control_plane.data;
    d_assert(sgw_s5c_teid, return NULL,);
    sgw = gtp_find_node(&spgw_self()->sgw_s5c_list, sgw_s5c_teid);
    if (!sgw)
    {
        sgw = gtp_add_node(&spgw_self()->sgw_s5c_list, sgw_s5c_teid,
            spgw_self()->gtpc_port,
            context_self()->parameter.no_ipv4,
            context_self()->parameter.no_ipv6,
            context_self()->parameter.prefer_ipv4);
        d_assert(sgw, return NULL,);

        rv = gtp_client(sgw);
        d_assert(rv == CORE_OK, return NULL,);
    }

    return sgw;
}

hash_index_t* pgw_sess_first()
{
    d_assert(self.sess_hash, return NULL, "Null param");
    return hash_first(self.sess_hash);
}

hash_index_t* pgw_sess_next(hash_index_t *hi)
{
    return hash_next(hi);
}

spgw_sess_t *spgw_sess_this(hash_index_t *hi)
{
    d_assert(hi, return NULL, "Null param");
    return hash_this_val(hi);
}

spgw_bearer_t* spgw_bearer_add(spgw_sess_t *sess)
{
    spgw_bearer_t *bearer = NULL;
    sgw_tunnel_t *tunnel = NULL;
    sgw_ue_t *sgw_ue = NULL;

    d_assert(sess, return NULL, "Null param");
    sgw_ue = sess->sgw_ue;
    d_assert(sgw_ue, return NULL, "Null param");

    index_alloc(&spgw_bearer_pool, &bearer);
    d_assert(bearer, return NULL, "Bearer context allocation failed");

    bearer->name = NULL;
    bearer->sgw_ue = sgw_ue;
    bearer->sess = sess;
    
    list_init(&bearer->pf_list);
    bearer->pgw_s5u_teid = bearer->index;


    list_init(&bearer->tunnel_list);

    tunnel = sgw_tunnel_add(bearer, GTP_F_TEID_S1_U_SGW_GTP_U);
    d_assert(tunnel, return NULL, "Tunnel context allocation failed");

    tunnel = sgw_tunnel_add(bearer, GTP_F_TEID_S5_S8_SGW_GTP_U);
    d_assert(tunnel, return NULL, "Tunnel context allocation failed");

    list_append(&sess->bearer_list, bearer);

    return bearer;
}

spgw_bearer_t* pgw_bearer_add(spgw_sess_t *sess)
{
    spgw_bearer_t *bearer = NULL;

    d_assert(sess, return NULL, "Null param");

    index_alloc(&spgw_bearer_pool, &bearer);
    d_assert(bearer, return NULL, "Bearer context allocation failed");

    bearer->name = NULL;

    list_init(&bearer->pf_list);

    bearer->pgw_s5u_teid = bearer->index;
    
    bearer->sess = sess;
    bearer->gnode = NULL;

    list_append(&sess->bearer_list, bearer);

    return bearer;
}

status_t pgw_bearer_remove(spgw_bearer_t *bearer)
{
    d_assert(bearer, return CORE_ERROR, "Null param");
    d_assert(bearer->sess, return CORE_ERROR, "Null param");

    list_remove(&bearer->sess->bearer_list, bearer);

    if (bearer->name)
        CORE_FREE(bearer->name);

    pgw_pf_remove_all(bearer);

    index_free(&spgw_bearer_pool, bearer);

    return CORE_OK;
}

status_t pgw_bearer_remove_all(spgw_sess_t *sess)
{
    spgw_bearer_t *bearer = NULL, *next_bearer = NULL;

    d_assert(sess, return CORE_ERROR, "Null param");
    
    bearer = list_first(&sess->bearer_list);
    while (bearer)
    {
        next_bearer = list_next(bearer);

        pgw_bearer_remove(bearer);

        bearer = next_bearer;
    }

    return CORE_OK;
}

spgw_bearer_t* spgw_bearer_find(index_t index)
{
    d_assert(index, return NULL, "Invalid Index");
    return index_find(&spgw_bearer_pool, index);
}

spgw_bearer_t* pgw_bearer_find_by_pgw_s5u_teid(c_uint32_t pgw_s5u_teid)
{
    return spgw_bearer_find(pgw_s5u_teid);
}

spgw_bearer_t* pgw_bearer_find_by_ebi(spgw_sess_t *sess, c_uint8_t ebi)
{
    spgw_bearer_t *bearer = NULL;
    
    d_assert(sess, return NULL, "Null param");

    bearer = spgw_bearer_first(sess);
    while (bearer)
    {
        if (bearer->ebi == ebi)
            break;

        bearer = spgw_bearer_next(bearer);
    }

    return bearer;
}

spgw_bearer_t* pgw_bearer_find_by_name(spgw_sess_t *sess, c_int8_t *name)
{
    spgw_bearer_t *bearer = NULL;
    
    d_assert(sess, return NULL, "Null param");
    d_assert(name, return NULL, "Null param");

    bearer = spgw_bearer_first(sess);
    while (bearer)
    {
        if (bearer->name && strcmp(bearer->name, name) == 0)
            return bearer;

        bearer = spgw_bearer_next(bearer);
    }

    return NULL;
}

spgw_bearer_t* pgw_bearer_find_by_qci_arp(spgw_sess_t *sess, 
                                c_uint8_t qci,
                                c_uint8_t priority_level,
                                c_uint8_t pre_emption_capability,
                                c_uint8_t pre_emption_vulnerability)
{
    spgw_bearer_t *bearer = NULL;

    d_assert(sess, return NULL, "Null param");

    bearer = pgw_default_bearer_in_sess(sess);
    if (!bearer) return NULL;

    if (sess->pdn.qos.qci == qci &&
        sess->pdn.qos.arp.priority_level == priority_level &&
        sess->pdn.qos.arp.pre_emption_capability == 
            pre_emption_capability &&
        sess->pdn.qos.arp.pre_emption_vulnerability == 
            pre_emption_vulnerability)
    {
        return bearer;
    }

    bearer = spgw_bearer_next(bearer);
    while(bearer)
    {
        if (bearer->qos.qci == qci &&
            bearer->qos.arp.priority_level == priority_level &&
            bearer->qos.arp.pre_emption_capability == 
                pre_emption_capability &&
            bearer->qos.arp.pre_emption_vulnerability == 
                pre_emption_vulnerability)
        {
            return bearer;
        }
        bearer = spgw_bearer_next(bearer);
    }

    return NULL;
}

spgw_bearer_t* spgw_bearer_find_by_s1u_teid(c_uint32_t sgw_s1u_teid)
{
    return spgw_bearer_find(SGW_S5C_TEID_TO_INDEX(sgw_s1u_teid));
}

spgw_bearer_t* pgw_default_bearer_in_sess(spgw_sess_t *sess)
{
    return spgw_bearer_first(sess);
}

spgw_bearer_t* spgw_bearer_first(spgw_sess_t *sess)
{
    d_assert(sess, return NULL, "Null param");
    return list_first(&sess->bearer_list);
}

spgw_bearer_t* spgw_bearer_next(spgw_bearer_t *bearer)
{
    return list_next(bearer);
}

pgw_pf_t *pgw_pf_add(spgw_bearer_t *bearer, c_uint32_t precedence)
{
    pgw_pf_t *pf = NULL;

    d_assert(bearer, return NULL, "Null param");

    pool_alloc_node(&pgw_pf_pool, &pf);
    d_assert(pf, return NULL, "Null param");

    pf->identifier = NEXT_ID(bearer->pf_identifier, 1, 15);
    pf->bearer = bearer;

    list_append(&bearer->pf_list, pf);

    return pf;
}

status_t pgw_pf_remove(pgw_pf_t *pf)
{
    d_assert(pf, return CORE_ERROR, "Null param");
    d_assert(pf->bearer, return CORE_ERROR, "Null param");

    list_remove(&pf->bearer->pf_list, pf);
    pool_free_node(&pgw_pf_pool, pf);

    return CORE_OK;
}

status_t pgw_pf_remove_all(spgw_bearer_t *bearer)
{
    pgw_pf_t *pf = NULL, *next_pf = NULL;

    d_assert(bearer, return CORE_ERROR, "Null param");
    
    pf = pgw_pf_first(bearer);
    while (pf)
    {
        next_pf = pgw_pf_next(pf);

        pgw_pf_remove(pf);

        pf = next_pf;
    }

    return CORE_OK;
}

pgw_pf_t* pgw_pf_find_by_id(spgw_bearer_t *bearer, c_uint8_t id)
{
    pgw_pf_t *pf = NULL;
    
    pf = pgw_pf_first(bearer);
    while (pf)
    {
        if (pf->identifier == id)
            return pf;

        pf = pgw_pf_next(pf);
    }

    return CORE_OK;
}

pgw_pf_t* pgw_pf_first(spgw_bearer_t *bearer)
{
    return list_first(&bearer->pf_list);
}

pgw_pf_t* pgw_pf_next(pgw_pf_t *pf)
{
    return list_next(pf);
}

status_t pgw_ue_pool_generate()
{
    int j;
    pgw_subnet_t *subnet = NULL;

    for (subnet = pgw_subnet_first(); subnet; subnet = pgw_subnet_next(subnet))
    {
        int index = 0;
        c_uint32_t mask_count;
        c_uint32_t broadcast[4];

        if (subnet->family == AF_INET)
        {
            if (subnet->prefixlen == 32)
                mask_count = 1;
            else if (subnet->prefixlen < 32)
                mask_count = (0xffffffff >> subnet->prefixlen) + 1;
            else
                d_assert(0, return CORE_ERROR,);
        }
        else if (subnet->family == AF_INET6)
        {
            if (subnet->prefixlen == 128)
                mask_count = 1;
            else if (subnet->prefixlen > 96 && subnet->prefixlen < 128)
                mask_count = (0xffffffff >> (subnet->prefixlen - 96)) + 1;
            else if (subnet->prefixlen <= 96)
                mask_count = 0xffffffff;
            else
                d_assert(0, return CORE_ERROR,);
        }
        else
            d_assert(0, return CORE_ERROR,);
        
        for (j = 0; j < 4; j++)
        {
            broadcast[j] = subnet->sub.sub[j] + ~subnet->sub.mask[j];
        }

        for (j = 0; j < mask_count && index < MAX_POOL_OF_UE; j++)
        {
            pgw_ue_ip_t *ue_ip = NULL;
            int maxbytes = 0;
            int lastindex = 0;

            ue_ip = &subnet->pool.pool[index];
            d_assert(ue_ip, return CORE_ERROR,);
            memset(ue_ip, 0, sizeof *ue_ip);

            if (subnet->family == AF_INET)
            {
                maxbytes = 4;
                lastindex = 0;
            }
            else if (subnet->family == AF_INET6)
            {
                maxbytes = 16;
                lastindex = 3;
            }

            memcpy(ue_ip->addr, subnet->sub.sub, maxbytes);
            ue_ip->addr[lastindex] += htonl(j);
            ue_ip->subnet = subnet;
            ue_ip->is_static = 0;

            /* Exclude Network Address */
            if (memcmp(ue_ip->addr, subnet->sub.sub, maxbytes) == 0) continue;

            /* Exclude Broadcast Address */
            if (memcmp(ue_ip->addr, broadcast, maxbytes) == 0) continue;

            /* Exclude TUN IP Address */
            if (memcmp(ue_ip->addr, subnet->gw.sub, maxbytes) == 0) continue;

            index++;
        }
        subnet->pool.size = subnet->pool.avail = index;
    }

    return CORE_OK;
}

static pgw_subnet_t *find_subnet(int family, const char *apn)
{
    pgw_subnet_t *subnet = NULL;

    d_assert(apn, return NULL,);
    d_assert(family == AF_INET || family == AF_INET6, return NULL,);

    for (subnet = pgw_subnet_first(); subnet; subnet = pgw_subnet_next(subnet))
    {
        if (strlen(subnet->apn))
        {
            if (subnet->family == family && strcmp(subnet->apn, apn) == 0 &&
                pool_avail(&subnet->pool))
            {
                return subnet;
            }
        }
    }

    for (subnet = pgw_subnet_first(); subnet; subnet = pgw_subnet_next(subnet))
    {
        if (strlen(subnet->apn) == 0)
        {
            if (subnet->family == family &&
                pool_avail(&subnet->pool))
            {
                return subnet;
            }
        }
    }

    if (subnet == NULL)
        d_error("CHECK CONFIGURATION: Cannot find UE Pool");

    return subnet;
}

pgw_ue_ip_t *pgw_ue_ip_alloc(int family, const char *apn)
{
    pgw_subnet_t *subnet = NULL;
    pgw_ue_ip_t *ue_ip = NULL;

    d_assert(apn, return NULL,);

    subnet = find_subnet(family, apn);
    d_assert(subnet, return NULL,);

    pool_alloc_node(&subnet->pool, &ue_ip);
    d_assert(ue_ip, return NULL,);

    return ue_ip;
}

status_t pgw_ue_ip_free(pgw_ue_ip_t *ue_ip)
{
    pgw_subnet_t *subnet = NULL;

    d_assert(ue_ip, return CORE_ERROR,);
    if (ue_ip->is_static == 0) {
        subnet = ue_ip->subnet;

        d_assert(subnet, return CORE_ERROR,);
        pool_free_node(&subnet->pool, ue_ip);
    }

    return CORE_OK;
}

pgw_dev_t *pgw_dev_add(const char *ifname)
{
    pgw_dev_t *dev = NULL;

    d_assert(ifname, return NULL,);

    pool_alloc_node(&pgw_dev_pool, &dev);
    d_assert(dev, return NULL,);
    memset(dev, 0, sizeof *dev);

    strcpy(dev->ifname, ifname);

    list_append(&self.dev_list, dev);

    return dev;
}

status_t pgw_dev_remove(pgw_dev_t *dev)
{
    d_assert(dev, return CORE_ERROR, "Null param");

    list_remove(&self.dev_list, dev);

    if (dev->link_local_addr)
        core_freeaddrinfo(dev->link_local_addr);

    pool_free_node(&pgw_dev_pool, dev);

    return CORE_OK;
}

status_t pgw_dev_remove_all()
{
    pgw_dev_t *dev = NULL, *next_dev = NULL;

    dev = pgw_dev_first();
    while (dev)
    {
        next_dev = pgw_dev_next(dev);

        pgw_dev_remove(dev);

        dev = next_dev;
    }

    return CORE_OK;
}

pgw_dev_t* pgw_dev_find_by_ifname(const char *ifname)
{
    pgw_dev_t *dev = NULL;

    d_assert(ifname, return NULL,);
    
    dev = pgw_dev_first();
    while (dev)
    {
        if (strcmp(dev->ifname, ifname) == 0)
            return dev;

        dev = pgw_dev_next(dev);
    }

    return CORE_OK;
}

pgw_dev_t* pgw_dev_first()
{
    return list_first(&self.dev_list);
}

pgw_dev_t* pgw_dev_next(pgw_dev_t *dev)
{
    return list_next(dev);
}

pgw_subnet_t *pgw_subnet_add(
        const char *ipstr, const char *mask_or_numbits,
        const char *apn, const char *ifname)
{
    status_t rv;
    pgw_dev_t *dev = NULL;
    pgw_subnet_t *subnet = NULL;

    d_assert(ipstr, return NULL,);
    d_assert(mask_or_numbits, return NULL,);
    d_assert(ifname, return NULL,);

    dev = pgw_dev_find_by_ifname(ifname);
    if (!dev)
        dev = pgw_dev_add(ifname);
    d_assert(dev, return NULL,);

    pool_alloc_node(&pgw_subnet_pool, &subnet);
    d_assert(subnet, return NULL,);
    memset(subnet, 0, sizeof *subnet);

    subnet->dev = dev;

    rv = core_ipsubnet(&subnet->gw, ipstr, NULL);
    d_assert(rv == CORE_OK, return NULL,);

    rv = core_ipsubnet(&subnet->sub, ipstr, mask_or_numbits);
    d_assert(rv == CORE_OK, return NULL,);

    if (apn)
        strcpy(subnet->apn, apn);

    subnet->family = subnet->gw.family;
    subnet->prefixlen = atoi(mask_or_numbits);

    pool_init(&subnet->pool, MAX_POOL_OF_UE);


    list_append(&self.subnet_list, subnet);

    if (strcmp(ifname, "pgwtun") == 0) {
        default_subnet = subnet;
    }

    return subnet;
}

status_t pgw_subnet_remove(pgw_subnet_t *subnet)
{
    d_assert(subnet, return CORE_ERROR, "Null param");

    list_remove(&self.subnet_list, subnet);

    if (pool_used(&subnet->pool))
    {
        d_warn("%d not freed in ue_ip_pool[%d] in PGW-Context",
                pool_used(&subnet->pool), pool_size(&subnet->pool));
    }
    d_trace(9, "%d not freed in ue_ip_pool[%d] in PGW-Context\n",
            pool_used(&subnet->pool), pool_size(&subnet->pool));
    pool_final(&subnet->pool);

    pool_free_node(&pgw_subnet_pool, subnet);

    return CORE_OK;
}

status_t pgw_subnet_remove_all()
{
    pgw_subnet_t *subnet = NULL, *next_subnet = NULL;

    subnet = pgw_subnet_first();
    while (subnet)
    {
        next_subnet = pgw_subnet_next(subnet);

        pgw_subnet_remove(subnet);

        subnet = next_subnet;
    }

    return CORE_OK;
}

pgw_subnet_t* pgw_subnet_first()
{
    return list_first(&self.subnet_list);
}

pgw_subnet_t* pgw_subnet_next(pgw_subnet_t *subnet)
{
    return list_next(subnet);
}

/********************************************************
 * SGW Start
 * ******************************************************/
gtp_node_t *spgw_mme_add_by_message(gtp_message_t *message)
{
    status_t rv;
    gtp_node_t *mme = NULL;
    gtp_f_teid_t *mme_s11_teid = NULL;
    gtp_create_session_request_t *req = &message->create_session_request;

    if (req->sender_f_teid_for_control_plane.presence == 0)
    {
        d_error("No Sender F-TEID");
        return NULL;
    }

    mme_s11_teid = req->sender_f_teid_for_control_plane.data;
    d_assert(mme_s11_teid, return NULL,);
    mme = gtp_find_node(&spgw_self()->mme_s11_list, mme_s11_teid);
    if (!mme)
    {
        mme = gtp_add_node(&spgw_self()->mme_s11_list, mme_s11_teid,
            spgw_self()->gtpc_port,
            context_self()->parameter.no_ipv4,
            context_self()->parameter.no_ipv6,
            context_self()->parameter.prefer_ipv4);
        d_assert(mme, return NULL,);

        rv = gtp_client(mme);
        d_assert(rv == CORE_OK, return NULL,);
    }

    return mme;
}

sgw_ue_t *spgw_ue_add_by_message(gtp_message_t *message)
{
    sgw_ue_t *sgw_ue = NULL;
    gtp_create_session_request_t *req = &message->create_session_request;

    if (req->imsi.presence == 0)
    {
        d_error("No IMSI");
        return NULL;
    }

    d_trace(9, "sgw_ue_add_by_message() - IMSI ");
    d_trace_hex(9, req->imsi.data, req->imsi.len);

    sgw_ue = sgw_ue_find_by_imsi(req->imsi.data, req->imsi.len);
    if (!sgw_ue)
    {
        sgw_ue = sgw_ue_add(req->imsi.data, req->imsi.len);
        d_assert(sgw_ue, return NULL,);
    }

    return sgw_ue;
}

sgw_ue_t *sgw_ue_add(c_uint8_t *imsi, int imsi_len)
{
    sgw_ue_t *sgw_ue = NULL;

    d_assert(imsi, return NULL, "Null param");
    d_assert(imsi_len, return NULL, "Null param");

    index_alloc(&sgw_ue_pool, &sgw_ue);
    d_assert(sgw_ue, return NULL, "Null param");

    sgw_ue->sgw_s11_teid = sgw_ue->index;

    /* Set IMSI */
    sgw_ue->imsi_len = imsi_len;
    memcpy(sgw_ue->imsi, imsi, sgw_ue->imsi_len);
    core_buffer_to_bcd(sgw_ue->imsi, sgw_ue->imsi_len, sgw_ue->imsi_bcd);

    list_init(&sgw_ue->sess_list);

    hash_set(self.imsi_ue_hash, sgw_ue->imsi, sgw_ue->imsi_len, sgw_ue);

    return sgw_ue;
}

static sgw_ue_t* sgw_ue_find(index_t index)
{
    d_assert(index, return NULL, "Invalid index = 0x%x", index);
    return index_find(&sgw_ue_pool, index);
}

sgw_ue_t* sgw_ue_find_by_imsi(c_uint8_t *imsi, int imsi_len)
{
    d_assert(imsi && imsi_len, return NULL,"Invalid param");

    return (sgw_ue_t *)hash_get(self.imsi_ue_hash, imsi, imsi_len);
}

sgw_ue_t* sgw_ue_find_by_teid(c_uint32_t teid)
{
    return sgw_ue_find(teid);
}

spgw_sess_t *sgw_sess_add(
        sgw_ue_t *sgw_ue, c_int8_t *apn, c_uint8_t ebi)
{
    spgw_sess_t *sess = NULL;
    spgw_bearer_t *bearer = NULL;

    d_assert(sgw_ue, return NULL, "Null param");
    d_assert(ebi, return NULL, "Invalid EBI(%d)", ebi);

    index_alloc(&spgw_sess_pool, &sess);
    d_assert(sess, return NULL, "Null param");

    sess->sgw_s5c_teid = SGW_S5C_INDEX_TO_TEID(sess->index);

    /* Set APN */
    core_cpystrn(sess->pdn.apn, apn, MAX_APN_LEN+1);

    sess->sgw_ue = sgw_ue;
    sess->gnode = NULL;

    list_init(&sess->bearer_list);

    bearer = sgw_bearer_add(sess);
    d_assert(bearer, sgw_sess_remove(sess); return NULL, 
            "Can't add default bearer context");
    bearer->ebi = ebi;

    list_append(&sgw_ue->sess_list, sess);

    return sess;
}

status_t sgw_sess_remove(spgw_sess_t *sess)
{
    d_assert(sess, return CORE_ERROR, "Null param");
    d_assert(sess->sgw_ue, return CORE_ERROR, "Null param");

    list_remove(&sess->sgw_ue->sess_list, sess);

    sgw_bearer_remove_all(sess);

    index_free(&spgw_sess_pool, sess);

    return CORE_OK;
}

status_t sgw_sess_remove_all(sgw_ue_t *sgw_ue)
{
    spgw_sess_t *sess = NULL, *next_sess = NULL;
    
    sess = sgw_sess_first(sgw_ue);
    while (sess)
    {
        next_sess = sgw_sess_next(sess);

        sgw_sess_remove(sess);

        sess = next_sess;
    }

    return CORE_OK;
}

static spgw_sess_t* sgw_sess_find(index_t index)
{
    d_assert(index, return NULL, "Invalid Index");
    return index_find(&spgw_sess_pool, index);
}

spgw_sess_t* sgw_sess_find_by_teid(c_uint32_t teid)
{
    return sgw_sess_find(SGW_S5C_TEID_TO_INDEX(teid));
}

spgw_sess_t* sgw_sess_find_by_apn(sgw_ue_t *sgw_ue, c_int8_t *apn)
{
    spgw_sess_t *sess = NULL;
    
    sess = sgw_sess_first(sgw_ue);
    while (sess)
    {
        if (strcmp(sess->pdn.apn, apn) == 0)
            return sess;

        sess = sgw_sess_next(sess);
    }

    return NULL;
}

spgw_sess_t* sgw_sess_find_by_ebi(sgw_ue_t *sgw_ue, c_uint8_t ebi)
{
    spgw_bearer_t *bearer = NULL;

    bearer = sgw_bearer_find_by_ue_ebi(sgw_ue, ebi);
    if (bearer)
        return bearer->sess;

    return NULL;
}

spgw_sess_t* sgw_sess_first(sgw_ue_t *sgw_ue)
{
    return list_first(&sgw_ue->sess_list);
}

spgw_sess_t* sgw_sess_next(spgw_sess_t *sess)
{
    return list_next(sess);
}

spgw_bearer_t* sgw_bearer_add(spgw_sess_t *sess)
{
    spgw_bearer_t *bearer = NULL;
    sgw_tunnel_t *tunnel = NULL;
    sgw_ue_t *sgw_ue = NULL;

    d_assert(sess, return NULL, "Null param");
    sgw_ue = sess->sgw_ue;
    d_assert(sgw_ue, return NULL, "Null param");

    index_alloc(&spgw_bearer_pool, &bearer);
    d_assert(bearer, return NULL, "Bearer context allocation failed");

    bearer->sgw_ue = sgw_ue;
    bearer->sess = sess;

    list_init(&bearer->tunnel_list);

    tunnel = sgw_tunnel_add(bearer, GTP_F_TEID_S1_U_SGW_GTP_U);
    d_assert(tunnel, return NULL, "Tunnel context allocation failed");

    tunnel = sgw_tunnel_add(bearer, GTP_F_TEID_S5_S8_SGW_GTP_U);
    d_assert(tunnel, return NULL, "Tunnel context allocation failed");

    list_append(&sess->bearer_list, bearer);

    return bearer;
}

status_t sgw_bearer_remove(spgw_bearer_t *bearer)
{
    int i;

    d_assert(bearer, return CORE_ERROR, "Null param");
    d_assert(bearer->sess, return CORE_ERROR, "Null param");

    list_remove(&bearer->sess->bearer_list, bearer);

    sgw_tunnel_remove_all(bearer);

    /* Free the buffered packets */
    for (i = 0; i < bearer->num_buffered_pkt; i++)
        pkbuf_free(bearer->buffered_pkts[i]);

    index_free(&spgw_bearer_pool, bearer);

    return CORE_OK;
}

status_t sgw_bearer_remove_all(spgw_sess_t *sess)
{
    spgw_bearer_t *bearer = NULL, *next_bearer = NULL;

    d_assert(sess, return CORE_ERROR, "Null param");
    
    bearer = list_first(&sess->bearer_list);
    while (bearer)
    {
        next_bearer = list_next(bearer);

        sgw_bearer_remove(bearer);

        bearer = next_bearer;
    }

    return CORE_OK;
}

spgw_bearer_t* sgw_bearer_find_by_sess_ebi(spgw_sess_t *sess, c_uint8_t ebi)
{
    spgw_bearer_t *bearer = NULL;

    bearer = spgw_bearer_first(sess);
    while(bearer)
    {
        if (ebi == bearer->ebi)
            return bearer;

        bearer = spgw_bearer_next(bearer);
    }

    return NULL;
}

spgw_bearer_t* sgw_bearer_find_by_ue_ebi(sgw_ue_t *sgw_ue, c_uint8_t ebi)
{
    spgw_sess_t *sess = NULL;
    spgw_bearer_t *bearer = NULL;
    
    sess = sgw_sess_first(sgw_ue);
    while (sess)
    {
        bearer = sgw_bearer_find_by_sess_ebi(sess, ebi);
        if (bearer)
        {
            return bearer;
        }

        sess = sgw_sess_next(sess);
    }

    return NULL;
}

spgw_bearer_t* sgw_default_bearer_in_sess(spgw_sess_t *sess)
{
    return spgw_bearer_first(sess);
}

sgw_tunnel_t* sgw_tunnel_add(spgw_bearer_t *bearer, c_uint8_t interface_type)
{
    sgw_tunnel_t *tunnel = NULL;

    d_assert(bearer, return NULL, "Null param");

    index_alloc(&sgw_tunnel_pool, &tunnel);
    d_assert(tunnel, return NULL, "Tunnel context allocation failed");

    tunnel->interface_type = interface_type;
    tunnel->local_teid = tunnel->index;

    tunnel->bearer = bearer;
    tunnel->gnode = NULL;

    list_append(&bearer->tunnel_list, tunnel);

    return tunnel;
}

status_t sgw_tunnel_remove(sgw_tunnel_t *tunnel)
{
    d_assert(tunnel, return CORE_ERROR, "Null param");
    d_assert(tunnel->bearer, return CORE_ERROR, "Null param");

    list_remove(&tunnel->bearer->tunnel_list, tunnel);
    index_free(&sgw_tunnel_pool, tunnel);

    return CORE_OK;
}

status_t sgw_tunnel_remove_all(spgw_bearer_t *bearer)
{
    sgw_tunnel_t *tunnel = NULL, *next_tunnel = NULL;

    d_assert(bearer, return CORE_ERROR, "Null param");
    
    tunnel = sgw_tunnel_first(bearer);
    while (tunnel)
    {
        next_tunnel = sgw_tunnel_next(tunnel);

        sgw_tunnel_remove(tunnel);

        tunnel = next_tunnel;
    }

    return CORE_OK;
}

sgw_tunnel_t* sgw_tunnel_find(index_t index)
{
    d_assert(index && index < MAX_POOL_OF_TUNNEL, return NULL, 
            "Invalid Index(%d)",index);

    return index_find(&sgw_tunnel_pool, index);
}

sgw_tunnel_t* sgw_tunnel_find_by_teid(c_uint32_t teid)
{
    return sgw_tunnel_find(teid);
}

sgw_tunnel_t* sgw_tunnel_find_by_interface_type(
        spgw_bearer_t *bearer, c_uint8_t interface_type)
{
    sgw_tunnel_t *tunnel = NULL;

    d_assert(bearer, return NULL,);

    tunnel = sgw_tunnel_first(bearer);
    while(tunnel)
    {
        if (tunnel->interface_type == interface_type)
        {
            return tunnel;
        }

        tunnel = sgw_tunnel_next(tunnel);
    }

    return NULL;
}

sgw_tunnel_t* sgw_s1u_tunnel_in_bearer(spgw_bearer_t *bearer)
{
    return sgw_tunnel_find_by_interface_type(
            bearer, GTP_F_TEID_S1_U_SGW_GTP_U);
}
sgw_tunnel_t* sgw_s5u_tunnel_in_bearer(spgw_bearer_t *bearer)
{
    return sgw_tunnel_find_by_interface_type(
            bearer, GTP_F_TEID_S5_S8_SGW_GTP_U);
}

sgw_tunnel_t* sgw_tunnel_first(spgw_bearer_t *bearer)
{
    d_assert(bearer, return NULL, "Null param");
    return list_first(&bearer->tunnel_list);
}

sgw_tunnel_t* sgw_tunnel_next(sgw_tunnel_t *tunnel)
{
    return list_next(tunnel);
}
/********************************************************
 * SGW End
 * ******************************************************/

/********************************************************
 * static ip start
 * TODO: we need a more efficient query algorithm
 * ******************************************************/
#define SPGW_STATIC_IP_CONFIG_FILE "/epc/cfg/static_ip.conf"
static void dump_all_static_ip()
{
    int i=0;
    spgw_static_ip_t *spgw_static_ip = NULL;
    
    spgw_static_ip = list_first(&self.static_ip_list);
    printf("static ip pool:\n");
    while (spgw_static_ip) {
        printf("\t");
        for(i=0; i<8; i++) {
            printf("%02x", spgw_static_ip->imsi[i]);
        }
        printf(" 0x%x\n", spgw_static_ip->ue_ip.addr[0]);
        spgw_static_ip = list_next(spgw_static_ip);
    }
}

spgw_static_ip_t *static_ip_add(c_uint8_t *imsi, int imsi_len, c_uint32_t ipv4)
{
    spgw_static_ip_t *spgw_static_ip = NULL;

    d_assert(imsi, return NULL, "Null param");
    d_assert(imsi_len, return NULL, "Null param");

    spgw_static_ip = (spgw_static_ip_t*) malloc(sizeof(spgw_static_ip_t));
    d_assert(spgw_static_ip, return NULL, "malloc spgw_static_ip_t failed.");

    spgw_static_ip->imsi_len = imsi_len;
    memcpy(spgw_static_ip->imsi, imsi, imsi_len);
    spgw_static_ip->ue_ip.addr[0] = ipv4;
    spgw_static_ip->ue_ip.is_static = 1;
    spgw_static_ip->ue_ip.subnet = default_subnet;
    list_append(&self.static_ip_list, spgw_static_ip);

    return spgw_static_ip;
}

pgw_ue_ip_t *get_static_ip_by_imsi(c_uint8_t *imsi, int imsi_len)
{
    spgw_static_ip_t *spgw_static_ip = NULL;
    
    spgw_static_ip = list_first(&self.static_ip_list);
    while (spgw_static_ip) {
        if (memcmp(spgw_static_ip->imsi, imsi, imsi_len) == 0) {
            printf("got ue_ip by imsi");
            return &spgw_static_ip->ue_ip;
        }
        spgw_static_ip = list_next(spgw_static_ip);
    }

    return NULL;
}


void remove_all_static_ip()
{
    spgw_static_ip_t *spgw_static_ip = NULL;

    spgw_static_ip = list_first(&self.static_ip_list);
    while (spgw_static_ip) {
        list_remove(&self.static_ip_list, spgw_static_ip);
        spgw_static_ip = list_first(&self.static_ip_list);
    }
}

static int parse_imsi_string(const char *imsi_str, unsigned char *imsi)
{
    int len=0;
    int l=0,h=0;
    int i=0;

    d_assert(imsi_str, return -1, "Null param");

    len = strlen(imsi_str);
    d_assert(len == 15, return -1, "invalid imsi %s", imsi_str);

    for (i=0; i<7; i++) {
        l = *(imsi_str+2*i) - 0x30;
        h = *(imsi_str+2*i+1) - 0x30;
        if (l<0 || l >9 || h<0 || h>9) {
            d_error("invalid imsi %s", imsi_str);
            return -1;
        }

        *(imsi+i) = (h << 4) + l;
    }

    l = *(imsi_str+2*i) - 0x30;
    if (l<0 || l >9) {
        d_error("invalid imsi %s", imsi_str);
        return -1;
    }
    *(imsi+i) = 0xF0 + l;

    return 8;
}

void spgw_load_static_ip_config()
{
    FILE *fp;
    char buf[256];
    char imsi_str[256], ip[256];
    unsigned char imsi[MAX_IMSI_LEN];
    c_uint32_t ipv4;
    int imsi_len=0;
    int count=0;

    fp = fopen(SPGW_STATIC_IP_CONFIG_FILE, "r");
    if (fp == NULL) {
        d_trace(1, "can not find SPGW_STATIC_IP_CONFIG_FILE");
        return;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (sscanf(buf, "%s %s\n", imsi_str, ip) != 2) {
            continue;
        }

        ipv4 = (c_uint32_t)inet_addr(ip);
        if (ipv4 == -1) {
            d_error("invalid ip addr: %s", ip);
            continue;
        }

        imsi_len = parse_imsi_string(imsi_str, imsi);
        if (imsi_len != 8) {
            d_error("invalid imsi: %s", imsi_str);
            continue;
        }

        if (static_ip_add((c_uint8_t*)imsi, imsi_len, ipv4) == NULL) {
            d_error("add static ip %s %d failed", imsi, ip);
        }
        count++;
    }
    d_trace(0, "got %d static ip entry.\n", count);
    dump_all_static_ip();
    
    fclose(fp);
}

spgw_ue_flow_stats_ctx_t* spgw_ue_flow_stats_memory_init()
{
    char *addr=NULL;
	int fd;

	fd = open(SPGW_UE_FLOW_STATS_FILE, O_RDWR|O_CREAT, S_IROTH);
	if (fd == -1) {
        d_error("open %s failed! %s", SPGW_UE_FLOW_STATS_FILE, strerror(errno));
        return NULL;
    }

    if (ftruncate(fd, sizeof(spgw_ue_flow_stats_ctx_t)) == -1) {
        d_error("ftruncate %s failed! %s", SPGW_UE_FLOW_STATS_FILE, strerror(errno));
        close(fd);
        return NULL;
    }


	addr = mmap(NULL, sizeof(spgw_ue_flow_stats_ctx_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
        d_error("mmap %s failed! %s", SPGW_UE_FLOW_STATS_FILE, strerror(errno));
        close(fd);
        return NULL; 
    }

	close(fd);

    memset(addr, 0, sizeof(spgw_ue_flow_stats_ctx_t));

    return ((spgw_ue_flow_stats_ctx_t *)addr);
}

void spgw_ue_flow_stat_update(c_uint32_t ipv4, c_uint32_t uplink_bytes, c_uint32_t downlink_bytes)
{
    spgw_ue_flow_stats_t *ue_flow_stat = NULL;
    c_uint32_t index = ntohl(ipv4) & 0xffff;

    if (index >= 65536) {
        d_error("%s ipv4:%x is wrong!", __FUNCTION__, ipv4);
        return;
    }

    ue_flow_stat = &self.ue_flow_stats_ctx->ue_flow_stats[index];
    ue_flow_stat->ipv4 = ipv4;
    ue_flow_stat->uplink_bytes += htonl(uplink_bytes);
    ue_flow_stat->downlink_bytes += htonl(downlink_bytes);

    return;
}
