#include "students.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <memory>
#include <iomanip>
#include <sstream> 
#include <set>     
#include <algorithm>
#ifdef _WIN32
    #include <windows.h>
#endif

bool generate_test_file(const std::string& full_file, const std::string& output_file, size_t num_rows) {
    std::ifstream in(full_file);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open full dataset file: " << full_file << std::endl;
        return false;
    }
    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Error: Could not create test file: " << output_file << std::endl;
        return false;
    }

    std::string line;
    if (std::getline(in, line)) {
        out << line << "\n";
    }

    for (size_t i = 0; i < num_rows && std::getline(in, line); ++i) {
        out << line << "\n";
    }
    
    std::cout << "Generated test file: " << output_file << " (" << num_rows << " rows)" << std::endl;
    return true;
}

struct BenchmarkResult {
    long long operation_count;
};

BenchmarkResult run_benchmark(IStudentDB& db, const std::string& csv_file) {
    auto load_start = std::chrono::high_resolution_clock::now();
    if (!db.initialize_from_csv(csv_file)) {
        std::cerr << "Failed to initialize DB from " << csv_file << std::endl;
        return {0};
    }
    auto load_end = std::chrono::high_resolution_clock::now();
    auto load_duration = std::chrono::duration_cast<std::chrono::milliseconds>(load_end - load_start).count();
    std::cout << "  DB initialized in " << load_duration << " ms. Starting 10-sec test..." << std::endl;

    const auto& students = db.get_all_students();
    if (students.empty()) {
        std::cerr << "  No students loaded, skipping benchmark." << std::endl;
        return {0};
    }

    std::vector<std::string> phones;
    std::vector<std::string> groups;
    std::vector<std::string> surnames;
    
    std::set<std::string> unique_groups;
    std::set<std::string> unique_surnames;

    for (const auto& s : students) {
        phones.push_back(s.m_phone_number);
        unique_groups.insert(s.m_group);
        unique_surnames.insert(s.m_surname);
    }
    groups.assign(unique_groups.begin(), unique_groups.end());
    surnames.assign(unique_surnames.begin(), unique_surnames.end());

    std::vector<std::string> new_groups = {"GR-01", "GR-02", "GR-03", "GR-04", "GR-05"};

    std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
    
    if (phones.empty() || groups.empty() || surnames.empty()) {
        std::cerr << "  Test data (phones, groups, or surnames) is empty. Skipping test." << std::endl;
        return {0};
    }
    
    // 50 + 10 + 5 = 65. Розподіл для вибору операції:
    // 0-49: Op1 (changeGroupByPhone) - 50
    // 50-59: Op2 (getSortedStudentsByGroup) - 10
    // 60-64: Op3 (getGroupsBySurname) - 5
    std::uniform_int_distribution<int> op_dist(0, 64); 
    
    std::uniform_int_distribution<size_t> phone_dist(0, phones.size() - 1);
    std::uniform_int_distribution<size_t> group_dist(0, groups.size() - 1);
    std::uniform_int_distribution<size_t> surname_dist(0, surnames.size() - 1);
    std::uniform_int_distribution<size_t> new_group_dist(0, new_groups.size() - 1);

    long long operation_count = 0;
    const auto benchmark_duration = std::chrono::seconds(10);
    auto benchmark_start = std::chrono::high_resolution_clock::now();
    
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        if (now - benchmark_start >= benchmark_duration) {
            break; 
        }

        int op_type = op_dist(rng);

        if (op_type < 50) { 
            // Операція 1 (50/65): changeGroupByPhone
            const std::string& phone = phones[phone_dist(rng)];
            const std::string& new_group = new_groups[new_group_dist(rng)];
            db.changeGroupByPhone(phone, new_group);
        } 
        else if (op_type < 60) { 
            // Операція 2 (10/65): getSortedStudentsByGroup
            const std::string& group = groups[group_dist(rng)];
            (void)db.getSortedStudentsByGroup(group); 
        } 
        else { 
            // Операція 3 (5/65): getGroupsBySurname
            const std::string& surname = surnames[surname_dist(rng)];
            (void)db.getGroupsBySurname(surname);
        }
        operation_count++;
    }

    std::cout << "  Test finished. Operations: " << operation_count << "." << std::endl;
    return {operation_count};
}


// =================================================================
// ===== ГОЛОВНА ФУНКЦІЯ =====
// =================================================================

int main() {
    std::cout << "--- Student DB Benchmark (50:10:5 Op1:Op2:Op3) ---" << std::endl;

    #ifdef _WIN32
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    #endif

    const std::string full_dataset_file = "students.csv"; 
    const std::vector<size_t> db_sizes = {100, 1000, 10000, 100000};
    const std::vector<std::string> db_types = {"V1_Hash", "V2_BST", "V3_Vector"};

    std::cout << "\n--- Generating Test Files ---" << std::endl;
    for (size_t size : db_sizes) {
        std::string test_file = "students_" + std::to_string(size) + ".csv";
        if (!generate_test_file(full_dataset_file, test_file, size)) {
            if (size > 100) {
                 std::cerr << "Warning: Failed to generate " << test_file << ". Continuing with smaller files." << std::endl;
                 continue;
            } else {
                 std::cerr << "Failed to generate test files. Aborting." << std::endl;
                 return 1;
            }
        }
    }

    std::cout << "\n--- Running Benchmarks (10 sec per test) ---" << std::endl;
    std::ofstream results_file("results.csv");
    if (!results_file.is_open()) {
        std::cerr << "Failed to create results.csv. Aborting." << std::endl;
        return 1;
    }
    
    results_file << "DB_Type,DB_Size,Operations_Per_10s\n";

    for (const std::string& type : db_types) {
        for (size_t size : db_sizes) {
            std::string test_file = "students_" + std::to_string(size) + ".csv";

            std::ifstream test_in(test_file);
            if (!test_in.is_open()) continue;
            test_in.close();

            std::cout << "\nTesting " << type << " with " << size << " rows..." << std::endl;
            
            std::unique_ptr<IStudentDB> db;
            if (type == "V1_Hash") {
                db = std::make_unique<StudentDB_V1_Hash>();
            } else if (type == "V2_BST") {
                db = std::make_unique<StudentDB_V2_BST>();
            } else if (type == "V3_Vector") {
                db = std::make_unique<StudentDB_V3_Vector>();
            }

            BenchmarkResult result = run_benchmark(*db, test_file);

            results_file << type << "," << size << "," << result.operation_count << "\n";
            results_file.flush(); 
        }
    }

    std::cout << "\n--- Running Sorting Benchmark (N=100000) ---" << std::endl;
    const std::string sorting_test_file = "students_100000.csv";

    StudentDB_V1_Hash sort_db; 
    
    std::cout << "Loading 100,000 students for sorting test..." << std::endl;
    if (!sort_db.initialize_from_csv(sorting_test_file)) {
        std::cerr << "Failed to load data for sorting benchmark. Aborting." << std::endl;
        return 1;
    }

    std::cout << "\nStarting std::sort benchmark..." << std::endl;
    sort_db.sort_standard("sorted_students_standard.csv");

    std::cout << "\nStarting Radix Sort benchmark..." << std::endl;
    sort_db.sort_custom_radix("sorted_students_radix.csv");

    std::cout << "\n--- Benchmark Complete ---" << std::endl;
    std::cout << "Results saved to results.csv" << std::endl;
    std::cout << "Now run 'python plot_results.py' to generate graphs." << std::endl;

    return 0;
}