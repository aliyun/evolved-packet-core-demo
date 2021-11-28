#define TRACE_MODULE _ext_context

#include "core_debug.h"
#include "core_pool.h"
#include "core_index.h"
#include "core_lib.h"
#include "core_network.h"
#include "core_epc_msg.h"

#include <mongoc.h>
#include <yaml.h>
#include "common/yaml_helper.h"

#include "fd/fd_lib.h"

#include "common/context.h"
#include "ext_context.h"
#include "ext_data_syn.h"

static ext_context_t self;

static int context_initiaized = 0;

status_t ext_context_init()
{
    d_assert(context_initiaized == 0, return CORE_ERROR,
            "EXT ctx already has been initialized");

    /* Initialize EXT context */
    memset(&self, 0, sizeof(ext_context_t));

    self.syn_port = EXT_DATA_SYN_PORT;

    context_initiaized = 1;

    return CORE_OK;
}

status_t ext_context_final()
{
    d_assert(context_initiaized == 1, return CORE_ERROR,
            "EXT ctx already has been finalized");

    context_initiaized = 0;
    
    return CORE_OK;
}

ext_context_t* ext_self()
{
    return &self;
}

static status_t ext_context_prepare()
{
    return CORE_OK;
}

static status_t ext_context_validation()
{
    return CORE_OK;
}

status_t ext_context_parse_config()
{
    status_t rv;
    config_t *config = &context_self()->config;
    yaml_document_t *document = NULL;
    yaml_iter_t root_iter;

    d_assert(config, return CORE_ERROR,);
    document = config->document;
    d_assert(document, return CORE_ERROR,);

    rv = ext_context_prepare();
    if (rv != CORE_OK) return rv;

    yaml_iter_init(&root_iter, document);
    while(yaml_iter_next(&root_iter)) {
        const char *root_key = yaml_iter_key(&root_iter);
        d_assert(root_key, return CORE_ERROR,);

        if (!strcmp(root_key, "ext")) {
            yaml_iter_t ext_iter;
            yaml_iter_recurse(&root_iter, &ext_iter);

            while(yaml_iter_next(&ext_iter)) {
                const char *ext_key = yaml_iter_key(&ext_iter);
                d_assert(ext_key, return CORE_ERROR,);

                if (!strcmp(ext_key, "syn_peer")) {
                    yaml_iter_t syn_peer_array, syn_peer_iter;
                    yaml_iter_recurse(&ext_iter, &syn_peer_array);

                    do {
                        if (yaml_iter_type(&syn_peer_array) == YAML_MAPPING_NODE) {
                            memcpy(&syn_peer_iter, &syn_peer_array, sizeof(yaml_iter_t));
                        } else if (yaml_iter_type(&syn_peer_array) == YAML_SEQUENCE_NODE) {
                            if (!yaml_iter_next(&syn_peer_array))
                                break;
                            yaml_iter_recurse(&syn_peer_array, &syn_peer_iter);
                        } else if (yaml_iter_type(&syn_peer_array) == YAML_SCALAR_NODE) {
                            break;
                        } else {
                            d_assert(0, return CORE_ERROR,);
                        }

                        while(yaml_iter_next(&syn_peer_iter)) {
                            const char *syn_peer_key = yaml_iter_key(&syn_peer_iter);
                            d_assert(syn_peer_key, return CORE_ERROR,);

                            if (!strcmp(syn_peer_key, "addr")) {
                                const char *v = yaml_iter_value(&syn_peer_iter);

                                self.syn_peer_ip = v;
                            } else if (!strcmp(syn_peer_key, "port")) {
                                const char *v = yaml_iter_value(&syn_peer_iter);

                                if (v) {
                                    self.syn_port = atoi(v);
                                }
                            } else {
                                d_warn("unknown key `%s`", syn_peer_key);
                            }
                        }
                    } while(yaml_iter_type(&syn_peer_array) == YAML_SEQUENCE_NODE);
                } else {
                    d_warn("unknown key `%s`", ext_key);
                }
            }
        }
    }

    rv = ext_context_validation();
    if (rv != CORE_OK) return rv;

    printf("---ext: %s %d\r\n", self.syn_peer_ip, self.syn_port);

    return CORE_OK;
}

status_t ext_context_setup_trace_module()
{
    extern int _ext_msg;
    d_trace_level(&_ext_msg, 1);

    return CORE_OK;
}

status_t ext_fds_init()
{
    status_t rv;

    if ((self.internal_fd = epc_msg_server_init(EPC_SK_T_EXT)) < 0) {
        d_error("[EXT] init internal srv fd failed.\n");
        return CORE_ERROR;
    }
    
    rv = ext_data_syn_init_server();
    d_assert(rv==CORE_OK, return rv, "[EXT] init data syn srv fd failed.");

    return CORE_OK;
}

void ext_fds_final()
{
    if (self.internal_fd > 0) {
        close(self.internal_fd);
    }

    if (self.syn_fd > 0) {
        close(self.syn_fd);
    }
}

c_int32_t ext_fill_fs_rset(fd_set *rset)
{
    c_int32_t max_fd = -1;

    FD_ZERO(rset);

    if (self.internal_fd > 0) {
        FD_SET(self.internal_fd, rset);
        max_fd = self.internal_fd;
    }

    if (self.syn_fd > 0) {
        FD_SET(self.syn_fd, rset);

        if (max_fd < self.syn_fd) {
            max_fd = self.syn_fd;
        }
    }

    if (max_fd != -1) {
        max_fd += 1;
    }

    return max_fd;
}
