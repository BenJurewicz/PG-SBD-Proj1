#include <algorithm>
#include <buffer.hpp>
#include <cstddef>
#include <file_buffering.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <queue>
#include <string>
#include <vector>

// TODO: make this into options later
constexpr size_t bufferCount = 5;
// constexpr size_t recordsPerPage = 20;  // Blocking factor

void createRunsInFile(
    BufferedFile& f, std::vector<std::vector<Record>>& buffers
);

int main() {
    // BufferedFile f("temp/testFile.txt");
    auto f = std::make_shared<BufferedFile>("temp/testFile.txt");

    // Buffer bf(f, 1, 1);

    size_t maxi = 100;
    for (size_t i = 0; i < maxi; i++) {
        f->write(maxi - 1 - i, std::to_string(i));
    }

    std::vector<std::vector<Record>> buffers(bufferCount);

    createRunsInFile(*f, buffers);

    std::cout << "Stage 2: Merging runs" << std::endl;

    // BufferedFile t("temp/tempFile");
    // BufferedFile& source = *f;
    // BufferedFile& dest = t;
    //
    // size_t pageCount = f->getPageCount();
    // size_t totalRecordCount = f->getRecordCount();
    // size_t runLenInPages = buffers.size();
    // size_t readPages = 0;
    //
    // auto cmp = [&](auto& a, auto& b) {
    //     return buffers[a.first][a.second] > buffers[b.first][b.second];
    // };
    // std::priority_queue<
    //     std::pair<size_t, size_t>,
    //     std::vector<std::pair<size_t, size_t>>,
    //     decltype(cmp)>
    //     pq(cmp);
    // size_t phaseCount = 0;
    //
    // while (runLenInPages * BufferedFile::recordsPerPage < totalRecordCount) {
    //     phaseCount++;
    // }

    // NOTE: Change BufferedFile so that reads returns a std::optional
    // so it can fail if you leave the file and write will write anywhere in
    // the already written section or directly after it

    std::cout << "Finished" << std::endl;
    return 0;
}

void createRunsInFile(
    BufferedFile& f, std::vector<std::vector<Record>>& buffers
) {
    std::cout << "Stage 1: Divide into runs" << std::endl;

    f.resetPageIndex();
    size_t pageIndex = f.getPageIndex();
    bool isFileEmpty = false;
    size_t recordIndex = 0;

    while (!isFileEmpty) {
        // TODO: This would be better as a iterator on pages
        // This way there would be no need for f.getPageIndex()
        // and f.resetPageIndex()
        f.setPageIndex(pageIndex);
        std::optional<BufferedFile::BufferType> page = f.readPage();

        // Fill bufffers
        for (auto& b : buffers) {
            if (!page.has_value()) {
                isFileEmpty = true;
                break;
            }
            b = page.value();
            page = f.readPage();
        }
        pageIndex = f.getPageIndex();

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
    }
}
