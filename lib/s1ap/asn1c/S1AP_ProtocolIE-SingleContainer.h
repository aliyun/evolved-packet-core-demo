/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-Containers"
 * 	found in "../support/r14.4.0/36413-e40.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps`
 */

#ifndef	_S1AP_ProtocolIE_SingleContainer_H_
#define	_S1AP_ProtocolIE_SingleContainer_H_


#include <asn_application.h>

/* Including external dependencies */
#include "S1AP_ProtocolIE-Field.h"

#ifdef __cplusplus
extern "C" {
#endif

/* S1AP_ProtocolIE-SingleContainer */
typedef S1AP_E_RABToBeSetupItemBearerSUReqIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P0_t;
typedef S1AP_E_RABSetupItemBearerSUResIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P1_t;
typedef S1AP_E_RABToBeModifiedItemBearerModReqIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P2_t;
typedef S1AP_E_RABModifyItemBearerModResIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P3_t;
typedef S1AP_E_RABReleaseItemBearerRelCompIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P4_t;
typedef S1AP_E_RABToBeSetupItemCtxtSUReqIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P5_t;
typedef S1AP_E_RABSetupItemCtxtSUResIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P6_t;
typedef S1AP_TAIItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P7_t;
typedef S1AP_UE_associatedLogicalS1_ConnectionItemRes_t	 S1AP_ProtocolIE_SingleContainer_6577P8_t;
typedef S1AP_UE_associatedLogicalS1_ConnectionItemResAck_t	 S1AP_ProtocolIE_SingleContainer_6577P9_t;
typedef S1AP_E_RABModifyItemBearerModConfIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P10_t;
typedef S1AP_Bearers_SubjectToStatusTransfer_ItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P11_t;
typedef S1AP_E_RABInformationListIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P12_t;
typedef S1AP_E_RABItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P13_t;
typedef S1AP_MDTMode_ExtensionIE_t	 S1AP_ProtocolIE_SingleContainer_6577P14_t;
typedef S1AP_RecommendedCellItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P15_t;
typedef S1AP_RecommendedENBItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P16_t;
typedef S1AP_SONInformation_ExtensionIE_t	 S1AP_ProtocolIE_SingleContainer_6577P17_t;
typedef S1AP_E_RABDataForwardingItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P18_t;
typedef S1AP_E_RABToBeSetupItemHOReqIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P19_t;
typedef S1AP_E_RABAdmittedItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P20_t;
typedef S1AP_E_RABFailedtoSetupItemHOReqAckIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P21_t;
typedef S1AP_E_RABToBeSwitchedDLItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P22_t;
typedef S1AP_E_RABToBeSwitchedULItemIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P23_t;
typedef S1AP_E_RABToBeModifiedItemBearerModIndIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P24_t;
typedef S1AP_E_RABNotToBeModifiedItemBearerModIndIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P25_t;
typedef S1AP_E_RABFailedToResumeItemResumeReqIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P26_t;
typedef S1AP_E_RABFailedToResumeItemResumeResIEs_t	 S1AP_ProtocolIE_SingleContainer_6577P27_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P0;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P0_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P0_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P0_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P0_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P0_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P0_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P0_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P0_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P0_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P0_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P0_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P0_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P0_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P1;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P1_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P1_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P1_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P1_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P1_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P1_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P1_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P1_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P1_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P1_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P1_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P1_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P1_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P2;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P2_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P2_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P2_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P2_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P2_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P2_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P2_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P2_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P2_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P2_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P2_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P2_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P2_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P3;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P3_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P3_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P3_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P3_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P3_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P3_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P3_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P3_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P3_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P3_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P3_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P3_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P3_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P4;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P4_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P4_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P4_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P4_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P4_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P4_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P4_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P4_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P4_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P4_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P4_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P4_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P4_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P5;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P5_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P5_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P5_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P5_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P5_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P5_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P5_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P5_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P5_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P5_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P5_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P5_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P5_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P6;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P6_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P6_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P6_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P6_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P6_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P6_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P6_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P6_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P6_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P6_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P6_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P6_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P6_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P7;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P7_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P7_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P7_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P7_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P7_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P7_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P7_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P7_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P7_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P7_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P7_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P7_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P7_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P8;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P8_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P8_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P8_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P8_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P8_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P8_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P8_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P8_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P8_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P8_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P8_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P8_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P8_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P9;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P9_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P9_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P9_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P9_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P9_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P9_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P9_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P9_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P9_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P9_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P9_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P9_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P9_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P10;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P10_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P10_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P10_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P10_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P10_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P10_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P10_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P10_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P10_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P10_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P10_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P10_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P10_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P11;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P11_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P11_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P11_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P11_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P11_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P11_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P11_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P11_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P11_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P11_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P11_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P11_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P11_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P12;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P12_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P12_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P12_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P12_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P12_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P12_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P12_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P12_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P12_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P12_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P12_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P12_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P12_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P13;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P13_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P13_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P13_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P13_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P13_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P13_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P13_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P13_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P13_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P13_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P13_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P13_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P13_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P14;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P14_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P14_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P14_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P14_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P14_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P14_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P14_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P14_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P14_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P14_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P14_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P14_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P14_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P15;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P15_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P15_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P15_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P15_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P15_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P15_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P15_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P15_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P15_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P15_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P15_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P15_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P15_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P16;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P16_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P16_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P16_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P16_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P16_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P16_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P16_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P16_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P16_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P16_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P16_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P16_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P16_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P17;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P17_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P17_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P17_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P17_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P17_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P17_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P17_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P17_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P17_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P17_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P17_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P17_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P17_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P18;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P18_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P18_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P18_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P18_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P18_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P18_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P18_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P18_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P18_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P18_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P18_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P18_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P18_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P19;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P19_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P19_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P19_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P19_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P19_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P19_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P19_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P19_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P19_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P19_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P19_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P19_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P19_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P20;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P20_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P20_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P20_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P20_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P20_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P20_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P20_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P20_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P20_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P20_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P20_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P20_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P20_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P21;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P21_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P21_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P21_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P21_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P21_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P21_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P21_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P21_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P21_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P21_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P21_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P21_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P21_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P22;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P22_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P22_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P22_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P22_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P22_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P22_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P22_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P22_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P22_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P22_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P22_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P22_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P22_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P23;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P23_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P23_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P23_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P23_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P23_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P23_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P23_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P23_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P23_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P23_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P23_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P23_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P23_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P24;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P24_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P24_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P24_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P24_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P24_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P24_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P24_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P24_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P24_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P24_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P24_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P24_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P24_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P25;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P25_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P25_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P25_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P25_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P25_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P25_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P25_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P25_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P25_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P25_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P25_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P25_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P25_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P26;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P26_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P26_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P26_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P26_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P26_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P26_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P26_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P26_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P26_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P26_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P26_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P26_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P26_encode_aper;
extern asn_TYPE_descriptor_t asn_DEF_S1AP_ProtocolIE_SingleContainer_6577P27;
asn_struct_free_f S1AP_ProtocolIE_SingleContainer_6577P27_free;
asn_struct_print_f S1AP_ProtocolIE_SingleContainer_6577P27_print;
asn_constr_check_f S1AP_ProtocolIE_SingleContainer_6577P27_constraint;
ber_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P27_decode_ber;
der_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P27_encode_der;
xer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P27_decode_xer;
xer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P27_encode_xer;
oer_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P27_decode_oer;
oer_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P27_encode_oer;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P27_decode_uper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P27_encode_uper;
per_type_decoder_f S1AP_ProtocolIE_SingleContainer_6577P27_decode_aper;
per_type_encoder_f S1AP_ProtocolIE_SingleContainer_6577P27_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _S1AP_ProtocolIE_SingleContainer_H_ */
#include <asn_internal.h>
