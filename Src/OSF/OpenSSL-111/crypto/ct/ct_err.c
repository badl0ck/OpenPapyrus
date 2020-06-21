/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#include "internal/cryptlib.h"
#pragma hdrstop
#include <openssl/err.h>
#include <openssl/cterr.h>

#ifndef OPENSSL_NO_ERR

static const ERR_STRING_DATA CT_str_functs[] = {
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_NEW, 0), "CTLOG_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_NEW_FROM_BASE64, 0),
	 "CTLOG_new_from_base64"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_NEW_FROM_CONF, 0), "ctlog_new_from_conf"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_STORE_LOAD_CTX_NEW, 0),
	 "ctlog_store_load_ctx_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_STORE_LOAD_FILE, 0),
	 "CTLOG_STORE_load_file"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_STORE_LOAD_LOG, 0),
	 "ctlog_store_load_log"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CTLOG_STORE_NEW, 0), "CTLOG_STORE_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CT_BASE64_DECODE, 0), "ct_base64_decode"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CT_POLICY_EVAL_CTX_NEW, 0),
	 "CT_POLICY_EVAL_CTX_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_CT_V1_LOG_ID_FROM_PKEY, 0),
	 "ct_v1_log_id_from_pkey"},
	{ERR_PACK(ERR_LIB_CT, CT_F_I2O_SCT, 0), "i2o_SCT"},
	{ERR_PACK(ERR_LIB_CT, CT_F_I2O_SCT_LIST, 0), "i2o_SCT_LIST"},
	{ERR_PACK(ERR_LIB_CT, CT_F_I2O_SCT_SIGNATURE, 0), "i2o_SCT_signature"},
	{ERR_PACK(ERR_LIB_CT, CT_F_O2I_SCT, 0), "o2i_SCT"},
	{ERR_PACK(ERR_LIB_CT, CT_F_O2I_SCT_LIST, 0), "o2i_SCT_LIST"},
	{ERR_PACK(ERR_LIB_CT, CT_F_O2I_SCT_SIGNATURE, 0), "o2i_SCT_signature"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_CTX_NEW, 0), "SCT_CTX_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_CTX_VERIFY, 0), "SCT_CTX_verify"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_NEW, 0), "SCT_new"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_NEW_FROM_BASE64, 0), "SCT_new_from_base64"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET0_LOG_ID, 0), "SCT_set0_log_id"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET1_EXTENSIONS, 0), "SCT_set1_extensions"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET1_LOG_ID, 0), "SCT_set1_log_id"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET1_SIGNATURE, 0), "SCT_set1_signature"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET_LOG_ENTRY_TYPE, 0),
	 "SCT_set_log_entry_type"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET_SIGNATURE_NID, 0),
	 "SCT_set_signature_nid"},
	{ERR_PACK(ERR_LIB_CT, CT_F_SCT_SET_VERSION, 0), "SCT_set_version"},
	{0, NULL}
};

static const ERR_STRING_DATA CT_str_reasons[] = {
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_BASE64_DECODE_ERROR), "base64 decode error"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_INVALID_LOG_ID_LENGTH),
	 "invalid log id length"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_INVALID), "log conf invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_INVALID_KEY),
	 "log conf invalid key"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_MISSING_DESCRIPTION),
	 "log conf missing description"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_CONF_MISSING_KEY),
	 "log conf missing key"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_LOG_KEY_INVALID), "log key invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_FUTURE_TIMESTAMP),
	 "sct future timestamp"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_INVALID), "sct invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_INVALID_SIGNATURE),
	 "sct invalid signature"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_LIST_INVALID), "sct list invalid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_LOG_ID_MISMATCH), "sct log id mismatch"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_NOT_SET), "sct not set"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_SCT_UNSUPPORTED_VERSION),
	 "sct unsupported version"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNRECOGNIZED_SIGNATURE_NID),
	 "unrecognized signature nid"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNSUPPORTED_ENTRY_TYPE),
	 "unsupported entry type"},
	{ERR_PACK(ERR_LIB_CT, 0, CT_R_UNSUPPORTED_VERSION), "unsupported version"},
	{0, NULL}
};

#endif

int ERR_load_CT_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if(ERR_func_error_string(CT_str_functs[0].error) == NULL) {
		ERR_load_strings_const(CT_str_functs);
		ERR_load_strings_const(CT_str_reasons);
	}
#endif
	return 1;
}
