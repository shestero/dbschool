//
// Created by shestero on 8/7/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TableView.h" resolved

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>
//#include <qt5/QtCore/qnamespace.h>

#include "ReadOnlyDelegate.h"
#include "csvtablemodel.h"
#include "tableview.h"

#include "CustomComboBoxSortFilterProxyModel.h"

TableView::TableView(QWidget *parent, CSVTableModel *model): QTableView(parent) {
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableView::customContextMenuRequested, this, &TableView::showContextMenu);

    //ReadOnlyDelegate *readOnlyDelegate = new ReadOnlyDelegate(this);
    //setItemDelegateForColumn(0, readOnlyDelegate);

    QSortFilterProxyModel *proxyModel = new CustomComboBoxSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    setModel(proxyModel);
    if (model->columnCount() >= 2)
        sortByColumn(1, Qt::AscendingOrder);
    setSortingEnabled(true);
}

TableView::~TableView() {
}

void TableView::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("Context menu"), this);

    QAction *copyAction = contextMenu.addAction(tr("&Copy"));

    // Connect actions to their respective slots/methods
    connect(copyAction, &QAction::triggered, this, &TableView::copySelected);

    // Display the menu at the global position of the right-click
    contextMenu.exec(mapToGlobal(pos));
}

void TableView::keyPressEvent(QKeyEvent *event) {
    QModelIndexList selectedRows = selectionModel()->selectedRows();
    // at least one entire row selected
    /*
    if(!selectedRows.isEmpty()){
        if(event->key() == Qt::Key_Insert)
            model()->insertRows(selectedRows.at(0).row(),
                                selectedRows.size());
        else if(event->key() == Qt::Key_Delete)
            model()->removeRows(selectedRows.at(0).row(),
                                selectedRows.size());
    }
    */
    // at least one cell selected
    if(!selectedIndexes().isEmpty()){
        if(event->matches(QKeySequence::Copy)){
                copySelected();
        }
        /*
        else if(event->key() == Qt::Key_Delete){
            foreach (QModelIndex index, selectedIndexes())
                model()->setData(index, QString());
        }
        else if(event->matches(QKeySequence::Paste)) {
                 QString text = QApplication::clipboard()->text();
                 QStringList rowContents = text.split("\n", QString::SkipEmptyParts);

                 QModelIndex initIndex = selectedIndexes().at(0);
                 auto initRow = initIndex.row();
                 auto initCol = initIndex.column();

                 for (auto i = 0; i < rowContents.size(); ++i) {
                     QStringList columnContents = rowContents.at(i).split("\t");
                     for (auto j = 0; j < columnContents.size(); ++j) {
                         model()->setData(model()->index(
                                              initRow + i, initCol + j), columnContents.at(j));
                     }
                 }
         }
         */
         else
            QTableView::keyPressEvent(event);
    }
}

void TableView::copySelected()
{
    QString text;
    QItemSelectionRange range = selectionModel()->selection().first();
    for (auto i = range.top(); i <= range.bottom(); ++i)
    {
        QStringList rowContents;
        for (auto j = range.left(); j <= range.right(); ++j)
            rowContents << model()->index(i,j).data().toString();
        text += rowContents.join("\t");
        text += "\n";
    }
    QApplication::clipboard()->setText(text);
}
