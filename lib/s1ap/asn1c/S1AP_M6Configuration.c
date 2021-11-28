/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "S1AP-IEs"
 * 	found in "../support/r14.4.0/36413-e40.asn"
 * 	`asn1c -pdu=all -fcompound-names -findirect-choice -fno-include-deps`
 */

#include "S1AP_M6Configuration.h"

#include "S1AP_ProtocolExtensionContainer.h"
static asn_TYPE_member_t asn_MBR_S1AP_M6Configuration_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct S1AP_M6Configuration, m6report_Interval),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1AP_M6report_Interval,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"m6report-Interval"
		},
	{ ATF_POINTER, 1, offsetof(struct S1AP_M6Configuration, m6delay_threshold),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1AP_M6delay_threshold,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"m6delay-threshold"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct S1AP_M6Configuration, m6_links_to_log),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1AP_Links_to_log,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"m6-links-to-log"
		},
	{ ATF_POINTER, 1, offsetof(struct S1AP_M6Configuration, iE_Extensions),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_S1AP_ProtocolExtensionContainer_6625P70,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"iE-Extensions"
		},
};
static const int asn_MAP_S1AP_M6Configuration_oms_1[] = { 1, 3 };
static const ber_tlv_tag_t asn_DEF_S1AP_M6Configuration_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_S1AP_M6Configuration_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* m6report-Interval */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* m6delay-threshold */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* m6-links-to-log */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* iE-Extensions */
};
static asn_SEQUENCE_specifics_t asn_SPC_S1AP_M6Configuration_specs_1 = {
	sizeof(struct S1AP_M6Configuration),
	offsetof(struct S1AP_M6Configuration, _asn_ctx),
	asn_MAP_S1AP_M6Configuration_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_S1AP_M6Configuration_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	4,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_S1AP_M6Configuration = {
	"M6Configuration",
	"M6Configuration",
	&asn_OP_SEQUENCE,
	asn_DEF_S1AP_M6Configuration_tags_1,
	sizeof(asn_DEF_S1AP_M6Configuration_tags_1)
		/sizeof(asn_DEF_S1AP_M6Configuration_tags_1[0]), /* 1 */
	asn_DEF_S1AP_M6Configuration_tags_1,	/* Same as above */
	sizeof(asn_DEF_S1AP_M6Configuration_tags_1)
		/sizeof(asn_DEF_S1AP_M6Configuration_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_S1AP_M6Configuration_1,
	4,	/* Elements count */
	&asn_SPC_S1AP_M6Configuration_specs_1	/* Additional specs */
};

