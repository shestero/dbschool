//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "csvtablemodel.h"
#include "tableview.h"
#include "ForeignKeyDelegate.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {
    setWindowTitle(tr("School accounting"));

    auto pTabs = new QTabWidget(this);

    auto pClassesModel = new CSVTableModel(this, "classes.tsv");
    TableView* pClasses = new TableView(this, pClassesModel);
    pClasses->setColumnWidth(1, 300);
    pTabs->addTab(pClasses, tr("Classes"));

    auto pStudentsModel = new CSVTableModel(this, "students.tsv", 2, pClassesModel);
    auto pStudents = new TableView(this, pStudentsModel);
    pStudents->setItemDelegateForColumn(2, new ForeignKeyDelegate(pClassesModel, this));
    pStudents->setColumnWidth(1, 400);
    pStudents->setColumnWidth(2, 240);
    pTabs->addTab(pStudents, tr("Students"));

    auto pTeacherModel = new CSVTableModel(this, "teachers.tsv");
    TableView* pTeachers = new TableView(this, pTeacherModel);
    pTeachers->setColumnWidth(1, 400);
    pTabs->addTab(pTeachers, tr("Teachers"));

    auto pSectionsModel = new CSVTableModel(this, "sections.tsv", 3, pTeacherModel);
    TableView* pSections = new TableView(this, pSectionsModel);
    pSections->setItemDelegateForColumn(3, new ForeignKeyDelegate(pTeacherModel, this));
    pSections->setColumnWidth(1, 400);
    pSections->setColumnWidth(3, 360);
    pTabs->addTab(pSections, tr("Sections"));

    // Tool bar
    auto toolBar = addToolBar(tr("toolbar")); // Adds a toolbar to the main window
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    QAction* newAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Generate tables"), this);
    QAction* sendAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowForward), tr("Send tables"), this);
    QAction* receiveAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowBack), tr("Receive tables"), this);

    toolBar->addAction(newAction);
    toolBar->addAction(sendAction);
    toolBar->addAction(receiveAction);

    // You can also add QToolButton directly if preferred
    auto updateStudentsButton = new QToolButton(this);
    updateStudentsButton->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)));
    updateStudentsButton->setText(tr("Renew students"));
    updateStudentsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addWidget(updateStudentsButton);

    auto centralWidget = new QWidget();
    auto verticalLayout = new QVBoxLayout(centralWidget);
    verticalLayout->addWidget(toolBar);
    verticalLayout->addWidget(pTabs);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
}
