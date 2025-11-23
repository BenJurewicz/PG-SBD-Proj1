#include "buffer.hpp"

#include "file_buffering.hpp"

Buffer::Buffer() : type(Type::UNINITIALIZED) {}

Buffer::Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end)
    : type(Type::INPUT),
      itBegin(begin),
      itCurrent(begin),
      itEnd(end),
      currentPageIndex(0) {
    if (itBegin.has_value() && *itBegin != *itEnd) {
        // TODO: Simplify how the records are counted
        // check if there is some std function to get the size of a iterator
        page = **itCurrent;
        recordCount = page.size();
        auto it = *itBegin;
        it++;
        while (it != *itEnd) {
            recordCount += std::vector<Record>(*it).size();
            it++;
        }
    } else {
        recordCount = 0;
    }
}

Buffer::Buffer(
    std::ranges::subrange<
        BufferedFile::PageIterator, BufferedFile::PageSentinel>
        range
)
    : type(Type::OUTPUT), outIter(range.begin()) {
    page.reserve(BufferedFile::recordsPerPage);
}

bool Buffer::empty() const {
    if (type == Type::INPUT) {
        return recordCount == 0;
    }
    return true;
}

Record Buffer::operator[](size_t index) {
    if (type != Type::INPUT || !itBegin.has_value()) {
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
    if (type != Type::OUTPUT) {
        throw std::logic_error("push_back called on non-output buffer");
    }

    page.push_back(r);
    writtenRecordsInPage++;

    if (writtenRecordsInPage == BufferedFile::recordsPerPage) {
        flush();
    }
}

size_t Buffer::size() const {
    if (type == Type::INPUT) {
        return recordCount;
    }
    return 0;
}

Buffer::~Buffer() { flush(); }

void Buffer::flush() {
    if (type == Type::OUTPUT && !page.empty() && outIter.has_value()) {
        **outIter = page;
        ++(*outIter);
        page.clear();
        writtenRecordsInPage = 0;
    }
}
