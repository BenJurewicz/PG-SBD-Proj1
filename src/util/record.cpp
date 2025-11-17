#include <record.hpp>

Record::Record(const std::string& str) : _data(str) {}
const std::string& Record::data() const { return _data; }
