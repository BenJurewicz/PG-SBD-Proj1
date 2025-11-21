#include "buffer.hpp"

#include <format>
#include <stdexcept>

Buffer::Buffer(
    std::shared_ptr<BufferedFile> file, size_t startPageIndex, size_t pageCount
)
    : startPageIndex(startPageIndex), pageCount(pageCount), file(file) {
    setPage(startPageIndex);
}

void Buffer::setPage(size_t pageIndex) {
    auto p = file->readPage(pageIndex);
    if (!p.has_value()) {
        throw std::invalid_argument(
            std::format(
                "\n{}:{}:\nFunction: {}\nInvalid pageIndex provided, "
                "pageIndex={}",
                __FILE__,
                __LINE__,
                __PRETTY_FUNCTION__,
                pageIndex
            )
        );
    }
    page = p.value();
}
