#ifndef TEMP_FILE_HPP
#define TEMP_FILE_HPP

#include "file_buffering.hpp"

class TempFile {
   public:
    TempFile();
    ~TempFile();

    operator BufferedFile&();
    std::string getFileName();

   private:
    static size_t counter;

    const std::filesystem::path filePath;
    BufferedFile file;

    static std::string generate_path();
};

#endif  // !TEMP_FILE_HPP
