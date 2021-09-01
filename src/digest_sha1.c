#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "digest_sha1.h"

#define S(N, X) (((X)<<(N))|((X)>>(32-(N))))
const uint32_t SHA1_KK[4] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
uint32_t SHA1_K;

unsigned char *Digest_SHA1(unsigned char *msg, ssize_t length)
{
	unsigned char *hdigest = malloc(sizeof(unsigned char)*41);
	unsigned char digest[20];
	unsigned char *pmsg;
	ssize_t blocks;
	char i;
	unsigned char c[2];

	hdigest[40] = '\0';
	pmsg = sha1_padding(msg, &blocks, length);
	sha1_process(pmsg, blocks, digest);
	free(pmsg);

	for(i=0; i<20; i++) {
		sprintf(c, "%02x", digest[i]);
		hdigest[i*2] = c[0];
		hdigest[i*2+1] = c[1];
	}

	return hdigest;
}

unsigned char *sha1_padding(unsigned char *msg, ssize_t *blocks, ssize_t length)
{
	unsigned char *pmsg = 0;
	ssize_t tmp = length % 64;
	ssize_t psize = sizeof(char)*length;
	ssize_t bsize = sizeof(char)*64;
	uint64_t bits = length*8;

	*blocks = length/64+1;
	*blocks += (tmp>55)?1:0;
	pmsg = realloc(pmsg, bsize*(*blocks));
	memcpy(pmsg, msg, psize); 
	memset(pmsg+psize, 0, bsize-tmp);
	if(tmp>55) memset(pmsg+((*blocks)-1)*64, 0, 64);
	pmsg[psize] = 0x80;

	pmsg[bsize*(*blocks)-8] |= bits>>56;
	pmsg[bsize*(*blocks)-7] |= (bits>>48) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-6] |= (bits>>40) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-5] |= (bits>>32) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-4] |= (bits>>24) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-3] |= (bits>>16) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-2] |= (bits>>8) & 0x00000000000000FF;
	pmsg[bsize*(*blocks)-1] |= bits & 0x00000000000000FF;

	return pmsg;
}

uint32_t sha1_func(char t, uint32_t B, uint32_t C, uint32_t D)
{
	uint32_t f = 0;
	if(t >= 0 && t <= 19) {
		f = (B & C) | ((~B) & D);
		SHA1_K = SHA1_KK[0];
	} else if(t >= 20 && t <= 39) {
		f = B ^ C ^ D;
		SHA1_K = SHA1_KK[1];
	} else if(t >= 40 && t <= 59) {
		f = (B & C) | (B & D) | (C & D);
		SHA1_K = SHA1_KK[2];
	} else if(t >= 60 && t <= 79) {
		f = B ^ C ^ D;
		SHA1_K = SHA1_KK[3];
	}
	return f;
}

void sha1_process(unsigned char *pmsg, ssize_t blocks, unsigned char *digest)
{
	char t;
	uint32_t W[80];
	uint32_t A, B, C, D, E;
	uint32_t H[] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
	uint32_t TEMP;

	while(blocks--) {
		/* Step A */
		for(t = 0; t < 16; t++) {
			W[t] = pmsg[t*4] << 24;
			W[t] |= pmsg[t*4+1] << 16;
			W[t] |= pmsg[t*4+2] << 8;
			W[t] |= pmsg[t*4+3];
		}
		/* Step B */
		for(t = 16; t < 80; t++)
			W[t] = S(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
		/* Step C */
		A = H[0]; B = H[1]; C = H[2]; D = H[3]; E = H[4];
		/* Step D */
		for(t=0; t < 80; t++) {
			TEMP = S(5, A) + sha1_func(t, B, C, D) + E + W[t] + SHA1_K;
			E = D; D = C; C = S(30, B); B = A; A = TEMP;
		}
		/* Step E */
		H[0] += A; H[1]+= B; H[2] += C; H[3] += D; H[4] += E;

		pmsg += 64;
	}

	for(t=0; t<5; t++) {
		digest[t*4] = (H[t]>>24) & 0x000000ff;
		digest[t*4+1] = (H[t]>>16) & 0x000000ff;
		digest[t*4+2] = (H[t]>>8) & 0x000000ff;
		digest[t*4+3] = H[t] & 0x000000ff;
	}
}
