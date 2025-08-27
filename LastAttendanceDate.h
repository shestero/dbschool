//
// Created by shestero on 8/27/25.
//

#ifndef DBSCHOOL1_LASTATTENDANCEDATE_H
#define DBSCHOOL1_LASTATTENDANCEDATE_H
#include "CustomComboBoxSortFilterProxyModel.h"

#include <QDate>

class LastAttendanceDate
{
public:
    LastAttendanceDate(const char *file_name) : file_name(file_name) {}

    const QDate get();
    void set(const QDate& date);

private:
    const char* file_name;
};


#endif //DBSCHOOL1_LASTATTENDANCEDATE_H