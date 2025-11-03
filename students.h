#ifndef STUDENTS_H
#define STUDENTS_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>

/*
Task 2

1) Змінити групу студенту за його номером телефону (m_phone_number)
2) Повернути студентів групи m_group в алфавітному порядку за прізвищем і ім’ям
(m_surname, m_name)
3) Повернути список груп, де навчаються студенти з прізвищем m_surname
*/

/*
Task 3

S1. Посортувати всіх студентів за (m_email) і зберегти в CSV файл
*/

struct Student
{
    std::string m_name;
    std::string m_surname;
    std::string m_email;
    int m_birth_year = 0;
    int m_birth_month = 0;
    int m_birth_day = 0;
    std::string m_group;
    float m_rating = 0.0f;
    std::string m_phone_number;

    void print() const;
    std::string to_csv_row() const;
};

void save_to_csv(const std::string& filename, const std::vector<Student>& students, const std::string& sort_name);

class IStudentDB {
public:
    virtual ~IStudentDB() = default;

    virtual bool initialize_from_csv(const std::string& filename) = 0;
    virtual bool changeGroupByPhone(const std::string& phone, const std::string& new_group) = 0;
    virtual std::vector<const Student*> getSortedStudentsByGroup(const std::string& group_name) const = 0;
    virtual std::vector<std::string> getGroupsBySurname(const std::string& surname) const = 0;
    
    virtual void sort_standard(const std::string& filename) = 0;
    virtual void sort_custom_radix(const std::string& filename) = 0;
    
    virtual size_t get_student_count() const = 0;
    virtual const std::vector<Student>& get_all_students() const = 0; 
};


// hash map version
class StudentDB_V1_Hash : public IStudentDB {
private:
    std::vector<Student> all_students_storage; 
    std::unordered_map<std::string, Student*> phone_to_student_ptr; 
    std::unordered_map<std::string, std::vector<Student*>> group_to_students_ptr;
    std::unordered_map<std::string, std::vector<Student*>> surname_to_students_ptr; 

public:
    StudentDB_V1_Hash();
    bool initialize_from_csv(const std::string& filename) override;
    bool changeGroupByPhone(const std::string& phone, const std::string& new_group) override;
    std::vector<const Student*> getSortedStudentsByGroup(const std::string& group_name) const override;
    std::vector<std::string> getGroupsBySurname(const std::string& surname) const override;
    
    void sort_standard(const std::string& filename) override;
    void sort_custom_radix(const std::string& filename) override;
    
    size_t get_student_count() const override;
    const std::vector<Student>& get_all_students() const override;
};

// bst map version
class StudentDB_V2_BST : public IStudentDB {
private:
    std::vector<Student> all_students_storage; 
    std::map<std::string, Student*> phone_to_student_ptr; 
    std::map<std::string, std::vector<Student*>> group_to_students_ptr;
    std::map<std::string, std::vector<Student*>> surname_to_students_ptr; 

public:
    StudentDB_V2_BST();
    bool initialize_from_csv(const std::string& filename) override;
    bool changeGroupByPhone(const std::string& phone, const std::string& new_group) override;
    std::vector<const Student*> getSortedStudentsByGroup(const std::string& group_name) const override;
    std::vector<std::string> getGroupsBySurname(const std::string& surname) const override;
    
    void sort_standard(const std::string& filename) override;
    void sort_custom_radix(const std::string& filename) override;
    
    size_t get_student_count() const override;
    const std::vector<Student>& get_all_students() const override;
};

// vector version
class StudentDB_V3_Vector : public IStudentDB {
private:
    std::vector<Student> all_students_storage; 

    std::vector<std::pair<std::string, Student*>> phone_to_student_ptr;
    std::vector<std::pair<std::string, std::vector<Student*>>> group_to_students_ptr;
    std::vector<std::pair<std::string, std::vector<Student*>>> surname_to_students_ptr;

public:
    StudentDB_V3_Vector();
    bool initialize_from_csv(const std::string& filename) override;
    bool changeGroupByPhone(const std::string& phone, const std::string& new_group) override;
    std::vector<const Student*> getSortedStudentsByGroup(const std::string& group_name) const override;
    std::vector<std::string> getGroupsBySurname(const std::string& surname) const override;
    
    void sort_standard(const std::string& filename) override;
    void sort_custom_radix(const std::string& filename) override;
    
    size_t get_student_count() const override;
    const std::vector<Student>& get_all_students() const override;
};

#endif