//
// Created by shestero on 9/8/25.
//

#include "global.h"
#include "Attendance.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <QDebug>

Attendance::Attendance(const QString& filePath)
{
    QFile file(filePath);

    // Попытка открыть файл в режиме чтения
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл:" << file.errorString();
    }

    // Создание потока для чтения текста
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // Чтение файла построчно
    auto acc = QMap<QString, QString>();
    while (!in.atEnd()) {
        QString line = in.readLine();

        // Разбиение с сохранением пустых полей (по умолчанию)
        QStringList withEmpty = line.split('\t');
        if (withEmpty.size() < 2)
            continue;

        acc.insert(withEmpty.at(0), line.section("\t", 1));
        // todo: save counters into separate collection
    }
    qDebug() << "attendance.acc" << acc;

    // Заполнение полей структуры
    id = QFileInfo(filePath).fileName();
    open = filePath.contains("/inbox/");
    th_id = acc["th_id"].toInt();
    th_name = acc["th_name"];
    ss_id = acc["ss_id"].toInt();
    ss_name = acc["ss_name"];
    date_min = QDate::fromString(acc["date_min"], "yyyy-MM-dd");
    date_max = QDate::fromString(acc["date_max"], "yyyy-MM-dd");
    date_filled = acc["date_filled"].isEmpty()? QDate(): QDate::fromString(acc["date_filled"], "yyyy-MM-dd");

    for (const QString& s : acc.keys())
    {
        int st_id = s.toInt();
        if (st_id <= 0)
            continue;

        QStringList row = acc[s].split('\t');
        students.insert(st_id, row.toVector());
    }

    // Закрытие файла
    file.close();
}

bool Attendance::serialize(const QString& dir) const
{
    const QString fileName = dir + "/" + id;
    rename_to_bak(fileName);

    QFile file(fileName);
    // Открываем файл на запись (QIODevice::WriteOnly)
    // и стираем старое содержимое (QIODevice::Truncate)
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qCritical() << "Не удалось открыть файл" << fileName << "для записи";
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");   // чтобы точно писать в UTF-8
    out << "th_id\t" << th_id << "\n";
    out << "th_name\t" << th_name << "\n";
    out << "ss_id\t" << ss_id << "\n";
    out << "ss_name\t" << ss_name << "\n";
    out << "date_min\t" << date_min.toString("yyyy-MM-dd") << "\n";
    out << "date_max\t" << date_max.toString("yyyy-MM-dd") << "\n";
    if (date_filled.isValid())
    {
        out << "date_filled\t" << date_filled.toString("yyyy-MM-dd") << "\n";
    }

    for (auto it = students.constBegin(); it != students.constEnd(); ++it) {
        out << it.key() << "\t" << it.value().toList().join("\t") << "\n";
    }

    file.close();
    return true;
}
