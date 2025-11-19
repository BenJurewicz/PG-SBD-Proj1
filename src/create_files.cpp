#include <fstream>
#include <iostream>
#include <random>
#include <record.hpp>
#include <string>
#include <vector>

std::default_random_engine rng{std::random_device{}()};

std::string generate_random_string(int maxLength) {
    std::uniform_int_distribution<int> len_dist(1, maxLength);
    // ASCII printable characters are in rage [32,126]
    // but whe start from 33 as (char)32 is space.
    // NOTE: (char)33 == '!'; (char)126 == '~';
    // TODO: Ocasinally got non ascii chars here
    std::uniform_int_distribution<int> char_dist(33, 126);

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
    std::cout
        << "Enter lines of text (up to 30 characters). Type '!q' to finish."
        << std::endl;

    while (true) {
        std::getline(std::cin, line);

        if (line == "!q") {
            break;
        }

        if (line.length() > Record::maxLen) {
            std::cout
                << "Error: String exceeds 30 characters. Please try again."
                << std::endl;
            break;
        }

        lines.push_back(line);
    }
}

void auto_mode(std::vector<std::string>& lines) {
    int num_strings;
    std::cout << "Enter the number of strings to generate: ";
    std::cin >> num_strings;
    std::cin.ignore();

    for (int i = 0; i < num_strings; ++i) {
        lines.push_back(generate_random_string(Record::maxLen));
    }
    std::cout << num_strings << " random strings generated." << std::endl;
}

void write_line(std::ofstream& file, const std::string& data) {
    auto tempStr = data;
    tempStr.resize(Record::maxLen);
    std::string nulls('\0', Record::maxLen - data.length());
    file << tempStr << nulls;
}

void save_file(std::vector<std::string>& lines) {
    std::string filename;
    std::cout
        << "Enter filename to save to (if empty defaults to \"data.txt\"): ";
    std::getline(std::cin, filename);
    if (filename.empty()) {
        filename = "data.txt";
    }
    filename = "data/" + filename;

    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        for (const auto& line : lines) {
            write_line(outfile, line);
        }
        outfile.close();
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Error: Could not open file for writing." << std::endl;
    }
}

int get_user_choice() {
    int choice;
    std::cout << "\nChoose an option:" << std::endl;
    std::cout << "1. Manual input" << std::endl;
    std::cout << "2. Automatic generation" << std::endl;
    std::cout << "3. Save and quit" << std::endl;
    std::cout << "Enter your choice: ";
    std::cin >> choice;
    std::cin.ignore();
    return choice;
}

int main() {
    std::vector<std::string> lines;
    int choice;

    do {
        choice = get_user_choice();
        switch (choice) {
            case 1:
                manual_mode(lines);
                break;
            case 2:
                auto_mode(lines);
                break;
            case 3:
                save_file(lines);
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    } while (choice != 3);

    return 0;
}
