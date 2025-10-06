//
// Created by shestero on 8/7/25.
//

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include "CustomComboBoxSortFilterProxyModel.h"

class CSVTableModel;

class TableView : public QTableView {
Q_OBJECT

public:
    explicit TableView(QWidget *parent, CSVTableModel *model);
    ~TableView() override;

    void showContextMenu(const QPoint &pos);
    void copySelected();

public slots:
    void onAddRow();
    void onDelRow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    int selectedRow();
    QStandardItemModel* internalModel;
    QSortFilterProxyModel *proxyModel;
};


#endif //TABLEVIEW_H
