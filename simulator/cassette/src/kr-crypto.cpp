//------------------------------------------------------------------------------
//
//      Криптоядро формата .kr: CRC32 / SHA-256 / AES-256-CBC
//
//------------------------------------------------------------------------------

#include    "kr-crypto.h"

#include    <algorithm>
#include    <cstring>

namespace
{

//------------------------------------------------------------------------------
// AES: таблица S-box (FIPS 197)
//------------------------------------------------------------------------------
const std::uint8_t SBOX[256] =
{
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

//------------------------------------------------------------------------------
// AES: обратная таблица S-box (для расшифрования)
//------------------------------------------------------------------------------
const std::uint8_t SBOX_INV[256] =
{
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

inline std::uint8_t xtime(std::uint8_t x)
{
    return static_cast<std::uint8_t>((x << 1) ^ ((x & 0x80u) ? 0x1Bu : 0x00u));
}

//------------------------------------------------------------------------------
/// Расширение ключа AES-256 (Nk=8, Nr=14): 60 слов по 4 байта
//------------------------------------------------------------------------------
void aesExpandKey256(const std::uint8_t* key, std::uint8_t* round_keys)
{
    std::memcpy(round_keys, key, 32);

    std::uint8_t temp[4];

    for (std::size_t i = 8; i < 60; ++i)
    {
        std::memcpy(temp, round_keys + (i - 1) * 4, 4);

        if (i % 8 == 0)
        {
            // RotWord + SubWord + Rcon
            const std::uint8_t t = temp[0];
            temp[0] = SBOX[temp[1]] ^ static_cast<std::uint8_t>(0x01u << (i / 8 - 1));
            temp[1] = SBOX[temp[2]];
            temp[2] = SBOX[temp[3]];
            temp[3] = SBOX[t];
        }
        else if (i % 8 == 4)
        {
            for (auto& b : temp)
                b = SBOX[b];
        }

        for (std::size_t j = 0; j < 4; ++j)
            round_keys[i * 4 + j] =
                    round_keys[(i - 8) * 4 + j] ^ temp[j];
    }
}

//------------------------------------------------------------------------------
/// Один блок AES (16 байт) в прямом/обратном направлении
//------------------------------------------------------------------------------
void aesEncryptBlock(const std::uint8_t* round_keys, std::uint8_t* block)
{
    // AddRoundKey(0)
    for (std::size_t i = 0; i < 16; ++i)
        block[i] ^= round_keys[i];

    for (std::size_t round = 1; round <= 14; ++round)
    {
        // SubBytes
        for (std::size_t i = 0; i < 16; ++i)
            block[i] = SBOX[block[i]];

        // ShiftRows (state по столбцам: s[r + 4c])
        std::uint8_t t = 0;

        // строка 1: сдвиг влево на 1
        t = block[1];
        block[1] = block[5];
        block[5] = block[9];
        block[9] = block[13];
        block[13] = t;

        // строка 2: сдвиг влево на 2
        t = block[2];
        block[2] = block[10];
        block[10] = t;
        t = block[6];
        block[6] = block[14];
        block[14] = t;

        // строка 3: сдвиг влево на 3 (вправо на 1)
        t = block[15];
        block[15] = block[11];
        block[11] = block[7];
        block[7] = block[3];
        block[3] = t;

        // MixColumns (кроме последнего раунда) + AddRoundKey
        const std::uint8_t* rk = round_keys + round * 16;

        for (std::size_t col = 0; col < 4; ++col)
        {
            std::uint8_t* c = block + col * 4;

            if (round < 14)
            {
                const std::uint8_t a0 = c[0];
                const std::uint8_t a1 = c[1];
                const std::uint8_t a2 = c[2];
                const std::uint8_t a3 = c[3];

                c[0] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
                c[1] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
                c[2] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
                c[3] = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            }

            for (std::size_t j = 0; j < 4; ++j)
                c[j] ^= rk[col * 4 + j];
        }
    }
}

void aesDecryptBlock(const std::uint8_t* round_keys, std::uint8_t* block)
{
    // AddRoundKey(14)
    for (std::size_t i = 0; i < 16; ++i)
        block[i] ^= round_keys[14 * 16 + i];

    for (int round = 13; round >= 0; --round)
    {
        // InvShiftRows
        std::uint8_t t = 0;

        // строка 1: сдвиг вправо на 1
        t = block[13];
        block[13] = block[9];
        block[9] = block[5];
        block[5] = block[1];
        block[1] = t;

        // строка 2: сдвиг на 2
        t = block[2];
        block[2] = block[10];
        block[10] = t;
        t = block[6];
        block[6] = block[14];
        block[14] = t;

        // строка 3: сдвиг влево на 1
        t = block[3];
        block[3] = block[7];
        block[7] = block[11];
        block[11] = block[15];
        block[15] = t;

        // InvSubBytes
        for (std::size_t i = 0; i < 16; ++i)
            block[i] = SBOX_INV[block[i]];

        // AddRoundKey(round)
        const std::uint8_t* rk = round_keys + static_cast<std::size_t>(round) * 16;

        for (std::size_t i = 0; i < 16; ++i)
            block[i] ^= rk[i];

        // InvMixColumns (кроме последней итерации)
        if (round > 0)
        {
            for (std::size_t col = 0; col < 4; ++col)
            {
                std::uint8_t* c = block + col * 4;

                const std::uint8_t a0 = c[0];
                const std::uint8_t a1 = c[1];
                const std::uint8_t a2 = c[2];
                const std::uint8_t a3 = c[3];

                c[0] = xtime(xtime(a0)) ^ (xtime(xtime(a1)) ^ a1) ^
                       (xtime(xtime(a2)) ^ a2) ^ a3;
                c[1] = a0 ^ (xtime(xtime(a1)) ^ a1) ^
                       (xtime(xtime(a2)) ^ a2) ^ (xtime(xtime(a3)) ^ a3);
                c[2] = (xtime(xtime(a0)) ^ a0) ^ a1 ^
                       (xtime(xtime(a2)) ^ a2) ^ (xtime(xtime(a3)) ^ a3);
                c[3] = (xtime(xtime(a0)) ^ a0) ^
                       (xtime(xtime(a1)) ^ a1) ^ a2 ^ (xtime(xtime(a3)) ^ a3);
            }
        }
    }
}

//------------------------------------------------------------------------------
// SHA-256 константы K (первые 32 бита дробных частей кубических корней
// простых 2..311, FIPS 180-4)
//------------------------------------------------------------------------------
const std::uint32_t SHA256_K[64] =
{
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

inline std::uint32_t rotr(std::uint32_t x, std::uint32_t n)
{
    return (x >> n) | (x << (32u - n));
}

} // namespace

namespace kr
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::uint32_t crc32(const std::uint8_t* data, std::size_t size,
                    std::uint32_t initial)
{
    std::uint32_t crc = initial;

    for (std::size_t i = 0; i < size; ++i)
    {
        crc ^= data[i];

        for (int bit = 0; bit < 8; ++bit)
        {
            const std::uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<std::uint8_t> sha256(const std::uint8_t* data, std::size_t size)
{
    std::uint32_t h[8] =
    {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // Дополнение: 0x80, нули, 64-битная длина (big-endian)
    const std::uint64_t bit_len = static_cast<std::uint64_t>(size) * 8u;

    const std::size_t padded_size = ((size + 8) / 64 + 1) * 64;

    std::vector<std::uint8_t> message(padded_size, 0);
    std::memcpy(message.data(), data, size);
    message[size] = 0x80;

    for (std::size_t i = 0; i < 8; ++i)
    {
        message[padded_size - 1 - i] =
                static_cast<std::uint8_t>(bit_len >> (8u * i));
    }

    std::uint32_t w[64];

    for (std::size_t chunk = 0; chunk < padded_size; chunk += 64)
    {
        for (std::size_t i = 0; i < 16; ++i)
        {
            w[i] = (static_cast<std::uint32_t>(message[chunk + i * 4]) << 24) |
                   (static_cast<std::uint32_t>(message[chunk + i * 4 + 1]) << 16) |
                   (static_cast<std::uint32_t>(message[chunk + i * 4 + 2]) << 8) |
                   static_cast<std::uint32_t>(message[chunk + i * 4 + 3]);
        }

        for (std::size_t i = 16; i < 64; ++i)
        {
            const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        std::uint32_t a = h[0];
        std::uint32_t b = h[1];
        std::uint32_t c = h[2];
        std::uint32_t d = h[3];
        std::uint32_t e = h[4];
        std::uint32_t f = h[5];
        std::uint32_t g = h[6];
        std::uint32_t hh = h[7];

        for (std::size_t i = 0; i < 64; ++i)
        {
            const std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const std::uint32_t ch = (e & f) ^ (~e & g);
            const std::uint32_t temp1 = hh + S1 + ch + SHA256_K[i] + w[i];
            const std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = S0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    std::vector<std::uint8_t> digest(32);

    for (std::size_t i = 0; i < 8; ++i)
    {
        for (std::size_t j = 0; j < 4; ++j)
        {
            digest[i * 4 + j] =
                    static_cast<std::uint8_t>(h[i] >> (24u - 8u * j));
        }
    }

    return digest;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string sha256Hex(const std::vector<std::uint8_t>& data)
{
    static const char digits[] = "0123456789abcdef";

    const std::vector<std::uint8_t> digest = sha256(data);

    std::string hex;
    hex.reserve(digest.size() * 2);

    for (std::uint8_t byte : digest)
    {
        hex.push_back(digits[byte >> 4]);
        hex.push_back(digits[byte & 0x0F]);
    }

    return hex;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool aes256CbcEncrypt(const std::vector<std::uint8_t>& key,
                      const std::vector<std::uint8_t>& iv,
                      const std::vector<std::uint8_t>& plain,
                      std::vector<std::uint8_t>& cipher)
{
    if (key.size() != 32 || iv.size() != 16)
        return false;

    // PKCS#7: дополнение до кратности 16
    const std::uint8_t pad = static_cast<std::uint8_t>(
                16 - (plain.size() % 16));

    std::vector<std::uint8_t> padded(plain);
    padded.resize(plain.size() + pad, pad);

    std::uint8_t round_keys[240];
    aesExpandKey256(key.data(), round_keys);

    cipher.resize(padded.size());

    std::uint8_t prev[16];
    std::memcpy(prev, iv.data(), 16);

    for (std::size_t offset = 0; offset < padded.size(); offset += 16)
    {
        std::uint8_t block[16];

        for (std::size_t i = 0; i < 16; ++i)
            block[i] = padded[offset + i] ^ prev[i];

        aesEncryptBlock(round_keys, block);

        std::memcpy(cipher.data() + offset, block, 16);
        std::memcpy(prev, block, 16);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool aes256CbcDecrypt(const std::vector<std::uint8_t>& key,
                      const std::vector<std::uint8_t>& iv,
                      const std::vector<std::uint8_t>& cipher,
                      std::vector<std::uint8_t>& plain)
{
    if (key.size() != 32 || iv.size() != 16 ||
        cipher.empty() || (cipher.size() % 16) != 0)
    {
        return false;
    }

    std::uint8_t round_keys[240];
    aesExpandKey256(key.data(), round_keys);

    plain.resize(cipher.size());

    std::uint8_t prev[16];
    std::memcpy(prev, iv.data(), 16);

    for (std::size_t offset = 0; offset < cipher.size(); offset += 16)
    {
        std::uint8_t cipher_block[16];
        std::memcpy(cipher_block, cipher.data() + offset, 16);

        std::uint8_t block[16];
        std::memcpy(block, cipher_block, 16);

        aesDecryptBlock(round_keys, block);

        for (std::size_t i = 0; i < 16; ++i)
            plain[offset + i] = block[i] ^ prev[i];

        std::memcpy(prev, cipher_block, 16);
    }

    // Проверка и снятие PKCS#7
    const std::uint8_t pad = plain.back();

    if (pad == 0 || pad > 16)
        return false;

    for (std::size_t i = plain.size() - pad; i < plain.size(); ++i)
    {
        if (plain[i] != pad)
            return false;
    }

    plain.resize(plain.size() - pad);

    return true;
}

} // namespace kr
