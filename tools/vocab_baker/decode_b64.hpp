#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace quanttok::tools::vocab_baker::decode {
const std::string base64_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

inline int char_to_value(char c) {
    std::size_t pos{base64_alphabet.find(c)};

    return static_cast<int>(pos);
}

inline std::vector<std::uint8_t> decode_b64(std::string_view token_part) {
    std::vector<std::uint8_t> result{};
    for (std::size_t i{}; i < token_part.size(); i += 4) {
        std::size_t padding_count{};

        if (token_part[i + 2] == '=')
            padding_count++;
        if (token_part[i + 3] == '=')
            padding_count++;

        int v0{char_to_value(token_part[i])};
        int v1{char_to_value(token_part[i + 1])};
        int v2{(token_part[i + 2] == '=') ? 0 : char_to_value(token_part[i + 2])};
        int v3{(token_part[i + 3] == '=') ? 0 : char_to_value(token_part[i + 3])};

        std::uint32_t combined{static_cast<std::uint32_t>((v0 << 18) | (v1 << 12) | (v2 << 6) | v3)};

        std::uint8_t byte0{static_cast<std::uint8_t>((combined >> 16) & 0xFF)};
        std::uint8_t byte1{static_cast<std::uint8_t>((combined >> 8) & 0xFF)};
        std::uint8_t byte2{static_cast<std::uint8_t>(combined & 0xFF)};

        result.push_back(byte0);
        if (padding_count < 2)
            result.push_back(byte1);
        if (padding_count < 1)
            result.push_back(byte2);
    }

    return result;
}
} // namespace quanttok::tools::vocab_baker::decode
