import socket
import copy

class UE:
    def __init__(self, eps_mob_id, enb_ue_s1ap_id, pdn_addr):
        #enb_ue_s1ap_id = [0x00, 0x07]
        #EPS_mob_id = [0x08, 0x49, 0x06, 0x89, 0x00, 0x10, 0x00, 0x00, 0x10]
        self.eps_mob_id = eps_mob_id
        self.enb_ue_s1ap_id = enb_ue_s1ap_id
        self.mme_ue_s1ap_id = None
        self.knasint = None
        self.epc_gtpu_ip = None
        self.s1u_ul_teid = None
        self.pdn_addr = None
        self.s1u_dl_teid = None
        self.ue_port = 0
        if pdn_addr is not None:
            self.pdn_addr = copy.deepcopy(socket.inet_ntoa(pdn_addr))

    def update_in_attach(self, epc_gtpu_ip, s1u_ul_teid, pdn_addr):
        self.epc_gtpu_ip = copy.deepcopy(socket.inet_ntoa(epc_gtpu_ip))
        self.s1u_ul_teid = copy.deepcopy(int(s1u_ul_teid.encode('hex'), 16))
        self.pdn_addr = copy.deepcopy(socket.inet_ntoa(pdn_addr))
        self.s1u_dl_teid = copy.deepcopy(list(s1u_ul_teid))
        self.s1u_dl_teid[0] = 0x01
        self.srv_port = 61000 + self.s1u_ul_teid

    def set_mme_ue_s1ap_id(self, v):
        if len(v) == 2:
            self.mme_ue_s1ap_id = [0x00, 0x00, 0x00, 0x02] + v
        elif len(v) == 3:
            self.mme_ue_s1ap_id = [0x00, 0x00, 0x00, 0x03] + v
