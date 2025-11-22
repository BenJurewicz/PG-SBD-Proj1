#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <vector>

#include "file_buffering.hpp"
#include "record.hpp"

class Buffer {
   public:
    Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end);

    Buffer operator=(std::ranges::subrange<
                     BufferedFile::PageIterator, BufferedFile::PageSentinel>
                         range);

    bool empty();
    Record operator[](size_t index);
    void push_back(Record r);
    size_t size();
    void clear();

   private:
    BufferedFile::PageIterator current;
    BufferedFile::PageIterator begin;
    BufferedFile::PageIterator end;

    size_t startIndex, endIndex;

    std::vector<Record> page;
};

#endif  // !BUFFER_HPP
