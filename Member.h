// Member.h - a registered borrower (student)
#ifndef MEMBER_H
#define MEMBER_H

#include <string>

struct Member {
    std::string id;       // student ID, e.g. "2024001" (unique)
    std::string name;
    std::string course;   // e.g. "BSIT"
};

#endif
