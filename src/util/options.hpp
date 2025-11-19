#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <cstddef>
#include <mutex>
class Options {
   public:
    Options(int argc, char* argv[]);

   private:
    size_t bufferCount = 5;
    size_t recordsPerPage = 20;  // Blocking factor
    bool printing = true;
    std::once_flag init_flag;

    void parseOptions(int argc, char* argv[]);

    void reportIncorret(int argc, char* argv[]);
    void parseBufferCount(int argc, char* argv[]);
    void parseRecordsPerPage(int argc, char* argv[]);
    void parsePrinting(int argc, char* argv[]);
    void parseHelp(int argc, char* argv[]);
    void printHelp();
};

#endif  // !OPTIONS_HPP
