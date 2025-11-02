#include <file_buffering.hpp>
#include <iostream>

int main() {
    BufferedFile f("temp/testFile.txt");
    f.write(0, "123456789012345678901234567890");

    std::cout << "Hello from sort_files!" << std::endl;
    return 0;
}
