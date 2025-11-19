#ifndef FILE_BUFFERING_HPP
#define FILE_BUFFERING_HPP

#include <cstddef>
#include <defines.hpp>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <optional>
#include <record.hpp>
#include <vector>

class BufferedFile {
   public:
    using BufferType = std::vector<Record>;

    BufferedFile(const char* s);
    ~BufferedFile();

    Record read(size_t index);
    void write(size_t index, Record data);
    void flush();
    bool isEmpty();
    std::optional<BufferType> getNextPage();
    void resetPageIndex();

   private:
    static constexpr size_t recordSize = MAX_STRING_LENGTH;
    static constexpr size_t recordsPerPage = 20;
    static constexpr size_t pageSize = recordsPerPage * recordSize;

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
    size_t bIndexToOffset(size_t index);

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
