# -*- coding: UTF-8 -*-
import time
import thread
import socket
import copy
import _sctp
from sctp import *
from CryptoMobile.Milenage import Milenage,conv_A2,conv_A7
from CryptoMobile.CM import SNOW3G
from ue import UE
from ue_sender import UeSender

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

Item_id_TAI = [0x00, 0x43, 0x00, 0x06, 0x00, 0x64, 0xf0, 0x89, 0x00, 0x01]
Item_id_EUTRAN_CGI = [0x00, 0x64, 0x40, 0x08, 0x00, 0x64, 0xf0, 0x89, 0x00, 0x00, 0x00, 0x70]
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

HandoverRequired_HEADER = [0x00, 0x00, 0x00, 0x81, 0x12, 0x00, 0x00, 0x06]
Item_id_HandoverType = [0x00, 0x01, 0x00, 0x01, 0x00]
Item_id_Cause = [0x00, 0x02, 0x40, 0x02, 0x02, 0x20]
Item_id_TargetID = [0x00, 0x04, 0x00, 0x0e, 0x00, 0x64, 0xf0, 0x89,
                    0x40, 0x00, 0x00, 0x00, 0x70, 0x64, 0xf0, 0x89,
                    0x00, 0x01]
Item_id_SourceToTargetTransparentContainer = [0x00, 0x68, 0x00, 0x80, 0xe1, 0x80, 0xdf, 0x40, 0x80, 0xb7, 0x0f, 0x10, 0x24, 0xd1, 0xbe, 0x01,
                                              0x01, 0xf8, 0x58, 0x10, 0x38, 0x1f, 0xf8, 0x37, 0x62, 0x68, 0x00, 0x80, 0xdc, 0x16, 0x00, 0x00,
                                              0x00, 0x00, 0x01, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x42, 0x21, 0x29, 0x80, 0x20, 0x15,
                                              0x45, 0x15, 0xa8, 0x00, 0x14, 0x7e, 0xc2, 0x30, 0x00, 0x00, 0x0e, 0xf1, 0x00, 0x15, 0x00, 0xcb,
                                              0xc5, 0x84, 0x09, 0xe0, 0x65, 0xe2, 0xc4, 0x01, 0x04, 0x32, 0xf1, 0x61, 0x00, 0x00, 0x20, 0x09,
                                              0xf9, 0x09, 0x12, 0x28, 0x08, 0x09, 0x3f, 0xd6, 0xc0, 0x47, 0xea, 0x81, 0xcc, 0x1c, 0x0e, 0x04,
                                              0x7e, 0xa8, 0x1c, 0xc1, 0xc0, 0x0f, 0xa8, 0x1b, 0xc3, 0x45, 0xdd, 0x03, 0x86, 0x42, 0x32, 0xe6,
                                              0x01, 0xec, 0x2f, 0x2d, 0x15, 0x9f, 0x86, 0x1c, 0x89, 0xc0, 0x00, 0x26, 0x28, 0xaa, 0x1a, 0x00,
                                              0x08, 0xd8, 0x18, 0x06, 0x00, 0x44, 0x01, 0xe6, 0x88, 0xb0, 0x02, 0x05, 0x18, 0x13, 0x10, 0x00,
                                              0x10, 0x00, 0x00, 0x08, 0xd2, 0x86, 0xbf, 0x10, 0x81, 0x04, 0x0c, 0x21, 0xc1, 0x34, 0x08, 0x90,
                                              0x81, 0x73, 0xa5, 0xb7, 0xf9, 0x97, 0xcd, 0xc3, 0x60, 0x47, 0x48, 0x10, 0x00, 0x10, 0x20, 0x02,
                                              0x0e, 0x27, 0xea, 0xae, 0x5b, 0x71, 0x1f, 0xb3, 0xe5, 0x07, 0x7f, 0x61, 0x08, 0x08, 0x02, 0x01,
                                              0x84, 0x00, 0x00, 0x4e, 0x40, 0x02, 0x45, 0x00, 0x00, 0x64, 0xf0, 0x89, 0x00, 0x00, 0x00, 0x71,
                                              0x00, 0x64, 0xf0, 0x89, 0x00, 0x00, 0x00, 0x80, 0x00, 0x01, 0x2f, 0x00, 0x64, 0xf0, 0x89, 0x00,
                                              0x00, 0x00, 0x70, 0x00, 0x00, 0xe6]


HandoverRequestAcknowledge_HEADER = [0x20, 0x01, 0x00, 0x80, 0xb5, 0x00, 0x00, 0x04]
Item_id_ERabAdmittedList = [0x00, 0x12, 0x40, 0x24, 0x00, 0x00, 0x14, 0x40, 0x1f, 0x78, 0xa1, 0xf0, 0x1e, 0x71, 0x02, 0x35,
                            0x01, 0x00, 0x02, 0x08, 0x0f, 0x80, 0x1e, 0x71, 0x02, 0x35, 0x01, 0x00, 0x02, 0x09, 0x0f, 0x80,
                            0x1e, 0x71, 0x02, 0x35, 0x01, 0x00, 0x02, 0x0a]
Item_id_SourceToTargetTransparentContainer_HoReqAck = [0x00, 0x7b, 0x00, 0x7a, 0x79, 0x00, 0x77, 0x0b, 0xa9, 0x00, 0xd8, 0xfd, 0x40, 0x00, 0xa3, 0xf6,
                                                       0x11, 0x80, 0x00, 0x00, 0x87, 0x80, 0x41, 0x00, 0x2a, 0x01, 0x97, 0x8b, 0x08, 0x13, 0xc0, 0xcb, 
                                                       0xc5, 0x80, 0x41, 0x00, 0x00, 0x10, 0x05, 0xf9, 0x09, 0x12, 0x28, 0x08, 0x09, 0x3e, 0xf0, 0x3f, 
                                                       0xf6, 0x13, 0xd9, 0xf2, 0xaa, 0x08, 0x00, 0xf2, 0xff, 0x2e, 0x74, 0xb6, 0xff, 0x3e, 0x6e, 0x1b, 
                                                       0x02, 0x2e, 0x40, 0x80, 0x00, 0x10, 0x20, 0x02, 0x0e, 0x27, 0xea, 0xae, 0xea, 0xa4, 0x3a, 0xd8, 
                                                       0x08, 0xfd, 0x50, 0x39, 0x83, 0x81, 0xc0, 0x8f, 0xd5, 0x03, 0x98, 0x38, 0x00, 0xd0, 0x37, 0x86, 
                                                       0x8b, 0xba, 0x07, 0x64, 0x23, 0x3e, 0x60, 0x0f, 0x61, 0x79, 0x68, 0xac, 0xfc, 0x30, 0xe4, 0x4e, 
                                                       0x00, 0x02, 0x71, 0x45, 0x50, 0xd0, 0x00, 0x46, 0xc0, 0xc0, 0x30, 0x04, 0x44, 0xc0]

EnbStatusTransfer_HEADER = [0x00, 0x18, 0x40, 0x24, 0x00, 0x00, 0x03]
Item_id_EnbStatusTransferTransparentContainer = [0x00, 0x5a, 0x00, 0x11, 0x00, 0x00, 0x00, 0x59, 0x40, 0x0b, 0x05, 0x00, 0x0c, 0x51, 0x00, 0x00,
                                                 0x00, 0x02, 0x0e, 0x00, 0x00]

HandoverNotify_HEADER = [0x00, 0x02, 0x40, 0x25, 0x00, 0x00, 0x04]
UeCtxReleaseComplete_HEADER = [0x20, 0x17, 0x00, 0x0f, 0x00, 0x00, 0x02]

class Enb:
    def __init__(self, enb_id, enb_ip, epc_ip, srv_ip):
        self.enb_id = enb_id
        self.enb_ip = enb_ip
        self.enb_port = 38412
        self.sock = None
        self.epc_ip = epc_ip
        self.srv_ip = srv_ip
        self.plmn_id = str(bytearray([0x64, 0xf0, 0x89]))
    
    def sctp_init(self):
        sctpport = 38412
        
        if _sctp.getconstant("IPPROTO_SCTP") != 132:
            raise "getconstant failed"
        
        sock = sctpsocket(socket.AF_INET, socket.SOCK_STREAM, None)
        sock.set_nodelay(True)
        saddr = (self.epc_ip, sctpport)
        print "SCTP", saddr
        
        sock.initparams.max_instreams = 3
        sock.initparams.num_ostreams = 3
        caddr = (self.enb_ip, self.enb_port)
        
        sock.bindx([caddr])
        sock.events.clear()
        sock.events.data_io = 1
        sock.connect(saddr)
        self.sock = sock
    
    def sctp_uninit(self):
        self.sock.close()

    def msg_to_hexString(self, msg):
        length = len(msg)
        msg_str = ''
        for i in range(length):
            msg_str = '%s%02x'%(msg_str, ord(msg[i]))
            if i%8 == 7:
                msg_str = msg_str + ' '
        return msg_str

    def get_value_by_type(self, msg, t):
        num = ord(msg[6])
        off = 7
        if ord(msg[1]) == 9 or (ord(msg[3])&0x80) == 0x80:
            num = ord(msg[7])
            off = 8
        #print "el num: %d"%(num)
        for i in range(num):
            tlv_t = ord(msg[off + 1])
            tlv_l = ord(msg[off + 3])
            if tlv_t == t:
                return msg[off+4:off+4+tlv_l]
            off =  off + 4 + tlv_l
            #print "t=%02x off=%d"%(tlv_t, off)

        return None

    def gen_enb_ue_s1ap_id(self,v):
        if v < 256:
            enb_ue_s1ap_id = [0x00, 0x08, 0x00, 0x02, 0x00, 0x00]
            enb_ue_s1ap_id[5] = v
            return enb_ue_s1ap_id

        if v <= 256*256:
            enb_ue_s1ap_id = [0x00, 0x08, 0x00, 0x03, 0x40, 0x00, 0x00]
            enb_ue_s1ap_id[5] = v/256
            enb_ue_s1ap_id[6] = v%256
            return enb_ue_s1ap_id
    
    def snow3g_EIA1(self, key, count, bearer, data):
        snow    = SNOW3G()
        direct  = 0
        bitlen  = len(data)*8
        return snow.EIA1(key, count, bearer, direct, data, bitlen)

    def set_s1ap_len(self, msg):
        msg_list = list(msg)
        msg_len = len(msg_list) - 4
        if msg_len < 128:
            msg_list[3] = msg_len
        else:
            msg_list[3] = 0x80 + msg_len//256
            msg_list[4] = (msg_len-1)%256
        return str(bytearray(msg_list))

    def calc_res(self, rand):
        opc = b'\x46\x09\x80\x00\x1E\x73\x8A\x38\x5C\xE2\x73\x9D\xA2\x63\xC0\x9E'
        key = b'\x46\x09\x80\x00\x1F\xAB\x28\x3F\x94\xAD\xF2\x65\xDC\x10\x10\xEF'
        mil = Milenage(None)
        mil.set_opc(opc)
        res, ck, ik, ak = mil.f2345(key, rand)
        return res, ck, ik, ak
    
    # S1Setup
    def S1Setup(self):
        S1SetupRequest_b = [0x00,0x15,0x00,0x25, 0x00,0x00,0x03,0x00,
                            0x66,0x00,0x0d,0x00, 0x00,0x00,0x00,0x01,
                            0x00,0x64,0xf0,0x89, 0x00,0x00,0x00,0x00,
                            0x00,0x15,0x40,0x01, 0x00,0x00,0x1b,0x00,
                            0x08,0x00,0x64,0xf0, 0x89,0x10,0x12,0x01,
                            0x66]
        #S1SetupRequest_b[19] = self.enb_id * 16
        S1SetupRequest_msg = str(bytearray(S1SetupRequest_b))
        self.sock.sctp_send(S1SetupRequest_msg)
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)
        msg_list = [ord(c) for c in msg]
        print msg_list

    # NGSetup
    def NGSetup(self):
        NGSetupRequest_b = [0x00,0x15,0x00,0x28, 0x00,0x00,0x03,0x00,
                            0x66,0x00,0x10,0x00, 0x00,0x12,0x01,0x01,
                            0x00,0x64,0xf0,0x89, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x15,0x40,0x01, 0x00,0x00,0x1b,0x00,
                            0x08,0x00,0x64,0xf0, 0x89,0x10,0x12,0x01,
                            0x66]
        #NGSetupRequest_b[19] = self.enb_id * 16
        NGSetupRequest_msg = str(bytearray(NGSetupRequest_b))
        self.sock.sctp_send(NGSetupRequest_msg)
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)
        msg_list = [ord(c) for c in msg]
        print msg_list


    def send_initialUeMessage(self, ue):
        msg = str(bytearray(InitialUEMessage_HEADER
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_NAS_PDU_12+ue.eps_mob_id+UE_net_cap+ESM_msg_con+Dev_prop_MS_net_feature
            +Item_id_TAI
            +Item_id_EUTRAN_CGI
            +Item_id_RRC_Establishment_Cause))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_downlinkNASTransport_AuthReq(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)
        print "msg=%s"%(self.msg_to_hexString(msg))
        MME_UE_S1AP_ID = self.get_value_by_type(msg, 0)
        NAS_PDU = self.get_value_by_type(msg, 26)
        if ord(NAS_PDU[0]) != 0x24:
            return None,None,None

        RAND = NAS_PDU[4:20]
        SQN_X_AK = NAS_PDU[21:27]
        return MME_UE_S1AP_ID,RAND,SQN_X_AK

    def send_uplinkNASTransport_AuthResp(self, ue, res):
        msg = str(bytearray(UplinkNASTransport_HEADER_AuthResp
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_NAS_PDU_13_AuthResp+res
            +Item_id_EUTRAN_CGI
            +Item_id_TAI))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_downlinkNASTransport_SecModeCmd(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def send_uplinkNASTransport_SecModeComplete(self, ue):
        nas_data = [0x00, 0x07, 0x5e]
        mac = self.snow3g_EIA1(ue.knasint, 0, 0, str(bytearray(nas_data)))
        for i in range(4):
            Item_id_NAS_PDU_13_SecModeComplete[6+i] = mac[i]

        msg = str(bytearray(UplinkNASTransport_HEADER_SecModeComplete
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_NAS_PDU_13_SecModeComplete+nas_data
            +Item_id_EUTRAN_CGI
            +Item_id_TAI))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_downlinkNASTransport_EsmInfoReq(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def send_uplinkNASTransport_EsmInfoResp(self, ue):
        nas_data = [0x01, 0x02, 0x01, 0xda, 0x28, 0x09, 0x08, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74]
        mac = self.snow3g_EIA1(ue.knasint, 1, 0, str(bytearray(nas_data)))
        for i in range(4):
            Item_id_NAS_PDU_13_EsmInfoResp[6+i] = mac[i]

        msg = str(bytearray(UplinkNASTransport_HEADER_EsmInfoResp
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_NAS_PDU_13_EsmInfoResp+nas_data
            +Item_id_EUTRAN_CGI
            +Item_id_TAI))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_InitialContextSetupRequest(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)
        print 'recv_InitialContextSetupRequest==>'
        ID_E_RABTOBESETUP = self.get_value_by_type(msg, 24)
        EPC_GTPU_IP = ID_E_RABTOBESETUP[11:15]
        print '\tEPC_GTPU_IP=%s'%(self.msg_to_hexString(EPC_GTPU_IP))
        UL_TEID = ID_E_RABTOBESETUP[15:19]
        print '\tUL_TEID=%s'%(self.msg_to_hexString(UL_TEID))
        nas_pdu_off = 20
        if ord(ID_E_RABTOBESETUP[nas_pdu_off+7]) == 0x42:
            esm_msg_off = nas_pdu_off + 17
            eps_qos_off = esm_msg_off + 5
            ap_name_off = eps_qos_off + ord(ID_E_RABTOBESETUP[eps_qos_off]) + 1
            pdn_addr_off = ap_name_off + ord(ID_E_RABTOBESETUP[ap_name_off]) + 1
            if ord(ID_E_RABTOBESETUP[pdn_addr_off+1]) == 1:
                PDN_ADDR = ID_E_RABTOBESETUP[pdn_addr_off+2:pdn_addr_off+6]
                print '\tPDN_ADDR=%s'%(self.msg_to_hexString(PDN_ADDR))
                return EPC_GTPU_IP,UL_TEID,PDN_ADDR
        
        print '\tPDN_ADDR=None'
        return None,None,None



    def send_UECapbilityInfoIndication(self, ue):
        msg = str(bytearray(UECapbilityInfoIndication_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_UERadioCapability))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def send_InitialContextSetupResponse(self, ue):
        #ip = [0x1e, 0x0c, 0x16, 0x00]  #30.12.22.0
        #ip = [0x1e, 0x0c, 0x17, 0x1a] #30.12.23.26
        #ip = [0x1e, 0x0c, 0x16, 0x1f] #30.12.22.31
        ip = [0x1e, 0x06, 0x21, 0x66]  #30.6.33.66
        msg = str(bytearray(InitialContextSetupResponse_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_E_RABSetupListCtxtSURes+ip+ue.s1u_dl_teid))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def send_uplinkNASTransport_AttachComplete(self, ue):
        nas_data = [0x02, 0x07, 0x43, 0x00, 0x03, 0x52, 0x00, 0xc2]
        mac = self.snow3g_EIA1(ue.knasint, 2, 0, str(bytearray(nas_data)))
        for i in range(4):
            Item_id_NAS_PDU_13_AttachComplete[6+i] = mac[i]

        msg = str(bytearray(UplinkNASTransport_HEADER_AttachComplete
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_NAS_PDU_13_AttachComplete+nas_data
            +Item_id_EUTRAN_CGI
            +Item_id_TAI))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_downlinkNASTransport_EmmInfo(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def ue_attach(self, ue):
        # Step1: send initialUEMessage
        self.send_initialUeMessage(ue)
        
        # Step2:  recv downlinkNASTransport, Auth Req
        #               parse id_MME_UE_S1AP_ID,RAND
        mme_ue_s1ap_id,rand,sqn_x_ak = self.recv_downlinkNASTransport_AuthReq()
        res, ck, ik, ak = self.calc_res(rand)
        
        kasme = conv_A2(ck, ik, self.plmn_id, sqn_x_ak)
        knasint = conv_A7(kasme, 0x02, 0x0001)

        ue.set_mme_ue_s1ap_id(list(mme_ue_s1ap_id))
        ue.knasint = knasint[16:]
        print 'mme_ue_s1ap_id=%s'%(self.msg_to_hexString(list(mme_ue_s1ap_id)))
        print 'knasint=%s'%(self.msg_to_hexString(ue.knasint))
        
        # Step3: send uplinkNASTransport, Auth Resp
        self.send_uplinkNASTransport_AuthResp(ue, list(res))
        
        # Step4: recv downlinkNASTransport Sec Mode Cmd
        self.recv_downlinkNASTransport_SecModeCmd()
        
        # Step5: send uplinkNASTransport Sec Mode Complete
        self.send_uplinkNASTransport_SecModeComplete(ue)
        
        # Step6: recv downlinkNASTransport , ESM information request
        self.recv_downlinkNASTransport_EsmInfoReq()
        
        # Step7: send uplinkNASTransport , ESM information response
        self.send_uplinkNASTransport_EsmInfoResp(ue)
        
        # Step8: recv InitialContextSetupRequest
        epc_gtpu_ip,ul_teid,pdn_addr = self.recv_InitialContextSetupRequest()
        if epc_gtpu_ip is not None:
            ue.update_in_attach(epc_gtpu_ip, ul_teid, pdn_addr)
            print "epc_gtpu_ip=%s, teid=%d, pdn_addr=%s s1u_dl_teid=%s"%(ue.epc_gtpu_ip,
                    ue.s1u_ul_teid, ue.pdn_addr, self.msg_to_hexString(str(bytearray(ue.s1u_dl_teid))))
        
        # Step9: send UECapbilityInfoIndication
        self.send_UECapbilityInfoIndication(ue)
        
        # Step10: send InitialContextSetupResponse
        self.send_InitialContextSetupResponse(ue)
        
        # Step11: send uplinkNAATransport, Attach complete
        self.send_uplinkNASTransport_AttachComplete(ue)
        
        # Step12: recv downlinkNASTransport, EMM info
        self.recv_downlinkNASTransport_EmmInfo()

    #########################################################################
    # APIs for handover
    #########################################################################
    def send_HandoverRequired(self, ue, t_end_id):
        Item_id_TargetID[12] = t_end_id * 16;
        Item_id_SourceToTargetTransparentContainer[207] = t_end_id * 16;
        Item_id_SourceToTargetTransparentContainer[215] = self.enb_id * 16;
        Item_id_SourceToTargetTransparentContainer[227] = t_end_id * 16;
        msg = str(bytearray(HandoverRequired_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_HandoverType 
            +Item_id_Cause
            +Item_id_TargetID
            +Item_id_SourceToTargetTransparentContainer))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_HandoverRequest(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)
        MME_UE_S1AP_ID = self.get_value_by_type(msg, 0)
        return MME_UE_S1AP_ID

    def send_HandoverRequestAcknowledge(self, ue):
        msg = str(bytearray(HandoverRequestAcknowledge_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_ERabAdmittedList
            +Item_id_SourceToTargetTransparentContainer_HoReqAck))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_HandoverCommand(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def send_EnbStatusTransfer(self, ue):
        msg = str(bytearray(EnbStatusTransfer_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_EnbStatusTransferTransparentContainer))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_MmeStatusTransfer(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def send_HandoverNotify(self, ue):
        msg = str(bytearray(HandoverNotify_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)
            +Item_id_EUTRAN_CGI
            +Item_id_TAI))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def recv_UeCtxReleaseCommand(self):
        fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def send_UeCtxReleaseComplete(self, ue):
        msg = str(bytearray(UeCtxReleaseComplete_HEADER
            +ue.mme_ue_s1ap_id
            +self.gen_enb_ue_s1ap_id(ue.enb_ue_s1ap_id)))
        self.sock.sctp_send(self.set_s1ap_len(msg))

    def handover_S_enb(self, ue, t_end_id):
        # HO1: send HandoverRequired
        self.send_HandoverRequired(ue, t_end_id)

        # HO4: recv HandoverCommand, RRCConnectionReconfiguration
        self.recv_HandoverCommand()

        # HO5: send EnbStatusTransfer
        self.send_EnbStatusTransfer(ue)

        # HO8: GTP-U End Markers

        # HO9: recv UECtxReleaseCommand
        self.recv_UeCtxReleaseCommand()

        # HO10: send UECtxReleaseComplete
        self.send_UeCtxReleaseComplete(ue)

    def handover_T_enb(self, ue):
        # HO2: recv HandoverRequest
        mme_ue_s1ap_id = self.recv_HandoverRequest()

        ue.set_mme_ue_s1ap_id(list(mme_ue_s1ap_id))

        # HO3: send HandoverRequestAcknowledge
        self.send_HandoverRequestAcknowledge(ue)

        # HO6: recv MMEStatusTransfer
        self.recv_MmeStatusTransfer()

        # HO7: send HandoverNotify
        self.send_HandoverNotify(ue)

    def sctp_recv_loop(self):
        while self.loop:
            print "+++++++++++++++++++++++++++++++++++++++++"
            fromaddr, flags, msg, notif = self.sock.sctp_recv(1024)

    def one_ue_test(self):
        self.sctp_init()
        enb_ue_s1ap_id = [1,2]
        eps_mod_id = [[0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10],
                [0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x20]]
        ue1 = UE(eps_mod_id[0],enb_ue_s1ap_id[0], None)
        ue2 = UE(eps_mod_id[1],enb_ue_s1ap_id[1], None)
        
        # NGSetup
        self.NGSetup()
        
        # UE attach
        #self.ue_attach(ue1)
        #self.ue_attach(ue2)

        time.sleep(5)
        self.sctp_uninit()

    def multi_ue_attach_test(self, n):
        enb_ue_s1ap_id = [ i+1 for i in range(n)]
        eps_mod_id = [[0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10] for i in range(n)]

        for i in range(n):
            num_7 = (i+1)/100
            num_8 = (i+1)%100
            eps_mod_id[i][7] = (num_7%10)*16 + (num_7/10)
            eps_mod_id[i][8] = (num_8%10)*16 + (num_8/10)

        print enb_ue_s1ap_id
        print eps_mod_id

        ue = []

        for i in range(n):
            ue.append(UE(eps_mod_id[i],enb_ue_s1ap_id[i], None))
        
        self.sctp_init()

        # S1Setup
        self.S1Setup()

        for i in range(n):
            self.ue_attach(ue[i])
            #time.sleep(0.1)

        time.sleep(30)
        self.sctp_uninit()

    def concurrent_gtp_ul_test(self):
        n = 200
        enb_ue_s1ap_id = [[0x00, 0x00] for i in range(n)]
        eps_mod_id = [[0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10] for i in range(n)]

        for i in range(n):
            enb_ue_s1ap_id[i][0] = (i+1)/256
            enb_ue_s1ap_id[i][1] = (i+1)%256
            num_7 = (i+1)/100
            num_8 = (i+1)%100
            eps_mod_id[i][7] = (num_7%10)*16 + (num_7/10)
            eps_mod_id[i][8] = (num_8%10)*16 + (num_8/10)

        print eps_mod_id

        ue = []

        for i in range(n):
            ue.append(UE(eps_mod_id[i],enb_ue_s1ap_id[i]))
        
        
        ueSender = UeSender(self.epc_ip, self.srv_ip)

        self.sctp_init()

        for i in range(n):
            self.ue_attach(ue[i])

        for i in range(n):
            ueSender.add_ue(ue[i])
        
        ueSender.ul_start()

        time.sleep(30)
        self.sctp_uninit()

    def concurrent_gtp_dl_test(self):
        n = 200
        enb_ue_s1ap_id = [[0x00, 0x00] for i in range(n)]
        eps_mod_id = [[0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10] for i in range(n)]

        for i in range(n):
            enb_ue_s1ap_id[i][0] = (i+1)/256
            enb_ue_s1ap_id[i][1] = (i+1)%256
            num_7 = (i+1)/100
            num_8 = (i+1)%100
            eps_mod_id[i][7] = (num_7%10)*16 + (num_7/10)
            eps_mod_id[i][8] = (num_8%10)*16 + (num_8/10)

        print eps_mod_id

        ue = []

        for i in range(n):
            ue.append(UE(eps_mod_id[i],enb_ue_s1ap_id[i]))
        
        
        ueSender = UeSender(self.epc_ip, self.srv_ip)

        self.sctp_init()

        for i in range(n):
            self.ue_attach(ue[i])

        for i in range(n):
            ueSender.add_ue(ue[i])
        
        ueSender.report_ue_addr_to_srv()

        time.sleep(30)
        self.sctp_uninit()

    def ho_test(self, mode):
        self.sctp_init()
        enb_ue_s1ap_id = [1,2]
        eps_mod_id = [[0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10],
                [0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x20]]
        ue1 = UE(eps_mod_id[0],enb_ue_s1ap_id[0], None)
        ue2 = UE(eps_mod_id[1],enb_ue_s1ap_id[1], None)
        
        # S1Setup
        self.S1Setup()
        
        # UE attach
        if mode == 'src':
            self.ue_attach(ue1)
            #self.ue_attach(ue2)
            self.handover_S_enb(ue1, 8)
        else:
            self.handover_T_enb(ue1)

        time.sleep(20)
        self.sctp_uninit()



if __name__ == '__main__':
    #enb_id, enb_ip, epc_ip, srv_ip
    #enb = Enb(7, '30.6.33.66', '30.7.99.245', '30.12.22.0')
    #enb.ho_test('src')
    enb = Enb(7, '30.6.33.66', '30.6.33.121', '30.12.22.0')
    #enb.ho_test('dst')
    enb.one_ue_test()
    #enb.multi_ue_attach_test(5000)
    #enb.concurrent_gtp_ul_test()
    #enb.concurrent_gtp_dl_test()
