#include "buffer.hpp"

#include "file_buffering.hpp"

Buffer::Buffer() : type(Type::UNINITIALIZED) {}

Buffer::Buffer(BufferedFile::PageIterator begin, BufferedFile::PageIterator end)
    : type(Type::INPUT),
      it_begin(begin),
      it_current(begin),
      it_end(end),
      current_page_idx(0) {
    if (it_begin.has_value() && *it_begin != *it_end) {
        // TODO: Simplify how the records are counted
        // check if there is some std function to get the size of a iterator
        page = **it_current;
        record_count = page.size();
        auto it = *it_begin;
        it++;
        while (it != *it_end) {
            record_count += std::vector<Record>(*it).size();
            it++;
        }
    } else {
        record_count = 0;
    }
}

Buffer::Buffer(
    std::ranges::subrange<
        BufferedFile::PageIterator, BufferedFile::PageSentinel>
        range
)
    : type(Type::OUTPUT), output_iterator(range.begin()) {
    page.reserve(BufferedFile::recordsPerPage);
}

bool Buffer::empty() const {
    if (type == Type::INPUT) {
        return record_count == 0;
    }
    return true;
}

Record Buffer::operator[](size_t index) {
    if (type != Type::INPUT || !it_begin.has_value()) {
        throw std::logic_error("operator[] called on non-input buffer");
    }

    size_t page_to_load = index / BufferedFile::recordsPerPage;
    size_t idx_in_page = index % BufferedFile::recordsPerPage;

    if (page_to_load != current_page_idx) {
        it_current = *it_begin;
        std::advance(*it_current, page_to_load);
        page = **it_current;
        current_page_idx = page_to_load;
    }

    return page[idx_in_page];
}

void Buffer::append(const Record& r) {
    if (type != Type::OUTPUT) {
        throw std::logic_error("push_back called on non-output buffer");
    }

    page.push_back(r);
    written_records_in_page++;

    if (written_records_in_page == BufferedFile::recordsPerPage) {
        flush_output();
    }
}

size_t Buffer::size() const {
    if (type == Type::INPUT) {
        return record_count;
    }
    return 0;
}

Buffer::~Buffer() { flush_output(); }

void Buffer::flush_output() {
    if (type == Type::OUTPUT && !page.empty() && output_iterator.has_value()) {
        **output_iterator = page;
        ++(*output_iterator);
        page.clear();
        written_records_in_page = 0;
    }
}
