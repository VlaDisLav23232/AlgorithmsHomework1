#include "students.h"

int main() {
    StudentDB_V1_Hash db;
    db.initialize_from_csv("students.csv");
    db.sort_standard("sorted_students.csv");
    db.sort_custom_radix("sorted_students_radix.csv");
    return 0;
}