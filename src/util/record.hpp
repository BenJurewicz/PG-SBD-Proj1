#include <cstddef>
#include <string>
#ifndef RECORD_HPP

class Record {
   public:
    static constexpr size_t maxLen = 30;
    static const Record empty;

    Record();
    Record(const std::string& str);
    Record(size_t count, char c);
    Record(const char* cStr);

    const std::string& data() const;
    size_t lenght() const;
    void resize(size_t size);

    auto operator<=>(const Record& other) const = default;
    bool operator==(const Record& other) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Record& r);

   private:
    std::string _data;
};

#endif  // !RECORD_HPP
