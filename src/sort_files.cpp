#include <file_buffering.hpp>
#include <iostream>
#include <string>

int main() {
    BufferedFile f("temp/testFile.txt");

    for (int i = 0; i < 20; i++) {
        f.write(i, std::to_string(i));
    }

    f.write(13, "123456789012345678901234567890");
    f.write(33, "123456789012345678901234567890");

    std::cout << "Hello from sort_files!" << std::endl;
    return 0;
}
