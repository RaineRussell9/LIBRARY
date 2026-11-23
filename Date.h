// Date.h - a simple calendar date and helper functions
#ifndef DATE_H
#define DATE_H

#include <string>

struct Date {
    int year = 0, month = 0, day = 0;
};

bool isLeap(int y);
int daysInMonth(int y, int m);
bool isValidDate(const Date& d);               // year must be 1900-2100
Date addDays(Date d, int n);                   // d + n days
long toSerial(const Date& d);                  // day number, so dates can be subtracted
long daysBetween(const Date& later, const Date& earlier);  // negative if "later" is earlier
std::string formatDate(const Date& d);         // "YYYY-MM-DD"
Date parseDate(const std::string& s);          // reads "YYYY-MM-DD"

#endif
