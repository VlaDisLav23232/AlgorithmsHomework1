#include "students.h"

#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>
#include <chrono>


/* ПЕРЕДСЛОВО: 

    асистенти, не бийте мене будь ласка за те що я вирішив написати весь код
    в одному файлі замість розділення на декілька різних файлів. мені було просто лінь, сидіти і
    розділяти все по різних файлах + в мене часу було небагато лишилося, того так. Сподіваюся,
    що коли ви подивитеся на цей шедеврокод, то ви не будете думати про мене чогось поганого.

    дякую за розуміння! :=)
*/

// helpers
namespace {
    struct PairKeyCompare {
        template <typename T>
        bool operator()(const T& pair, const std::string& key) const {
            return pair.first < key;
        }
    };

    struct PairPairCompare {
        template <typename T>
        bool operator()(const T& a, const T& b) const {
            return a.first < b.first;
        }
    };
    
    //for Radix Sort (the next 2 functions)
    const int ALPHABET_SIZE = 256;
    
    inline int msd_get_char(const Student& s, size_t d) {
        if (d >= s.m_email.length()) {
            return -1;
        }
        return static_cast<unsigned char>(s.m_email[d]);
    }
    
    void msd_radix_sort_recursive(std::vector<Student>& students, size_t from, size_t to, size_t d, std::vector<Student>& buffer) {
        if (to <= from + 1) {
            return;
        }

        std::vector<int> count(ALPHABET_SIZE + 2, 0);

        for (size_t i = from; i < to; ++i) {
            count[msd_get_char(students[i], d) + 2]++;
        }
        
        for (size_t r = 0; r < ALPHABET_SIZE + 1; ++r) {
            count[r + 1] += count[r];
        }

        for (size_t i = from; i < to; ++i) {
            buffer[from + count[msd_get_char(students[i], d) + 1]++] = std::move(students[i]);
        }

        for (size_t i = from; i < to; ++i) {
            students[i] = std::move(buffer[i]);
        }

        for (size_t r = 0; r < ALPHABET_SIZE; ++r) {
            size_t bucket_start = from + count[r + 1];
            size_t bucket_end = from + count[r + 2];
            if (bucket_end > bucket_start + 1) {
                msd_radix_sort_recursive(students, bucket_start, bucket_end, d + 1, buffer);
            }
        }
    }
}


// Student methods
void Student::print() const {
    //actually just used for checking if everything is ok
    std::cout << "Name: " << m_name << " " << m_surname
              << ", Group: " << m_group
              << ", Phone: " << m_phone_number
              << ", Email: " << m_email << "\n";
}

std::string Student::to_csv_row() const {
    return m_name + "," + m_surname + "," + m_email + "," +
           std::to_string(m_birth_year) + "," +
           std::to_string(m_birth_month) + "," +
           std::to_string(m_birth_day) + "," +
           m_group + "," +
           std::to_string(m_rating) + "," +
           m_phone_number;
}

void save_to_csv(const std::string& filename, const std::vector<Student>& students, const std::string& sort_name) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not save to " << filename << "\n";
        return;
    }
    file << "m_name,m_surname,m_email,m_birth_year,m_birth_month,m_birth_day,m_group,m_rating,m_phone_number\n";
    for (const auto& s : students) {
        file << s.to_csv_row() << "\n";
    }
    std::cout << "Successfully saved sorted list (" << sort_name << ") to " << filename << "\n";
}




// hash map version
StudentDB_V1_Hash::StudentDB_V1_Hash() {}

bool StudentDB_V1_Hash::initialize_from_csv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    if (!std::getline(file, line)) return false; // Skip header

    size_t line_count = 0;
    while (std::getline(file, line)) {
        line_count++;
    }

    if (line_count == 0) return true;

    all_students_storage.reserve(line_count);

    file.clear();
    file.seekg(0);
    if (!std::getline(file, line)) return false;

    while (std::getline(file, line)) {
        
        std::stringstream ss(line);
        std::string token;
        Student s;
        std::getline(ss, s.m_name, ',');
        std::getline(ss, s.m_surname, ','); 
        std::getline(ss, s.m_email, ','); 
        std::getline(ss, token, ','); s.m_birth_year = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_month = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_day = std::stoi(token);
        std::getline(ss, s.m_group, ',');
        std::getline(ss, token, ','); s.m_rating = std::stof(token);
        std::getline(ss, s.m_phone_number);

        all_students_storage.push_back(std::move(s));

        Student* new_student_ptr = &all_students_storage.back();
        phone_to_student_ptr[new_student_ptr->m_phone_number] = new_student_ptr;
        group_to_students_ptr[new_student_ptr->m_group].push_back(new_student_ptr);
        surname_to_students_ptr[new_student_ptr->m_surname].push_back(new_student_ptr);
    }
    return true;
}

// 1) Змінити групу студенту за його номером телефону (m_phone_number)
bool StudentDB_V1_Hash::changeGroupByPhone(const std::string& phone, const std::string& new_group) {
    auto it = phone_to_student_ptr.find(phone); // O(1)
    if (it == phone_to_student_ptr.end()) return false;

    Student* s = it->second;
    const std::string old_group = s->m_group;
    if (old_group == new_group) return true;

    auto& old_list = group_to_students_ptr[old_group];
    old_list.erase(std::remove(old_list.begin(), old_list.end(), s), old_list.end());
    
    group_to_students_ptr[new_group].push_back(s);
    s->m_group = new_group;
    return true;
}

// 2) Повернути студентів групи m_group в алфавітному порядку за прізвищем і ім’ям (m_surname, m_name)
std::vector<const Student*> StudentDB_V1_Hash::getSortedStudentsByGroup(const std::string& group_name) const {
    auto it = group_to_students_ptr.find(group_name);
    if (it == group_to_students_ptr.end()) return {};

    std::vector<const Student*> result(it->second.begin(), it->second.end()); 
    std::sort(result.begin(), result.end(), [](const Student* a, const Student* b) {
        if (a->m_surname != b->m_surname) return a->m_surname < b->m_surname;
        return a->m_name < b->m_name;
    });
    return result;
}

// 3) Повернути список груп, де навчаються студенти з прізвищем m_surname
std::vector<std::string> StudentDB_V1_Hash::getGroupsBySurname(const std::string& surname) const {
    auto it = surname_to_students_ptr.find(surname);
    if (it == surname_to_students_ptr.end()) return {};
    
    std::set<std::string> unique_groups; 
    for (const Student* s : it->second) {
        unique_groups.insert(s->m_group);
    }
    return std::vector<std::string>(unique_groups.begin(), unique_groups.end());
}


void StudentDB_V1_Hash::sort_standard(const std::string& filename) {
    std::vector<Student> sorted_list = all_students_storage;
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(sorted_list.begin(), sorted_list.end(), [](const Student& a, const Student& b) {
        return a.m_email < b.m_email;
    });
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "std::sort time: " << duration << " ms\n";
    save_to_csv(filename, sorted_list, "std::sort");
}

void StudentDB_V1_Hash::sort_custom_radix(const std::string& filename) {
    if (all_students_storage.empty()) return;

    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Student> sorted_list = all_students_storage;
    std::vector<Student> buffer(sorted_list.size());
    
    msd_radix_sort_recursive(sorted_list, 0, sorted_list.size(), 0, buffer);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Radix Sort time: " << duration << " ms\n";

    save_to_csv(filename, sorted_list, "Radix Sort");
}

size_t StudentDB_V1_Hash::get_student_count() const { return all_students_storage.size(); }

const std::vector<Student>& StudentDB_V1_Hash::get_all_students() const {
    return all_students_storage;
}


// bst map version
StudentDB_V2_BST::StudentDB_V2_BST() {}

bool StudentDB_V2_BST::initialize_from_csv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    if (!std::getline(file, line)) return false; // Skip header

    size_t line_count = 0;
    while (std::getline(file, line)) {
        line_count++;
    }

    if (line_count == 0) return true;

    all_students_storage.reserve(line_count);

    file.clear();
    file.seekg(0);
    if (!std::getline(file, line)) return false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        Student s;
        std::getline(ss, s.m_name, ',');
        std::getline(ss, s.m_surname, ',');
        std::getline(ss, s.m_email, ',');
        std::getline(ss, token, ','); s.m_birth_year = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_month = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_day = std::stoi(token);
        std::getline(ss, s.m_group, ','); 
        std::getline(ss, token, ','); s.m_rating = std::stof(token);
        std::getline(ss, s.m_phone_number); 

        all_students_storage.push_back(std::move(s));

        Student* new_student_ptr = &all_students_storage.back();
        phone_to_student_ptr[new_student_ptr->m_phone_number] = new_student_ptr;
        group_to_students_ptr[new_student_ptr->m_group].push_back(new_student_ptr);
        surname_to_students_ptr[new_student_ptr->m_surname].push_back(new_student_ptr);
    }
    return true;
}

// 1) Змінити групу студенту за його номером телефону (m_phone_number)
bool StudentDB_V2_BST::changeGroupByPhone(const std::string& phone, const std::string& new_group) {
    auto it = phone_to_student_ptr.find(phone);
    if (it == phone_to_student_ptr.end()) return false;

    Student* s = it->second;
    const std::string old_group = s->m_group;
    if (old_group == new_group) return true;

    auto& old_list = group_to_students_ptr[old_group];
    old_list.erase(std::remove(old_list.begin(), old_list.end(), s), old_list.end());

    group_to_students_ptr[new_group].push_back(s);
    s->m_group = new_group;
    return true;
}

// 2) Повернути студентів групи m_group в алфавітному порядку за прізвищем і ім’ям (m_surname, m_name)
std::vector<const Student*> StudentDB_V2_BST::getSortedStudentsByGroup(const std::string& group_name) const {
    auto it = group_to_students_ptr.find(group_name);
    if (it == group_to_students_ptr.end()) return {};

    std::vector<const Student*> result(it->second.begin(), it->second.end()); 
    std::sort(result.begin(), result.end(), [](const Student* a, const Student* b) {
        if (a->m_surname != b->m_surname) return a->m_surname < b->m_surname;
        return a->m_name < b->m_name;
    });
    return result;
}

// 3) Повернути список груп, де навчаються студенти з прізвищем m_surname
std::vector<std::string> StudentDB_V2_BST::getGroupsBySurname(const std::string& surname) const {
    auto it = surname_to_students_ptr.find(surname);
    if (it == surname_to_students_ptr.end()) return {};

    std::set<std::string> unique_groups;
    for (const Student* s : it->second) {
        unique_groups.insert(s->m_group);
    }
    return std::vector<std::string>(unique_groups.begin(), unique_groups.end());
}


void StudentDB_V2_BST::sort_standard(const std::string& filename) {
    std::vector<Student> sorted_list = all_students_storage;
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(sorted_list.begin(), sorted_list.end(), [](const Student& a, const Student& b) {
        return a.m_email < b.m_email;
    });
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "std::sort time: " << duration << " ms\n";
    save_to_csv(filename, sorted_list, "std::sort");
}

void StudentDB_V2_BST::sort_custom_radix(const std::string& filename) {
    if (all_students_storage.empty()) return;

    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Student> sorted_list = all_students_storage;
    std::vector<Student> buffer(sorted_list.size());
    
    msd_radix_sort_recursive(sorted_list, 0, sorted_list.size(), 0, buffer);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Radix Sort time: " << duration << " ms\n";

    save_to_csv(filename, sorted_list, "Radix Sort");
}

size_t StudentDB_V2_BST::get_student_count() const { return all_students_storage.size(); }

const std::vector<Student>& StudentDB_V2_BST::get_all_students() const {
    return all_students_storage;
}


// vector version
StudentDB_V3_Vector::StudentDB_V3_Vector() {}

bool StudentDB_V3_Vector::initialize_from_csv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    if (!std::getline(file, line)) return false; // Skip header

    //needed for temporary storage, to be sorted later
    std::map<std::string, Student*> temp_phone_index;
    std::map<std::string, std::vector<Student*>> temp_group_index;
    std::map<std::string, std::vector<Student*>> temp_surname_index;

    size_t line_count = 0;
    while (std::getline(file, line)) {
        line_count++;
    }

    if (line_count == 0) return true;

    all_students_storage.reserve(line_count);

    file.clear();
    file.seekg(0);
    if (!std::getline(file, line)) return false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        Student s;
        std::getline(ss, s.m_name, ',');
        std::getline(ss, s.m_surname, ',');
        std::getline(ss, s.m_email, ',');
        std::getline(ss, token, ','); s.m_birth_year = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_month = std::stoi(token);
        std::getline(ss, token, ','); s.m_birth_day = std::stoi(token);
        std::getline(ss, s.m_group, ',');
        std::getline(ss, token, ','); s.m_rating = std::stof(token);
        std::getline(ss, s.m_phone_number); 

        all_students_storage.push_back(std::move(s));
        Student* new_student_ptr = &all_students_storage.back();

        temp_phone_index[new_student_ptr->m_phone_number] = new_student_ptr;
        temp_group_index[new_student_ptr->m_group].push_back(new_student_ptr);
        temp_surname_index[new_student_ptr->m_surname].push_back(new_student_ptr);
    }

    phone_to_student_ptr.assign(temp_phone_index.begin(), temp_phone_index.end());
    group_to_students_ptr.assign(temp_group_index.begin(), temp_group_index.end());
    surname_to_students_ptr.assign(temp_surname_index.begin(), temp_surname_index.end());

    return true;
}

// 1) Змінити групу студенту за його номером телефону (m_phone_number)
bool StudentDB_V3_Vector::changeGroupByPhone(const std::string& phone, const std::string& new_group) {
    auto it_phone = std::lower_bound(
        phone_to_student_ptr.begin(), 
        phone_to_student_ptr.end(), 
        phone, 
        PairKeyCompare()
    );

    if (it_phone == phone_to_student_ptr.end() || it_phone->first != phone) {
        return false;
    }

    Student* s = it_phone->second;
    const std::string old_group = s->m_group;
    if (old_group == new_group) return true;

    auto it_group_old = std::lower_bound(
        group_to_students_ptr.begin(), 
        group_to_students_ptr.end(), 
        old_group, 
        PairKeyCompare()
    );

    if (it_group_old != group_to_students_ptr.end() && it_group_old->first == old_group) {
        auto& old_list = it_group_old->second;
        old_list.erase(std::remove(old_list.begin(), old_list.end(), s), old_list.end());
    }

    auto it_group_new = std::lower_bound(
        group_to_students_ptr.begin(), 
        group_to_students_ptr.end(), 
        new_group, 
        PairKeyCompare()
    );

    if (it_group_new != group_to_students_ptr.end() && it_group_new->first == new_group) {
        it_group_new->second.push_back(s);
    } else {
        group_to_students_ptr.insert(it_group_new, {new_group, {s}});
    }

    s->m_group = new_group;
    return true;
}

// 2) Повернути студентів групи m_group в алфавітному порядку за прізвищем і ім’ям (m_surname, m_name)
std::vector<const Student*> StudentDB_V3_Vector::getSortedStudentsByGroup(const std::string& group_name) const {
    auto it = std::lower_bound(
        group_to_students_ptr.begin(), 
        group_to_students_ptr.end(), 
        group_name, 
        PairKeyCompare()
    );

    if (it == group_to_students_ptr.end() || it->first != group_name) {
        return {};
    }

    std::vector<const Student*> result(it->second.begin(), it->second.end()); 

    std::sort(result.begin(), result.end(), [](const Student* a, const Student* b) { 
        if (a->m_surname != b->m_surname) return a->m_surname < b->m_surname;
        return a->m_name < b->m_name;
    });
    return result;
}

// 3) Повернути список груп, де навчаються студенти з прізвищем m_surname
std::vector<std::string> StudentDB_V3_Vector::getGroupsBySurname(const std::string& surname) const {
    auto it = std::lower_bound(
        surname_to_students_ptr.begin(), 
        surname_to_students_ptr.end(), 
        surname, 
        PairKeyCompare()
    );

    if (it == surname_to_students_ptr.end() || it->first != surname) {
        return {};
    }

    std::set<std::string> unique_groups; 
    for (const Student* s : it->second) {
        unique_groups.insert(s->m_group);
    }
    return std::vector<std::string>(unique_groups.begin(), unique_groups.end());
}

void StudentDB_V3_Vector::sort_standard(const std::string& filename) {
    std::vector<Student> sorted_list = all_students_storage;
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(sorted_list.begin(), sorted_list.end(), [](const Student& a, const Student& b) {
        return a.m_email < b.m_email;
    });
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "std::sort time: " << duration << " ms\n";
    save_to_csv(filename, sorted_list, "std::sort");
}

void StudentDB_V3_Vector::sort_custom_radix(const std::string& filename) {
    if (all_students_storage.empty()) return;

    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<Student> sorted_list = all_students_storage;
    std::vector<Student> buffer(sorted_list.size());
    
    msd_radix_sort_recursive(sorted_list, 0, sorted_list.size(), 0, buffer);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Radix Sort time: " << duration << " ms\n";

    save_to_csv(filename, sorted_list, "Radix Sort");
}

size_t StudentDB_V3_Vector::get_student_count() const { return all_students_storage.size(); }

const std::vector<Student>& StudentDB_V3_Vector::get_all_students() const {
    return all_students_storage;
}
