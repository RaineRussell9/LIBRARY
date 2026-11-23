// Library.cpp - implementation of the library rules
//
// Big-O summary (n = number of books, m = number of students, L = number of loans):
//   find book by ID ........ O(log n)   binary search on the sorted vector
//   is book borrowed? ...... O(1)       hash lookup in activeByBook
//   find student by ID ..... O(1)       hash lookup in members
//   add book ............... O(1)       new ID is the largest, so it goes at the end
//   remove book ............ O(n)       elements after it shift left
//   search by keyword ...... O(n)       checks every book
//   list sorted by title ... O(n log n) std::sort
//   overdue report ......... O(L log L) scan the loans, then sort by due date
#include "Library.h"
#include <algorithm>
#include <cctype>
using namespace std;

// ---------- small helpers ----------
static string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

// Trims spaces and replaces '|' (the file separator) so it cannot break the files.
static string clean(const string& in) {
    string s = in;
    for (size_t i = 0; i < s.size(); i++) if (s[i] == '|') s[i] = '/';
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ---------- start up ----------
Library::Library(Storage& s) : storage(s), nextBookId(1), nextLoanId(1) {
    vector<Member> memberList;
    storage.load(books, memberList, loans);

    // the binary search needs the books sorted by ID
    sort(books.begin(), books.end(),
         [](const Book& a, const Book& b) { return a.id < b.id; });

    for (size_t i = 0; i < memberList.size(); i++)
        members[memberList[i].id] = memberList[i];

    // never reuse an old ID, even one that only appears in loan history
    for (size_t i = 0; i < books.size(); i++)
        nextBookId = max(nextBookId, books[i].id + 1);
    for (size_t i = 0; i < loans.size(); i++) {
        nextBookId = max(nextBookId, loans[i].bookId + 1);
        nextLoanId = max(nextLoanId, loans[i].id + 1);
        if (!loans[i].returned) activeByBook[loans[i].bookId] = i;
    }
}

void Library::save() {
    storage.save(books, allMembers(), loans);
}

int Library::findBookIndex(int id) const {
    vector<Book>::const_iterator it = lower_bound(
        books.begin(), books.end(), id,
        [](const Book& b, int value) { return b.id < value; });
    if (it != books.end() && it->id == id) return (int)(it - books.begin());
    return -1;
}

// =====================================================================
// Books
// =====================================================================
Result Library::addBook(const string& title, const string& author,
                        const string& category, int& newId) {
    Book b;
    b.title    = clean(title);
    b.author   = clean(author);
    b.category = clean(category);
    if (b.title.empty() || b.author.empty()) return Result::InvalidInput;
    if (b.category.empty()) b.category = "General";
    b.id = nextBookId++;
    books.push_back(b);          // largest ID so far -> vector stays sorted
    save();
    newId = b.id;
    return Result::Ok;
}

Result Library::editBook(int id, const string& title, const string& author,
                         const string& category) {
    int i = findBookIndex(id);
    if (i == -1) return Result::NotFound;
    string t = clean(title), a = clean(author), c = clean(category);
    if (!t.empty()) books[i].title = t;
    if (!a.empty()) books[i].author = a;
    if (!c.empty()) books[i].category = c;
    save();
    return Result::Ok;
}

Result Library::removeBook(int id, string& removedTitle) {
    int i = findBookIndex(id);
    if (i == -1) return Result::NotFound;
    if (isBorrowed(id)) return Result::CurrentlyBorrowed;
    removedTitle = books[i].title;
    books.erase(books.begin() + i);
    save();
    return Result::Ok;
}

const Book* Library::findBook(int id) const {
    int i = findBookIndex(id);
    return i == -1 ? NULL : &books[i];
}

const vector<Book>& Library::allBooks() const { return books; }

vector<Book> Library::booksSortedByTitle() const {
    vector<Book> copy = books;
    sort(copy.begin(), copy.end(), [](const Book& a, const Book& b) {
        return toLower(a.title) < toLower(b.title);
    });
    return copy;
}

vector<Book> Library::searchBooks(const string& keyword) const {
    string k = toLower(clean(keyword));
    vector<Book> found;
    for (size_t i = 0; i < books.size(); i++) {
        if (toLower(books[i].title).find(k)    != string::npos ||
            toLower(books[i].author).find(k)   != string::npos ||
            toLower(books[i].category).find(k) != string::npos)
            found.push_back(books[i]);
    }
    return found;
}

bool Library::isBorrowed(int bookId) const {
    return activeByBook.count(bookId) > 0;
}

const Loan* Library::activeLoanFor(int bookId) const {
    unordered_map<int, size_t>::const_iterator it = activeByBook.find(bookId);
    return it == activeByBook.end() ? NULL : &loans[it->second];
}

// =====================================================================
// Students
// =====================================================================
Result Library::registerMember(const string& id, const string& name, const string& course) {
    Member m;
    m.id     = clean(id);
    m.name   = clean(name);
    m.course = clean(course);
    if (m.id.empty() || m.name.empty()) return Result::InvalidInput;
    if (m.course.empty()) m.course = "N/A";
    if (members.count(m.id)) return Result::MemberExists;
    members[m.id] = m;
    save();
    return Result::Ok;
}

Result Library::editMember(const string& id, const string& name, const string& course) {
    unordered_map<string, Member>::iterator it = members.find(clean(id));
    if (it == members.end()) return Result::MemberNotFound;
    string n = clean(name), c = clean(course);
    if (!n.empty()) {
        it->second.name = n;
        // keep old loan records showing the new name
        for (size_t i = 0; i < loans.size(); i++)
            if (loans[i].studentId == it->first) loans[i].studentName = n;
    }
    if (!c.empty()) it->second.course = c;
    save();
    return Result::Ok;
}

Result Library::removeMember(const string& id, string& removedName) {
    unordered_map<string, Member>::iterator it = members.find(clean(id));
    if (it == members.end()) return Result::MemberNotFound;
    if (activeLoanCount(it->first) > 0) return Result::MemberHasLoans;
    removedName = it->second.name;
    members.erase(it);
    save();
    return Result::Ok;
}

const Member* Library::findMember(const string& id) const {
    unordered_map<string, Member>::const_iterator it = members.find(clean(id));
    return it == members.end() ? NULL : &it->second;
}

vector<Member> Library::allMembers() const {
    vector<Member> list;
    for (unordered_map<string, Member>::const_iterator it = members.begin();
         it != members.end(); ++it)
        list.push_back(it->second);
    sort(list.begin(), list.end(),
         [](const Member& a, const Member& b) { return a.id < b.id; });
    return list;
}

int Library::activeLoanCount(const string& studentId) const {
    int n = 0;
    for (size_t i = 0; i < loans.size(); i++)
        if (!loans[i].returned && loans[i].studentId == studentId) n++;
    return n;
}

bool Library::studentHasOverdue(const string& studentId, const Date& today) const {
    for (size_t i = 0; i < loans.size(); i++)
        if (!loans[i].returned && loans[i].studentId == studentId &&
            daysBetween(today, loans[i].dueDate) > 0)
            return true;
    return false;
}

// =====================================================================
// Borrow / return
// =====================================================================
Result Library::borrowBook(int bookId, const string& studentId,
                           const Date& today, Loan& out) {
    int bi = findBookIndex(bookId);
    if (bi == -1) return Result::NotFound;

    const Member* m = findMember(studentId);
    if (m == NULL) return Result::MemberNotFound;

    if (isBorrowed(bookId)) return Result::AlreadyBorrowed;
    if (activeLoanCount(m->id) >= MAX_LOANS_PER_STUDENT) return Result::LimitReached;
    if (studentHasOverdue(m->id, today)) return Result::HasOverdue;

    Loan l;
    l.id          = nextLoanId++;
    l.bookId      = bookId;
    l.bookTitle   = books[bi].title;
    l.studentId   = m->id;
    l.studentName = m->name;
    l.borrowDate  = today;
    l.dueDate     = addDays(today, LOAN_DAYS);
    l.returned    = false;

    loans.push_back(l);
    activeByBook[bookId] = loans.size() - 1;
    save();
    out = l;
    return Result::Ok;
}

// Fine counts until the return date, or until "asOf" if the book is not back yet.
int Library::fineFor(const Loan& l, const Date& asOf) const {
    const Date& end = l.returned ? l.returnDate : asOf;
    long late = daysBetween(end, l.dueDate);
    return late > 0 ? (int)(late * FINE_PER_DAY) : 0;
}

Result Library::returnBook(int bookId, const Date& today, Loan& out, int& fine) {
    if (findBookIndex(bookId) == -1) return Result::NotFound;
    unordered_map<int, size_t>::iterator it = activeByBook.find(bookId);
    if (it == activeByBook.end()) return Result::NotBorrowed;

    Loan& l = loans[it->second];
    if (daysBetween(today, l.borrowDate) < 0) return Result::BadReturnDate;

    l.returned   = true;
    l.returnDate = today;
    fine = fineFor(l, today);
    out = l;
    activeByBook.erase(it);
    save();
    return Result::Ok;
}

// =====================================================================
// Reports
// =====================================================================
vector<Loan> Library::activeLoans() const {
    vector<Loan> r;
    for (size_t i = 0; i < loans.size(); i++)
        if (!loans[i].returned) r.push_back(loans[i]);
    return r;
}

vector<Loan> Library::overdueLoans(const Date& today) const {
    vector<Loan> r;
    for (size_t i = 0; i < loans.size(); i++)
        if (!loans[i].returned && daysBetween(today, loans[i].dueDate) > 0)
            r.push_back(loans[i]);
    // most overdue first = earliest due date first
    sort(r.begin(), r.end(), [](const Loan& a, const Loan& b) {
        return toSerial(a.dueDate) < toSerial(b.dueDate);
    });
    return r;
}

vector<Loan> Library::historyFor(const string& studentId) const {
    string sid = clean(studentId);
    vector<Loan> r;
    for (size_t i = 0; i < loans.size(); i++)
        if (loans[i].studentId == sid) r.push_back(loans[i]);
    return r;
}
