#include "buffer.hpp"

#include "file_buffering.hpp"

Buffer::Buffer() : mode(Mode::UNINITIALIZED) {}

Buffer::Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end)
    : mode(Mode::INPUT),
      itBegin(begin),
      itCurrent(begin),
      itEnd(end),
      currentPageIndex(0) {
    if (*itBegin != *itEnd) {
        page = **itCurrent;
        size_t pageCount = *itEnd - *itBegin;
        recordCount = pageCount * BufferedFile::recordsPerPage;
    } else {
        recordCount = 0;
    }
}

Buffer::Buffer(
    std::ranges::subrange<
        BufferedFile::PageIterator, BufferedFile::PageSentinel>
        range
)
    : mode(Mode::OUTPUT), outIter(range.begin()) {
    page.reserve(BufferedFile::recordsPerPage);
}

bool Buffer::empty() const {
    if (mode == Mode::INPUT) {
        return recordCount == 0;
    }
    return true;
}

Record Buffer::operator[](size_t index) {
    if (mode != Mode::INPUT || !itBegin.has_value()) {
        throw std::logic_error("operator[] called on non-input buffer");
    }

    size_t pageToLoad = index / BufferedFile::recordsPerPage;
    size_t indexInPage = index % BufferedFile::recordsPerPage;

    if (pageToLoad != currentPageIndex) {
        itCurrent = *itBegin;
        std::advance(*itCurrent, pageToLoad);
        page = **itCurrent;
        currentPageIndex = pageToLoad;
    }

    return page[indexInPage];
}

void Buffer::append(const Record& r) {
    if (mode != Mode::OUTPUT) {
        throw std::logic_error("push_back called on non-output buffer");
    }

    page.push_back(r);
    writtenRecordsInPage++;

    if (writtenRecordsInPage == BufferedFile::recordsPerPage) {
        flush();
    }
}

size_t Buffer::size() const {
    if (mode == Mode::INPUT) {
        return recordCount;
    }
    return 0;
}

Buffer::~Buffer() { flush(); }

void Buffer::flush() {
    if (mode == Mode::OUTPUT && !page.empty() && outIter.has_value()) {
        **outIter = page;
        ++(*outIter);
        page.clear();
        writtenRecordsInPage = 0;
    }
}
