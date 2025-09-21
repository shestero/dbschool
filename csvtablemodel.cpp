//
// Created by shestero on 8/6/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CSVTableModel.h" resolved

#include "csvtablemodel.h"
#include "ForeignKeyDelegate.h"
#include "tableview.h"

#include <QColor>
#include <QFile>
#include <QTextStream>
#include <QDebug>

CSVTableModel::CSVTableModel(QObject *parent, const QString& file_name):
    QStandardItemModel(parent), file_name(file_name)
{
    load();
}

CSVTableModel::CSVTableModel(QObject *parent, const QString& file_name, int fkColumn, CSVTableModel *pFkModel):
    QStandardItemModel(parent), file_name(file_name)
{
    QMap<int, QString> map;
    for (int r = 0; r < pFkModel->rowCount(); r++)
    {
        bool ok = false;
        int index = pFkModel->item(r)->text().toInt(&ok);
        if (ok)
        {
            map.insert(index, pFkModel->item(r, 1)->text());
        }
    }
    load({ { fkColumn, map } });

    //parent->setItemDelegateForColumn(fkColumn, new ForeignKeyDelegate(pFkModel, this));
}

CSVTableModel::~CSVTableModel() {
    save();
}

Qt::ItemFlags CSVTableModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags flag = QStandardItemModel::flags(index);
    if (
        index.isValid() &&
        horizontalHeaderItem(index.column()) != nullptr &&
        horizontalHeaderItem(index.column())->text() == "id"
        )
    {
        flag &= ~Qt::ItemIsEnabled;
    }
    return flag;
}

void CSVTableModel::load(QMap<int, QMap<int, QString>> foreign)
{
    QFile file(file_name);
    if (file.open(QIODevice::ReadOnly)) {

        int lineindex = 0;                     // file line counter
        QTextStream in(&file);                 // read to text stream
        in.setCodec("UTF-8");

        while (!in.atEnd()) {
            // read one line from textstream(separated by "\n")
            QString lineString = in.readLine();

            // parse the read line into separate pieces(tokens) with ";" as the delimiter
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            const auto keep_empty_parts = Qt::KeepEmptyParts;
#else
            const auto keep_empty_parts = QString::KeepEmptyParts;
#endif

            QStringList lineList = lineString.split("\t", keep_empty_parts);

            if (lineindex == 0)
            {
                setHorizontalHeaderLabels(lineList);
            } else {
                // load parsed data to model accordingly
                for (int j = 0; j < lineList.size(); j++)
                {
                    QString value = lineList.at(j);

                    auto it = foreign.find(j);
                    QStandardItem *item;
                    if (it != foreign.end())
                    {
                        int index = value.toInt();
                        auto it1 = it->find(index);
                        if (it1 == it->end())
                        {
                            qDebug() << "Warning: key " << value << " not found!";
                            item = new QStandardItem(value);
                            item->setData(QColor(Qt::red), Qt::ForegroundRole);

                        } else
                        {
                            //qDebug() << "info: key=" << *it1 << " index=" << index;
                            item = new QStandardItem(*it1);
                            item->setData(index, Qt::UserRole);
                        }
                        //item->setFlags( item->flags() &~ Qt::ItemIsEditable );
                    } else {
                        item = new QStandardItem(value);
                    }
                    setItem(lineindex-1, j, item);
                }
            }

            lineindex++;
        }

        file.close();
    }
}

void CSVTableModel::save()
{
    QFile file(QString("new-") + file_name);

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");

        for (int row = -1; row < rowCount(); row++)
        {
            QStringList line;
            for (int column = 0; column < columnCount(); column++)
            {
                if (row == -1)
                {
                    line << horizontalHeaderItem(column)->text();
                } else {
                    QModelIndex index = this->index(row, column);
                    line << data(index).toString();
                }
            }

            out << line.join("\t") << "\n"; // Qt::endl;
        }

        file.close();
    }
}

QMap<int, QString> CSVTableModel::dictionary(int titleCol) const
{
    QMap<int, QString> ret;
    for (int i = 0; i < rowCount(); i++)
    {
        auto primary = item(i, 0);
        if (!primary)
        {
            qWarning() << "Warning: no item for the primary column of row" << i <<
                "at file" << file_name;
            continue;
        }
        bool ok = false;
        int key = primary->text().toInt(&ok);
        if (!ok)
        {
            qWarning() << "Warning: cannot parse key from the primary column of row" << i <<
                "at file" << file_name;
            qDebug() << "It's value" << primary->text();
            continue;
        }
        auto secondary = item(i, titleCol);
        if (!secondary)
        {
            qWarning() << "Warning: no item for the secondary column (" << titleCol << ") of row" << i <<
                "at file" << file_name;
            continue;
        }
        ret.insert(key, secondary->text()); // ->data().toString());
    }
    return ret;
}

QMap<int, int> CSVTableModel::codeDictionary(int titleCol) const
{
    QMap<int, int> ret;
    for (int i = 0; i < rowCount(); i++)
    {
        auto primary = item(i, 0);
        if (!primary)
        {
            qWarning() << "Warning: no item for the primary column of row" << i <<
                "at file" << file_name;
            continue;
        }

        bool ok = false;
        int key = primary->text().toInt(&ok);
        if (!ok)
        {
            qWarning() << "Warning: cannot parse key from the primary column of row" << i <<
                "at file" << file_name;
            qDebug() << "It's value" << primary->text();
            continue;
        }
        auto secondary = item(i, titleCol);
        if (!secondary)
        {
            qWarning() << "Warning: no item for the secondary column (" << titleCol << ") of row" << i <<
                "at file" << file_name;
            continue;
        }

        int idx = secondary->data(Qt::UserRole).toInt(&ok);
        if (ok)
            ret.insert(key, idx);
    }
    return ret;
}

/*
int CSVTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.row();
}

int CSVTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.column();
}
*/
