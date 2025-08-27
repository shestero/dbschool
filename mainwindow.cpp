//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "LastAttendanceDate.h"
#include "NetworkInteraction.h"
#include "csvtablemodel.h"
#include "tableview.h"
#include "ForeignKeyDelegate.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDebug>

#define LASTDATE_FILENAME   "last-date.txt"
static LastAttendanceDate gl_lastAttendanceDate = LastAttendanceDate(LASTDATE_FILENAME);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), network_interaction(new NetworkInteraction(this))
{
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
    QAction* createAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Generate tables"), this);
    QAction* sendAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowForward), tr("Send tables"), this);
    QAction* receiveAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowBack), tr("Receive tables"), this);
    QAction* refreshStudentAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), tr("Renew students"), this);

    toolBar->addAction(createAction);
    toolBar->addAction(sendAction);
    toolBar->addAction(receiveAction);
    toolBar->addAction(refreshStudentAction);

    connect(createAction, &QAction::triggered, this, &MainWindow::onCreateAttendanceTables);
    connect(sendAction, &QAction::triggered, this, &MainWindow::onSendAttendanceTables);
    connect(receiveAction, &QAction::triggered, this, &MainWindow::onReceiveAttendanceTables);
    connect(refreshStudentAction, &QAction::triggered, this, &MainWindow::onRefreshStudentTable);

    // You can also add QToolButton directly if preferred
    /*
    auto updateStudentsButton = new QToolButton(this);
    updateStudentsButton->setIcon(QIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload)));
    updateStudentsButton->setText(tr("Renew students"));
    updateStudentsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addWidget(updateStudentsButton);
    */

    auto centralWidget = new QWidget();
    auto verticalLayout = new QVBoxLayout(centralWidget);
    verticalLayout->addWidget(toolBar);
    verticalLayout->addWidget(pTabs);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
}

void MainWindow::onCreateAttendanceTables()
{
    QDate date = gl_lastAttendanceDate.get();
    date = date.addDays(1);
    gl_lastAttendanceDate.set(date);
}

void MainWindow::onSendAttendanceTables()
{
}

void MainWindow::onReceiveAttendanceTables()
{
}

void MainWindow::onRefreshStudentTable()
{
    auto hash = network_interaction->getStudentsHash();
    qDebug() << QString("hash=") << hash;
}