//------------------------------------------------------------------------------
//
//      Криптоядро формата кассет регистрации .kr (ТЗ "Кассеты")
//      Самодостаточные реализации без внешних зависимостей:
//      CRC32 (IEEE 802.3), SHA-256 (FIPS 180-4), AES-256-CBC (FIPS 197)
//
//      Цепочка защиты кассеты (ТЗ п.8): бинарные блоки с CRC32 ->
//      шифрование AES-256-CBC ключом, встроенным в программу
//      расшифровки -> мастер-хеш SHA-256 по всем блокам.
//
//------------------------------------------------------------------------------

#ifndef     KR_CRYPTO_H
#define     KR_CRYPTO_H

#include    <cstddef>
#include    <cstdint>
#include    <string>
#include    <vector>

namespace kr
{

/// CRC-32 (IEEE 802.3, полином 0xEDB88320)
std::uint32_t crc32(const std::uint8_t* data, std::size_t size,
                    std::uint32_t initial = 0xFFFFFFFFu);

inline std::uint32_t crc32(const std::vector<std::uint8_t>& data)
{
    return crc32(data.data(), data.size());
}

//------------------------------------------------------------------------------
/// SHA-256 (FIPS 180-4). Возвращает 32 байта дайджеста
//------------------------------------------------------------------------------
std::vector<std::uint8_t> sha256(const std::uint8_t* data, std::size_t size);

inline std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t>& data)
{
    return sha256(data.data(), data.size());
}

/// Дайджест в виде hex-строки (64 символа)
std::string sha256Hex(const std::vector<std::uint8_t>& data);

//------------------------------------------------------------------------------
/// AES-256-CBC (FIPS 197). Ключ - 32 байта, IV - 16 байт.
/// encrypt: дополнение PKCS#7; decrypt возвращает false при неверном
/// дополнении (данные повреждены или ключ неверен)
//------------------------------------------------------------------------------
bool aes256CbcEncrypt(const std::vector<std::uint8_t>& key,
                      const std::vector<std::uint8_t>& iv,
                      const std::vector<std::uint8_t>& plain,
                      std::vector<std::uint8_t>& cipher);

bool aes256CbcDecrypt(const std::vector<std::uint8_t>& key,
                      const std::vector<std::uint8_t>& iv,
                      const std::vector<std::uint8_t>& cipher,
                      std::vector<std::uint8_t>& plain);

} // namespace kr

#endif // KR_CRYPTO_H
