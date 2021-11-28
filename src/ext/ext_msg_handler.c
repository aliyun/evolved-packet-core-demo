#define TRACE_MODULE _ext_msg

#include "core_debug.h"
#include "core_thread.h"
#include "ext_context.h"
#include "core_epc_msg.h"
#include "ext_data_syn.h"

//#include <sys/socket.h>
//#include <sys/un.h>

status_t epc_ext_msg_handle(EpcMsgHeader* msg)
{
    switch(msg->type) {
        case MSG_T_GTP_FLOW_SYN:
            if (ext_data_syn_send_one_flow((char*)(msg+1)) != CORE_OK) {
                d_error("send flow cmd %s failed.", msg+1);
            }
            break;

        default:
            d_error("Unknown msg type %d\n", msg->type);
    }
    

    return 0;
}
