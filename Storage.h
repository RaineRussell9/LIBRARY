// Storage.h - saves and loads the data in text files (fields separated by '|')
#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include "Book.h"
#include "Member.h"
#include "Loan.h"

class Storage {
public:
    Storage(const std::string& booksFile, const std::string& membersFile,
            const std::string& loansFile);

    void load(std::vector<Book>& books, std::vector<Member>& members,
              std::vector<Loan>& loans) const;
    void save(const std::vector<Book>& books, const std::vector<Member>& members,
              const std::vector<Loan>& loans) const;

private:
    std::string booksFile, membersFile, loansFile;
};

#endif
