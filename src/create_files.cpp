#include <fstream>
#include <iostream>
#include <random>
#include <record.hpp>
#include <string>
#include <vector>

#include "util/create_options.hpp"

std::default_random_engine rng{std::random_device{}()};

std::string generate_random_string(int maxLength, bool numOnly = false) {
    std::uniform_int_distribution<int> len_dist(1, maxLength);
    // ASCII printable characters are in rage [32,126]
    // but whe start from 33 as (char)32 is space.
    // NOTE: (char)33 == '!'; (char)126 == '~';
    // TODO: Ocasinally got non ascii chars here
    std::uniform_int_distribution<int> all_char_dist(33, 126);
    std::uniform_int_distribution<int> num_dist('0', '9');

    auto char_dist = numOnly ? num_dist : all_char_dist;

    int length = len_dist(rng);
    std::string random_string;
    random_string.reserve(length);
    for (int i = 0; i < length; ++i) {
        random_string += static_cast<char>(char_dist(rng));
    }
    return random_string;
}

void manual_mode(std::vector<std::string>& lines) {
    std::string line;
    // clang-format off
    std::cout << 
        std::format(
            "Enter lines of text (up to {} characters)."
            " Type '!q' to finish.",
            Record::maxLen
        ) << std::endl;
    // clang-format on

    while (true) {
        std::getline(std::cin, line);

        if (line == "!q") {
            break;
        }

        if (line.length() > Record::maxLen) {
            // clang-format off
            std::cout << 
                std::format(
                    "Error: String exceeds {} characters. "
                    "Please try again.",
                    Record::maxLen
                ) << std::endl;
            // clang-format on
            break;
        }

        lines.push_back(line);
    }
}

void auto_mode(
    std::vector<std::string>& lines, size_t num_strings, bool numOnly = false
) {
    for (size_t i = 0; i < num_strings; ++i) {
        lines.push_back(generate_random_string(Record::maxLen, numOnly));
    }
    std::cout << num_strings << " random strings generated." << std::endl;
}

void write_line(std::ofstream& file, const std::string& data) {
    std::string tempStr = data;
    tempStr.resize(Record::maxLen, '\0');
    file.write(tempStr.data(), Record::maxLen);
}

void save_file(std::vector<std::string>& lines, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        for (const auto& line : lines) {
            write_line(outfile, line);
        }
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Error: Could not open file for writing." << std::endl;
    }
}

int main(int argc, char** argv) {
    CreateOptions options(argc, argv);
    std::vector<std::string> lines;

    if (options.isInteractiveMode()) {
        manual_mode(lines);
    } else if (options.isRandomMode()) {
        auto_mode(lines, options.getRandomCount(), options.isNumbersOnly());
    }

    if (!lines.empty()) {
        save_file(lines, options.getFileName());
    }

    return 0;
}
