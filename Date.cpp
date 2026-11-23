// Date.cpp - implementation of the date helpers
#include "Date.h"
#include <cstdio>
using namespace std;

bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeap(y)) return 29;
    return days[m - 1];
}

bool isValidDate(const Date& d) {
    if (d.year < 1900 || d.year > 2100) return false;
    if (d.month < 1 || d.month > 12) return false;
    return d.day >= 1 && d.day <= daysInMonth(d.year, d.month);
}

Date addDays(Date d, int n) {
    for (int i = 0; i < n; i++) {
        d.day++;
        if (d.day > daysInMonth(d.year, d.month)) {
            d.day = 1;
            d.month++;
            if (d.month > 12) { d.month = 1; d.year++; }
        }
    }
    return d;
}

// Counts days since 1970-01-01 (valid for years >= 1900), so two dates can be subtracted.
long toSerial(const Date& d) {
    int y = d.year, m = d.month;
    if (m <= 2) y--;
    long era = y / 400;
    long yoe = y - era * 400;
    long doy = (153L * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d.day - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

long daysBetween(const Date& later, const Date& earlier) {
    return toSerial(later) - toSerial(earlier);
}

string formatDate(const Date& d) {
    string mm = (d.month < 10 ? "0" : "") + to_string(d.month);
    string dd = (d.day < 10 ? "0" : "") + to_string(d.day);
    return to_string(d.year) + "-" + mm + "-" + dd;
}

Date parseDate(const string& s) {
    Date d;
    int y = 0, m = 0, dd = 0;
    if (sscanf(s.c_str(), "%d-%d-%d", &y, &m, &dd) == 3) {
        d.year = y; d.month = m; d.day = dd;
    }
    return d;
}
