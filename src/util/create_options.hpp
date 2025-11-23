#ifndef CREATE_OPTIONS_HPP
#define CREATE_OPTIONS_HPP

#include <cstddef>
#include <string>

class CreateOptions {
   public:
    CreateOptions(int argc, char** argv);

    bool isRandomMode() const { return randomMode; }
    size_t getRandomCount() const { return randomCount; }
    bool isNumbersOnly() const { return numbersOnly; }
    bool isInteractiveMode() const { return interactiveMode; }
    const std::string& getFileName() const { return fileName; }

   private:
    void parse(int argc, char** argv);
    void parseArgument(const std::string& arg, int& i, int argc, char** argv);
    void handleFlag(const std::string& flag, int& i, int argc, char** argv);

    /**
     * Tries to get the next argument
     *
     * Returns it as string
     *
     * If the argument is missing, exits with a error message:
     * `"Error: " << argv[i] << " requires a value.\n"`
     */
    std::string getVal(int& i, int argc, char** argv);

    void parseRandomCount(int& i, int argc, char** argv);
    void parseFileName(int& i, int argc, char** argv);

    void checkConstraints() const;
    void printHelpAndExit(int exitCode = 1) const;

    bool randomMode = false;
    size_t randomCount = 0;
    bool numbersOnly = false;
    bool interactiveMode = false;
    std::string fileName = "data/data.bin";
    std::string scriptName;
};

#endif  // !CREATE_OPTIONS_HPP
