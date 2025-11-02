#ifndef FILE_BUFFERING_HPP
#define FILE_BUFFERING_HPP

#include <cstddef>
#include <defines.hpp>
#include <fstream>
#include <vector>

class BufferedFile {
    using BufferType = std::vector<Record>;

   public:
    BufferedFile(const char* s);
    ~BufferedFile();

    Record read(size_t index);
    void write(size_t index, Record data);
    void flush();

   private:
    static constexpr size_t recordSize = MAX_STRING_LENGTH;
    static constexpr size_t recordsPerBlock = 20;
    static constexpr size_t blockSize = recordsPerBlock * recordSize;

    std::fstream file;
    std::vector<Record> block;
    size_t currentBlockIndex = -1;
    bool isBlockModified = false;

    // Converts a record index to the corresponding block index
    size_t rIndexToBlockIndex(size_t index);
    // Converts a record index witinh a file to a record index within the
    // corresponding block
    size_t rIndexToInBlockIndex(size_t index);

    void loadBlock(size_t blockIndex);
};

#endif
