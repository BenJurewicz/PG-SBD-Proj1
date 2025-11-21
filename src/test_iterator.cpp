#include <iostream>
#include <vector>
#include <string_view>
#include <stdexcept>
#include "util/file_buffering.hpp"
#include "util/record.hpp"

void print_page(const std::vector<Record>& page) {
    for (const auto& record : page) {
        // Only print the actual content, not the padding
        std::cout << std::string_view(record.data()).substr(0, record.data().find('\0')) << std::endl;
    }
}

int main() {
    const char* filename = "test_iterator.bin";
    
    // 1. Create a BufferedFile
    BufferedFile bf(filename);

    // 2. Create some data
    std::vector<Record> page1_data;
    for (int i = 0; i < 5; ++i) {
        page1_data.push_back(Record("Page 1, Record " + std::to_string(i)));
    }

    // 3. Write data using the iterator
    auto it = bf.begin();
    *it = page1_data;
    
    // Flush the data to make getPageCount() accurate
    bf.flush();

    // 4. Read data back using the iterator and verify
    std::cout << "--- Reading Page 1 ---" << std::endl;
    it = bf.begin();
    std::vector<Record> read_page1 = *it;
    print_page(read_page1);

    if (read_page1.size() != 10) {
         std::cerr << "Test failed: Page 1 size is not 10" << std::endl;
         return 1;
    }
    for(size_t i = 0; i < page1_data.size(); ++i) {
        if (read_page1[i].data().rfind(page1_data[i].data(), 0) != 0) {
            std::cerr << "Test failed: Page 1 content mismatch" << std::endl;
            return 1;
        }
    }
    std::cout << "Page 1 content OK" << std::endl;

    // 5. Test out of bounds read
    std::cout << "--- Testing out of bounds read ---" << std::endl;
    bool exception_thrown = false;
    try {
        // bf.getPageCount() should be 1. We try to read page index 1.
        auto page_proxy = bf.begin()[bf.getPageCount()];
        page_proxy.records(); // This should trigger the exception
    } catch (const std::out_of_range& e) {
        exception_thrown = true;
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }

    if (!exception_thrown) {
        std::cerr << "Test failed: Did not throw for out of bounds read" << std::endl;
        // Clean up before failing
        bf.flush();
        remove(filename);
        return 1;
    }
    std::cout << "Out of bounds read test OK" << std::endl;

    // 6. Test writing to a new page (append)
    std::cout << "--- Testing append write ---" << std::endl;
    std::vector<Record> page2_data;
    page2_data.push_back(Record("Appended Page Record 0"));
    size_t initial_page_count = bf.getPageCount();
    try {
        bf.begin()[initial_page_count] = page2_data; // Write to the page immediately after the last existing page
    } catch (const std::out_of_range& e) {
        std::cerr << "Test failed: Threw exception on append write: " << e.what() << std::endl;
        // Clean up before failing
        bf.flush();
        remove(filename);
        return 1;
    }
    bf.flush(); // Make sure the write is committed to disk
    
    if (bf.getPageCount() != initial_page_count + 1) {
        std::cerr << "Test failed: Page count did not increase after append." << std::endl;
        remove(filename);
        return 1;
    }
    std::cout << "Append write test OK" << std::endl;

    // 7. Test out of bounds write (not an append)
    std::cout << "--- Testing out of bounds write (not append) ---" << std::endl;
    exception_thrown = false;
    try {
        // Try to write to a page far beyond the current end
        bf.begin()[bf.getPageCount() + 5] = page2_data; 
    } catch (const std::out_of_range& e) {
        exception_thrown = true;
        std::cout << "Caught expected exception for OOB write: " << e.what() << std::endl;
    }

    if (!exception_thrown) {
        std::cerr << "Test failed: Did not throw for out of bounds write" << std::endl;
        bf.flush();
        remove(filename);
        return 1;
    }
    std::cout << "Out of bounds write test OK" << std::endl;


    std::cout << "All tests passed!" << std::endl;
    
    // Clean up the test file
    remove(filename);

    return 0;
}
