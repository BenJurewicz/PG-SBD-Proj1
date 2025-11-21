#include <iostream>
#include <vector>
#include <string_view>
#include <stdexcept>
#include <numeric>
#include "util/file_buffering.hpp"
#include "util/record.hpp"

// Helper to print a page of records
void print_page(const std::vector<Record>& page) {
    for (const auto& record : page) {
        // Only print the actual content, not the padding
        std::cout << std::string_view(record.data()).substr(0, record.data().find('\0')) << std::endl;
    }
}

// Helper to create dummy data for a page
std::vector<Record> create_page_data(int page_num, int num_records) {
    std::vector<Record> page_data;
    for (int i = 0; i < num_records; ++i) {
        page_data.push_back(Record("Page " + std::to_string(page_num) + ", Record " + std::to_string(i)));
    }
    return page_data;
}

// Helper to compare two pages of records
bool compare_pages(const std::vector<Record>& page1, const std::vector<Record>& page2) {
    if (page1.size() != page2.size()) {
        std::cerr << "Page size mismatch: " << page1.size() << " vs " << page2.size() << std::endl;
        return false;
    }
    for (size_t i = 0; i < page1.size(); ++i) {
        if (page1[i].data().rfind(page2[i].data(), 0) != 0) {
            std::cerr << "Record content mismatch at index " << i << std::endl;
            return false;
        }
    }
    return true;
}


int main() {
    const char* filename = "test_iterator.bin";
    
    // Clean up from previous runs
    remove(filename);

    // --- Test 1: Empty File ---
    std::cout << "--- Test 1: Empty File ---" << std::endl;
    std::cout << "This test checks that for a newly created file, the begin() and end() iterators are the same, and the page count is 0." << std::endl;
    {
        BufferedFile bf(filename);
        if (bf.begin() != bf.end()) {
            std::cerr << "Test 1 Failed: For empty file, begin() should be equal to end()" << std::endl;
            return 1;
        }
        if (bf.getPageCount() != 0) {
            std::cerr << "Test 1 Failed: Empty file should have 0 pages" << std::endl;
            return 1;
        }
    }
    std::cout << "Test 1 Passed!" << std::endl;


    // --- Test 2: Write and Read Multiple Pages ---
    std::cout << "\n--- Test 2: Write and Read Multiple Pages ---" << std::endl;
    const int num_pages_to_test = 3;
    {
        BufferedFile bf(filename);
        // Write
        std::cout << "Writing " << num_pages_to_test << " pages of data..." << std::endl;
        for (int i = 0; i < num_pages_to_test; ++i) {
            auto data = create_page_data(i, 5);
            bf.begin()[i] = data;
        }
        bf.flush();

        // Verify page count
        if (bf.getPageCount() != num_pages_to_test) {
            std::cerr << "Test 2 Failed: Page count should be " << num_pages_to_test << " but is " << bf.getPageCount() << std::endl;
            return 1;
        }

        // Verify content with forward iteration
        std::cout << "Iterating forwards (begin() to end()) to read and verify all pages..." << std::endl;
        int page_idx = 0;
        for (auto it = bf.begin(); it != bf.end(); ++it, ++page_idx) {
            std::vector<Record> read_data = *it;
            auto expected_data = create_page_data(page_idx, 5);
            // The buffer pads with empty records up to recordsPerPage
            expected_data.resize(BufferedFile::recordsPerPage, Record::empty);

            if (!compare_pages(read_data, expected_data)) {
                 std::cerr << "Test 2 Failed: Content mismatch on page " << page_idx << std::endl;
                 return 1;
            }
        }
    }
    std::cout << "Test 2 Passed!" << std::endl;

    // --- Test 3: Reverse Iteration ---
    std::cout << "\n--- Test 3: Reverse Iteration ---" << std::endl;
    std::cout << "Iterating backwards (from end() - 1) to read and verify all pages..." << std::endl;
    {
        BufferedFile bf(filename);
        int page_idx = num_pages_to_test - 1;
        for (auto it = bf.end() - 1; it != bf.begin() -1; --it, --page_idx) {
            std::vector<Record> read_data = *it;
            auto expected_data = create_page_data(page_idx, 5);
            expected_data.resize(BufferedFile::recordsPerPage, Record::empty);

            if (!compare_pages(read_data, expected_data)) {
                 std::cerr << "Test 3 Failed: Content mismatch on page " << page_idx << std::endl;
                 return 1;
            }
        }
    }
    std::cout << "Test 3 Passed!" << std::endl;


    // --- Test 4: Random Access and Overwrite ---
    std::cout << "\n--- Test 4: Random Access and Overwrite ---" << std::endl;
    {
        BufferedFile bf(filename);
        // Overwrite page 1
        std::cout << "Overwriting page 1 with new data..." << std::endl;
        auto new_page1_data = create_page_data(99, 7); // new data
        bf.begin()[1] = new_page1_data;
        bf.flush();

        std::cout << "Verifying overwritten page and checking that other pages are unchanged..." << std::endl;
        // Check page 0 (should be unchanged)
        auto page0_data = create_page_data(0, 5);
        page0_data.resize(BufferedFile::recordsPerPage, Record::empty);
        if (!compare_pages(*(bf.begin()), page0_data)) {
            std::cerr << "Test 4 Failed: Page 0 was modified unexpectedly" << std::endl;
            return 1;
        }
        
        // Check page 1 (should be new data)
        new_page1_data.resize(BufferedFile::recordsPerPage, Record::empty);
        if (!compare_pages(bf.begin()[1], new_page1_data)) {
            std::cerr << "Test 4 Failed: Page 1 was not overwritten correctly" << std::endl;
            return 1;
        }

        // Check page 2 (should be unchanged) with iterator arithmetic
        auto page2_data = create_page_data(2, 5);
        page2_data.resize(BufferedFile::recordsPerPage, Record::empty);
        if (!compare_pages(*(bf.begin() + 2), page2_data)) {
            std::cerr << "Test 4 Failed: Page 2 was modified unexpectedly" << std::endl;
            return 1;
        }

        std::cout << "Verifying iterator subtraction (end() - begin())..." << std::endl;
        if ((bf.end() - bf.begin()) != num_pages_to_test) {
             std::cerr << "Test 4 Failed: Iterator difference is incorrect." << std::endl;
             return 1;
        }
    }
    std::cout << "Test 4 Passed!" << std::endl;

    // --- Test 5: Out of Bounds Access ---
    std::cout << "\n--- Test 5: Out of Bounds Access ---" << std::endl;
    {
        BufferedFile bf(filename);
        bool exception_thrown = false;
        
        std::cout << "Trying to read past the end of the file, expecting an exception..." << std::endl;
        try {
            auto page_proxy = bf.begin()[bf.getPageCount()];
            page_proxy.records(); 
        } catch (const std::out_of_range& e) {
            exception_thrown = true;
        }
        if (!exception_thrown) {
            std::cerr << "Test 5 Failed: Did not throw for out of bounds read" << std::endl;
            return 1;
        }

        std::cout << "Trying to write far past the end of the file, expecting an exception..." << std::endl;
        exception_thrown = false;
        try {
            bf.begin()[bf.getPageCount() + 5] = create_page_data(0, 1);
        } catch (const std::out_of_range& e) {
            exception_thrown = true;
        }
        if (!exception_thrown) {
            std::cerr << "Test 5 Failed: Did not throw for out of bounds write" << std::endl;
            return 1;
        }
    }
    std::cout << "Test 5 Passed!" << std::endl;

    // --- Test 6: Append ---
    std::cout << "\n--- Test 6: Append ---" << std::endl;
    std::cout << "Writing to a new page at the end of the file to test append..." << std::endl;
    {
        BufferedFile bf(filename);
        size_t initial_page_count = bf.getPageCount();
        
        auto append_data = create_page_data(initial_page_count, 3);
        bf.begin()[initial_page_count] = append_data;
        bf.flush();

        if (bf.getPageCount() != initial_page_count + 1) {
            std::cerr << "Test 6 Failed: Page count did not increase after append." << std::endl;
            return 1;
        }
        
        append_data.resize(BufferedFile::recordsPerPage, Record::empty);
        if(!compare_pages(bf.begin()[initial_page_count], append_data)){
            std::cerr << "Test 6 Failed: Appended page content is incorrect." << std::endl;
            return 1;
        }
    }
    std::cout << "Test 6 Passed!" << std::endl;


    std::cout << "\nAll iterator tests passed!" << std::endl;
    
    // Clean up the test file
    remove(filename);

    return 0;
}