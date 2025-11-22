#include <algorithm>
#include <buffer.hpp>
#include <cstddef>
#include <file_buffering.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

// TODO: make this into options later
constexpr size_t bufferCount = 5;
// constexpr size_t recordsPerPage = 20;  // Blocking factor

void createRunsInFile(BufferedFile& f);

int main() {
    BufferedFile f("temp/testFile.txt");

    // Buffer bf(f, 1, 1);

    size_t maxi = 100;
    for (size_t i = 0; i < maxi; i++) {
        f.write(i, std::to_string(maxi - 1 - i));
    }

    createRunsInFile(f);

    std::cout << "Stage 2: Merging runs" << std::endl;

    std::vector<Buffer> buffers;
    buffers.reserve(bufferCount);

    BufferedFile t("temp/tempFile");
    BufferedFile* source = &f;
    BufferedFile* dest = &t;

    size_t totalPageCount = f.getPageCount();
    size_t totalRecordCount = f.getRecordCount();
    size_t runLenInPages = buffers.size();
    size_t readPages = 0;

    auto cmp = [&](auto& a, auto& b) {
        return buffers[a.first][a.second] > buffers[b.first][b.second];
    };
    std::priority_queue<
        std::pair<size_t, size_t>,
        std::vector<std::pair<size_t, size_t>>,
        decltype(cmp)>
        pq(cmp);
    // size_t phaseCount = 0;

    auto srcPages = source->pages();
    auto dstPages = dest->pages();

    // NOTE: Do until one run remains
    while (runLenInPages * BufferedFile::recordsPerPage < totalRecordCount) {
        srcPages = source->pages();
        dstPages = dest->pages();
        buffers.back() = dstPages;
        // NOTE: Do one merge
        while (readPages < totalPageCount) {
            // NOTE: Fill all buffers except the last one
            for (size_t i = 0; i < buffers.size() - 1 || !srcPages.empty();
                 i++) {
                // NOTE: Initialize buffer with it's start and end page
                buffers.emplace_back(
                    srcPages.begin(),
                    srcPages.advance(runLenInPages).begin()
                );
                // NOTE: Initialize pq with first element from each non-empty
                // buffer
                if (!buffers[i].empty()) {
                    pq.push({i, 0});
                }
            }

            // NOTE: K-way merge
            while (!pq.empty()) {
                auto [bufIdx, elemIdx] = pq.top();
                pq.pop();

                // NOTE: This should do buffered writes to the file
                buffers.back().push_back(buffers[bufIdx][elemIdx]);

                // NOTE: Add next element from same buffer
                if (elemIdx + 1 < buffers[bufIdx].size()) {
                    pq.push({bufIdx, elemIdx + 1});
                }
            }

            for (auto& b : buffers) {
                b.clear();
            }
        }

        runLenInPages *= buffers.size() - 1;
        readPages = 0;

        BufferedFile* temp = dest;
        dest = source;
        source = temp;

        // phaseCount++;
    }

    // NOTE: Change BufferedFile so that reads returns a std::optional
    // so it can fail if you leave the file and write will write anywhere in
    // the already written section or directly after it

    std::cout << "Finished" << std::endl;
    std::cout << "Write Count: " << BufferedFile::writeCount << std::endl;
    std::cout << "Read Count: " << BufferedFile::readCout << std::endl;
    return 0;
}

void createRunsInFile(BufferedFile& f) {
    std::vector<std::vector<Record>> buffers(bufferCount);

    std::cout << "Stage 1: Divide into runs" << std::endl;

    // auto [fBegin, fEnd] = f.pages();
    auto p = f.pages() | std::views::drop(0);
    auto fBegin = p.begin();
    auto fEnd = p.end();
    bool isFileEmpty = false;
    size_t recordIndex = 0;

    while (!isFileEmpty) {
        for (auto& b : buffers) {
            if (fBegin == fEnd) {
                isFileEmpty = true;
                break;
            }
            b = *fBegin++;
        }
        if (std::ranges::all_of(buffers, [](auto& b) { return b.empty(); })) {
            break;
        }

        // Sort:
        for (auto& b : buffers) {
            std::ranges::sort(b);
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
        for (size_t i = 0; i < buffers.size(); i++) {
            if (!buffers[i].empty()) {
                pq.push({i, 0});
            }
        }

        while (!pq.empty()) {
            auto [bufIdx, elemIdx] = pq.top();
            pq.pop();

            f.write(recordIndex++, buffers[bufIdx][elemIdx]);

            // Add next element from same buffer
            if (elemIdx + 1 < buffers[bufIdx].size()) {
                pq.push({bufIdx, elemIdx + 1});
            }
        }

        for (auto& b : buffers) {
            b.clear();
        }
    }
}
