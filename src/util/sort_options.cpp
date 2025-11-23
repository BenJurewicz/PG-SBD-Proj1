#include "sort_options.hpp"

#include <format>
#include <iostream>
#include <string>

SortOptions::SortOptions(int argc, char** argv) : scriptName(argv[0]) {
    parse(argc, argv);
    checkRequired();
    if (logging) {
        // clang-format off
        std::cout << std::format(
            "Starting the script with arguments:\n"
            "fileName={}\n"
            "bufferCount={}\n"
            "blockingFactor={}\n"
            "logging={}\n",
            fileName,
            bufferCount,
            blockingFactor,
            logging
        ) << std::endl;
        // clang-format on
    }
}

void SortOptions::parse(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        parseArgument(argv[i], i, argc, argv);
    }
}

void SortOptions::parseArgument(
    const std::string& arg, int& i, int argc, char** argv
) {
    if (arg.starts_with('-')) {
        handleFlag(arg, i, argc, argv);
    } else {
        handleFileName(arg);
    }
}

void SortOptions::handleFlag(
    const std::string& flag, int& i, int argc, char** argv
) {
    if ((flag == "-h") || (flag == "--help")) {
        printHelpAndExit(0);
    } else if ((flag == "-n") || (flag == "--bufferCount")) {
        parseBufferCount(i, argc, argv);
    } else if ((flag == "-b") || (flag == "--blockingFactor")) {
        parseBlockingFactor(i, argc, argv);
    } else if ((flag == "-l") || (flag == "--logging")) {
        logging = false;
    } else {
        std::cerr << "Error: Unknown argument '" << flag << "'\n";
        printHelpAndExit();
    }
}

void SortOptions::handleFileName(const std::string& name) { fileName = name; }

std::string SortOptions::getVal(int& i, int argc, char** argv) {
    if (i + 1 >= argc) {
        std::cerr << "Error: " << argv[i] << " requires a value.\n";
        printHelpAndExit();
    }
    return argv[++i];
}

void SortOptions::parseBufferCount(int& i, int argc, char** argv) {
    auto val = getVal(i, argc, argv);
    try {
        bufferCount = std::stoul(val);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid value for " << argv[i - 1] << ": " << val
                  << std::endl;
        printHelpAndExit();
    }
}

void SortOptions::parseBlockingFactor(int& i, int argc, char** argv) {
    auto val = getVal(i, argc, argv);
    try {
        blockingFactor = std::stoul(val);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid value for " << argv[i - 1] << ": " << val
                  << std::endl;
        printHelpAndExit();
    }
}

void SortOptions::checkRequired() const {
    if (fileName.empty()) {
        std::cerr << "Error: A file name must be provided." << std::endl;
        printHelpAndExit();
    }
}

void SortOptions::printHelpAndExit(int exitCode) const {
    // clang-format off
    std::cout <<
        "Usage: " << scriptName << " [options] <fileName>\n\n"
        "Options:\n"
        "\t-h, --help\tShow this help message\n\n"
        "\t-n, --bufferCount <value>\n"
        "\t\tSet buffer count (min: 3, default: 5)\n\n"
        "\t-b, --blockingFactor <value>\n"
        "\t\tSet blocking factor (min: 1, default: 10)\n\n"
        "\t-l, --logging\tDisable logging\n\n"
        "Arguments:\n"
        "\t<fileName>\tRequired: Path to the file to be sorted\n";
    // clang-format on
    exit(exitCode);
}
