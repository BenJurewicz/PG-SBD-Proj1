#ifndef SORT_OPTIONS_HPP
#define SORT_OPTIONS_HPP

#include <cstddef>
#include <string>

class SortOptions {
   public:
    SortOptions(int argc, char** argv);

    size_t getBufferCount() const { return bufferCount; }
    size_t getBlockingFactor() const { return blockingFactor; }
    bool isLogging() const { return logging; }
    const std::string& getFileName() const { return fileName; }

   private:
    void parse(int argc, char** argv);
    void parseArgument(const std::string& arg, int& i, int argc, char** argv);
    void handleFlag(const std::string& flag, int& i, int argc, char** argv);
    void handleFileName(const std::string& name);

    /**
     * Tries to get the next argument
     *
     * Returns it as string
     *
     * If the argument is missing, exits with a error message:
     * `"Error: " << argv[i] << " requires a value.\n"`
     */
    std::string getVal(int& i, int argc, char** argv);

    void parseBufferCount(int& i, int argc, char** argv);
    void parseBlockingFactor(int& i, int argc, char** argv);

    void checkRequired() const;
    void printHelpAndExit(int exitCode = 1) const;

    size_t bufferCount = 5;
    size_t blockingFactor = 10;
    bool logging = true;
    std::string fileName;
    std::string scriptName;
};

#endif  // !SORT_OPTIONS_HPP
