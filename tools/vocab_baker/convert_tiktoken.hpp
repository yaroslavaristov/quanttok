#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <print>
#include <quanttok/tools/vocab_baker/decode_b64.hpp>
#include <string>
#include <utility>
#include <vector>

namespace quanttok::tools::vocab_baker::convert {
inline std::vector<std::pair<std::vector<std::uint8_t>, std::size_t>> convert_tiktoken(
    const std::string& tiktoken_path) noexcept {
    std::ifstream file{(tiktoken_path).c_str()};
    std::string   line{};

    if (!file.is_open()) {
        std::print(std::cerr, "[FATAL] Cannot open .tiktoken file!\n");
        std::abort();
    }

    std::vector<std::pair<std::vector<std::uint8_t>, std::size_t>> tokens{};

    while (std::getline(file, line)) {
        std::size_t space_pos{line.find(' ')};
        if (space_pos == std::string::npos) {
            std::print(std::cerr, "[WARN] Line corrupted.\n");
        }

        std::string_view token_part{std::string_view(line).substr(0, space_pos)};
        std::string_view rank_part{std::string_view(line).substr(space_pos + 1)};

        tokens.emplace_back(std::make_pair(decode_b64(token_part), std::stoi(static_cast<std::string>(rank_part))));
    }
}
} // namespace quanttok::tools::vocab_baker::convert
