#define TRACE_MODULE _zmq_path

#include "core_debug.h"
#include "core_zmq.h"


static zmq_pollitem_t       pullers[MAX_PULLER_NUM];
static zmq_puller_handler   puller_handler[MAX_PULLER_NUM];
static c_uint32_t           num_of_puller = 0;

status_t zmq_context_init(void **context)
{
    void *ctx = zmq_ctx_new();

    if (ctx) {
        *context = ctx;
        return CORE_OK;
    } else {
        d_error("zmq_ctx_new() failed. (%d:%s)", errno, strerror(errno));
        return CORE_ERROR;
    }
}

void zmq_context_final(void *context)
{
    zmq_ctx_shutdown(context);
}

void *zmq_create_pusher(void *context, char *path)
{
    int rv = 0;
    void *pusher = zmq_socket(context, ZMQ_PUSH);

    d_assert(pusher, return NULL, "zmq_socket failed. (%d:%s)", errno, strerror(errno));

    rv = zmq_connect(pusher, path);
    d_assert(rv == 0, return NULL, "zmq_connect failed. (%d:%s)", errno, strerror(errno));

    return pusher;
}

void zmq_close_pusher_puller(void *pusher_puller)
{
    if (pusher_puller) {
        zmq_close(pusher_puller);
    }
}

status_t zmq_send_ngap(void *pusher, char *buf, int len, zmq_ngap_hdr *ngap_hdr)
{
    zmq_msg_t msg;
    int rv;
    int msg_len = len + sizeof(zmq_ngap_hdr);

    rv = zmq_msg_init_size(&msg, msg_len);
    d_assert (rv == 0, return CORE_ERROR, "zmq init msg failed!");

    memcpy(zmq_msg_data(&msg), buf, len);
    memcpy(zmq_msg_data(&msg)+len, ngap_hdr, sizeof(zmq_ngap_hdr));

    rv = zmq_msg_send(&msg, pusher, ZMQ_DONTWAIT);
    if (rv == msg_len)
    {
        zmq_msg_close(&msg);
        return CORE_OK;
    }

    if (rv == -1 && errno == EAGAIN)
    {
        d_info("zmq msg send in blocking mode");
        rv = zmq_msg_send(&msg, pusher, 0);
        if (rv == msg_len)
        {
            zmq_msg_close(&msg);
            return CORE_OK;
        }
    }

    d_error("zmq send msg failed!");
    zmq_msg_close(&msg);
    return CORE_ERROR;
}

status_t zmq_pusher_send_buf(void *pusher, char *buf, int len)
{
    int rv;
    zmq_msg_t msg;

    rv = zmq_msg_init_size(&msg, len);
    d_assert (rv == 0, return CORE_ERROR, "zmq init msg failed!");

    memcpy(zmq_msg_data(&msg), buf, len);

    rv = zmq_msg_send(&msg, pusher, 0);
    d_assert (rv == len, zmq_msg_close(&msg);return CORE_ERROR, "zmq send msg failed!");

    zmq_msg_close(&msg);

    return CORE_OK;
}

void *zmq_create_puller(void *context, char *path, int need_bind)
{
    int rv = 0;
    void *puller = zmq_socket(context, ZMQ_PULL);

    d_assert(puller, return NULL, "zmq_socket failed. (%d:%s)", errno, strerror(errno));

    if (need_bind) {
        rv = zmq_bind(puller, path);
        d_assert(rv == 0, return NULL, "zmq_bind failed. (%d:%s)", errno, strerror(errno));
    } else {
        rv = zmq_connect(puller, path);
        d_assert(rv == 0, return NULL, "zmq_connect failed. (%d:%s)", errno, strerror(errno));
    }

    return puller;
}

void *zmq_create_puller_recv_buffer(void *context, char *path, int need_bind, int buffer_size)
{
    int rv = 0;
    void *puller = zmq_socket(context, ZMQ_PULL);

    d_assert(puller, return NULL, "zmq_socket failed. (%d:%s)", errno, strerror(errno));

    if(buffer_size > 0)
    {
        int recv_buffer = buffer_size; //80M
        size_t optlen = sizeof(recv_buffer);
        rv = zmq_setsockopt(puller, ZMQ_RCVBUF, (void *)&recv_buffer, optlen);
        d_assert(rv == 0, return NULL, "zmq_setsocktopt failed. (%d:%s)", errno, strerror(errno));
    }

    if (need_bind) {
        rv = zmq_bind(puller, path);
        d_assert(rv == 0, return NULL, "zmq_bind failed. (%d:%s)", errno, strerror(errno));
    } else {
        rv = zmq_connect(puller, path);
        d_assert(rv == 0, return NULL, "zmq_connect failed. (%d:%s)", errno, strerror(errno));
    }

    return puller;
}

status_t zmq_puller_register(void *puller, zmq_puller_handler handler)
{
    d_assert(puller, return CORE_ERROR, "puller is NULL!");
    d_assert(handler, return CORE_ERROR, "handler is NULL!");
    d_assert(num_of_puller <= MAX_PULLER_NUM, return CORE_ERROR, "num_of_puller is %d!", MAX_PULLER_NUM);

    pullers[num_of_puller].socket = puller;
    pullers[num_of_puller].fd = 0;
    pullers[num_of_puller].events = ZMQ_POLLIN;
    pullers[num_of_puller].revents = 0;

    puller_handler[num_of_puller] = handler;
    num_of_puller++;

    return CORE_OK;
}

void zmq_poll_loop(long timeout)
{
    int i=0, rc=0, nbytes=0;

    rc = zmq_poll(pullers, num_of_puller, timeout);
    if (rc <= 0) {
        return;
    }

    for (i=0; i<num_of_puller; i++) {
        if (pullers[i].revents & ZMQ_POLLIN) {
            zmq_msg_t msg;

            rc = zmq_msg_init(&msg);
            if (rc != 0) {
                continue;
            }

            nbytes = zmq_msg_recv(&msg, pullers[i].socket, 0);
            if (nbytes != -1) {
                puller_handler[i](zmq_msg_data(&msg), zmq_msg_size(&msg));
            }
            zmq_msg_close(&msg);
        }
    }
}

inline c_int64_t zmq_gen_seq_id(void)
{
    return time_now();
}

inline void zmq_build_auth_ctx_id(c_uint8_t *auth_ctx_id, int instance_id)
{
    snprintf((char *)auth_ctx_id, MAX_ZMQ_AUTH_CTX_ID_LEN, MAX_ZMQ_AUTH_CTX_ID_FMT, instance_id, zmq_gen_seq_id());
    //d_trace(0, "%s: %s", auth_ctx_id);
}

status_t zmq_gen_uuid_for_msg(c_int8_t *uuid,
        sbi_entity_type_e src, c_uint16_t src_instance_id,
        sbi_entity_type_e dst, c_uint16_t dst_instance_id,
        sbi_msg_type_e msg_type, c_int64_t seq)
{
    d_assert(uuid, return CORE_ERROR, "null param");

#ifdef ZMQ_SHORT_REQ_ID
    snprintf(uuid, MAX_ZMQ_MSG_ID_LEN, MAX_ZMQ_MSG_ID_FMT, src_instance_id, msg_type, seq);
#else
    snprintf(uuid, MAX_ZMQ_MSG_ID_LEN, MAX_ZMQ_MSG_ID_FMT, src, src_instance_id, dst, dst_instance_id, msg_type, seq);
#endif

    return CORE_OK;
}

status_t zmq_set_msg_type_to_msg_uuid(c_int8_t *uuid, sbi_msg_type_e msg_type)
{
#ifndef ZMQ_SHORT_REQ_ID
    c_uint32_t src, src_instance_id, dst, dst_instance_id;
#endif
    c_uint32_t instance_id;
    c_uint32_t cur_msg_type;
    c_int64_t seq;

    d_assert(uuid, return CORE_ERROR, "null param");

#ifdef ZMQ_SHORT_REQ_ID
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &instance_id, &cur_msg_type, &seq) <= 0)
#else
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &src, &src_instance_id, &dst, &dst_instance_id, &cur_msg_type, &seq) <= 0)
#endif
    {
        d_error("msg id %s is wrong format!", uuid);
        return CORE_ERROR;
    }

#ifdef ZMQ_SHORT_REQ_ID
    snprintf(uuid, MAX_ZMQ_MSG_ID_LEN, MAX_ZMQ_MSG_ID_FMT, instance_id, msg_type, seq);
#else
    snprintf(uuid, MAX_ZMQ_MSG_ID_LEN, MAX_ZMQ_MSG_ID_FMT, src, src_instance_id, dst, dst_instance_id, msg_type, seq);
#endif
    return CORE_OK;
}

sbi_msg_type_e zmq_get_msg_type_from_msg_uuid(c_int8_t *uuid)
{
#ifndef ZMQ_SHORT_REQ_ID
    c_uint32_t src, src_instance_id, dst, dst_instance_id;
#endif
    c_uint32_t instance_id;
    c_uint32_t msg_type;
    c_int64_t seq;

    d_assert(uuid, return SBI_MSG_T_NONE, "null param");

#ifdef ZMQ_SHORT_REQ_ID
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &instance_id, &msg_type, &seq) > 0)
#else
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &src, &src_instance_id, &dst, &dst_instance_id, &msg_type, &seq) > 0)
#endif
    {
        return msg_type;
    }
    else
    {
        d_error("msg id %s is wrong format!", uuid);
        return SBI_MSG_T_NONE;
    }
}

c_int64_t zmq_get_seq_from_msg_uuid(c_int8_t *uuid)
{
#ifndef ZMQ_SHORT_REQ_ID
    c_uint32_t src, src_instance_id, dst, dst_instance_id;
#endif
    c_uint32_t instance_id;
    c_uint32_t msg_type;
    c_int64_t seq;

    d_assert(uuid, return SBI_MSG_T_NONE, "null param");

#ifdef ZMQ_SHORT_REQ_ID
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &instance_id, &msg_type, &seq) > 0)
#else
    if (sscanf(uuid, MAX_ZMQ_MSG_ID_FMT, &src, &src_instance_id, &dst, &dst_instance_id, &msg_type, &seq) > 0)
#endif
    {
        return seq;
    }
    else
    {
        d_error("msg id %s is wrong format!", uuid);
        return -1;
    }
}
