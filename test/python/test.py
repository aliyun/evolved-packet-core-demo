import time
import _sctp
from sctp import *
from CryptoMobile.Milenage import Milenage,conv_A2,conv_A7
from CryptoMobile.CM import SNOW3G


def snow3g_EIA1(key, count, bearer, data):
    snow    = SNOW3G()
    direct  = 0
    bitlen  = len(data)*8
    return snow.EIA1(key, count, bearer, direct, data, bitlen)


def sctp_connect():
    server = '30.12.22.22'
    sctpport = 36412 
    
    if _sctp.getconstant("IPPROTO_SCTP") != 132:
        raise "getconstant failed"
    
    sock = sctpsocket(socket.AF_INET, socket.SOCK_STREAM, None)
    sock.set_nodelay(True)
    saddr = (server, sctpport)
    print "SCTP", saddr, " ----------------------------------------------"
    
    sock.initparams.max_instreams = 3
    sock.initparams.num_ostreams = 3
    client = '30.12.22.1'
    cport = 36412
    caddr = (client, cport)
    
    sock.bindx([caddr])
    sock.events.clear()
    sock.events.data_io = 1
    sock.connect(saddr)
    return sock

def sctp_disconnect(sock):
    sock.close()

##############################################################################
# S1Setup
##############################################################################
def S1Setup(sock):
    S1SetupRequest_b = [0x00,0x11,0x00,0x26,0x00,0x00,0x04,0x00,0x3b,
                        0x00,0x09,0x00,0x64,0xf0,0x89,0x40,0x00,0x00,
                        0x00,0x70,0x00,0x3c,0x40,0x02,0x80,0x00,0x00,
                        0x40,0x00,0x07,0x00,0x00,0x00,0x40,0x64,0xf0,
                        0x89,0x00,0x89,0x40,0x01,0x40]
    S1SetupRequest_msg = str(bytearray(S1SetupRequest_b))
    sock.sctp_send(S1SetupRequest_msg)
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)
    msg_list = [ord(c) for c in msg]
    print msg_list

##############################################################################
# Attach
##############################################################################
InitialUEMessage_HEADER = [0x00, 0x0c, 0x40, 0x64, 0x00, 0x00, 0x05]
Item_id_eNB_UE_S1AP_ID = [0x00, 0x08, 0x00, 0x02]
Item_id_NAS_PDU_12 = [0x00, 0x1a, 0x00, 0x3c, 0x3b, 0x07, 0x41, 0x71]
EPS_mob_id = [0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10]
UE_net_cap = [0x05, 0xe0, 0xe0, 0x00, 0x00, 0x00]
ESM_msg_con = [0x00, 0x25,
        0x02, 0x01, 0xd0, 0x11, 0xd1, 0x27, 0x1d, 0x80,
        0x80, 0x21, 0x10, 0x01, 0x00, 0x00, 0x10, 0x81, 
        0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x0d, 
        0x00, 0x00, 0x10, 0x00, 0xc0]
Dev_prop_MS_net_feature = [0xd0, 0xc1]

Item_id_TAI_12 = [0x00, 0x43, 0x00, 0x06, 0x00, 0x64, 0xf0, 0x89, 0x00, 0x01]
Item_id_EUTRAN_CGI_12 = [0x00, 0x64, 0x40, 0x08,
        0x00, 0x64, 0xf0, 0x89, 0x00, 0x00, 0x00, 0x70]
Item_id_RRC_Establishment_Cause = [0x00, 0x86, 0x40, 0x01, 0x30]

UplinkNASTransport_HEADER_AuthResp = [0x00, 0x0d, 0x40, 0x36, 0x00, 0x00, 0x05]
UplinkNASTransport_HEADER_SecModeComplete = [0x00, 0x0d, 0x40, 0x33, 0x00, 0x00, 0x05]
UplinkNASTransport_HEADER_EsmInfoResp = [0x00, 0x0d, 0x40, 0x40, 0x00, 0x00, 0x05]
UplinkNASTransport_HEADER_AttachComplete = [0x00, 0x0d, 0x40, 0x39, 0x00, 0x00, 0x05]
Item_id_MME_UE_S1AP_ID = [0x00, 0x00, 0x00, 0x03]
#Item_id_eNB_UE_S1AP_ID
Item_id_NAS_PDU_13_AuthResp = [0x00, 0x1a, 0x00, 0x0c, 0x0b, 0x07, 0x53, 0x08]
Item_id_NAS_PDU_13_SecModeComplete = [0x00, 0x1a, 0x00, 0x09, 0x08, 0x47, 0xdc, 0x51, 0x81, 0xb4]
Item_id_NAS_PDU_13_EsmInfoResp = [0x00, 0x1a, 0x00, 0x15, 0x14, 0x27, 0x00, 0x80, 0x61, 0x3b]
Item_id_NAS_PDU_13_AttachComplete = [0x00, 0x1a, 0x00, 0x0e, 0x0d, 0x27, 0xbb, 0xd9, 0x43, 0x46]
Item_id_EUTRAN_CGI_13 = Item_id_EUTRAN_CGI_12
Item_id_TAI_13 = Item_id_TAI_12
#Item_id_GW_TransportLayerAddress = []


UECapbilityInfoIndication_HEADER = [0x00, 0x16, 0x40, 0x3f, 0x00, 0x00, 0x03]
Item_id_UERadioCapability = [0x00, 0x4a, 0x40, 0x2a, 0x29,
        0x01, 0x38, 0x01, 0x02, 0x4d, 0x1b, 0xe0, 0x10,
        0x1f, 0x85, 0x81, 0x03, 0x81, 0xff, 0x83, 0x76,
        0x26, 0x80, 0x08, 0x0d, 0xc1, 0x60, 0x00, 0x00,
        0x00, 0x00, 0x12, 0x20, 0x00, 0x00, 0x00, 0x00,
        0x05, 0x14, 0x22, 0x12, 0x98, 0x02, 0x01, 0x54,
        0x50]

InitialContextSetupResponse_HEADER = [0x20, 0x09, 0x00, 0x24, 0x00, 0x00, 0x03]
Item_id_E_RABSetupListCtxtSURes = [0x00, 0x33, 0x40, 0x0f, 0x00, 0x00, 0x32, 0x40, 0x0a, 0x0a, 0x1f]

def msg_to_hexString(msg):
    length = len(msg)
    msg_str = ''
    for i in range(length):
        msg_str = '%s%02x'%(msg_str, ord(msg[i]))
        if i%8 == 7:
            msg_str = msg_str + ' '
    return msg_str

def get_value_by_type(msg, t):
    num = ord(msg[6])
    off = 7
    for i in range(num):
        tlv_t = ord(msg[off + 1])
        tlv_l = ord(msg[off + 3])
        if tlv_t == t:
            return msg[off+4:off+4+tlv_l]
        off =  off + 4 + tlv_l
        print "t=%02x off=%d"%(tlv_t, off)

    return None

def calc_res(rand):
    opc = b'\x46\x09\x80\x00\x1E\x73\x8A\x38\x5C\xE2\x73\x9D\xA2\x63\xC0\x9E'
    key = b'\x46\x09\x80\x00\x1F\xAB\x28\x3F\x94\xAD\xF2\x65\xDC\x10\x10\xEF'
    mil = Milenage(None)
    mil.set_opc(opc)
    res, ck, ik, ak = mil.f2345(key, rand)
    return res, ck, ik, ak

def send_initialUeMessage(sock):
    msg = str(bytearray(InitialUEMessage_HEADER
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_NAS_PDU_12+EPS_mob_id+UE_net_cap+ESM_msg_con+Dev_prop_MS_net_feature
        +Item_id_TAI_12
        +Item_id_EUTRAN_CGI_12
        +Item_id_RRC_Establishment_Cause))
    sock.sctp_send(msg)

def recv_downlinkNASTransport_AuthReq(sock):
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)
    MME_UE_S1AP_ID = get_value_by_type(msg, 0)
    print "MME_UE_S1AP_ID=%s"%(msg_to_hexString(MME_UE_S1AP_ID))
    NAS_PDU = get_value_by_type(msg, 26)
    print "NAS_PDU=",msg_to_hexString(NAS_PDU)
    if ord(NAS_PDU[0]) == 0x24:
        RAND = NAS_PDU[4:20]
        SQN_X_AK = NAS_PDU[21:27]
    print "RAND=",msg_to_hexString(RAND)
    return MME_UE_S1AP_ID,RAND,SQN_X_AK

def send_uplinkNASTransport_AuthResp(sock, mme_ue_s1ap_id, res):
    Item_id_MME_UE_S1AP_ID[3] = len(mme_ue_s1ap_id)
    msg = str(bytearray(UplinkNASTransport_HEADER_AuthResp
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_NAS_PDU_13_AuthResp+res
        +Item_id_EUTRAN_CGI_13
        +Item_id_TAI_13))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def recv_downlinkNASTransport_SecModeCmd(sock):
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)
    print "SecModeCmd=%s"%(msg_to_hexString(msg))

def send_uplinkNASTransport_SecModeComplete(sock, mme_ue_s1ap_id, knasint):
    print "1 knasint=",msg_to_hexString(knasint)
    nas_data = [0x00, 0x07, 0x5e]
    mac = snow3g_EIA1(knasint, 0, 0, str(bytearray(nas_data)))
    for i in range(4):
        Item_id_NAS_PDU_13_SecModeComplete[6+i] = mac[i]

    msg = str(bytearray(UplinkNASTransport_HEADER_SecModeComplete
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_NAS_PDU_13_SecModeComplete+nas_data
        +Item_id_EUTRAN_CGI_13
        +Item_id_TAI_13))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def recv_downlinkNASTransport_EsmInfoReq(sock):
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)

def send_uplinkNASTransport_EsmInfoResp(sock, mme_ue_s1ap_id, knasint):
    print "2 knasint=",msg_to_hexString(knasint)
    nas_data = [0x01, 0x02, 0x01, 0xda, 0x28, 0x09, 0x08, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74]
    mac = snow3g_EIA1(knasint, 1, 0, str(bytearray(nas_data)))
    for i in range(4):
        Item_id_NAS_PDU_13_EsmInfoResp[6+i] = mac[i]

    msg = str(bytearray(UplinkNASTransport_HEADER_EsmInfoResp
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_NAS_PDU_13_EsmInfoResp+nas_data
        +Item_id_EUTRAN_CGI_13
        +Item_id_TAI_13))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def recv_InitialContextSetupRequest(sock):
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)

def send_UECapbilityInfoIndication(sock, mme_ue_s1ap_id):
    msg = str(bytearray(UECapbilityInfoIndication_HEADER
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_UERadioCapability))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def send_InitialContextSetupResponse(sock, mme_ue_s1ap_id):
    ip = [0x1e, 0x0c, 0x16, 0x01]
    s1u_dl_teid = [0x01, 0x00, 0x09, 0x08]
    msg = str(bytearray(InitialContextSetupResponse_HEADER
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_E_RABSetupListCtxtSURes+ip+s1u_dl_teid))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def send_uplinkNASTransport_AttachComplete(sock, mme_ue_s1ap_id, knasint):
    print "3 knasint=",msg_to_hexString(knasint)
    nas_data = [0x02, 0x07, 0x43, 0x00, 0x03, 0x52, 0x00, 0xc2]
    mac = snow3g_EIA1(knasint, 2, 0, str(bytearray(nas_data)))
    for i in range(4):
        Item_id_NAS_PDU_13_AttachComplete[6+i] = mac[i]

    msg = str(bytearray(UplinkNASTransport_HEADER_AttachComplete
        +Item_id_MME_UE_S1AP_ID+mme_ue_s1ap_id
        +Item_id_eNB_UE_S1AP_ID+enb_ue_s1ap_id
        +Item_id_NAS_PDU_13_AttachComplete+nas_data
        +Item_id_EUTRAN_CGI_13
        +Item_id_TAI_13))
    msg_list = list(msg)
    msg_list[3] = len(msg_list) - 4
    msg = str(bytearray(msg_list))
    sock.sctp_send(msg)

def recv_downlinkNASTransport_EmmInfo(sock):
    fromaddr, flags, msg, notif = sock.sctp_recv(1024)


######################################################################################
#  testing start....
######################################################################################
client_sock = sctp_connect()
enb_ue_s1ap_id = [0x00, 0x07]

# Step1: S1Setup
S1Setup(client_sock)

# Step2: send initialUEMessage
send_initialUeMessage(client_sock)

# Step3:  recv downlinkNASTransport, Auth Req
#               parse id_MME_UE_S1AP_ID,RAND
mme_ue_s1ap_id,rand,sqn_x_ak = recv_downlinkNASTransport_AuthReq(client_sock)
res, ck, ik, ak = calc_res(rand)


plmn_id = str(bytearray([0x64, 0xf0, 0x89]))
kasme = conv_A2(ck, ik, plmn_id, sqn_x_ak)
knasint = conv_A7(kasme, 0x02, 0x0001)
print 'ik=',msg_to_hexString(ik)
print 'knasint=',msg_to_hexString(knasint)

# Step4: send uplinkNASTransport, Auth Resp
send_uplinkNASTransport_AuthResp(client_sock, list(mme_ue_s1ap_id), list(res))

# Step5: recv downlinkNASTransport Sec Mode Cmd
recv_downlinkNASTransport_SecModeCmd(client_sock)

# Step6: send uplinkNASTransport Sec Mode Complete
send_uplinkNASTransport_SecModeComplete(client_sock, list(mme_ue_s1ap_id), knasint[16:])

# Step7: recv downlinkNASTransport , ESM information request
recv_downlinkNASTransport_EsmInfoReq(client_sock)

# Step8: send uplinkNASTransport , ESM information response
send_uplinkNASTransport_EsmInfoResp(client_sock, list(mme_ue_s1ap_id), knasint[16:])

# Step9: recv InitialContextSetupRequest
recv_InitialContextSetupRequest(client_sock)

# S10: send UECapbilityInfoIndication
send_UECapbilityInfoIndication(client_sock, list(mme_ue_s1ap_id))

# S11: send InitialContextSetupResponse
send_InitialContextSetupResponse(client_sock, list(mme_ue_s1ap_id))

# S12: send uplinkNAATransport, Attach complete
send_uplinkNASTransport_AttachComplete(client_sock, list(mme_ue_s1ap_id), knasint[16:])

# S13: recv downlinkNASTransport, EMM info
recv_downlinkNASTransport_EmmInfo(client_sock)

sctp_disconnect(client_sock)
