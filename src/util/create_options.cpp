#include "create_options.hpp"

#include <iostream>
#include <string>

CreateOptions::CreateOptions(int argc, char** argv) : scriptName(argv[0]) {
    parse(argc, argv);
    checkConstraints();
}

void CreateOptions::parse(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        parseArgument(argv[i], i, argc, argv);
    }
}

void CreateOptions::parseArgument(
    const std::string& arg, int& i, int argc, char** argv
) {
    if (arg.starts_with('-')) {
        handleFlag(arg, i, argc, argv);
    } else {
        std::cerr << "Error: Unexpected argument '" << arg << "'\n";
        printHelpAndExit();
    }
}

void CreateOptions::handleFlag(
    const std::string& flag, int& i, int argc, char** argv
) {
    if ((flag == "-h") || (flag == "--help")) {
        printHelpAndExit(0);
    } else if ((flag == "-r") || (flag == "--random")) {
        randomMode = true;
        parseRandomCount(i, argc, argv);
    } else if ((flag == "-n") || (flag == "--numbers-only")) {
        numbersOnly = true;
    } else if ((flag == "-f") || (flag == "--file")) {
        parseFileName(i, argc, argv);
    } else if ((flag == "-i") || (flag == "--interactive")) {
        interactiveMode = true;
    } else {
        std::cerr << "Error: Unknown argument '" << flag << "'\n";
        printHelpAndExit();
    }
}

std::string CreateOptions::getVal(int& i, int argc, char** argv) {
    if (i + 1 >= argc) {
        std::cerr << "Error: " << argv[i] << " requires a value.\n";
        printHelpAndExit();
    }
    return argv[++i];
}

void CreateOptions::parseFileName(int& i, int argc, char** argv) {
    fileName = getVal(i, argc, argv);
}

void CreateOptions::parseRandomCount(int& i, int argc, char** argv) {
    auto val = getVal(i, argc, argv);
    try {
        randomCount = std::stoul(val);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid value for " << argv[i - 1] << ": " << val
                  << std::endl;
        printHelpAndExit();
    }
}

void CreateOptions::checkConstraints() const {
    if (interactiveMode && randomMode) {
        std::cerr << "Error: -i (--interactive) and -r (--random) flags are "
                     "mutually exclusive."
                  << std::endl;
        printHelpAndExit();
    }

    if (!interactiveMode && !randomMode) {
        std::cerr << "Error: Either -i (--interactive) or -r (--random) flag "
                     "must be provided."
                  << std::endl;
        printHelpAndExit();
    }

    if (numbersOnly && !randomMode) {
        std::cerr << "Error: -n (--numbers-only) can only be used with -r "
                     "(--random)."
                  << std::endl;
        printHelpAndExit();
    }
}

void CreateOptions::printHelpAndExit(int exitCode) const {
    // clang-format off
    std::cout
        << "Usage: " << scriptName << " [options]\n"
           "Options:\n"
           "\t-h, --help\t\tShow this help message\n"
           "\t-r, --random <count>\tGenerate <count> random records.\n"
           "\t-n, --numbers-only\tGenerate only numbers (only with -r).\n"
           "\t-f, --file <filename>\tSet output file (default: data/data.bin).\n"
           "\t-i, --interactive\tEnter interactive mode to write records.\n";
    // clang-format on
    exit(exitCode);
}
