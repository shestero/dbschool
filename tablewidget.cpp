//
// Created by shestero on 9/28/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TableWidget.h" resolved

#include "tablewidget.h"

#include <QApplication>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "tableview.h"


TableWidget::TableWidget(QWidget* parent, CSVTableModel *model) :
    QWidget(parent),
    tableView(new TableView(this, model))
{
    QPushButton *addBtn = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Add row");
    QPushButton *delBtn = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), "Delete");
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 4, 0, 4);
    buttonLayout->setSpacing(6);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(delBtn);
    buttonLayout->addStretch();

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(tableView);
    vLayout->addLayout(buttonLayout);
    setContentsMargins(0, 0, 0, 0);
}

TableWidget::~TableWidget()
{
}