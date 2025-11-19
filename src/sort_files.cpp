#include <algorithm>
#include <cstddef>
#include <file_buffering.hpp>
#include <iostream>
#include <ostream>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

// TODO: make this into options later
constexpr unsigned int bufferCount = 5;
// TODO add recordsPerPage here

int main() {
    BufferedFile f("temp/testFile.txt");

    for (int i = 0; i < 100; i++) {
        f.write(99 - i, std::to_string(i));
    }
    //
    // f.write(13, "123456789012345678901234567890");
    // f.write(33, "123456789012345678901234567890");
    //
    // size_t pageCount = 0;
    // f.resetPageIndex();
    // auto page = f.readPage();
    // for (int i = 0; i < 5; i++) {
    //     f.advancePageIndex();
    //     f.writePage(page.value());
    // }
    // f.resetPageIndex();
    // page = f.readPage();
    // f.advancePageIndex();
    //
    // while (page.has_value()) {
    //     std::cout << "\nPage: " << pageCount++ << '\n';
    //     for (auto& s : page.value()) {
    //         std::cout << s << '\n';
    //     }
    //     std::cout << std::endl;
    //     page = f.readPage();
    //     f.advancePageIndex();
    // }

    std::vector<std::vector<Record>> buffers(bufferCount);

    f.resetPageIndex();
    std::optional<BufferedFile::BufferType> page;

    for (auto& b : buffers) {
        page = f.readPage();
        if (!page.has_value()) {
            break;
        }
        f.advancePageIndex();
        b = page.value();
    }

    // Sort:

    for (auto& b : buffers) {
        std::sort(b.begin(), b.end());
    }

    // K-way merge
    auto cmp = [&](auto& a, auto& b) {
        return buffers[a.first][a.second] > buffers[b.first][b.second];
    };
    std::priority_queue<
        std::pair<size_t, size_t>,
        std::vector<std::pair<size_t, size_t>>,
        decltype(cmp)>
        pq(cmp);

    // Initialize with first element from each non-empty buffer
    for (size_t i = 0; i < buffers.size(); ++i) {
        if (!buffers[i].empty()) {
            pq.push({i, 0});
        }
    }

    size_t recordIndex = 0;
    while (!pq.empty()) {
        auto [bufIdx, elemIdx] = pq.top();
        pq.pop();

        f.write(recordIndex++, buffers[bufIdx][elemIdx]);

        // Add next element from same buffer
        if (elemIdx + 1 < buffers[bufIdx].size()) {
            pq.push({bufIdx, elemIdx + 1});
        }
    }

    // NOTE: Change BufferedFile so that reads returns a std::optional
    // so it can fail if you leave the file and write will write anywhere in
    // the already written or directly after it
    return 0;
}
