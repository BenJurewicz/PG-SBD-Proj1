#include "buffer.hpp"

#include <format>
#include <stdexcept>

Buffer::Buffer(
    std::shared_ptr<BufferedFile> file, size_t startPageIndex, size_t pageCount
)
    : startPageIndex(startPageIndex), pageCount(pageCount), file(file) {}

void Buffer::readPage(size_t pageIndex) {
    auto p = file->readPage(pageIndex);
    if (!p.has_value()) {
        throw std::invalid_argument(
            std::format("Invalid pageIndex provided, pageIndex={}", pageIndex)
        );
    }
    page = p.value();
}
