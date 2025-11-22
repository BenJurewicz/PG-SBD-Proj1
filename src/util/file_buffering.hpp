#ifndef FILE_BUFFERING_HPP
#define FILE_BUFFERING_HPP

#include <compare>
#include <concepts>
#include <cstddef>
#include <error.hpp>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <optional>
#include <record.hpp>
#include <utility>
#include <vector>

template <typename R>
concept RangeOfRecords = std::ranges::range<R> &&
                         std::same_as<std::ranges::range_value_t<R>, Record>;

class BufferedFile {
   public:
    class PageProxy;
    class PageIterator;

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
    void writePage(RangeOfRecords auto const& page) {
        writePage(currentPageIndex, page);
    }

    void writePage(size_t pageIndex, RangeOfRecords auto const& newPage) {
        if (pageIndex > getPageCount()) {
            THROW_FORMATTED(
                std::out_of_range,
                "Writing Page failed. "
                "Provided pageIndex={} is beyond current file "
                "content and is not an append.",
                pageIndex
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

    PageIterator begin();
    PageIterator end();

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

class BufferedFile::PageProxy {
   public:
    friend class PageIterator;

    operator std::vector<Record>() const;
    std::vector<Record> records() const;
    Record operator[](size_t recordIndexInPage) const;

    void operator=(RangeOfRecords auto const& newPage) {
        file->writePage(pageIndex, newPage);
    }
    auto operator<=>(const PageProxy& other) const = default;

   private:
    PageProxy(BufferedFile* file, size_t pageIndex);

    BufferedFile* file;
    size_t pageIndex;
};

class BufferedFile::PageIterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = PageProxy;
    using difference_type = std::ptrdiff_t;
    using pointer = PageProxy;
    using reference = PageProxy;

    PageIterator(BufferedFile* file, size_t pageIndex);

    reference operator*();

    pointer operator->();

    PageIterator& operator++();
    PageIterator operator++(int);
    PageIterator& operator--();
    PageIterator operator--(int);

    PageIterator& operator+=(difference_type n);
    PageIterator& operator-=(difference_type n);

    friend PageIterator operator+(PageIterator it, difference_type n);
    friend PageIterator operator+(difference_type n, PageIterator it);
    friend PageIterator operator-(PageIterator it, difference_type n);
    friend difference_type operator-(
        const PageIterator& a, const PageIterator& b
    );

    reference operator[](difference_type n) const;

    bool operator==(const PageIterator& other) const;
    std::partial_ordering operator<=>(const PageIterator& other) const;

   private:
    BufferedFile* file;
    size_t pageIndex;
};

#endif
