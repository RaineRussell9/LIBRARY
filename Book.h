// Book.h - one book in the library
#ifndef BOOK_H
#define BOOK_H

#include <string>

struct Book {
    int         id = 0;
    std::string title;
    std::string author;
    std::string category;
    // No "available" flag: a book is borrowed if it has an active Loan.
    // This keeps the data consistent (one source of truth).
};

#endif
