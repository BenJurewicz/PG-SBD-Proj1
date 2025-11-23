#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <optional>
#include <ranges>
#include <vector>

#include "file_buffering.hpp"
#include "record.hpp"

class Buffer {
   public:
    // TODO: Rename to mode
    enum class Mode { UNINITIALIZED, INPUT, OUTPUT };

    Buffer();

    Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end);
    Buffer(
        std::ranges::subrange<
            BufferedFile::PageIterator, BufferedFile::PageSentinel>
            range
    );

    bool empty() const;
    Record operator[](size_t index);
    void append(const Record& r);
    size_t size() const;
    ~Buffer();

   private:
    void flush();

    Mode mode = Mode::UNINITIALIZED;

    // For input
    std::optional<BufferedFile::PageIterator> itBegin;
    std::optional<BufferedFile::PageIterator> itCurrent;
    std::optional<BufferedFile::PageIterator> itEnd;
    size_t recordCount = 0;
    size_t currentPageIndex = -1;

    // For output
    std::optional<BufferedFile::PageIterator> outIter;
    size_t writtenRecordsInPage = 0;

    std::vector<Record> page;
};

#endif  // !BUFFER_HPP
