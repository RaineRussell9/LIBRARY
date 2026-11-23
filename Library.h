// Library.h - all the library rules (no cout / cin in this class)
//
// DATA STRUCTURES USED (see the Big-O notes in Library.cpp):
//   vector<Book>                  books kept sorted by ID  -> binary search by ID
//   unordered_map<string,Member>  members by student ID    -> hash lookup
//   vector<Loan>                  every loan ever made (history, append only)
//   unordered_map<int,size_t>     book ID -> its active loan -> "is borrowed?" check
#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Book.h"
#include "Date.h"
#include "Loan.h"
#include "Member.h"
#include "Storage.h"

// ---------- Rules you can change ----------
const int LOAN_DAYS = 7;                // days before the due date
const int MAX_LOANS_PER_STUDENT = 3;    // books one student can hold at once
const int FINE_PER_DAY = 5;             // fine for each day a book is late

// Every action returns a Result; main.cpp turns it into a message.
enum class Result {
    Ok,
    NotFound,            // book ID does not exist
    InvalidInput,        // a required field is empty
    AlreadyBorrowed,
    CurrentlyBorrowed,   // cannot remove a borrowed book
    LimitReached,        // student holds too many books
    HasOverdue,          // student has an overdue book
    NotBorrowed,         // trying to return a book that is not out
    BadReturnDate,       // return date is before the borrow date
    MemberNotFound,
    MemberExists,
    MemberHasLoans       // cannot remove a student who still holds books
};

class Library {
public:
    explicit Library(Storage& storage);

    // ----- Books (Create / Read / Update / Delete) -----
    Result addBook(const std::string& title, const std::string& author,
                   const std::string& category, int& newId);
    Result editBook(int id, const std::string& title, const std::string& author,
                    const std::string& category);          // empty text = keep old value
    Result removeBook(int id, std::string& removedTitle);
    const Book* findBook(int id) const;                    // NULL if not found
    const std::vector<Book>& allBooks() const;             // sorted by ID
    std::vector<Book> booksSortedByTitle() const;
    std::vector<Book> searchBooks(const std::string& keyword) const;
    bool isBorrowed(int bookId) const;
    const Loan* activeLoanFor(int bookId) const;           // NULL if available

    // ----- Students (Create / Read / Update / Delete) -----
    Result registerMember(const std::string& id, const std::string& name,
                          const std::string& course);
    Result editMember(const std::string& id, const std::string& name,
                      const std::string& course);          // empty text = keep old value
    Result removeMember(const std::string& id, std::string& removedName);
    const Member* findMember(const std::string& id) const; // NULL if not found
    std::vector<Member> allMembers() const;                // sorted by student ID
    int activeLoanCount(const std::string& studentId) const;

    // ----- Borrow / return -----
    Result borrowBook(int bookId, const std::string& studentId,
                      const Date& today, Loan& out);
    Result returnBook(int bookId, const Date& today, Loan& out, int& fine);
    int fineFor(const Loan& loan, const Date& asOf) const;

    // ----- Reports -----
    std::vector<Loan> activeLoans() const;
    std::vector<Loan> overdueLoans(const Date& today) const;   // most overdue first
    std::vector<Loan> historyFor(const std::string& studentId) const;

private:
    Storage& storage;
    std::vector<Book> books;                          // always sorted by id
    std::unordered_map<std::string, Member> members;  // key = student ID
    std::vector<Loan> loans;                          // history, append only
    std::unordered_map<int, size_t> activeByBook;     // bookId -> index in loans
    int nextBookId;
    int nextLoanId;

    int  findBookIndex(int id) const;
    bool studentHasOverdue(const std::string& studentId, const Date& today) const;
    void save();
};

#endif
