//
// Copyright (c) 2014-2018 halx99 - All Rights Reserved
//
#ifndef _CRYPTO_UTILS_H_
#define _CRYPTO_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "math_ext.hpp"
#include "aes.h"

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */

#define CODEC_OK 0
#define CODEC_BAD_ALLOC -2


///
/// all follow api regardless input as alignment BLOCK_SIZE: 16
/// overlapped api
///

#include <string>
#include <vector>

namespace crypto {
    
/* basic char convertors */
uint8_t hex2uchr(const uint8_t hex);

uint8_t uchr2hex(const uint8_t ch);

uint8_t hex2chr(const uint8_t hex);

uint8_t chr2hex(const uint8_t ch);

char *hex2chrp(const uint8_t hex, char charp[2]);

/* convert hexstream to charstream */
void bin2hex(const void *source, unsigned int sourceLen, char *dest,
             unsigned int destLen);
/* -- end of basic convertors -- */
    
namespace aes {

// ժҪ:
//     ָ�����ڼ��ܵĿ�����ģʽ��
enum CipherMode {
  // ժҪ:
  //     ������� (CBC)
  //     ģʽ�����˷�����ÿ�����ı����ڼ���ǰ��ͨ����λ����򡱲�����ǰһ����������ı���ϡ�����ȷ���˼�ʹ���ı����������ͬ�Ŀ飬��Щ���е�ÿһ��Ҳ�����Ϊ��ͬ�������ı��顣�ڼ��ܿ�֮ǰ����ʼ������ͨ����λ����򡱲������һ�����ı����ϡ���������ı�������һ��λ������Ӧ�Ĵ��ı���Ҳ���������⣬����Ŀ�����ԭ����λ��λ����ͬ��λҲ������
  CBC = 1,
  //
  // ժҪ:
  //     �������뱾 (ECB)
  //     ģʽ�ֱ����ÿ���顣����ζ���κδ��ı���ֻҪ��ͬ������ͬһ��Ϣ�У�����������ͬ����Կ���ܵĲ�ͬ��Ϣ�У�������ת����ͬ���������ı��顣���Ҫ���ܵĴ��ı����������ظ��Ŀ飬������ƽ������ı��ǿ��еġ����⣬��ʱ׼�������Ķ��ֿ�������û�в�������������ͽ�������Ŀ顣��������ı�������һ��λ������Ӧ���������ı���Ҳ������
  ECB = 2,
  //
  // ժҪ:
  //     ������� (OFB)
  //     ģʽ�����������Ĵ��ı�����������ı���������һ�δ��������顣��ģʽ��
  //     CFB
  //     ���ƣ�������ģʽ��Ψһ�������λ�Ĵ�������䷽ʽ��ͬ����������ı�����һ��λ�������ı�����Ӧ��λҲ���������ǣ���������ı����ж������ȱ�ٵ�λ�����Ǹ�λ֮��Ĵ��ı���������
  OFB = 3,
  //
  // ժҪ:
  //     ���뷴�� (CFB)
  //     ģʽ�����������Ĵ��ı�����������ı���������һ�δ��������顣��ģʽʹ���ڳ�����Ϊһ�����ұ���Ϊ�����ֵ���λ�Ĵ��������磬������СΪ
  //     8 ���ֽڣ�����ÿ�δ���һ���ֽڣ�����λ�Ĵ�������Ϊ 8
  //     �����֡���������ı�����һ��λ������һ�����ı�λ����������λ�Ĵ����𻵡��⽫���½��������ɴε����Ĵ��ı�����ֱ������λ����λ�Ĵ������Ƴ�Ϊֹ��
  CFB = 4,
  //
  // ժҪ:
  //     �����ı����� (CTS)
  //     ģʽ�����κγ��ȵĴ��ı������������봿�ı�����ƥ��������ı�����������������ı����⣬�������������飬��ģʽ��
  //     CBC ģʽ����Ϊ��ͬ��
  CTS = 5,
};

// ժҪ:
//     ָ������Ϣ���ݿ�ȼ��ܲ��������ȫ���ֽ�����ʱӦ�õ�������͡�
enum PaddingMode {
  // ժҪ:
  //     ����䡣
  None = 1,
  //
  // ժҪ:
  //     PKCS #7 ����ַ�����һ���ֽ�������ɣ�ÿ���ֽ������ֽ����еĳ��ȡ�
  PKCS7 = 2,
  //
  // ժҪ:
  //     ����ַ���������Ϊ����ֽ���ɡ�
  Zeros = 3,
  //
  // ժҪ:
  //     ANSIX923
  //     ����ַ�����һ���ֽ�������ɣ����ֽ����е����һ���ֽ�����ֽ����еĳ��ȣ������ֽھ���������㡣
  ANSIX923 = 4,
  //
  // ժҪ:
  //     ISO10126
  //     ����ַ�����һ���ֽ�������ɣ����ֽ����е����һ���ֽ�����ֽ����еĳ��ȣ������ֽ����������ݡ�
  ISO10126 = 5,
};

#define _BYTE_SEQ_CONT _ByteSeqCont

static const char *DEFAULT_KEY = "ZQnNQmA1iIQ3z3ukoPoATdE88OJ0qsMm";

namespace detail {

struct cbc_block_state
{
    AES_KEY key;
    unsigned char iv[16];
};

namespace padding {

template <typename _ByteSeqCont>
inline size_t PKCS7(_BYTE_SEQ_CONT &plaintext,
                    size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(typename _BYTE_SEQ_CONT::value_type) == 1,
                "PKCS7: only allow stl sequently byte conatiner!");
  size_t padding_size = blocksize - plaintext.size() % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    plaintext.push_back((char)padding_size);
  }
  return padding_size;
}

template <typename _ByteSeqCont>
inline size_t ZEROS(_BYTE_SEQ_CONT &plaintext,
                    size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ZEROS: only allow stl sequently byte conatiner!");
  size_t padding_size = blocksize - plaintext.size() % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    plaintext.push_back((char)0);
  }
  return padding_size;
}

template <typename _ByteSeqCont>
inline size_t ANSIX923(_BYTE_SEQ_CONT &plaintext,
                       size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ANSIX923: only allow stl sequently byte conatiner!");
  size_t padding_size = blocksize - plaintext.size() % blocksize;
  for (size_t offst = 0; offst < padding_size - 1; ++offst) {
    plaintext.push_back((char)0);
  }
  plaintext.push_back((char)padding_size);
  return padding_size;
}

template <typename _ByteSeqCont>
inline size_t ISO10126(_BYTE_SEQ_CONT &plaintext,
                       size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ISO10126: only allow stl sequently byte conatiner!");
  size_t padding_size = blocksize - plaintext.size() % blocksize;
  for (size_t offst = 0; offst < padding_size - 1; ++offst) {
    plaintext.push_back((char)(unsigned char)math_ext::rrand(0, 256));
  }
  plaintext.push_back((char)padding_size);
  return padding_size;
}

template <typename _ByteSeqCont = std::string>
inline _BYTE_SEQ_CONT PKCS7(size_t datasize,
                            size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ISO10126: only allow stl sequently byte conatiner!");
  _BYTE_SEQ_CONT padding;
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    padding.push_back((char)padding_size);
  }
  return (padding);
}

template <typename _ByteSeqCont = std::string>
inline _BYTE_SEQ_CONT ZEROS(size_t datasize,
                            size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ISO10126: only allow stl sequently byte conatiner!");
  _BYTE_SEQ_CONT padding;
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    padding.push_back((char)0);
  }
  return (padding);
}

template <typename _ByteSeqCont = std::string>
inline _BYTE_SEQ_CONT ANSIX923(size_t datasize,
                               size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ISO10126: only allow stl sequently byte conatiner!");
  _BYTE_SEQ_CONT padding;
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size - 1; ++offst) {
    padding.push_back((char)0);
  }
  padding.push_back((char)padding_size);
  return (padding);
}

template <typename _ByteSeqCont = std::string>
inline _BYTE_SEQ_CONT ISO10126(size_t datasize,
                               size_t blocksize = AES_BLOCK_SIZE) {
  static_assert(sizeof(_BYTE_SEQ_CONT::value_type) == 1,
                "ISO10126: only allow stl sequently byte conatiner!");
  _BYTE_SEQ_CONT padding;
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size - 1; ++offst) {
    padding.push_back((char)(unsigned char)math_ext::rrand(0, 256));
  }
  padding.push_back((char)padding_size);
  return (padding);
}

inline size_t PKCS7(size_t datasize, char padding[16],
                    size_t blocksize = AES_BLOCK_SIZE) {
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    padding[AES_BLOCK_SIZE - 1 - offst] = (unsigned char)padding_size;
  }
  return padding_size;
}

inline size_t ZEROS(size_t datasize, char padding[16],
                    size_t blocksize = AES_BLOCK_SIZE) {
  size_t padding_size = blocksize - datasize % blocksize;
  for (size_t offst = 0; offst < padding_size; ++offst) {
    padding[AES_BLOCK_SIZE - 1 - offst] = 0;
  }
  return padding_size;
}

inline size_t ANSIX923(size_t datasize, char padding[16],
                       size_t blocksize = AES_BLOCK_SIZE) {
  size_t padding_size = blocksize - datasize % blocksize;
  padding[AES_BLOCK_SIZE - 1] = (unsigned char)padding_size;
  for (size_t offst = 1; offst < padding_size; ++offst) {
    padding[AES_BLOCK_SIZE - 1 - offst] = 0;
  }
  return padding_size;
}

inline size_t ISO10126(size_t datasize, char padding[16],
                       size_t blocksize = AES_BLOCK_SIZE) {
  size_t padding_size = blocksize - datasize % blocksize;
  padding[AES_BLOCK_SIZE - 1] = (unsigned char)padding_size;
  for (size_t offst = 1; offst < padding_size; ++offst) {
    padding[AES_BLOCK_SIZE - 1 - offst] = (unsigned char)math_ext::rrand(0, 256);
  }
  return padding_size;
}
} // namespace padding

/// AES ecb
extern void (*ecb_encrypt)(const void *in, size_t inlen, void *out,
                           size_t outlen, const void *private_key, int keybits);
extern void (*ecb_decrypt)(const void *in, size_t inlen, void *out,
                           size_t &outlen, const void *private_key,
                           int keybits);

/// AES CBC
extern void (*cbc_encrypt)(const void *in, size_t inlen, void *out,
                           size_t outlen, const void *private_key,
                           int keybits /*128,192,256*/, const void* ivec);

extern void (*cbc_decrypt)(const void *in, size_t inlen, void *out,
                           size_t &outlen, const void *private_key,
                           int keybits /*128,192,256*/, const void* ivec);

/// AES CBC block encrypt/decrypt
extern void (*cbc_encrypt_init)(cbc_block_state* state, const void *private_key,
                                int keybits /*128,192,256*/, const void* ivec);

extern void (*cbc_decrypt_init)(cbc_block_state* state, const void *private_key,
                                int keybits /*128,192,256*/, const void* ivec);

extern void (*cbc_encrypt_block)(cbc_block_state* state, const void *in, size_t inlen, void *out,
                                 size_t outlen);

extern void (*cbc_decrypt_block)(cbc_block_state* state, const void *in, size_t inlen, void *out,
                                 size_t outlen);
}; // namespace detail
}; // namespace aes
}; // namespace crypto

#endif /* _CRYPTO_UTILS_H_ */
