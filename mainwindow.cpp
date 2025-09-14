//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "CustomComboBoxSortFilterProxyModel.h"
#include "cmake-build-debug/_deps/qxlsx-src/QXlsx/header/xlsxcellreference.h"
#include "cmake-build-debug/_deps/qxlsx-src/QXlsx/header/xlsxformat.h"

#include <tuple>
#include <utility>
template<typename T1, typename T2>
std::pair<T1,T2> toStdPair(const QPair<T1,T2>& qp) {
    return {qp.first, qp.second};
}

#include "Attendance.h"
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
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
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

// https://github.com/dbzhang800/QtXlsxWriter
// https://github.com/j2doll/QXlsx.git
#include "xlsxdocument.h"

#include <QDebug>

#define LASTDATE_FILENAME   "last-date.txt"
static LastAttendanceDate gl_lastAttendanceDate = LastAttendanceDate(LASTDATE_FILENAME);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), network(new NetworkInteraction(this))
{
    setWindowTitle(tr("School accounting"));

    boldFormat.setFontBold(true);
    wrapFormat.setTextWrap(true);
    wrapFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    wrapFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    moneyFormat.setNumberFormat("₽#,##0");

    // Tabs
    auto tabs = new QTabWidget(this);

    pClassesModel = new CSVTableModel(this, "classes.tsv");
    TableView* pClasses = new TableView(this, pClassesModel);
    pClasses->setColumnWidth(1, 300);
    tabs->addTab(pClasses, tr("Classes"));

    pStudentsModel = new CSVTableModel(this, "students.tsv", 2, pClassesModel);
    auto pStudents = new TableView(this, pStudentsModel);
    pStudents->setItemDelegateForColumn(2, new ForeignKeyDelegate(pClassesModel, this));
    pStudents->setColumnWidth(1, 400);
    pStudents->setColumnWidth(2, 240);
    tabs->addTab(pStudents, tr("Students"));

    pTeacherModel = new CSVTableModel(this, "teachers.tsv");
    TableView* pTeachers = new TableView(this, pTeacherModel);
    pTeachers->setColumnWidth(1, 400);
    tabs->addTab(pTeachers, tr("Teachers"));

    pSectionsModel = new CSVTableModel(this, "sections.tsv", 3, pTeacherModel);
    TableView* pSections = new TableView(this, pSectionsModel);
    pSections->setItemDelegateForColumn(3, new ForeignKeyDelegate(pTeacherModel, this));
    pSections->setColumnWidth(1, 400);
    pSections->setColumnWidth(3, 360);
    tabs->addTab(pSections, tr("Sections"));

    // Report's calendar
    reportCalendarWidget = new ToolBarCalendarWidget;

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
    reportCalendarWidget->setPalette(toolBar->palette());

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

    // Top layout
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tabs);
    splitter->addWidget(logger = new ProtocolWidget);
    splitter->setStretchFactor(1, 0); // правая часть не растягивается сама, но можно тянуть мышкой
    splitter->setSizes({800, 600});
    setCentralWidget(splitter);
    statusBar()->setWindowTitle(tr("Ready..."));

    connect(network, &NetworkInteraction::appendLog, logger, &ProtocolWidget::appendLog);

    // check timer
    QTimer *checkTimer = new QTimer(this);
    checkTimer->setInterval(1000);
    connect(checkTimer, &QTimer::timeout, this, &MainWindow::onRegularChecks);
    checkTimer->start();
}

MainWindow::~MainWindow() = default;

QPair<QMap<int, QString>, QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>>>
    MainWindow::scan(const QDate& date_start, const QDate& date_end)
{
    const QStringList files =
        QDir("attendance/inbox").entryList(QStringList() << "*.tsv", QDir::Files);

    if (files.isEmpty())
    {
        logger->appendLog(tr("Warning: no existing attendance tables found!"));
    }
    logger->progress_max(files.size());

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"
    QMap<int, QString> students;
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc; // ss_id => st_id  => дата => сумма
    int i = 0;
    for (const QString& file : files)
    {
        qDebug() << "file:" << file;
        logger->progress(++i);
        QCoreApplication::processEvents(); // обновляем окно

        try
        {
            QString filename = QString("attendance/inbox/%1").arg(file);
            Attendance attendance(filename);
            qDebug() << "attendance.students.size=" << attendance.students.size();
            for (int st_id : attendance.students.keys())
            {
                const QVector<QString>& av = attendance.students[st_id];
                qDebug() << "av=" << av;
                if (!av.isEmpty())
                    students[st_id] = av.at(0);

                int j = 0;
                const QDate minDate = std::min(attendance.date_min, date_start);
                const QDate maxDate = std::max(attendance.date_max, date_end);
                for (QDate d = minDate; d <= maxDate; d = d.addDays(1)) {
                    const QString& s = av.value(++j, ""); // нулевой элемет содержит ФИО - тут пропускаем
                    if (s.isEmpty())
                        continue;
                    bool ok = false;
                    int cnt = s.toInt(&ok);
                    if (!ok)
                    {
                        qDebug() << "!ok!" << s;
                        logger->appendLog(
                            tr("Something wrong in file %1 student %2 (%3), date %4, index=%5: s=%6")
                                .arg(file)
                                .arg(attendance.students[st_id].value(0, "???"))
                                .arg(st_id)
                                .arg(d.toString(Configuration::date_format))
                                .arg(j)
                                .arg(s)
                        );
                    }
                    acc[attendance.ss_id].first = attendance.ss_name;
                    acc[attendance.ss_id].second[st_id][d] += cnt;
                }
            }
        } catch (...)
        {
            logger->appendLog(tr("Error: cannot process file %1").arg(file));
        }
    }
    QApplication::restoreOverrideCursor(); // вернуть обычный курсор

    return { students, acc };
}

void MainWindow::onCreateAttendanceTables()
{
    QDate date = gl_lastAttendanceDate.get();
    const QDate searchDate = date.addDays(-Configuration().window_days); // Ищем учеников по курсам за последние 100 дней // TODO

    logger->writeTimestamp(
        tr("Creating tables by attendances from %1").arg(searchDate.toString(Configuration::date_format))
    );

    QMap<int, QString> _students;
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc;
    std::tie(_students, acc) = toStdPair(scan(searchDate, QDate::currentDate()));
    qDebug() << "acc.size=" << acc.size();
    for (auto it = acc.constBegin(); it != acc.constEnd(); ++it) {
        qDebug() << it.key() << it.value().first << it.value().second;

    }

    /*
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
    */
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
    qDebug() << "MainWindow::onReportForTeacher: date=" << reportCalendarWidget->selectedDate();
    QDate date = reportCalendarWidget->selectedDate();
    const QDate end = QDate(date.year(), date.month(), 14);
    QDate prevMonth = end.addMonths(-1);
    const QDate start = QDate(prevMonth.year(), prevMonth.month(), 15);

    logger->writeTimestamp(
        tr("Creating the teacher's report from %1 till %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
    );

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"

    QMap<int, QString> reportStudents;
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc;
    std::tie(reportStudents, acc) = toStdPair(scan(start, end));

    // Цены за занятия
    QMap<int, QString> prices = pSectionsModel->dictionary(2);

    QApplication::restoreOverrideCursor(); // вернуть обычный курсор

    const QString defaultFileName =
        QDate::currentDate().toString("yyyy-MM-dd") + tr("-report-teacher.xlsx");
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save teacher's report"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + defaultFileName,
        tr("Excel Files (*.xlsx);;All Files (*)") // File filters
    );

    if (filePath.isEmpty())
    {
        qDebug() << "cancelled";
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"

    QMap<int, QString> allTeachers = pTeacherModel->dictionary();
    QList<QPair<int, QString>> sortedTeachers;
    for (auto it = allTeachers.constBegin(); it != allTeachers.constEnd(); ++it)
    {
        // todo: filter teachers
        // todo: check repeating
        sortedTeachers.append(QPair<int, QString>(it.key(), it.value()));
    }
    std::sort(sortedTeachers.begin(), sortedTeachers.end(), [](QPair<int, QString>& a, QPair<int, QString>& b)
    {
        if (a.second == b.second)
            return a.first < b.first;
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    // В каких классах учатся эти ученики?
    QMap<int, QString> allStudents = pStudentsModel->dictionary(2);
    // todo: check that names in reportStudents and allStudents are the same
    QMap<QString, QSet<int>> studentsByClasses;
    for (auto it = allStudents.constBegin(); it != allStudents.constEnd(); ++it)
    {
        if (reportStudents.contains(it.key()))
            studentsByClasses[it.value()].insert(it.key());
    }

    using namespace QXlsx;
    Document doc;

    QMap<int, QString> allClasses = pClassesModel->dictionary();
    QList<QPair<int, QString>> sortedCalsses;
    for (auto it = allClasses.constBegin(); it != allClasses.constEnd(); ++it)
    {
        if (studentsByClasses.keys().contains(it.value())) // filter the classes mentioned in report only
            sortedCalsses.append(QPair<int, QString>(it.key(), it.value()));
    }
    std::sort(sortedCalsses.begin(), sortedCalsses.end(), [](QPair<int, QString>& a, QPair<int, QString>& b)
    {
        if (a.second == b.second)
            return a.first < b.first;
        return CustomComboBoxSortFilterProxyModel::lessThan(a.second, b.second);
    });

    // Перебор учителей
    for (auto it = sortedTeachers.constBegin(); it != sortedTeachers.constEnd(); ++it)
    {
        doc.addSheet(it->second);
        doc.write(1, 1, it->second);
        doc.write(2, 1, tr("Расчётный период с %1 по %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
        );

        doc.write(4, 1, tr("Class"), boldFormat);
        doc.write(4, 3, tr("Subject name"), boldFormat);
        doc.write(5, 2, tr("Total"), boldFormat);
        //doc.currentWorksheet()->mergeCells(CellRange("B4:B6"));
        doc.mergeCells(CellRange("A4:A6"));
        doc.mergeCells(CellRange("B5:B6"));
        int j = 3;


        QStringList total;
        int i = 6;
        for (auto cls : sortedCalsses)
        {
            doc.write(++i, 1, cls.second);

            int j = 3;
            QStringList total;
            for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
            {
                doc.write(5, j, itss.value().first, wrapFormat);
                doc.setRowHeight(5, 60);
                doc.mergeCells(CellRange(5, j, 5, j+1));
                doc.write(6, j, tr("Amount"));
                doc.write(6, j+1, tr("Summa"));

                bool ok = false;
                double price = prices[itss.key()].toDouble(&ok);
                if (ok)
                {
                    doc.write(i, j+1, QString("=%1*%2").arg(QXlsx::CellReference(i, j).toString()).arg(price), moneyFormat);
                    total += QXlsx::CellReference(i, j+1).toString();
                } else
                {
                    qWarning() << "No price for" << itss.value().first;
                    doc.write(i, j+1, "?");
                }

                int sum = 0;
                // TODO
                doc.write(i, j, 1234);
                /*
                for (int v : itss.value().second[student.first])
                    sum += v;
                doc.write(i, j, sum);
                */
                j += 2;
            }
            doc.write(i, 2, total.join("+").prepend("="), moneyFormat);
        }
        doc.write(i+1, 2, QString("=sum(B7:B%1)").arg(i), moneyFormat);
    }

    doc.selectSheet(0);
    doc.saveAs(filePath);

    QApplication::restoreOverrideCursor(); // вернуть обычный курсор
}

void MainWindow::onReportForDirector()
{
    qDebug() << "MainWindow::onReportForDirector: date=" << reportCalendarWidget->selectedDate();
    QDate date = reportCalendarWidget->selectedDate();
    const QDate end = QDate(date.year(), date.month(), 14);
    QDate prevMonth = end.addMonths(-1);
    const QDate start = QDate(prevMonth.year(), prevMonth.month(), 15);
    qDebug() << start << " - " << end;

    logger->writeTimestamp(
        tr("Creating the director's report from %1 till %2").arg(start.toString(Configuration::date_format)).arg(end.toString(Configuration::date_format))
    );

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"

    QMap<int, QString> reportStudents;
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc;
    std::tie(reportStudents, acc) = toStdPair(scan(start, end));

    // Цены за занятия
    QMap<int, QString> prices = pSectionsModel->dictionary(2);

    QApplication::restoreOverrideCursor(); // вернуть обычный курсор

    const QString defaultFileName =
        QDate::currentDate().toString("yyyy-MM-dd") + tr("-report-director.xlsx");
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save director's report"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + defaultFileName,
        tr("Excel Files (*.xlsx);;All Files (*)") // File filters
    );

    if (filePath.isEmpty())
    {
        qDebug() << "cancelled";
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"

    // В каких классах учатся эти ученики?
    QMap<int, QString> allStudents = pStudentsModel->dictionary(2);
    // todo: check that names in reportStudents and allStudents are the same
    QMap<QString, QSet<int>> studentsByClasses;
    for (auto it = allStudents.constBegin(); it != allStudents.constEnd(); ++it)
    {
        if (reportStudents.contains(it.key()))
            studentsByClasses[it.value()].insert(it.key());
    }

    using namespace QXlsx;
    Document doc;

    QList<QString> sortedCalsses = studentsByClasses.keys();
    std::sort(sortedCalsses.begin(), sortedCalsses.end(), [](const auto& a, const auto& b)
    {
        return CustomComboBoxSortFilterProxyModel::lessThan(a, b);
    });

    // Перебор классов
    for (QString className : sortedCalsses)
    {
        doc.addSheet(className);
        doc.write(1, 1, tr("%1 класс").arg(className));
        doc.write(2, 1, tr("Расчётный период с %1 по %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
        );
        doc.write(4, 1, tr("Full name"), boldFormat);
        doc.setColumnWidth(1, 30);
        doc.write(4, 3, tr("Subject name"), boldFormat);
        doc.write(5, 2, tr("Total"), boldFormat);
        //doc.currentWorksheet()->mergeCells(CellRange("B4:B6"));
        doc.mergeCells(CellRange("A4:A6"));
        doc.mergeCells(CellRange("B5:B6"));

        QList<QPair<int, QString>> sortedStudents;
        for (int studentCode : studentsByClasses[className])
        {
            sortedStudents.append(QPair<int, QString>(studentCode, reportStudents[studentCode]));
        }
        std::sort(sortedStudents.begin(), sortedStudents.end(), [](QPair<int, QString>& a, QPair<int, QString>& b)
        {
            if (a.second == b.second)
                return a.first < b.first;
            return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
        });

        int i = 6;
        for (auto student : sortedStudents)
        {
            doc.write(++i, 1, student.second);

            int j = 3;
            QStringList total;
            for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
            {
                doc.write(5, j, itss.value().first, wrapFormat);
                doc.setRowHeight(5, 60);
                doc.mergeCells(CellRange(5, j, 5, j+1));
                doc.write(6, j, tr("Amount"));
                doc.write(6, j+1, tr("Summa"));

                bool ok = false;
                double price = prices[itss.key()].toDouble(&ok);
                if (ok)
                {
                    doc.write(i, j+1, QString("=%1*%2").arg(QXlsx::CellReference(i, j).toString()).arg(price), moneyFormat);
                    total += QXlsx::CellReference(i, j+1).toString();
                } else
                {
                    qWarning() << "No price for" << itss.value().first;
                    doc.write(i, j+1, "?");
                }

                int sum = 0;
                for (int v : itss.value().second[student.first])
                    sum += v;
                doc.write(i, j, sum);
                j += 2;
            }
            doc.write(i, 2, total.join("+").prepend("="), moneyFormat);
        }
        doc.write(i+1, 2, QString("=sum(B7:B%1)").arg(i), moneyFormat);
    }

    doc.selectSheet(0);
    doc.saveAs(filePath);

    QApplication::restoreOverrideCursor(); // вернуть обычный курсор
}

void MainWindow::onRegularChecks() {
    sendAction->setEnabled(
        !QDir("attendance/outbox").entryList(QStringList() << "*.tsv", QDir::Files).isEmpty()
    );
}