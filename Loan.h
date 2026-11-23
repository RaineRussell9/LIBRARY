// Loan.h - one borrowing record (kept forever as history)
#ifndef LOAN_H
#define LOAN_H

#include <string>
#include "Date.h"

struct Loan {
    int         id = 0;
    int         bookId = 0;
    std::string bookTitle;     // copied so history survives book removal
    std::string studentId;
    std::string studentName;
    Date        borrowDate;
    Date        dueDate;
    Date        returnDate;    // only meaningful when returned == true
    bool        returned = false;
};

#endif
