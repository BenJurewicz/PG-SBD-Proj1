#include "buffer.hpp"

#include "file_buffering.hpp"

Buffer::Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end)
    : current(begin), begin(begin), end(end) {
    page = *begin;
}

bool Buffer::empty() { return current == end; }
