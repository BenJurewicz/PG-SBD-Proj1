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

    // size_t maxi = 100;
    // for (size_t i = 0; i < maxi; i++) {
    //     f.write(i, std::to_string(maxi - 1 - i));
    // }
    // f.flush();

    createRunsInFile(f);
    return 0;

    std::cout << "Stage 2: Merging runs" << std::endl;

    std::vector<Buffer> buffers(bufferCount);

    BufferedFile t("temp/tempFile");
    BufferedFile* source = &f;
    BufferedFile* dest = &t;

    size_t totalPageCount = f.getPageCount();
    // size_t totalRecordCount = f.getRecordCount();
    size_t runLenInPages = bufferCount;
    // size_t readPages = 0;

    auto cmp = [&](auto& a, auto& b) {
        return buffers[a.first][a.second] > buffers[b.first][b.second];
    };
    std::priority_queue<
        std::pair<size_t, size_t>,
        std::vector<std::pair<size_t, size_t>>,
        decltype(cmp)>
        pq(cmp);
    size_t phaseCount = 0;

    // NOTE: Do until one run remains
    while (runLenInPages < totalPageCount) {
        auto srcPages = source->pages();
        auto dstPages = dest->pages();
        size_t readPages = 0;

        std::cout << "One Phase Iteration" << std::endl;
        std::cout << "phaseCount = " << phaseCount << std::endl;
        std::cout << "dest = "
                  << (dest == &f ? "temp/testFile.txt" : "temp/tempFile")
                  << std::endl;
        std::cout << "source = "
                  << (source == &f ? "temp/testFile.txt" : "temp/tempFile")
                  << std::endl;

        // NOTE: Do one merge pass
        while (readPages < totalPageCount) {
            std::cout << "One Merge" << std::endl;
            std::cout << "readPages = " << readPages << std::endl;
            std::cout << "readPages = " << readPages << std::endl;
            buffers.clear();
            size_t input_buffers_used = 0;

            // NOTE: Fill all input buffers
            for (size_t i = 0; i < bufferCount - 1; i++) {
                if (srcPages.begin() == srcPages.end()) {
                    break;
                }

                auto run_begin = srcPages.begin();
                auto run_end = srcPages.begin();

                size_t pages_in_this_run = std::min(
                    (size_t)std::ranges::distance(srcPages),
                    runLenInPages
                );
                std::ranges::advance(run_end, pages_in_this_run);

                buffers.emplace_back(run_begin, run_end);
                input_buffers_used++;

                srcPages = std::ranges::subrange(run_end, srcPages.end());
            }

            readPages = totalPageCount - std::ranges::distance(srcPages);

            if (input_buffers_used == 0) {
                break;
            }

            // Setup output buffer
            buffers.emplace_back();
            buffers.back() = dstPages;

            // NOTE: Initialize pq with first element from each non-empty buffer
            for (size_t i = 0; i < input_buffers_used; ++i) {
                if (!buffers[i].empty()) {
                    pq.push({i, 0});
                }
            }

            // NOTE: K-way merge
            while (!pq.empty()) {
                auto [bufIdx, elemIdx] = pq.top();
                pq.pop();

                buffers.back().push_back(buffers[bufIdx][elemIdx]);

                if (elemIdx + 1 < buffers[bufIdx].size()) {
                    pq.push({bufIdx, elemIdx + 1});
                }
            }
        }

        runLenInPages *= (bufferCount - 1);

        BufferedFile* temp = dest;
        dest = source;
        source = temp;
        phaseCount++;
    }

    std::cout << "Finished" << std::endl;
    std::cout << "Write Count: " << BufferedFile::writeCount << std::endl;
    std::cout << "Read Count: " << BufferedFile::readCout << std::endl;
    return 0;
}

void createRunsInFile(BufferedFile& f) {
    std::vector<std::vector<Record>> buffers(bufferCount);

    std::cout << "Stage 1: Divide into runs" << std::endl;

    auto [fBegin, fEnd] = f.pages();
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
