#include <cstddef>
#include <file_buffering.hpp>
#include <fstream>
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

    // TODO: See what happens when the seekp here goes beyond eof
    size_t offset = bIndexToOffset(currentBlockIndex);
    file.seekg(offset, std::ios::beg);

    block.clear();
    size_t i = 0;
    std::string str;
    while (i < recordsPerBlock && std::getline(file, str)) {
        block.push_back(str);
        i++;
    }
    file.clear();  // Clear flags in case we stumbled upon eof

    // Fill the block in case we run into the end of the file
    while (i < recordsPerBlock) {
        block.push_back("");
        i++;
    }

    currentBlockIndex = blockIndex;
}

void BufferedFile::flush() {
    if (!isBlockModified) {
        return;
    }

    // TODO: See what happens when the seekp here goes beyond eof
    size_t offset = bIndexToOffset(currentBlockIndex);
    file.seekp(offset, std::ios::beg);

    for (const auto& line : block) {
        file.write(line.c_str(), line.length());
        file.write("\n", 1);
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

    data.resize(recordSize);

    block.at(inBlockIndex) = data;
    isBlockModified = true;
}
