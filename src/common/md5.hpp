/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */
#ifndef MD5_HPP_
#define MD5_HPP_

#include <cstdint> 
 

struct MD5_CTX
{
    uint32_t lo, hi;
    uint32_t abcd[4];
    unsigned char buffer[64];
    uint32_t block[16];
};

 
class cMD5
{
public:
    cMD5 ();
    void update(const void *data, size_t size);
    const uint8_t* final();
    const uint8_t* calc(const void *data, size_t size);
   
private:
    void init ();
    MD5_CTX m_ctx;

#ifdef WITH_UNITTESTS
public:
    static void unitTest ();
#endif
};
 #endif
 