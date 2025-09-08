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
#include <QSplitter>
#include <QStatusBar>
#include <QThread>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcher>

#include <QDebug>
#include <qt5/QtWidgets/qmainwindow.h>

#include "ui_generatetables.h"

#define LASTDATE_FILENAME   "last-date.txt"
static LastAttendanceDate gl_lastAttendanceDate = LastAttendanceDate(LASTDATE_FILENAME);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), network(new NetworkInteraction(this))
{
    setWindowTitle(tr("School accounting"));

    // Tabs
    auto tabs = new QTabWidget(this);

    auto pClassesModel = new CSVTableModel(this, "classes.tsv");
    TableView* pClasses = new TableView(this, pClassesModel);
    pClasses->setColumnWidth(1, 300);
    tabs->addTab(pClasses, tr("Classes"));

    auto pStudentsModel = new CSVTableModel(this, "students.tsv", 2, pClassesModel);
    auto pStudents = new TableView(this, pStudentsModel);
    pStudents->setItemDelegateForColumn(2, new ForeignKeyDelegate(pClassesModel, this));
    pStudents->setColumnWidth(1, 400);
    pStudents->setColumnWidth(2, 240);
    tabs->addTab(pStudents, tr("Students"));

    auto pTeacherModel = new CSVTableModel(this, "teachers.tsv");
    TableView* pTeachers = new TableView(this, pTeacherModel);
    pTeachers->setColumnWidth(1, 400);
    tabs->addTab(pTeachers, tr("Teachers"));

    auto pSectionsModel = new CSVTableModel(this, "sections.tsv", 3, pTeacherModel);
    TableView* pSections = new TableView(this, pSectionsModel);
    pSections->setItemDelegateForColumn(3, new ForeignKeyDelegate(pTeacherModel, this));
    pSections->setColumnWidth(1, 400);
    pSections->setColumnWidth(3, 360);
    tabs->addTab(pSections, tr("Sections"));

    // Report's calendar
    reportCalendarWidget = new QCalendarWidget;
    reportCalendarWidget->setMaximumSize(220, 30);
    reportCalendarWidget->setLocale(QLocale(QLocale::Russian, QLocale::Russia));
    reportCalendarWidget->setHorizontalHeaderFormat(QCalendarWidget::NoHorizontalHeader);
    reportCalendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    reportCalendarWidget->setGridVisible(false);
    reportCalendarWidget->setNavigationBarVisible(true);
    reportCalendarWidget->setDateEditEnabled(true);
    reportCalendarWidget->setStyleSheet(R"(
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background-color: white;       /* фон */
        }
        QCalendarWidget QToolButton {
            background: transparent;       /* кнопки прозрачные */
            color: black;                  /* цвет текста/стрелок */
        }
        QCalendarWidget QToolButton:hover {
            background: lightgray;         /* фон при наведении */
            color: black;                  /* текст остаётся чёрным */
        }
    )");
    if (QTableView *table = reportCalendarWidget->findChild<QTableView*>()) { // hook
        table->hide();
    }
    reportCalendarWidget->setContentsMargins(5, 0, 5, 0);

    // Actions
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
    QAction* reportForTeacher =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Teacher's report"), this);
    QAction* reportForDirector =
        new QAction(QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Director's report"), this);

    // Tool bars
    QToolBar *toolBar;
    QLabel *label;
    toolBar = addToolBar(tr("Attendance tools"));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    label = new QLabel(tr("Attendance:"));
    label->setStyleSheet("QLabel { text-decoration: underline; padding-right: 10px; }"); // 5.14+ ?
    // label->setContentsMargins(0, 0, 10, 0);
    toolBar->addWidget(label);

    toolBar->addAction(createAction);
    toolBar->addAction(sendAction);
    toolBar->addAction(receiveAction);
    toolBar->addAction(refreshStudentAction);

    addToolBarBreak();
    toolBar = addToolBar(tr("Report tools"));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    label = new QLabel(tr("Reports:"));
    label->setStyleSheet("QLabel { text-decoration: underline; padding-right: 10px; }"); // 5.14+ ?
    // label->setContentsMargins(0, 0, 10, 0);
    toolBar->addWidget(label);

    toolBar->addWidget(reportCalendarWidget);

    toolBar->addAction(reportForTeacher);
    toolBar->addAction(reportForDirector);
    toolBar->addSeparator();
    toolBar->addAction(issueInvoicesAction);
    /*
    // как выровнять влево при вертикальном режиме?
    QToolButton* btn = qobject_cast<QToolButton*>(toolBar->widgetForAction(issueInvoicesAction));
    if (btn) {
        qDebug() << "found!";
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        btn->setStyleSheet("QToolButton { text-align: left; }");
    }
    */

    connect(createAction, &QAction::triggered, this, &MainWindow::onCreateAttendanceTables);
    connect(sendAction, &QAction::triggered, this, &MainWindow::onSendAttendanceTables);
    connect(receiveAction, &QAction::triggered, this, &MainWindow::onReceiveAttendanceTables);
    connect(refreshStudentAction, &QAction::triggered, this, &MainWindow::onRefreshStudentTable);
    connect(reportForTeacher, &QAction::triggered, this, &MainWindow::onReportForTeacher);
    connect(reportForDirector, &QAction::triggered, this, &MainWindow::onReportForDirector);
    connect(issueInvoicesAction, &QAction::triggered, this, &MainWindow::onIssueInvoices);

    connect(network, &NetworkInteraction::appendLog, logger, &ProtocolWidget::appendLog);

    // Top layout
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tabs);
    splitter->addWidget(logger = new ProtocolWidget);
    splitter->setStretchFactor(1, 0); // правая часть не растягивается сама, но можно тянуть мышкой
    splitter->setSizes({800, 400});
    setCentralWidget(splitter);
    statusBar()->setWindowTitle(tr("Ready..."));

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
    logger->writeTimestamp(tr("Receiving tables"));
    network->receiveTables();
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
    const QString hash_local = calculateSha256Hash(Configuration::students_file);
    if (hash_local.isEmpty())
        return;

    const QString hash_remote = network->getStudentsHash();
    qDebug() << "hash_local" << hash_local << "hash_remote=" << hash_remote;
    if (hash_remote.isEmpty())
    {
        logger->appendLog(tr("Unknown problem..."));
        QMessageBox::critical(
            this,
            tr("Renew students table"),
            tr("Cannot get information from the server %1").arg(Configuration().teach_server.c_str())
        );
        // return;
    }
    if (hash_local == hash_remote)
    {
        logger->appendLog(tr("The file is up to date"));
        QMessageBox::information(
            this,
            tr("Renew students table"),
            tr("Students table at the server is up to date")
        );
        return;
    }

    // Проверки прошли; действительно трубуется обновление
    logger->writeTimestamp(tr("Renew %1").arg(Configuration::students_file));
    if (network->sendStudents())
    {
        logger->appendLog(tr("Success"));
    }
}

void MainWindow::onIssueInvoices()
{
    qDebug() << "date=" << reportCalendarWidget->selectedDate();
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

void MainWindow::onReportForTeacher()
{
    qDebug() << "date=" << reportCalendarWidget->selectedDate();
    QMessageBox::critical(
        this,
        tr("Teacher's report"),
        tr("Under construction"));
}

void MainWindow::onReportForDirector()
{
    qDebug() << "date=" << reportCalendarWidget->selectedDate();
    QMessageBox::critical(
        this,
        tr("Director's report"),
        tr("Under construction"));
}

void MainWindow::onRegularChecks() {
    sendAction->setEnabled(
        !QDir("attendance/outbox").entryList(QStringList() << "*.tsv", QDir::Files).isEmpty()
    );
}