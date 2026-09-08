//---------------------------------------------------------------------------

#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <malloc.h>

#include "Crypto.h"
#include "sha256.h"
#include "sha256.c"
#include "rijndael.h"
#include "rijndael.c"
#include "sboxes.c"
#include "tiger.c"

#pragma comment(lib, "bcrypt.lib")

//---------------------------------------------------------------------------

bool GetSecureRandomBytes(byte* buffer, uint32_t bufferSize)
{
   NTSTATUS status = BCryptGenRandom(NULL, buffer, bufferSize, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

   return (status == 0);
}
//---------------------------------------------------------------------------
void GetTigerHash(const byte* src, uint32_t srcLength, byte* output)
{
   //192-bit (24-bytes)
   tiger((word64*)src, srcLength, (word64*)output);
}
//---------------------------------------------------------------------------

bool GetPasswordHash(const byte* password, uint32_t passwordSize, byte* outputSalt, byte* outputHash)
{
   if (!GetSecureRandomBytes(outputSalt, ARGON_SALT_SIZE)) {
      return false;
   }

   SetPasswordHash(password, passwordSize, outputSalt, outputHash);

   return true;
}
//---------------------------------------------------------------------------

void SetPasswordHash(const byte* password, uint32_t passwordSize, const byte* inputSalt, byte* outputHash)
{
   void* workArea = _aligned_malloc(ARGON_MEM_BLOCK * 1024, 64);

   crypto_argon2_config argonConfig;
   argonConfig.algorithm = CRYPTO_ARGON2_ID; // Recommended standard hybrid variant
   argonConfig.nb_blocks = ARGON_MEM_BLOCK;  // Memory cost: 64 Megabytes (65536 * 1024 bytes)                   static_cast<size_t>(ARGON_MEM_BLOCK)
   argonConfig.nb_passes = 3;                // Time cost: 3 iterations over memory
   argonConfig.nb_lanes  = 1;                // Execution lanes (Monocypher runs single-threaded)

   crypto_argon2_inputs argonInputs;
   argonInputs.pass      = reinterpret_cast<const uint8_t*>(password);
   argonInputs.pass_size = passwordSize;
   argonInputs.salt      = reinterpret_cast<const uint8_t*>(inputSalt);
   argonInputs.salt_size = ARGON_SALT_SIZE;

   crypto_argon2_extras noExtras = crypto_argon2_no_extras;

   crypto_argon2(reinterpret_cast<uint8_t*>(outputHash), ARGON_HASH_SIZE,
                 workArea,
                 argonConfig, argonInputs,
                 noExtras);

   crypto_wipe(workArea, ARGON_MEM_BLOCK * 1024);

   _aligned_free(workArea);
}
//---------------------------------------------------------------------------

void GetHMAC_SHA256(const byte* key, uint32_t keySize, const byte* message, uint32_t messageSize, byte* outputBuffer)
{
   byte innerKey[SHA256_BLOCK_SIZE];
   byte outerKey[SHA256_BLOCK_SIZE];
   memset(innerKey, 0x00, SHA256_BLOCK_SIZE);
   memset(outerKey, 0x00, SHA256_BLOCK_SIZE);

   SHA256_CTX innerCtx;
   SHA256_CTX outterCtx;
   sha256_init(&innerCtx);
   sha256_init(&outterCtx);

   if (keySize > TIGER_BLOCK_SIZE) {
      sha256_update(&innerCtx, key, keySize);
      sha256_final(&innerCtx, innerKey);
      sha256_init(&innerCtx);
      sha256_update(&outterCtx, key, keySize);
      sha256_final(&outterCtx, outerKey);
      sha256_init(&outterCtx);
   } else {
      memcpy(innerKey, key, keySize);
      memcpy(outerKey, key, keySize);
   }

   byte innerPad[SHA256_BLOCK_SIZE];
   byte outerPad[SHA256_BLOCK_SIZE];
   memset(innerPad, 0x36, SHA256_BLOCK_SIZE);
   memset(outerPad, 0x5C, SHA256_BLOCK_SIZE);

   for (uint32_t i=0; i<SHA256_BLOCK_SIZE; i++) {
      innerKey[i] = innerKey[i] ^ innerPad[i];
      outerKey[i] = outerKey[i] ^ outerPad[i];
   }

   sha256_update(&innerCtx, innerKey, SHA256_BLOCK_SIZE);
   sha256_update(&innerCtx, message, messageSize);

   sha256_final(&innerCtx, outputBuffer);

   sha256_update(&outterCtx, outerKey, SHA256_BLOCK_SIZE);
   sha256_update(&outterCtx, outputBuffer, SHA256_HASH_SIZE);

   sha256_final(&outterCtx, outputBuffer);
}
//---------------------------------------------------------------------------

void RunCipherNonce(byte* data, uint32_t dataSize, const byte* nonce, const byte* key)
{
   unsigned long rk[RKLENGTH(AES_KEY_BITS)];
   int nrounds = rijndaelSetupEncrypt(rk, key, AES_KEY_BITS);
   
   uint32_t offset = 0;
   byte clearText[AES_BLOCK_SIZE];
   memcpy(clearText, nonce, FILE_NONCE_SIZE);

   while (offset < dataSize) {
      
      byte cipherText[AES_BLOCK_SIZE];
      uint64_t counter = 0;

      memcpy(clearText+FILE_NONCE_SIZE, reinterpret_cast<void*>(&counter), sizeof(uint64_t));

      rijndaelEncrypt(rk, nrounds, clearText, cipherText);

      uint32_t limit = dataSize-offset;

      if (limit > AES_BLOCK_SIZE) {
         limit = AES_BLOCK_SIZE;
      }      

      for (unsigned int i=0; i<limit; i++) {
         data[offset+i] = data[offset+i] ^ cipherText[i];
      }

      counter++;
      offset += AES_BLOCK_SIZE; 
   }
}
//---------------------------------------------------------------------------

void RunCipherIV(byte* data, uint32_t dataSize, byte* iv, const byte* key)
{
   unsigned long rk[RKLENGTH(AES_KEY_BITS)];
   int nrounds = rijndaelSetupEncrypt(rk, key, AES_KEY_BITS);
   
   uint32_t offset = 0;

   while (offset < dataSize) {
      byte cipherText[AES_BLOCK_SIZE];

      rijndaelEncrypt(rk, nrounds, iv, cipherText);

      uint32_t limit = dataSize-offset;

      if (limit > AES_BLOCK_SIZE) {
         limit = AES_BLOCK_SIZE;
      }

      for (uint32_t i=0; i<limit; i++) {
         data[offset+i] = data[offset+i] ^ cipherText[i];
      }

      offset += AES_BLOCK_SIZE;

      uint64_t halfVal;
      //++(*reinterpret_cast<uint64_t*>(&iv[sizeof(halfVal)]));
      memcpy(&halfVal, &iv[sizeof(halfVal)], sizeof(halfVal));
      halfVal++;
      memcpy(&iv[sizeof(halfVal)], &halfVal, sizeof(halfVal));
   }
}
//---------------------------------------------------------------------------

void GetHMAC_Tiger(const byte* key, uint32_t keySize, const byte* message, uint32_t messageSize, byte* outputBuffer)
{
   byte innerKey[TIGER_BLOCK_SIZE];
   byte outerKey[TIGER_BLOCK_SIZE];
   memset(innerKey, 0x00, TIGER_BLOCK_SIZE);
   memset(outerKey, 0x00, TIGER_BLOCK_SIZE);

   if (keySize > TIGER_BLOCK_SIZE) {
      GetTigerHash(key, keySize, innerKey);
      GetTigerHash(key, keySize, outerKey);
   } else {
      memcpy(innerKey, key, keySize);
      memcpy(outerKey, key, keySize);
   }

   byte innerPad[TIGER_BLOCK_SIZE];
   byte outerPad[TIGER_BLOCK_SIZE];
   memset(innerPad, 0xC9, TIGER_BLOCK_SIZE);
   memset(outerPad, 0xA3, TIGER_BLOCK_SIZE);

   for (uint32_t i=0; i<TIGER_BLOCK_SIZE; i++) {
      innerKey[i] = innerKey[i] ^ innerPad[i];
      outerKey[i] = outerKey[i] ^ outerPad[i];
   }

   byte* inputBuffer = reinterpret_cast<byte*>(malloc(TIGER_BLOCK_SIZE+messageSize));
   byte buffer[TIGER_BLOCK_SIZE+TIGER_HASH_SIZE];
   memset(buffer, 0x00, sizeof(buffer));

   memcpy(inputBuffer, innerKey, TIGER_BLOCK_SIZE);
   memcpy(inputBuffer+TIGER_BLOCK_SIZE, message, messageSize);
   GetTigerHash(inputBuffer, TIGER_BLOCK_SIZE+messageSize, buffer+TIGER_BLOCK_SIZE);

   memcpy(buffer, outerKey, TIGER_BLOCK_SIZE);
   GetTigerHash(buffer, sizeof(buffer), outputBuffer);

   free(inputBuffer);
}
//---------------------------------------------------------------------------
