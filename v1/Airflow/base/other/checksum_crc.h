#pragma once
#define MD5_DIGEST_LENGTH 16

typedef struct
{
	unsigned int buf[4];
	unsigned int bits[2];
	unsigned char in[64];
} MD5Context_t;

void MD5Init(MD5Context_t* context);
void MD5Update(MD5Context_t* context, unsigned char const* buf, unsigned int len);
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t* context);

char* MD5_Print(unsigned char* digest, int hashlen);

unsigned int MD5_PseudoRandom(unsigned int nSeed);

typedef unsigned long crc32_t;

void crc32_Init(crc32_t* pul_crc);
void crc32_process_buffer(crc32_t* pul_crc, const void* p, int len);
void crc32_final(crc32_t* pul_crc);
crc32_t crc32_get_table_entry(unsigned int slot);

inline crc32_t CRC32_ProcessSingleBuffer(const void* p, int len)
{
	crc32_t crc;

	crc32_Init(&crc);
	crc32_process_buffer(&crc, p, len);
	crc32_final(&crc);

	return crc;
}