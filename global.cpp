//
// Created by shestero on 10/5/25.
//

#include "global.h"

#include <QFile>
#include <QDebug>

void rename_to_bak(const QString& fileName)
{
    if (QFile::exists(fileName))
    {
        QString bakName = fileName + ".bak";
        QFile::remove(bakName); // удаляем, если есть
        if (!QFile::rename(fileName, bakName)) {
            qWarning() << "Не удалось переименовать" << fileName << "в" << bakName;
            if (!QFile::remove(fileName))
            {
                qWarning() << "Не удалось удалить" << fileName;
            }
        }
    }
}
