//
// Created by shestero on 8/7/25.
//

#ifndef READONLYDELEGATE_H
#define READONLYDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

class ReadOnlyDelegate : public QStyledItemDelegate
{
    Q_OBJECT // Required for moc
public:
    explicit ReadOnlyDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(parent);
        Q_UNUSED(option);
        Q_UNUSED(index);
        return nullptr; // No editor is created, making the column read-only
    }
};

#endif //READONLYDELEGATE_H
