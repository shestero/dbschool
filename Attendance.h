//
// Created by shestero on 9/8/25.
//

#ifndef DBSCHOOL1_ATTENDANCE_H
#define DBSCHOOL1_ATTENDANCE_H
#include "LastAttendanceDate.h"


class Attendance
{
public:
    QString id;
    bool open;
    int th_id;
    QString th_name;
    int ss_id;
    QString ss_name;
    QDate date_min;
    QDate date_max;
    QDate date_filled; // Use QDate() (invalid date) as None
    QMap<int, QPair<QString, QVector<QString>>> students;

    Attendance(const QString& file_name);
    bool serialize(const QString& dir) const;
};


#endif //DBSCHOOL1_ATTENDANCE_H