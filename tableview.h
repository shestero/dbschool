//
// Created by shestero on 8/7/25.
//

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class CSVTableModel;

class TableView : public QTableView {
Q_OBJECT

public:
    explicit TableView(QWidget *parent, CSVTableModel *model);
    ~TableView() override;

    void showContextMenu(const QPoint &pos);
    void copySelected();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};


#endif //TABLEVIEW_H
