//
// Created by shestero on 8/9/25.
//

#include "CustomComboBoxSortFilterProxyModel.h"

#include <QDebug>

bool CustomComboBoxSortFilterProxyModel::lessThan(QString l, QString r)
{

    bool okl = false, okr = false;
    int li = l.toInt(&okl);
    int lr = r.toInt(&okr);
    if (okl && okr)
    {
        return li < lr;
    }

    if (l.length() == 1 && "0" <= l && l <= "9") l.prepend("0");
    if (r.length() == 1 && "0" <= r && r <= "9") r.prepend("0");
    if (l.length() == 2 && l.at(0).isDigit() && !l.at(1).isDigit()) l.prepend("0");
    if (r.length() == 2 && r.at(0).isDigit() && !r.at(1).isDigit()) r.prepend("0");
    if (l.length() == 3 && l.at(0).isDigit() && l.at(1).isDigit() && !l.at(2).isDigit()) l.prepend("0");
    if (r.length() == 3 && r.at(0).isDigit() && r.at(1).isDigit() && !r.at(2).isDigit()) r.prepend("0");

    return l < r;
}

bool CustomComboBoxSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Retrieve data from the source model
    QString l = sourceModel()->data(left, Qt::DisplayRole).toString();
    QString r = sourceModel()->data(right, Qt::DisplayRole).toString();

    return lessThan(l, r);
}
