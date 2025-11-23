#include <algorithm>
#include <buffer.hpp>
#include <cstddef>
#include <file_buffering.hpp>
#include <iostream>
#include <ostream>
#include <queue>
#include <ranges>
#include <vector>

#include "temp_file.hpp"
#include "util/sort_options.hpp"

void createRunsInFile(BufferedFile& f, const SortOptions& options);

int main(int argc, char** argv) {
    SortOptions options(argc, argv);
    BufferedFile::setRecordsPerPage(options.getBlockingFactor());

    BufferedFile f(options.getFileName());
    std::cout << "Loaded file: " << options.getFileName() << std::endl;
    f.printFileContent();
    std::cout << std::endl;

    // size_t maxi = 100;
    // for (size_t i = 0; i < maxi; i++) {
    //     f.write(i, std::to_string(maxi - 1 - i));
    // }
    // f.flush();

    createRunsInFile(f, options);

    if (options.isLogging()) {
        std::cout << "Stage 2: Merging runs\n" << std::endl;
    }
    std::vector<Buffer> buffers(options.getBufferCount());

    TempFile t;
    BufferedFile* src = &f;
    BufferedFile* dest = &static_cast<BufferedFile&>(t);

    size_t totalPageCount = f.getPageCount();
    size_t runLenInPages = options.getBufferCount();

    auto cmp = [&](auto& a, auto& b) {
        return buffers[a.first][a.second] > buffers[b.first][b.second];
    };
    std::priority_queue<
        std::pair<size_t, size_t>,
        std::vector<std::pair<size_t, size_t>>,
        decltype(cmp)>
        pq(cmp);

    size_t mergeCount = 0;

    // NOTE: Do until one run remains
    while (runLenInPages < totalPageCount) {
        auto srcPages = src->pages();
        auto dstPages = dest->pages();
        size_t readPages = 0;

        if (options.isLogging()) {
            std::cout << "Merge " << mergeCount++ << std::endl;
            std::cout << "Merging " << options.getBufferCount() - 1
                      << " runs of lenght "
                      << runLenInPages * BufferedFile::recordsPerPage
                      << std::endl;

            std::cout << "dest = "
                      << (dest == &f ? options.getFileName() : t.getFileName())
                      << std::endl;
            std::cout << "src = "
                      << (src == &f ? options.getFileName() : t.getFileName())
                      << std::endl;
        }

        // NOTE: Do one merge pass
        while (readPages < totalPageCount) {
            buffers.clear();
            size_t inputBuffersUsed = 0;

            // NOTE: Fill all input buffers
            for (size_t i = 0; i < options.getBufferCount() - 1; i++) {
                if (srcPages.empty()) {
                    break;
                }

                size_t pagesInThisRun =
                    std::min(srcPages.size(), runLenInPages);

                auto runBegin = srcPages.begin();
                auto runEnd = std::ranges::next(runBegin, pagesInThisRun);

                buffers.emplace_back(runBegin, runEnd);
                inputBuffersUsed++;

                srcPages = {runEnd, srcPages.end()};
            }

            readPages = totalPageCount - std::ranges::distance(srcPages);

            if (inputBuffersUsed == 0) {
                break;
            }

            // Setup output buffer
            buffers.emplace_back(dstPages);

            // NOTE: Initialize pg with first element from each nonempty buffer
            for (size_t i = 0; i < inputBuffersUsed; i++) {
                if (!buffers[i].empty()) {
                    pq.push({i, 0});
                }
            }

            // NOTE: K-way merge
            while (!pq.empty()) {
                auto [bufIdx, elemIdx] = pq.top();
                pq.pop();

                buffers.back().append(buffers[bufIdx][elemIdx]);

                // Add next element from same buffer
                if (elemIdx + 1 < buffers[bufIdx].size()) {
                    pq.push({bufIdx, elemIdx + 1});
                }
            }
        }

        if (options.isLogging()) {
            std::cout << "File contents:" << std::endl;
            dest->printFileContent();
        }

        runLenInPages *= (options.getBufferCount() - 1);

        BufferedFile* temp = dest;
        dest = src;
        src = temp;
    }

    std::cout << "\nFinished" << std::endl;
    std::cout << "Write Count: " << BufferedFile::writeCount << std::endl;
    std::cout << "Read Count: " << BufferedFile::readCout << std::endl;
    std::cout << "Sorted contents: " << std::endl;
    f.printFileContent();
    return 0;
}

void createRunsInFile(BufferedFile& f, const SortOptions& options) {
    if (options.isLogging()) {
        std::cout << "Stage 1: Divide into runs" << std::endl;
    }

    std::vector<std::vector<Record>> buffers(options.getBufferCount());

    auto cmp = [&](auto& a, auto& b) {
        return buffers[a.first][a.second] > buffers[b.first][b.second];
    };
    std::priority_queue<
        std::pair<size_t, size_t>,
        std::vector<std::pair<size_t, size_t>>,
        decltype(cmp)>
        pq(cmp);

    auto [fBegin, fEnd] = f.pages();
    bool isFileEmpty = false;

    Buffer outBuf;
    outBuf = f.pages();

    size_t runCount = 0;

    while (!isFileEmpty) {
        // NOTE: Fill all buffers
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

        // NOTE: Sort:
        for (auto& b : buffers) {
            std::ranges::sort(b);
        }

        // NOTE: Initialize pg with first element from each nonempty buffer
        for (size_t i = 0; i < buffers.size(); i++) {
            if (!buffers[i].empty()) {
                pq.push({i, 0});
            }
        }

        // NOTE: K-way merge
        while (!pq.empty()) {
            auto [bufIdx, elemIdx] = pq.top();
            pq.pop();

            outBuf.append(buffers[bufIdx][elemIdx]);

            // Add next element from same buffer
            if (elemIdx + 1 < buffers[bufIdx].size()) {
                pq.push({bufIdx, elemIdx + 1});
            }
        }

        for (auto& b : buffers) {
            b.clear();
        }

        if (options.isLogging()) {
            std::cout << "Run " << ++runCount << ":" << std::endl;
            std::cout << "File contents:" << std::endl;
            f.printFileContent();
            std::cout << std::endl;
        }
    }
}
