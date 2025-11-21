#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <memory>
#include <vector>

#include "file_buffering.hpp"
#include "record.hpp"

class Buffer {
    Buffer(
        std::shared_ptr<BufferedFile> file, size_t startPageIndex,
        size_t pageCount
    );

   private:
    size_t startPageIndex;
    size_t pageCount;

    std::vector<Record> page;
    std::shared_ptr<BufferedFile> file;

    void readPage(size_t pageIndex);
};

#endif  // !BUFFER_HPPP
