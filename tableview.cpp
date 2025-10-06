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

#include <QMessageBox>


TableView::TableView(QWidget *parent, CSVTableModel *model)
: QTableView(parent), internalModel(model) {
    proxyModel = new CustomComboBoxSortFilterProxyModel(this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTableView::customContextMenuRequested, this, &TableView::showContextMenu);

    //ReadOnlyDelegate *readOnlyDelegate = new ReadOnlyDelegate(this);
    //setItemDelegateForColumn(0, readOnlyDelegate);

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

int TableView::selectedRow()
{
    // 1. Get the current index from the view's selection model.
    QModelIndex proxyIndex = selectionModel()->currentIndex();
    qDebug() << "proxyIndex=" << proxyIndex;

    if (!proxyIndex.isValid()) {
        qDebug() << "No item is currently selected in the view.";
        return -2;
    }

    // 2. Map the proxy index to the source index.
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    qDebug() << "sourceIndex=" << sourceIndex;
    if (!proxyIndex.isValid()) {
        qDebug() << "No item is currently selected in the view.";
        return -3;
    }

    qDebug() << "Map selected row: " << selectionModel()->currentIndex().row() << "into" << sourceIndex.row();
    return sourceIndex.row();
}

void TableView::onAddRow()
{
    int max = 0;
    for (int i = 0; i < internalModel->rowCount(); i++)
    {
        int id = internalModel->index(i,0).data().toInt();
        if (id > max)
            max = id;
    }
    // int current = selectedRow();
    int current = internalModel->rowCount();

    if (internalModel->insertRow(current))
    {
        qDebug() << "the row was inserted at" << current;
        internalModel->setItem(current, 0, new QStandardItem(QString::number(max + 1)));

        QModelIndex index = internalModel->index(current, 0);
        if (index.isValid()) {
            index = proxyModel->mapFromSource(index);
            if (index.isValid())
                scrollTo(index);
        }
    }
}

void TableView::onDelRow()
{
    int current = selectedRow();
    if (current < 0)
    {
        QMessageBox::critical(this,
            tr("Cannot delete row"),
            tr("Please select row to delete!")
        );
        return;
    }

    QModelIndex index0 = internalModel->index(current, 0, QModelIndex());
    QModelIndex index1 = internalModel->index(current, 1, QModelIndex());
    qDebug() << "index to delete:" << index0;
    if (index0.isValid())
    {
        auto index = proxyModel->mapFromSource(index0);
        if (index.isValid())
            scrollTo(index);
    }

    QString k = internalModel->data(index0).toString();
    QString n = internalModel->data(index1).toString();

    int result = QMessageBox::question(
        this,
        tr("Confirm delete the row"),
        tr("The row starts %1 %2 ...").arg(k).arg(n)
    );
    if (result != QMessageBox::Yes)
    {
        return;
    }

    qDebug() << "going to remove row" << current;
    internalModel->removeRow(current);
}
