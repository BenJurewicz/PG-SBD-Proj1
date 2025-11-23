#include "temp_file.hpp"

#include <filesystem>
#include <format>

size_t TempFile::counter = 0;

std::string TempFile::generate_path() {
    const std::filesystem::path dir = "temp";

    std::filesystem::create_directories(dir);
    return (dir / std::format("tempFile{}", counter++));
}

TempFile::TempFile() : filePath(generate_path()), file(filePath) {}
TempFile::~TempFile() { std::filesystem::remove(filePath); }

TempFile::operator BufferedFile&() { return file; }

std::string TempFile::getFileName() { return filePath; }
