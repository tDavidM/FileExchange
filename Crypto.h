#pragma once

#ifndef TIGER_CRYPTO_H
#define TIGER_CRYPTO_H

#include "rijndael.h"
#include "sha256.h"
#include "Argon2.h"

typedef unsigned char byte;

static const uint32_t ARGON_HASH_SIZE = 32;
static const uint32_t ARGON_SALT_SIZE = 16;
static const uint32_t ARGON_MEM_BLOCK = 65536; // Memory cost: 64 Megabytes (65536 * 1024 bytes)
static const uint32_t TIGER_HASH_SIZE = 24;
static const uint32_t TIGER_BLOCK_SIZE = 64;
static const uint32_t SHA256_HASH_SIZE = 32;
static const uint32_t SHA256_BLOCK_SIZE = 64;
static const uint32_t AES_KEY_BITS = 256;
static const uint32_t AES_KEY_SIZE = 32;
static const uint32_t AES_BLOCK_SIZE = 16;

static uint8_t applicationSalt[24] = {0xdd, 0x00, 0x23, 0x07, 0x99, 0xf5, 0x00, 0x9f,
                                      0xec, 0x6d, 0xeb, 0xc8, 0x38, 0xbb, 0x6a, 0x27,
                                      0xdf, 0x2b, 0x9d, 0x6f, 0x11, 0x0c, 0x79, 0x37 };
static const uint32_t SESSION_SALT_SIZE = 32;
static const uint32_t PASSWORD_MAX_SIZE = 64;
static const uint32_t FILE_NONCE_SIZE = 8;
static const uint32_t FILE_IV_SIZE = 16;
static const uint32_t FILE_META_HMAC_SIZE = TIGER_HASH_SIZE;
static const uint32_t FILE_DATA_HMAC_SIZE = TIGER_HASH_SIZE;

//---------------------------------------------------------------------------

void GetTigerHash(const byte* src, uint32_t srcLength, byte* output);
bool GetSecureRandomBytes(byte* buffer, uint32_t bufferSize);
bool GetPasswordHash(const byte* password, uint32_t passwordSize, byte* outputSalt, byte* outputHash);
void SetPasswordHash(const byte* password, uint32_t passwordSize, const byte* inputSalt, byte* outputHash);
void GetHMAC_SHA256(const byte* key, uint32_t keySize, const byte* message, uint32_t messageSize, byte* outputBuffer);
void RunCipherNonce(byte* data, uint32_t dataSize, const byte* nonce, const byte* key);
void RunCipherIV(byte* data, uint32_t dataSize, byte* iv, const byte* key);
void GetHMAC_Tiger(const byte* key, uint32_t keySize, const byte* message, uint32_t messageSize, byte* outputBuffer);
//---------------------------------------------------------------------------

#endif
