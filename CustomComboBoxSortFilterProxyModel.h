//
// Created by shestero on 8/9/25.
//

#ifndef CUSTOMCOMBOBOXSORTFILTERPROXYMODEL_H
#define CUSTOMCOMBOBOXSORTFILTERPROXYMODEL_H

#include <QModelIndex>
#include <QSortFilterProxyModel>

class CustomComboBoxSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit CustomComboBoxSortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent) {}

    static bool lessThan(QString l, QString r);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif //CUSTOMCOMBOBOXSORTFILTERPROXYMODEL_H
