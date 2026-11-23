// Storage.cpp - file handling with <fstream>
#include "Storage.h"
#include <cstdlib>
#include <fstream>
using namespace std;

// Splits "a|b|c" into {"a","b","c"} (empty fields are kept)
static vector<string> splitLine(const string& line, char sep) {
    vector<string> parts;
    string cur;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == sep) { parts.push_back(cur); cur.clear(); }
        else cur += line[i];
    }
    parts.push_back(cur);
    return parts;
}

static bool readNextLine(ifstream& in, string& line) {
    if (!getline(in, line)) return false;
    if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
    return true;
}

Storage::Storage(const string& b, const string& m, const string& l)
    : booksFile(b), membersFile(m), loansFile(l) {}

void Storage::load(vector<Book>& books, vector<Member>& members, vector<Loan>& loans) const {
    books.clear();
    members.clear();
    loans.clear();
    string line;

    ifstream fb(booksFile.c_str());
    while (readNextLine(fb, line)) {
        vector<string> p = splitLine(line, '|');
        if (p.size() < 4) continue;
        Book b;
        b.id = atoi(p[0].c_str());
        b.title = p[1]; b.author = p[2]; b.category = p[3];
        books.push_back(b);
    }

    ifstream fm(membersFile.c_str());
    while (readNextLine(fm, line)) {
        vector<string> p = splitLine(line, '|');
        if (p.size() < 3) continue;
        Member m;
        m.id = p[0]; m.name = p[1]; m.course = p[2];
        members.push_back(m);
    }

    ifstream fl(loansFile.c_str());
    while (readNextLine(fl, line)) {
        vector<string> p = splitLine(line, '|');
        if (p.size() < 9) continue;
        Loan l;
        l.id          = atoi(p[0].c_str());
        l.bookId      = atoi(p[1].c_str());
        l.bookTitle   = p[2];
        l.studentId   = p[3];
        l.studentName = p[4];
        l.borrowDate  = parseDate(p[5]);
        l.dueDate     = parseDate(p[6]);
        l.returnDate  = parseDate(p[7]);
        l.returned    = (p[8] == "1");
        loans.push_back(l);
    }
}

void Storage::save(const vector<Book>& books, const vector<Member>& members,
                   const vector<Loan>& loans) const {
    ofstream fb(booksFile.c_str());
    for (size_t i = 0; i < books.size(); i++)
        fb << books[i].id << '|' << books[i].title << '|' << books[i].author
           << '|' << books[i].category << '\n';

    ofstream fm(membersFile.c_str());
    for (size_t i = 0; i < members.size(); i++)
        fm << members[i].id << '|' << members[i].name << '|' << members[i].course << '\n';

    ofstream fl(loansFile.c_str());
    for (size_t i = 0; i < loans.size(); i++) {
        const Loan& l = loans[i];
        fl << l.id << '|' << l.bookId << '|' << l.bookTitle << '|'
           << l.studentId << '|' << l.studentName << '|'
           << formatDate(l.borrowDate) << '|' << formatDate(l.dueDate) << '|'
           << (l.returned ? formatDate(l.returnDate) : string("-")) << '|'
           << (l.returned ? "1" : "0") << '\n';
    }
}
