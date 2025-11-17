#include <string>
#ifndef RECORD_HPP

class Record {
   public:
    Record() = default;
    Record(const std::string& str);
    const std::string& data() const;

   private:
    std::string _data;
};

#endif  // !RECORD_HPP
