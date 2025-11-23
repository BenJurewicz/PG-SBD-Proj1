#include <fstream>
#include <iostream>
#include <random>
#include <record.hpp>
#include <string>
#include <vector>

#include "util/create_options.hpp"

std::default_random_engine rng{std::random_device{}()};

std::string generateRandomString(int maxLength, bool numOnly = false) {
    std::uniform_int_distribution<int> lenDist(1, maxLength);
    // ASCII printable characters are in rage [32,126]
    // but whe start from 33 as (char)32 is space.
    // NOTE: (char)33 == '!'; (char)126 == '~';
    std::uniform_int_distribution<int> allCharDist(33, 126);
    std::uniform_int_distribution<int> numDist('0', '9');

    auto charDist = numOnly ? numDist : allCharDist;

    int length = lenDist(rng);
    std::string random_string;
    random_string.reserve(length);
    for (int i = 0; i < length; ++i) {
        random_string += static_cast<char>(charDist(rng));
    }
    return random_string;
}

void manualMode(std::vector<std::string>& lines) {
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
        }

        lines.push_back(line);
    }
}

void autoMode(
    std::vector<std::string>& lines, size_t numStrings, bool numOnly = false
) {
    for (size_t i = 0; i < numStrings; ++i) {
        lines.push_back(generateRandomString(Record::maxLen, numOnly));
    }
    std::cout << numStrings << " random strings generated." << std::endl;
}

void writeLine(std::ofstream& file, const std::string& data) {
    std::string tempStr = data;
    tempStr.resize(Record::maxLen, '\0');
    file.write(tempStr.data(), Record::maxLen);
}

void saveFile(std::vector<std::string>& lines, const std::string& filename) {
    std::filesystem::path p = filename;
    std::filesystem::path dir = p.parent_path();
    std::filesystem::create_directories(dir);

    std::ofstream outfile(filename, std::ios::out | std::ios::binary);
    if (outfile.is_open()) {
        for (const auto& line : lines) {
            writeLine(outfile, line);
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
        manualMode(lines);
    } else if (options.isRandomMode()) {
        autoMode(lines, options.getRandomCount(), options.isNumbersOnly());
    }

    if (!lines.empty()) {
        saveFile(lines, options.getFileName());
    }

    return 0;
}
