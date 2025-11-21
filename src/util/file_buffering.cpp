#include <algorithm>
#include <cmath>
#include <cstddef>
#include <file_buffering.hpp>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <optional>
#include <string>

BufferedFile::BufferedFile(const char* fileName)
    : file(fileName, std::ios::in | std::ios::out) {
    // If file does not exist create it
    if (!file.is_open()) {
        file.open(fileName, std::ios::out);
        file.close();
        file.open(fileName, std::ios::out | std::ios::in);
    }
    loadPage(0);
};

BufferedFile::~BufferedFile() { flush(); }

Record BufferedFile::read(size_t index) {
    size_t pageIndex = rIndexToPageIndex(index);
    if (pageIndex >= getPageCount()) {
        throw std::out_of_range("Record index is out of bounds.");
    }
    size_t inPageIndex = rIndexToInPageIndex(index);
    loadPage(pageIndex);
    return page.at(inPageIndex);
}

void BufferedFile::write(size_t index, Record data) {
    size_t pageIndex = rIndexToPageIndex(index);

    if (pageIndex > getPageCount()) {
        throw std::out_of_range(
            "Cannot write: record index is beyond current file content and not "
            "an append operation."
        );
    }

    size_t inPageIndex = rIndexToInPageIndex(index);
    loadPage(pageIndex);

    data.resize(recordSize);

    page.at(inPageIndex) = data;
    isPageModified = true;
}

void BufferedFile::flush() {
    if (!isPageModified) {
        return;
    }

    std::fstream::off_type offset = pIndexToOffset(currentPageIndex);
    seekpWithExtend(offset, std::ios::beg);

    for (const auto& line : page) {
        file.write(line.data().data(), line.lenght());
    }

    file.flush();
    isPageModified = false;
}

bool BufferedFile::isCurrentPageEmpty() {
    return std::ranges::all_of(page, [](auto& s) {
        return s == Record::empty;
    });
}

std::optional<BufferedFile::BufferType> BufferedFile::readPage(
    size_t pageIndex
) {
    if (pageIndex >= getPageCount()) {
        throw std::out_of_range("Page index is out of bounds.");
    }
    loadPage(pageIndex);
    return isCurrentPageEmpty() ? std::nullopt : std::optional(page);
}

std::optional<BufferedFile::BufferType> BufferedFile::readPage() {
    return readPage(currentPageIndex);
}

void BufferedFile::resetPageIndex() { loadPage(0); }
void BufferedFile::setPageIndex(size_t index) { loadPage(index); }
size_t BufferedFile::getPageIndex() { return currentPageIndex; }

size_t BufferedFile::getPageCount() {
    return std::ceil(static_cast<float>(getRecordCount()) / recordsPerPage);
}

size_t BufferedFile::getRecordCount() {
    flush();
    file.seekg(0, std::ios::end);
    auto lastFileIndex = static_cast<float>(file.tellg());
    return std::ceil(lastFileIndex / recordSize);
}

BufferedFile::PageIterator BufferedFile::begin() {
    return PageIterator(this, 0);
};
BufferedFile::PageIterator BufferedFile::end() {
    return PageIterator(this, getPageCount());
};

size_t BufferedFile::rIndexToPageIndex(size_t index) {
    return index / recordsPerPage;
}

size_t BufferedFile::rIndexToInPageIndex(size_t index) {
    return (index % recordsPerPage);
}

size_t BufferedFile::pIndexToOffset(size_t pageIndex) {
    return pageIndex * pageSize;
}

void BufferedFile::loadPage(size_t pageIndex) {
    if (pageIndex == currentPageIndex) {
        return;
    }
    flush();

    size_t offset = pIndexToOffset(pageIndex);
    file.seekg(offset, std::ios::beg);

    page.clear();
    size_t i = 0;
    std::string emptyStr(recordSize, '\0');
    std::string currentStr = emptyStr;

    while (i < recordsPerPage && file.good()) {
        file.read(currentStr.data(), recordSize);
        page.emplace_back(currentStr);
        currentStr = emptyStr;
        i++;
    }

    file.clear();  // Clear flags in case we stumbled upon eof

    // Fill the page in case we run into the end of the file
    // TODO: Use page.resize(pageSize, Record::empty); here
    while (i < recordsPerPage) {
        page.push_back(Record::empty);
        i++;
    }

    currentPageIndex = pageIndex;
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
    std::string str;
    for (auto i = 0; i < diffSize; i += recordSize) {
        file.write(str.data(), str.length());
    }
}

void BufferedFile::seekpWithExtend(
    std::ifstream::off_type offset, std::ios_base::seekdir dir
) {
    extendFile(offset + pageSize);
    file.seekp(offset, dir);
}

// ============================================================================
// PageProxy
// ============================================================================

BufferedFile::PageProxy::PageProxy(BufferedFile* file, size_t pageIndex)
    : file(file), pageIndex(pageIndex) {}

BufferedFile::PageProxy::operator std::vector<Record>() const {
    return records();
}

std::vector<Record> BufferedFile::PageProxy::records() const {
    auto pageOpt = file->readPage(pageIndex);
    if (pageOpt) {
        return *pageOpt;
    }
    return {};
}

Record BufferedFile::PageProxy::operator[](size_t recordIndexInPage) const {
    if (recordIndexInPage >= recordsPerPage) {
        throw std::out_of_range("Record index out of page bounds");
    }
    return file->read(pageIndex * recordsPerPage + recordIndexInPage);
}

void BufferedFile::PageProxy::setPageIndex(size_t newPageIndex) {
    pageIndex = newPageIndex;
}

// ============================================================================
// PageIterator
// ============================================================================

BufferedFile::PageIterator::PageIterator(BufferedFile* file, size_t pageIndex)
    : pageIndex(pageIndex), proxy(file, pageIndex) {}

BufferedFile::PageIterator::reference BufferedFile::PageIterator::operator*() {
    proxy.setPageIndex(pageIndex);
    return proxy;
}

BufferedFile::PageIterator::pointer BufferedFile::PageIterator::operator->() {
    proxy.setPageIndex(pageIndex);
    return &proxy;
}

BufferedFile::PageIterator& BufferedFile::PageIterator::operator++() {
    pageIndex++;
    return *this;
}

BufferedFile::PageIterator BufferedFile::PageIterator::operator++(int) {
    PageIterator tmp = *this;
    ++(*this);
    return tmp;
}

BufferedFile::PageIterator& BufferedFile::PageIterator::operator--() {
    pageIndex--;
    return *this;
}

BufferedFile::PageIterator BufferedFile::PageIterator::operator--(int) {
    PageIterator tmp = *this;
    --(*this);
    return tmp;
}

BufferedFile::PageIterator& BufferedFile::PageIterator::operator+=(
    difference_type n
) {
    pageIndex += n;
    return *this;
}

BufferedFile::PageIterator& BufferedFile::PageIterator::operator-=(
    difference_type n
) {
    pageIndex -= n;
    return *this;
}

BufferedFile::PageIterator operator+(
    BufferedFile::PageIterator it, BufferedFile::PageIterator::difference_type n
) {
    it += n;
    return it;
}

BufferedFile::PageIterator operator+(
    BufferedFile::PageIterator::difference_type n, BufferedFile::PageIterator it
) {
    it += n;
    return it;
}

BufferedFile::PageIterator operator-(
    BufferedFile::PageIterator it, BufferedFile::PageIterator::difference_type n
) {
    it -= n;
    return it;
}

BufferedFile::PageIterator::difference_type operator-(
    const BufferedFile::PageIterator& a, const BufferedFile::PageIterator& b
) {
    return a.pageIndex - b.pageIndex;
}

BufferedFile::PageIterator::reference BufferedFile::PageIterator::operator[](
    difference_type n
) const {
    return *(*this + n);
}

bool BufferedFile::PageIterator::operator==(const PageIterator& other) const {
    return pageIndex == other.pageIndex;
}
