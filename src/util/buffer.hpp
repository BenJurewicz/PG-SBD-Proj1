#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <vector>
#include <ranges>
#include <optional>

#include "file_buffering.hpp"
#include "record.hpp"

class Buffer {
   public:
    enum class Type { UNINITIALIZED, INPUT, OUTPUT };

    Buffer();

    // Input Buffer constructor
    Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end);

    Buffer& operator=(std::ranges::subrange<
                     BufferedFile::PageIterator, BufferedFile::PageSentinel>
                         range);

    bool empty() const;
    Record operator[](size_t index);
    void push_back(const Record& r);
    size_t size() const;
    void clear();
    ~Buffer();

   private:
    void flush_output();

    Type type = Type::UNINITIALIZED;

    // For input
    std::optional<BufferedFile::PageIterator> it_begin;
    std::optional<BufferedFile::PageIterator> it_current;
    std::optional<BufferedFile::PageIterator> it_end;
    size_t record_count = 0;
    size_t current_page_idx = -1;

    // For output
    std::optional<BufferedFile::PageIterator> output_iterator;
    size_t written_records_in_page = 0;

    std::vector<Record> page;
};

#endif  // !BUFFER_HPP