#include <algorithm>
#include <cstddef>
#include <file_buffering.hpp>
#include <iostream>
#include <ostream>
#include <queue>
#include <string>
#include <vector>

// TODO: make this into options later
constexpr size_t bufferCount = 5;
constexpr size_t recordsPerPage = 20;  // Blocking factor

int main() {
    BufferedFile f("temp/testFile.txt");

    for (int i = 0; i < 100; i++) {
        f.write(99 - i, std::to_string(i));
    }

    std::vector<std::vector<Record>> buffers(bufferCount);

    f.resetPageIndex();
    std::optional<BufferedFile::BufferType> page = f.readPage();

    for (auto& b : buffers) {
        if (!page.has_value()) {
            break;
        }
        b = page.value();
        page = f.readPage();
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

    std::cout << "Finished" << std::endl;
    return 0;
}
