#include <cstddef>
#include <file_buffering.hpp>
#include <iostream>
#include <string>
#include <vector>

// TODO: make this into options later
constexpr unsigned int bufferCount = 5;

int main() {
    BufferedFile f("temp/testFile.txt");

    for (int i = 0; i < 20; i++) {
        f.write(i, std::to_string(i));
    }

    f.write(13, "123456789012345678901234567890");
    f.write(33, "123456789012345678901234567890");

    size_t pageCount = 0;
    f.resetPageIndex();
    auto page = f.getNextPage();
    while (page.has_value()) {
        std::cout << "\nPage: " << pageCount << '[';
        for (auto& s : page.value()) {
            std::cout << s << std::endl;
        }
        std::cout << "]\n";
        page = f.getNextPage();
    }

    std::vector<std::vector<Record>> buffers(bufferCount);
    // TODO: Fill each buffer with a single page from the BufferedFile
    // TODO: Figure out how to sort buffers
    // etiher a random access iterator and then std::sort
    // or std::sort(buffers | std::views::join) in c++20
    // TODO: Write the buffers back to the file
    // NOTE: BufferedFile.writePage and BufferedFile.readPage would probabbly be
    // usefull
    // NOTE: Change BufferedFile so that reads returns a std::optional
    // so it can fail if you leave the file and write will write anywhere in
    // the already written or directly after it
    return 0;
}
