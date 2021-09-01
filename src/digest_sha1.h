#ifndef DIGEST_SHA1_H
#define DIGEST_SHA1_H

#ifndef FREEBSD4
#include <stdint.h>
#endif
#include <sys/types.h>

unsigned char *Digest_SHA1(unsigned char *msg, ssize_t length);

unsigned char *sha1_padding(unsigned char *msg, ssize_t *blocks, ssize_t length);
uint32_t sha1_func(char t, uint32_t B, uint32_t C, uint32_t D);
void sha1_process(unsigned char *pmsg, ssize_t blocks, unsigned char *digest);

#endif
