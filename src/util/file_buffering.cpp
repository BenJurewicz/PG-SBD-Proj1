#include <cstddef>
#include <file_buffering.hpp>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <string>

BufferedFile::BufferedFile(const char* fileName)
    : file(fileName, std::ios::in | std::ios::out), block(recordsPerBlock) {
    // If file does not exist create it
    if (!file.is_open()) {
        file.open(fileName, std::ios::out);
        file.close();
        file.open(fileName, std::ios::out | std::ios::in);
    }
    loadBlock(0);
};

BufferedFile::~BufferedFile() { flush(); }

void BufferedFile::loadBlock(size_t blockIndex) {
    if (blockIndex == currentBlockIndex) {
        return;
    }
    flush();

    size_t offset = bIndexToOffset(blockIndex);
    file.seekg(offset, std::ios::beg);

    block.clear();
    size_t i = 0;
    std::string emptyStr(recordSize, '\0');    // NOTE: String specific
    std::string currentStr(recordSize, '\0');  // NOTE: String specific

    while (i < recordsPerBlock && file.good()) {
        file.read(currentStr.data(), recordSize);
        block.push_back(currentStr);
        currentStr = emptyStr;
        i++;
    }

    file.clear();  // Clear flags in case we stumbled upon eof

    // Fill the block in case we run into the end of the file
    while (i < recordsPerBlock) {
        block.emplace_back(30, '\0');
        i++;
    }

    currentBlockIndex = blockIndex;
}

void BufferedFile::flush() {
    if (!isBlockModified) {
        return;
    }

    std::fstream::off_type offset = bIndexToOffset(currentBlockIndex);
    seekpWithExtend(offset, std::ios::beg);

    for (const auto& line : block) {
        file.write(line.data(), line.length());
        // file.write("\n", 1);
    }

    file.flush();
    isBlockModified = false;
}

size_t BufferedFile::rIndexToBlockIndex(size_t index) {
    return index / recordsPerBlock;
}

size_t BufferedFile::rIndexToInBlockIndex(size_t index) {
    return (index % recordsPerBlock);
}

size_t BufferedFile::bIndexToOffset(size_t blockIndex) {
    return blockIndex * blockSize;
}

Record BufferedFile::read(size_t index) {
    size_t blockIndex = rIndexToBlockIndex(index);
    size_t inBlockIndex = rIndexToInBlockIndex(index);
    loadBlock(blockIndex);
    return block.at(inBlockIndex);
}

void BufferedFile::write(size_t index, Record data) {
    size_t blockIndex = rIndexToBlockIndex(index);
    size_t inBlockIndex = rIndexToInBlockIndex(index);
    loadBlock(blockIndex);

    data.resize(recordSize);  // NOTE: String specific

    block.at(inBlockIndex) = data;
    isBlockModified = true;
}

std::streampos BufferedFile::getFileSize() {
    file.seekg(0, std::ios::end);
    return file.tellg();
}

void BufferedFile::extendFile(std::streampos size) {
    auto curentSize = getFileSize();
    if (curentSize >= size) {
        return;
    }
    file.seekp(0, std::ios::end);
    auto diffSize = size - curentSize;
    std::string str('\0', recordSize);  // NOTE: String specific
    for (auto i = 0; i < diffSize; i += recordSize) {
        file.write(str.data(), str.length());
    }
}

void BufferedFile::seekpWithExtend(std::ifstream::off_type offset,
                                   std::ios_base::seekdir dir) {
    extendFile(offset + blockSize);
    file.seekp(offset, dir);
}
