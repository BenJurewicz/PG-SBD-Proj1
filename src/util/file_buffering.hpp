#ifndef FILE_BUFFERING_HPP
#define FILE_BUFFERING_HPP

#include <compare>
#include <concepts>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <optional>
#include <record.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename R>
concept RangeOfRecords = std::ranges::range<R> &&
                         std::same_as<std::ranges::range_value_t<R>, Record>;

class BufferedFile {
   public:
    class PageIterator;

    class PageProxy {
       public:
        friend class PageIterator;

        operator std::vector<Record>() const { return records(); }

        std::vector<Record> records() const {
            auto pageOpt = file->readPage(pageIndex);
            if (pageOpt) {
                return *pageOpt;
            }
            return {};
        }

        Record operator[](size_t recordIndexInPage) const {
            if (recordIndexInPage >= recordsPerPage) {
                throw std::out_of_range("Record index out of page bounds");
            }
            return file->read(pageIndex * recordsPerPage + recordIndexInPage);
        }

        void operator=(RangeOfRecords auto const& newPage) {
            file->writePage(pageIndex, newPage);
        }

        auto operator<=>(const PageProxy& other) const = default;

       private:
        PageProxy(BufferedFile* file, size_t pageIndex)
            : file(file), pageIndex(pageIndex) {}

        void setPageIndex(size_t newPageIndex) { pageIndex = newPageIndex; }

        BufferedFile* file;
        size_t pageIndex;
    };

    class PageIterator {
       public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = PageProxy;
        using difference_type = std::ptrdiff_t;
        using pointer = PageProxy*;
        using reference = PageProxy;

        PageIterator(BufferedFile* file, size_t pageIndex)
            : pageIndex(pageIndex), proxy(file, pageIndex) {}

        reference operator*() {
            proxy.setPageIndex(pageIndex);
            return proxy;
        }

        pointer operator->() {
            proxy.setPageIndex(pageIndex);
            return &proxy;
        }

        PageIterator& operator++() {
            pageIndex++;
            return *this;
        }

        PageIterator operator++(int) {
            PageIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        PageIterator& operator--() {
            pageIndex--;
            return *this;
        }

        PageIterator operator--(int) {
            PageIterator tmp = *this;
            --(*this);
            return tmp;
        }

        PageIterator& operator+=(difference_type n) {
            pageIndex += n;
            return *this;
        }

        PageIterator& operator-=(difference_type n) {
            pageIndex -= n;
            return *this;
        }

        friend PageIterator operator+(PageIterator it, difference_type n) {
            it += n;
            return it;
        }

        friend PageIterator operator+(difference_type n, PageIterator it) {
            it += n;
            return it;
        }

        friend PageIterator operator-(PageIterator it, difference_type n) {
            it -= n;
            return it;
        }

        friend difference_type operator-(
            const PageIterator& a, const PageIterator& b
        ) {
            return a.pageIndex - b.pageIndex;
        }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const PageIterator& other) const {
            return pageIndex == other.pageIndex;
        }

        auto operator<=>(const PageIterator& other) const {
            return pageIndex <=> other.pageIndex;
        }

       private:
        size_t pageIndex;
        PageProxy proxy;
    };

    PageIterator begin() { return PageIterator(this, 0); };
    PageIterator end() { return PageIterator(this, getPageCount()); };

    using BufferType = std::vector<Record>;

    BufferedFile(const char* s);
    ~BufferedFile();

    Record read(size_t index);
    void write(size_t index, Record data);
    void flush();
    // Checks if the current page is empty
    bool isCurrentPageEmpty();
    // Returns the page with a given index if it exists
    std::optional<BufferType> readPage(size_t pageIndex);
    // Returns the current page and increments the page index
    std::optional<BufferType> readPage();
    // Completly overwrites the current page and increments the page index
    // TODO: This should call writePageAtIndex also change name of
    // writePageAtIndex
    void writePage(RangeOfRecords auto const& page) {
        writePage(currentPageIndex, page);
    }

    void writePage(size_t pageIndex, RangeOfRecords auto const& newPage) {
        if (pageIndex > getPageCount()) {
            throw std::out_of_range(
                "Cannot write: page index is beyond current file content and "
                "not an append operation."
            );
        }

        loadPage(pageIndex);

        BufferType tmp;
        tmp.reserve(recordsPerPage);

        for (auto r : newPage) {
            if (tmp.size() >= recordsPerPage) {
                break;
            }
            r.resize(recordSize);
            tmp.push_back(r);
        }

        while (tmp.size() < recordsPerPage) {
            tmp.push_back(Record::empty);
        }

        this->page = std::move(tmp);
        this->isPageModified = true;
    }

    // Resets the page index back to the first page
    void resetPageIndex();
    void setPageIndex(size_t index);
    size_t getPageIndex();

    size_t getPageCount();
    size_t getRecordCount();

    // Record Size in bytes
    static constexpr size_t recordSize = Record::maxLen;
    static constexpr size_t recordsPerPage = 10;
    // Page size in bytes
    static constexpr size_t pageSize = recordsPerPage * recordSize;

   private:
    std::fstream file;
    std::vector<Record> page;
    size_t currentPageIndex = -1;
    bool isPageModified = false;

    // Converts a record index to the corresponding page index
    size_t rIndexToPageIndex(size_t index);
    // Converts a record index within a file to a record index within the
    // corresponding page
    size_t rIndexToInPageIndex(size_t index);
    // Converts a page index into a character offset within the file
    size_t pIndexToOffset(size_t index);

    void loadPage(size_t pageIndex);
    std::streampos getFileSize();
    // Extends a file to the total desired size given in bytes
    void extendFile(std::streampos size);

    // Sets the put cursor in the desired place, in addition validates the given
    // offset and extends the file if needed
    void seekpWithExtend(
        std::ifstream::off_type offset, std::ios_base::seekdir dir
    );
};

#endif
