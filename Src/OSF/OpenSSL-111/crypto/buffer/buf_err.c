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
//#include <openssl/err.h>
//#include <openssl/buffererr.h>

#ifndef OPENSSL_NO_ERR

static const ERR_STRING_DATA BUF_str_functs[] = {
	{ERR_PACK(ERR_LIB_BUF, BUF_F_BUF_MEM_GROW, 0), "BUF_MEM_grow"},
	{ERR_PACK(ERR_LIB_BUF, BUF_F_BUF_MEM_GROW_CLEAN, 0), "BUF_MEM_grow_clean"},
	{ERR_PACK(ERR_LIB_BUF, BUF_F_BUF_MEM_NEW, 0), "BUF_MEM_new"},
	{0, NULL}
};

static const ERR_STRING_DATA BUF_str_reasons[] = {
	{0, NULL}
};

#endif

int ERR_load_BUF_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if(ERR_func_error_string(BUF_str_functs[0].error) == NULL) {
		ERR_load_strings_const(BUF_str_functs);
		ERR_load_strings_const(BUF_str_reasons);
	}
#endif
	return 1;
}
