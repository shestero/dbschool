#include "csvtablemodel.h"
//
// Created by shestero on 9/28/25.
//

#ifndef DBSCHOOL1_TABLEWIDGET_H
#define DBSCHOOL1_TABLEWIDGET_H

#include <QWidget>


class TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableWidget(QWidget* parent, CSVTableModel *model);
    ~TableWidget() override;

    TableView* operator->() const {
        return tableView;
    }

private:
    TableView* tableView;
};


#endif //DBSCHOOL1_TABLEWIDGET_H