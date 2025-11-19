#include <defines.hpp>
#include <ostream>
#include <record.hpp>

Record const Record::empty = Record();

Record::Record() : _data(MAX_STRING_LENGTH, '\0') {}
Record::Record(const std::string& str) : _data(str) {}
Record::Record(size_t count, char c) : _data(count, c) {}
Record::Record(const char* cStr) : _data(cStr) {}

const std::string& Record::data() const { return _data; }
size_t Record::lenght() const { return _data.length(); }
void Record::resize(size_t size) { _data.resize(size); }

std::ostream& operator<<(std::ostream& os, const Record& r) {
    os << r.data();
    return os;
}
