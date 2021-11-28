#ifndef __UPF_LIBGTPNL_H__
#define __UPF_LIBGTPNL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) upf_libgtpnl_init();
CORE_DECLARE(status_t) upf_libgtpnl_uninit(void);
CORE_DECLARE(status_t) upf_libgtpnl_add_tunnel(struct in_addr *ue, struct in_addr *enb, c_uint32_t i_tei, c_uint32_t o_tei, c_uint8_t bearer_id);
CORE_DECLARE(status_t) upf_libgtpnl_del_tunnel(struct in_addr *ue, c_uint32_t i_tei, c_uint32_t o_tei);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPF_LIBGTPNL_H__ */
