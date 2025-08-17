//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "csvtablemodel.h"
#include "tableview.h"
#include "ForeignKeyDelegate.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {
    setWindowTitle(tr("School accounting"));

    QTabWidget* pTabs = new QTabWidget(this);

    CSVTableModel* pClassesModel = new CSVTableModel(this, "classes.tsv");
    TableView* pClasses = new TableView(this, pClassesModel);
    pClasses->setColumnWidth(1, 300);
    pTabs->addTab(pClasses, tr("Classes"));

    CSVTableModel* pStudentsModel = new CSVTableModel(this, "students.tsv", 2, pClassesModel);
    TableView* pStudents = new TableView(this, pStudentsModel);
    pStudents->setItemDelegateForColumn(2, new ForeignKeyDelegate(pClassesModel, this));
    pStudents->setColumnWidth(1, 400);
    pStudents->setColumnWidth(2, 240);
    pTabs->addTab(pStudents, tr("Students"));

    CSVTableModel* pTeacherModel = new CSVTableModel(this, "teachers.tsv");
    TableView* pTeachers = new TableView(this, pTeacherModel);
    pTeachers->setColumnWidth(1, 400);
    pTabs->addTab(pTeachers, tr("Teachers"));

    CSVTableModel* pSectionsModel = new CSVTableModel(this, "sections.tsv", 3, pTeacherModel);
    TableView* pSections = new TableView(this, pSectionsModel);
    pSections->setItemDelegateForColumn(3, new ForeignKeyDelegate(pTeacherModel, this));
    pSections->setColumnWidth(1, 400);
    pSections->setColumnWidth(3, 360);
    pTabs->addTab(pSections, tr("Sections"));

    setCentralWidget(pTabs);
}

MainWindow::~MainWindow() {
}
