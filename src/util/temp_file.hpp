#ifndef TEMP_FILE_HPP
#define TEMP_FILE_HPP

#include "file_buffering.hpp"

class TempFile {
   public:
    TempFile();

    operator BufferedFile&();

   private:
    static size_t counter;
    BufferedFile file;

    static std::string generate_path();
};

#endif  // !TEMP_FILE_HPP
