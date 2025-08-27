//
// Created by shestero on 8/27/25.
//

#include "LastAttendanceDate.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDebug>

#include <iostream>


#define LASTDATE_FORMAT     Qt::ISODate
#define LASTDATE_DEFAULT    QDate(2020, 9, 1)
/*
#define LASTDATE_DEFAULT    std::chrono::year_month_day(std::chrono::year(2020), \
                                         std::chrono::month(4), \
                                         std::chrono::day(23))
*/

const QDate LastAttendanceDate::get()
{
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr <<
            "Can't open file " << file_name <<
            "; going to use default date " <<
                LASTDATE_DEFAULT.toString(LASTDATE_FORMAT).toStdString() <<
                    "!" << std::endl;
        return LASTDATE_DEFAULT;
    }

    const QString line = QTextStream(&file).readLine();
    const QDate date = QDate::fromString(line, LASTDATE_FORMAT);
    if (!date.isValid())
    {
        std::cerr <<
            "Can't parse line " << line.toStdString() <<
            "; going to use default date " <<
                LASTDATE_DEFAULT.toString(LASTDATE_FORMAT).toStdString() <<
                    "!" << std::endl;
        return LASTDATE_DEFAULT;
    }

    return date;
}

void LastAttendanceDate::set(const QDate& date)
{
    const QString line = date.toString(LASTDATE_FORMAT);
    qDebug() << "LastAttendanceDate::set: " << line;

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr <<
            "Can't open file " << file_name <<
            " to save date " << line.toStdString() << "!" << std::endl;
        return;
    }

    QTextStream(&file) << line;
}
