// main.cpp - Library Management System (console version)
//
// Entry point and menu. This file only talks to the user (cin / cout);
// all the rules are in the Library class (Library.h / Library.cpp).
//
// Compile:  g++ -std=c++11 -o library main.cpp Date.cpp Storage.cpp Library.cpp
//           (or just run "make" if you have make installed)
// Run:      ./library        (on Windows: library.exe)
// Data is saved automatically in books.txt, members.txt and loans.txt.

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "Library.h"
using namespace std;

// ---------- messages ----------
string describe(Result r) {
    switch (r) {
        case Result::Ok:                return "Done.";
        case Result::NotFound:          return "Book not found.";
        case Result::InvalidInput:      return "Required fields cannot be empty.";
        case Result::AlreadyBorrowed:   return "Sorry, this book is already borrowed.";
        case Result::CurrentlyBorrowed: return "Cannot remove: the book is currently borrowed.";
        case Result::LimitReached:      return "This student has reached the borrowing limit ("
                                               + to_string(MAX_LOANS_PER_STUDENT) + " books).";
        case Result::HasOverdue:        return "This student has an overdue book and must return it first.";
        case Result::NotBorrowed:       return "That book is not currently borrowed.";
        case Result::BadReturnDate:     return "Return date cannot be before the borrow date.";
        case Result::MemberNotFound:    return "Student not registered. Register the student first (option 6).";
        case Result::MemberExists:      return "A student with that ID is already registered.";
        case Result::MemberHasLoans:    return "Cannot remove: the student still has borrowed books.";
    }
    return "Unknown error.";
}

// ---------- input helpers ----------
string readLine(const string& prompt) {
    string s;
    cout << prompt;
    if (!getline(cin, s)) exit(0);       // input ended
    return s;
}

int readInt(const string& prompt) {
    int n;
    while (true) {
        cout << prompt;
        if (cin >> n) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return n;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a number.\n";
    }
}

Date readDate(const string& label) {
    Date d;
    while (true) {
        cout << label << " (YYYY MM DD): ";
        if (cin >> d.year >> d.month >> d.day && isValidDate(d)) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return d;
        }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid date. Example: 2026 11 06\n";
    }
}

// ---------- printing ----------
string fit(const string& s, size_t width) {
    return s.size() > width ? s.substr(0, width - 3) + "..." : s;
}

void printBooks(const Library& lib, const vector<Book>& list) {
    if (list.empty()) { cout << "No books found.\n"; return; }

    cout << "\n" << left << setw(5) << "ID" << setw(30) << "Title"
         << setw(20) << "Author" << setw(14) << "Category" << "Status\n";
    cout << string(80, '-') << "\n";

    for (size_t i = 0; i < list.size(); i++) {
        const Book& b = list[i];
        const Loan* loan = lib.activeLoanFor(b.id);
        cout << left << setw(5) << b.id << setw(30) << fit(b.title, 28)
             << setw(20) << fit(b.author, 18) << setw(14) << fit(b.category, 12)
             << (loan ? "Borrowed" : "Available") << "\n";
        if (loan) {
            cout << "     Borrowed by: " << loan->studentName
                 << " (ID: " << loan->studentId << ")"
                 << " | Borrowed: " << formatDate(loan->borrowDate)
                 << " | Due: " << formatDate(loan->dueDate) << "\n";
        }
    }
}

void printMembers(const Library& lib) {
    vector<Member> list = lib.allMembers();
    if (list.empty()) { cout << "No students registered.\n"; return; }

    cout << "\n" << left << setw(14) << "Student ID" << setw(26) << "Name"
         << setw(12) << "Course" << "Books out\n";
    cout << string(62, '-') << "\n";
    for (size_t i = 0; i < list.size(); i++)
        cout << left << setw(14) << list[i].id << setw(26) << fit(list[i].name, 24)
             << setw(12) << fit(list[i].course, 10)
             << lib.activeLoanCount(list[i].id) << "\n";
}

void printLoanHeader() {
    cout << "\n" << left << setw(6) << "Loan" << setw(7) << "Book" << setw(26) << "Title"
         << setw(14) << "Student ID" << setw(12) << "Borrowed"
         << setw(12) << "Due" << "Returned\n";
    cout << string(90, '-') << "\n";
}

void printLoanRow(const Loan& l) {
    cout << left << setw(6) << l.id << setw(7) << l.bookId
         << setw(26) << fit(l.bookTitle, 24) << setw(14) << fit(l.studentId, 12)
         << setw(12) << formatDate(l.borrowDate) << setw(12) << formatDate(l.dueDate)
         << (l.returned ? formatDate(l.returnDate) : string("not yet")) << "\n";
}

// ---------- one function per menu option ----------
// BOOKS
void uiAddBook(Library& lib) {
    string title    = readLine("Title   : ");
    string author   = readLine("Author  : ");
    string category = readLine("Category (Enter = General): ");
    int newId = 0;
    Result r = lib.addBook(title, author, category, newId);
    if (r == Result::Ok) cout << "Book added. Book ID: " << newId << "\n";
    else cout << describe(r) << "\n";
}

void uiEditBook(Library& lib) {
    int id = readInt("Book ID to edit: ");
    const Book* b = lib.findBook(id);
    if (b == NULL) { cout << describe(Result::NotFound) << "\n"; return; }
    cout << "Current: " << b->title << " | " << b->author << " | " << b->category << "\n";
    string title    = readLine("New title    (Enter = keep): ");
    string author   = readLine("New author   (Enter = keep): ");
    string category = readLine("New category (Enter = keep): ");
    Result r = lib.editBook(id, title, author, category);
    cout << (r == Result::Ok ? "Book updated." : describe(r)) << "\n";
}

void uiRemoveBook(Library& lib) {
    int id = readInt("Book ID to remove: ");
    string title;
    Result r = lib.removeBook(id, title);
    if (r == Result::Ok) cout << "Removed: " << title << "\n";
    else cout << describe(r) << "\n";
}

void uiListBooks(const Library& lib) {
    cout << "Sort by: 1 = Book ID, 2 = Title (A-Z): ";
    int c = readInt("");
    if (c == 2) printBooks(lib, lib.booksSortedByTitle());
    else        printBooks(lib, lib.allBooks());
}

void uiSearch(const Library& lib) {
    string keyword = readLine("Search (title, author or category): ");
    printBooks(lib, lib.searchBooks(keyword));
}

// STUDENTS
void uiRegisterMember(Library& lib) {
    string id     = readLine("Student ID : ");
    string name   = readLine("Student Name: ");
    string course = readLine("Course (Enter = N/A): ");
    Result r = lib.registerMember(id, name, course);
    cout << (r == Result::Ok ? "Student registered." : describe(r)) << "\n";
}

void uiEditMember(Library& lib) {
    string id = readLine("Student ID to edit: ");
    const Member* m = lib.findMember(id);
    if (m == NULL) { cout << describe(Result::MemberNotFound) << "\n"; return; }
    cout << "Current: " << m->name << " | " << m->course << "\n";
    string name   = readLine("New name   (Enter = keep): ");
    string course = readLine("New course (Enter = keep): ");
    Result r = lib.editMember(id, name, course);
    cout << (r == Result::Ok ? "Student updated." : describe(r)) << "\n";
}

void uiRemoveMember(Library& lib) {
    string id = readLine("Student ID to remove: ");
    string name;
    Result r = lib.removeMember(id, name);
    if (r == Result::Ok) cout << "Removed student: " << name << "\n";
    else cout << describe(r) << "\n";
}

// BORROWING
void uiBorrow(Library& lib) {
    int id       = readInt("Book ID: ");
    string sid   = readLine("Student ID: ");
    Date today   = readDate("Borrow date");
    Loan loan;
    Result r = lib.borrowBook(id, sid, today, loan);
    if (r != Result::Ok) { cout << describe(r) << "\n"; return; }

    cout << "\n--- Borrow Record ---\n";
    cout << "Book         : " << loan.bookTitle << "\n";
    cout << "Student Name : " << loan.studentName << "\n";
    cout << "Student ID   : " << loan.studentId << "\n";
    cout << "Date Borrowed: " << formatDate(loan.borrowDate) << "\n";
    cout << "Due Date     : " << formatDate(loan.dueDate) << "\n";
}

void uiReturn(Library& lib) {
    int id     = readInt("Book ID being returned: ");
    Date today = readDate("Return date");
    Loan loan;
    int fine = 0;
    Result r = lib.returnBook(id, today, loan, fine);
    if (r != Result::Ok) { cout << describe(r) << "\n"; return; }

    cout << "\n--- Return Record ---\n";
    cout << "Book         : " << loan.bookTitle << "\n";
    cout << "Returned by  : " << loan.studentName << " (ID: " << loan.studentId << ")\n";
    cout << "Due Date     : " << formatDate(loan.dueDate) << "\n";
    cout << "Date Returned: " << formatDate(loan.returnDate) << "\n";
    if (fine > 0) cout << "LATE! Fine   : " << fine << "\n";
    else          cout << "Returned on time. No fine.\n";
}

// REPORTS
void uiListBorrowed(const Library& lib) {
    vector<Loan> list = lib.activeLoans();
    if (list.empty()) { cout << "No books are currently borrowed.\n"; return; }
    printLoanHeader();
    for (size_t i = 0; i < list.size(); i++) printLoanRow(list[i]);
}

void uiOverdue(const Library& lib) {
    Date today = readDate("Today's date");
    vector<Loan> list = lib.overdueLoans(today);
    if (list.empty()) { cout << "No overdue books.\n"; return; }
    printLoanHeader();
    for (size_t i = 0; i < list.size(); i++) {
        printLoanRow(list[i]);
        cout << "     " << list[i].studentName << " is "
             << daysBetween(today, list[i].dueDate) << " day(s) late. Fine so far: "
             << lib.fineFor(list[i], today) << "\n";
    }
}

void uiHistory(const Library& lib) {
    string sid = readLine("Student ID: ");
    vector<Loan> list = lib.historyFor(sid);
    if (list.empty()) { cout << "No records for that student.\n"; return; }
    printLoanHeader();
    for (size_t i = 0; i < list.size(); i++) printLoanRow(list[i]);
}

// ---------- menu ----------
void showMenu() {
    cout << "\n===== LIBRARY MANAGEMENT SYSTEM =====\n"
         << "--- Books ---\n"
         << " 1. Add book\n"
         << " 2. Edit book\n"
         << " 3. Remove book\n"
         << " 4. List books\n"
         << " 5. Search books\n"
         << "--- Students ---\n"
         << " 6. Register student\n"
         << " 7. Edit student\n"
         << " 8. Remove student\n"
         << " 9. List students\n"
         << "--- Borrowing ---\n"
         << "10. Borrow book\n"
         << "11. Return book\n"
         << "12. List borrowed books\n"
         << "13. Overdue books\n"
         << "14. Student borrowing history\n"
         << " 0. Exit\n";
}

int main() {
    Storage storage("books.txt", "members.txt", "loans.txt");
    Library library(storage);
    int choice;

    do {
        showMenu();
        choice = readInt("Enter choice: ");
        cout << "\n";

        switch (choice) {
            case 1:  uiAddBook(library);        break;
            case 2:  uiEditBook(library);       break;
            case 3:  uiRemoveBook(library);     break;
            case 4:  uiListBooks(library);      break;
            case 5:  uiSearch(library);         break;
            case 6:  uiRegisterMember(library); break;
            case 7:  uiEditMember(library);     break;
            case 8:  uiRemoveMember(library);   break;
            case 9:  printMembers(library);     break;
            case 10: uiBorrow(library);         break;
            case 11: uiReturn(library);         break;
            case 12: uiListBorrowed(library);   break;
            case 13: uiOverdue(library);        break;
            case 14: uiHistory(library);        break;
            case 0:  cout << "Goodbye!\n";      break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    return 0;
}
