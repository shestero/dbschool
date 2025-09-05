//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "Configuration.h"
#include "LastAttendanceDate.h"
#include "NetworkInteraction.h"
#include "csvtablemodel.h"
#include "generateinvoices.h"
#include "generatetables.h"
#include "tableview.h"
#include "ForeignKeyDelegate.h"
#include "ProtocolDialog.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QThread>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcher>

#include <QDebug>

#include "ui_generatetables.h"

#define LASTDATE_FILENAME   "last-date.txt"
static LastAttendanceDate gl_lastAttendanceDate = LastAttendanceDate(LASTDATE_FILENAME);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), network(new NetworkInteraction(this))
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
    sendAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowForward), tr("Send tables"), this);
    QAction* receiveAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_ArrowBack), tr("Receive tables"), this);
    QAction* refreshStudentAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), tr("Renew students"), this);
    QAction* issueInvoicesAction =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon), tr("Issue invoices"), this);

    toolBar->addAction(createAction);
    toolBar->addAction(sendAction);
    toolBar->addAction(receiveAction);
    toolBar->addAction(refreshStudentAction);
    toolBar->addAction(issueInvoicesAction);
    toolBar->insertSeparator(issueInvoicesAction);

    connect(createAction, &QAction::triggered, this, &MainWindow::onCreateAttendanceTables);
    connect(sendAction, &QAction::triggered, this, &MainWindow::onSendAttendanceTables);
    connect(receiveAction, &QAction::triggered, this, &MainWindow::onReceiveAttendanceTables);
    connect(refreshStudentAction, &QAction::triggered, this, &MainWindow::onRefreshStudentTable);
    connect(issueInvoicesAction, &QAction::triggered, this, &MainWindow::onIssueInvoices);

    auto centralWidget = new QWidget();
    auto verticalLayout = new QVBoxLayout(centralWidget);
    verticalLayout->addWidget(toolBar);
    verticalLayout->addWidget(pTabs);
    setCentralWidget(centralWidget);

    // check timer
    QTimer *checkTimer = new QTimer(this);
    checkTimer->setInterval(1000);
    connect(checkTimer, &QTimer::timeout, this, &MainWindow::onRegularChecks);
    checkTimer->start();
}

MainWindow::~MainWindow() = default;

void MainWindow::onCreateAttendanceTables()
{
    QDate date = gl_lastAttendanceDate.get();
    date = date.addDays(1);
    gl_lastAttendanceDate.set(date);

    GenerateTables* dialog = new GenerateTables(date, this);

    int result = dialog->exec(); // Show the dialog modally

    if (result == QDialog::Accepted) {
        // Dialog was accepted (e.g., OK button clicked)
        qDebug() << "Custom dialog accepted!";

    } else {
        // Dialog was rejected (e.g., Cancel button clicked or closed)
        qDebug() << "Custom dialog rejected!";
    }

    dialog->deleteLater();
}

// todo
template<typename T>
inline QFuture<T> makeReadyFuture(const T &value) {
    QFutureInterface<T> iface;
    iface.reportStarted();
    iface.reportResult(value);
    iface.reportFinished();
    return iface.future();
}

void MainWindow::onSendAttendanceTables()
{
    // Чтение списка таблиц на отправку
    const QStringList files =
        QDir("attendance/outbox").entryList(QStringList() << "*.tsv", QDir::Files);

    if (files.isEmpty())
    {
        QMessageBox::information(
            this,
            tr("Sending attendance tables"),
            tr("No tables ready to send. Please, create tables first!"));
        return;
    }

    network->sendTables(files);
}

void MainWindow::onReceiveAttendanceTables()
{
    QMessageBox::critical(
        this,
        tr("Receiving attendance tables"),
        tr("Under construction"));
}

QString MainWindow::calculateSha256Hash(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(
            this,
            tr("Calculating hash"),
            tr("Could not open file: %1").arg(filePath)
        );
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return hash.result().toHex();
    } else {
        QMessageBox::warning(
            this,
            tr("Calculating hash"),
            tr("Failed to add data to hash calculation for file: %1").arg(filePath)
        );
        return QString();
    }
}

void MainWindow::onRefreshStudentTable()
{
    const QString hash_local = calculateSha256Hash("students.tsv");
    if (hash_local.isEmpty())
        return;

    const QString hash_remote = network->getStudentsHash();
    qDebug() << "hash_local" << hash_local << "hash_remote=" << hash_remote;
    if (hash_remote.isEmpty())
    {
        QMessageBox::critical(
            this,
            tr("Renew students table"),
            tr("Cannot get information from the server %1").arg(Configuration().teach_server.c_str())
        );
        return;
    }
    if (hash_local == hash_remote)
    {
        QMessageBox::information(
            this,
            tr("Renew students table"),
            tr("Students table at the server is up to date")
        );
        return;
    }

    QMessageBox::critical(
        this,
        tr("Renew students table"),
        tr("Under construction"));
}

void MainWindow::onIssueInvoices()
{
    GenerateInvoices* dialog = new GenerateInvoices(this);

    int result = dialog->exec(); // Show the dialog modally

    if (result == QDialog::Accepted) {
        // Dialog was accepted (e.g., OK button clicked)
        qDebug() << "Custom dialog (issue invoices) accepted!";

    } else {
        // Dialog was rejected (e.g., Cancel button clicked or closed)
        qDebug() << "Custom dialog (issue invoices) rejected!";
    }

    dialog->deleteLater();

}

void MainWindow::onRegularChecks() {
    sendAction->setEnabled(
        !QDir("attendance/outbox").entryList(QStringList() << "*.tsv", QDir::Files).isEmpty()
    );
}