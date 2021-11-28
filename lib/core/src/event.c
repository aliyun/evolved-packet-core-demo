/**
 * @file event.c
 */

/* Core libaray */
#define TRACE_MODULE _event
#include "core_debug.h"
#include "core_msgq.h"
#include "core_event.h"
#include "core_zmq.h"

#define EVT_Q_DEPTH 64

char *EVT_NAME_UNKNOWN = "UNKNOWN";

msgq_id event_create(int opt)
{
    msgq_id queue_id = 0;

    /* Start threads */
    queue_id = msgq_create(EVT_Q_DEPTH, EVENT_SIZE, opt);
    d_assert(queue_id != 0, return CORE_ERROR, "Message queue creation failed");

    return queue_id;
}

status_t event_delete(msgq_id queue_id)
{
    msgq_delete(queue_id);

    return CORE_OK;
}

status_t event_send(msgq_id queue_id, event_t *e)
{
    status_t rv;

    d_assert(e, return -1, "Null param");
    d_assert(queue_id, return -1, "event queue isn't initialized");

    rv = msgq_send(queue_id, (const char*)e, EVENT_SIZE);
    if (rv == CORE_EAGAIN)
    {
        d_warn("msgq_send full");
    }
    else if (rv == CORE_ERROR)
    {
        d_error("msgq_send failed");
    }

    return rv;
}

status_t event_recv(msgq_id queue_id, event_t *e)
{
    status_t rv;

    d_assert(e, return -1, "Null param");
    d_assert(queue_id, return -1, "event queue isn't initialized");

    rv = msgq_recv(queue_id, (char*)e, EVENT_SIZE);
    if (rv == CORE_ERROR)
    {
        d_error("msgq_timedrecv failed", rv);
    }

    return rv;
}

status_t event_timedrecv(msgq_id queue_id, event_t *e, c_time_t timeout)
{
    status_t rv;

    d_assert(e, return -1, "Null param");
    d_assert(queue_id, return -1, "event queue isn't initialized");

    rv = msgq_timedrecv(queue_id, (char*)e, EVENT_SIZE, timeout);
    if (rv == CORE_ERROR)
    {
        d_error("msgq_timedrecv failed", rv);
    }

    return rv;
}

status_t event_timer_expire_func(c_uintptr_t queue_id, c_uintptr_t param1,
        c_uintptr_t param2, c_uintptr_t param3, c_uintptr_t param4,
        c_uintptr_t param5, c_uintptr_t param6)
{
    event_t e;
    status_t rv;

    d_assert(queue_id, return CORE_ERROR, "Null param");
    event_set(&e, param1);
    event_set_param1(&e, param2);
    event_set_param2(&e, param3);
    event_set_param3(&e, param4);
    event_set_param4(&e, param5);
    event_set_param5(&e, param6);

    rv = event_send(queue_id, &e);
    if (rv != CORE_OK)
    {
        d_error("event_send error:%d", rv);
    } 

    return rv;
}

tm_block_id event_timer_create(tm_service_t *tm_service, tm_type_e type, c_uint32_t duration, c_uintptr_t event)
{
    tm_block_id id;

    if (tm_service->expire_func) {
        id = tm_create(tm_service, type, duration, tm_service->expire_func);
    } else {
        id = tm_create(tm_service, type, duration, (expire_func_t)event_timer_expire_func);
    }
    tm_set_param1(id, event);
    d_assert(id, return 0, "tm_create() failed");

    return id;
}

void* event_timer_expire_func_zmq(void *sender, c_uintptr_t param1,
                              c_uintptr_t param2, c_uintptr_t param3, c_uintptr_t param4,
                              c_uintptr_t param5, c_uintptr_t param6)
{
    event_t e;
    status_t rv;

    d_assert(sender, return NULL, "Null param");
    event_set(&e, param1);
    event_set_param1(&e, param2);
    event_set_param2(&e, param3);
    event_set_param3(&e, param4);
    event_set_param4(&e, param5);
    event_set_param5(&e, param6);

    rv = event_send_by_zmq(sender, &e);
    if (rv != CORE_OK)
    {
        d_error("event_send_by_zmq error:%d", rv);
    }

    return NULL;
}

tm_block_id event_timer_create_zmq(tm_service_t *tm_service, tm_type_e type, c_uint32_t duration, c_uintptr_t event)
{
    tm_block_id id;

    id = tm_create(tm_service, type, duration, (expire_func_t)event_timer_expire_func);
    tm_set_param1(id, event);
    d_assert(id, return 0, "tm_create() failed");

    return id;
}

status_t event_init(void *context, char *path, void **receiver, void **sender)
{
    d_assert(context, return CORE_ERROR, );
    d_assert(path, return CORE_ERROR, );

    if (receiver)
    {
        *receiver = zmq_create_puller(context, path, 1);
        d_assert(*receiver, return CORE_ERROR, "init event reciever for %s failed!", path);
    }

    if (sender)
    {
        *sender = zmq_create_pusher(context, path);
        d_assert(*sender, return CORE_ERROR, "init event send for %s failed!", path);
    }

    return CORE_OK;
}

status_t event_final(void **receiver, void **sender)
{
    //d_assert(receiver, return CORE_ERROR, );
    //d_assert(sender, return CORE_ERROR, );

    if (receiver)
    {
        if (*receiver)
        {
            zmq_close_pusher_puller(*receiver);
            *receiver = NULL;
        }
    }

    if (sender)
    {
        if (*sender)
        {
            zmq_close_pusher_puller(*sender);
            *sender = NULL;
        }
    }

    return CORE_OK;
}

status_t event_recv_with_timeout(void *receiver, event_t *e, c_time_t timeout)
{
    int rc, nbytes=0;
    zmq_msg_t msg;

    d_assert(e, return CORE_ERROR, );
    d_assert(receiver, return CORE_ERROR, );

    zmq_pollitem_t items[1];
    items[0].socket = receiver;
    items[0].events = ZMQ_POLLIN;

    rc = zmq_poll(items, 1, timeout);
    if (rc == 0)
    {
        if (timeout == 0)
        {
            return CORE_EAGAIN;
        }
        return CORE_TIMEUP;
    }

    d_assert(rc > 0, return CORE_ERROR, );

    if (items[0].revents & ZMQ_POLLIN)
    {
        rc = zmq_msg_init(&msg);
        d_assert(rc == 0, return CORE_ERROR, );

        nbytes = zmq_msg_recv(&msg, items[0].socket, 0);
        if (-1 == nbytes)
        {
            d_error("zmq msg recv failed. errno:%d, reason:%s", errno, strerror(errno));
            // need to close the msg?
            return CORE_ERROR;
        }

        if (nbytes != sizeof(event_t))
        {
            d_error("expect: %d, got %d, size in ctx %d.", sizeof(event_t), nbytes,  zmq_msg_size(&msg));
            zmq_msg_close(&msg);
            return CORE_ERROR;
        }
        else
        {
            memcpy(e, zmq_msg_data(&msg), sizeof(event_t));
            zmq_msg_close(&msg);
        }
    }

    return CORE_OK;
}

status_t event_send_by_zmq(void *sender, event_t *e)
{
    d_assert(sender, return CORE_ERROR, );
    d_assert(e, return CORE_ERROR, );

    return zmq_pusher_send_buf(sender, (char*)e, sizeof(event_t));
}
