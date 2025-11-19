#include <cstddef>
#include <string>
#ifndef RECORD_HPP

class Record {
   public:
    Record();
    Record(const std::string& str);
    Record(size_t count, char c);
    Record(const char* cStr);

    const std::string& data() const;
    size_t lenght() const;
    void resize(size_t size);

    bool operator<=>(const Record& other) const = default;

   private:
    std::string _data;
};

#endif  // !RECORD_HPP
