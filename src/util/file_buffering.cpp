#include <cstddef>
#include <file_buffering.hpp>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <string>

BufferedFile::BufferedFile(const char* fileName)
    : file(fileName, std::ios::in | std::ios::out), page(recordsPerPage) {
    // If file does not exist create it
    if (!file.is_open()) {
        file.open(fileName, std::ios::out);
        file.close();
        file.open(fileName, std::ios::out | std::ios::in);
    }
    loadPage(0);
};

BufferedFile::~BufferedFile() { flush(); }

void BufferedFile::loadPage(size_t pageIndex) {
    if (pageIndex == currentPageIndex) {
        return;
    }
    flush();

    size_t offset = bIndexToOffset(pageIndex);
    file.seekg(offset, std::ios::beg);

    page.clear();
    size_t i = 0;
    std::string emptyStr(recordSize, '\0');    // NOTE: String specific
    std::string currentStr(recordSize, '\0');  // NOTE: String specific

    while (i < recordsPerPage && file.good()) {
        file.read(currentStr.data(), recordSize);
        page.push_back(currentStr);
        currentStr = emptyStr;
        i++;
    }

    file.clear();  // Clear flags in case we stumbled upon eof

    // Fill the page in case we run into the end of the file
    // TODO: Change to std::vector::assign
    while (i < recordsPerPage) {
        page.emplace_back(30, '\0');
        i++;
    }

    currentPageIndex = pageIndex;
}

void BufferedFile::flush() {
    if (!isPageModified) {
        return;
    }

    std::fstream::off_type offset = bIndexToOffset(currentPageIndex);
    seekpWithExtend(offset, std::ios::beg);

    for (const auto& line : page) {
        file.write(line.data(), line.length());
        // file.write("\n", 1);
    }

    file.flush();
    isPageModified = false;
}

size_t BufferedFile::rIndexToPageIndex(size_t index) {
    return index / recordsPerPage;
}

size_t BufferedFile::rIndexToInPageIndex(size_t index) {
    return (index % recordsPerPage);
}

size_t BufferedFile::bIndexToOffset(size_t pageIndex) {
    return pageIndex * pageSize;
}

Record BufferedFile::read(size_t index) {
    size_t pageIndex = rIndexToPageIndex(index);
    size_t inPageIndex = rIndexToInPageIndex(index);
    loadPage(pageIndex);
    return page.at(inPageIndex);
}

void BufferedFile::write(size_t index, Record data) {
    size_t pageIndex = rIndexToPageIndex(index);
    size_t inPageIndex = rIndexToInPageIndex(index);
    loadPage(pageIndex);

    data.resize(recordSize);  // NOTE: String specific

    page.at(inPageIndex) = data;
    isPageModified = true;
}

std::streampos BufferedFile::getFileSize() {
    file.seekg(0, std::ios::end);
    return file.tellg();
}

void BufferedFile::extendFile(std::streampos size) {
    auto curentSize = getFileSize();
    if (curentSize >= size) {
        return;
    }
    file.seekp(0, std::ios::end);
    auto diffSize = size - curentSize;
    std::string str('\0', recordSize);  // NOTE: String specific
    for (auto i = 0; i < diffSize; i += recordSize) {
        file.write(str.data(), str.length());
    }
}

void BufferedFile::seekpWithExtend(std::ifstream::off_type offset,
                                   std::ios_base::seekdir dir) {
    extendFile(offset + pageSize);
    file.seekp(offset, dir);
}
