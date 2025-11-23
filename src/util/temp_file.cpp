#include "temp_file.hpp"

#include <filesystem>
#include <format>

size_t TempFile::counter = 0;

std::string TempFile::generate_path() {
    const std::filesystem::path dir = "temp";

    std::filesystem::create_directories(dir);
    return (dir / std::format("tempFile{}", counter++));
}

TempFile::TempFile() : file(generate_path()) {}

TempFile::operator BufferedFile&() { return file; }
