//
// Created by shestero on 8/6/25.
//

#include "mainwindow.h"
#include "CustomComboBoxSortFilterProxyModel.h"
#include "cmake-build-debug/_deps/qxlsx-src/QXlsx/header/xlsxcellrange.h"
#include "cmake-build-debug/_deps/qxlsx-src/QXlsx/header/xlsxcellreference.h"
#include "cmake-build-debug/_deps/qxlsx-src/QXlsx/header/xlsxformat.h"

#include <qt5/QtWidgets/qmainwindow.h>
#include <qt5/QtWidgets/qtoolbar.h>
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

QXlsx::Format center(QXlsx::Format format = QXlsx::Format())
{
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    return format;
}

QXlsx::Format right(QXlsx::Format format = QXlsx::Format())
{
    format.setHorizontalAlignment(QXlsx::Format::AlignRight);
    return format;
}

QXlsx::Format small(QXlsx::Format format = QXlsx::Format())
{
    format.setFontSize(8);
    return format;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), network(new NetworkInteraction(this))
{
    setWindowTitle(tr("School accounting"));

    boldFormat.setFontBold(true);
    wrapFormat.setTextWrap(true);
    wrapFormat = center(wrapFormat);
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
    addToolBar(/* Qt::LeftToolBarArea, */ toolBar = new QToolBar(tr("Attendance tools")));
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
    addToolBar(/* Qt::LeftToolBarArea, */ toolBar = new QToolBar(tr("Report tools")));
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
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc; // ss_id => st_id  => дата => кол-во
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
                // TODO: swap max and min
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
    const QDate searchDate = date.addDays(-Configuration().window_days);

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

    // TODO ...
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

void MainWindow::invoices(const QDate& start, const QDate& end)
{
    MainWindow::logger->writeTimestamp(
        tr("Creating invoices for period from %1 till %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
    );

    QApplication::setOverrideCursor(Qt::WaitCursor);  // курсор "ожидание"

    // [ studends map, ss_id => st_id => дата => кол-во ]
    QMap<int, QString> reportStudents;
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc;
    std::tie(reportStudents, acc) = toStdPair(scan(start, end));

    // st_id => ss_id => дата => кол-во
    QMap<int, QMap<int, const QMap<QDate, int>*>> acc2;
    for (auto i1 = acc.constBegin(); i1 != acc.constEnd(); ++i1)
        for (auto i2 = i1.value().second.constBegin(); i2 != i1.value().second.constEnd(); ++i2)
            acc2[i2.key()][i1.key()] = &i2.value();

    // Классы
    QMap<int, QString> allClasses = pClassesModel->dictionary();

    // Занятия и цены
    QMap<int, QString> sections = pSectionsModel->dictionary();
    QMap<int, QString> prices = pSectionsModel->dictionary(2);

    QList<QPair<int, QString>> sortedStudents;
    for (auto it = reportStudents.constBegin(); it != reportStudents.constEnd(); ++it)
    {
        sortedStudents.append(QPair<int, QString>(it.key(), it.value()));
    }
    std::sort(sortedStudents.begin(), sortedStudents.end(), [](QPair<int, QString>& a, QPair<int, QString>& b)
    {
        if (a.second == b.second)
            return a.first < b.first;
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
    });

    // В каких классах учатся эти ученики?
    QMap<int, QString> allStudents = pStudentsModel->dictionary(2);
    // todo: check that names in reportStudents and allStudents are the same


    const QString datePrefix = QDate::currentDate().toString("yyyy-MM-dd");

    for (const auto& student : sortedStudents)
    {
        if (!acc2.contains(student.first))
            continue;

        const QString className = allStudents.value(student.first);

        const QString filePath =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
            QDir::separator() +
            datePrefix + "-" + className + "-" +
            QString(student.second).replace(" ", "_") +
            ".xlsx";
        logger->appendLog(tr("Creating invoice for %1").arg(student.second));

        using namespace QXlsx;
        Document doc;

        //doc.addSheet(student.second);
        //doc.currentWorksheet()->... pageOrientation
        // see: QString WorksheetPrivate::Porientation;
        // pagesetup and print settings add by liufeijin 20181028, liufeijin

        doc.write(1, 2, student.second, boldFormat);
        if (!className.isEmpty())
            doc.write(1, 4, tr("%1 class").arg(className));
        doc.setColumnWidth(1, 3);
        doc.write(2, 2, tr("The inovice for period is from %1 till %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
        );

        doc.write(4, 2, tr("Subject"), center(boldFormat));
        doc.setColumnWidth(2, 20);
        doc.write(4, 3, tr("Amount"), center(boldFormat));
        doc.setColumnWidth(3, 8);
        doc.write(4, 4, tr("Dates"), center(boldFormat));
        doc.setColumnWidth(4, 16);
        doc.write(4, 5, tr("Price"), center(boldFormat));
        doc.write(4, 6, tr("Summa"), center(boldFormat));
        doc.setColumnWidth(6, 12);

        int i = 4;
        for (auto it = acc2[student.first].constBegin(); it != acc2[student.first].constEnd(); ++it)
        {
            doc.write(++i, 2, sections[it.key()], small(wrapFormat));

            int cnt = 0;
            QList<QPair<QDate, int>> dates;
            for (auto itc = it.value()->constBegin(); itc != it.value()->constEnd(); ++itc)
            {
                cnt += itc.value();
                if (itc.value() > 0)
                    dates.append(QPair<QDate, int>(itc.key(), itc.value()));
            }
            std::sort(dates.begin(), dates.end(), [](auto &a, auto &b)
            {
                return a.first < b.first;
            });
            QStringList strDates;
            for (const QPair<QDate, int>& p : dates)
            {
                QString date = p.first.toString(Configuration::date_format);
                if (p.second > 1)
                    date += QString(" (x %1)").arg(p.second);
                strDates.append(date);
            }

            if (!strDates.isEmpty())
                doc.setRowHeight(i, 16 * strDates.size());

            doc.write(i, 3, cnt, center());
            doc.write(i, 4, strDates.join(", \r\n"), wrapFormat);
            doc.write(i, 5, prices[it.key()], center(moneyFormat));
            doc.write(i, 6, QString("=C%1*E%1").arg(i), center(moneyFormat));
        }
        doc.write(++i, 2, tr("Total"), right(boldFormat));
        doc.mergeCells(CellRange(i, 2, i, 5));
        if (i > 5)
        {
            doc.write(i, 6, QString("=SUM(F5:F%1)").arg(i - 1), center(moneyFormat));
        }

        logger->appendLog(tr("Saving invoice for %1 into file %2.").arg(student.second).arg(filePath));
        doc.saveAs(filePath);
    }

    QApplication::restoreOverrideCursor(); // вернуть обычный курсор
}

void MainWindow::onIssueInvoices()
{
    qDebug() << "date=" << reportCalendarWidget->selectedDate();
    GenerateInvoices* dialog = new GenerateInvoices(this);

    int result = dialog->exec(); // Show the dialog modally

    if (result == QDialog::Accepted) {
        // Dialog was accepted (e.g., OK button clicked)
        qDebug() << "Custom dialog (issue invoices) accepted!";

        QDate date = reportCalendarWidget->selectedDate();
        const QDate end = QDate(date.year(), date.month(), 14);
        QDate prevMonth = end.addMonths(-1);
        const QDate start = QDate(prevMonth.year(), prevMonth.month(), 15);

        invoices(start, end);
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
    QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>> acc; // ss_id => st_id  => дата => кол-во
    std::tie(reportStudents, acc) = toStdPair(scan(start, end));

    QMap<int, QString> prices = pSectionsModel->dictionary(2); // Цены за занятия
    QMap<int, int> sectionsToTeachers = pSectionsModel->codeDictionary(3);
    QMap<int, QSet<int>> sectionsByTeacher;
    for (auto it = sectionsToTeachers.constBegin(); it != sectionsToTeachers.constEnd(); ++it)
    {
        sectionsByTeacher[it.value()].insert(it.key());
    }

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
    QMap<int, int> allStudents = pStudentsModel->codeDictionary(2);
    // todo: check that names in reportStudents and allStudents are the same
    QMap<int, QSet<int>> studentsByClasses;
    for (auto it = allStudents.constBegin(); it != allStudents.constEnd(); ++it)
    {
        if (reportStudents.contains(it.key()))
            studentsByClasses[it.value()].insert(it.key());
    }

    using namespace QXlsx;
    Document doc;

    QMap<int, QString> allClasses = pClassesModel->dictionary();

    // Перебор учителей
    for (auto itth = sortedTeachers.constBegin(); itth != sortedTeachers.constEnd(); ++itth)
    {
        const QSet<int>& sections = sectionsByTeacher[itth->first];
        qDebug() << "th" << *itth << "sections" << sections;
        if (sections.isEmpty())
            continue;

        QList<QPair<int, QString>> sortedCalsses;
        for (auto itcl = allClasses.constBegin(); itcl != allClasses.constEnd(); ++itcl)
        {
            // filter the classes mentioned in report only and those that are in teacher's sections
            if (studentsByClasses.keys().contains(itcl.key()))
                sortedCalsses.append(QPair<int, QString>(itcl.key(), itcl.value()));
        }

        std::sort(sortedCalsses.begin(), sortedCalsses.end(), [](QPair<int, QString>& a, QPair<int, QString>& b)
        {
            if (a.second == b.second)
                return a.first < b.first;
            return CustomComboBoxSortFilterProxyModel::lessThan(a.second, b.second);
        });

        QStringList total;
        int i = 5;
        for (auto cls : sortedCalsses)
        {
            i++;
            QStringList total;
            QMap<int, int> sums;
            for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
            {
                if (!sections.contains(itss.key()))
                    continue;

                for (int st_id : studentsByClasses[cls.first])
                    for (int v : itss.value().second[st_id])
                    {
                        if (i == 6)
                        {
                            doc.addSheet(itth->second);
                        }
                        sums[itss.key()] += v;
                    }
            }
            if (sums.isEmpty()) {
                i--;
            } else {
                int j = 3;
                for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
                {
                    if (!sections.contains(itss.key()))
                        continue;

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

                    doc.write(i, 1, cls.second);
                    doc.write(i, 2, total.join("+").prepend("="), moneyFormat);
                    doc.write(i, j, sums[itss.key()]);

                    j += 2;
                }
            }
        }

        if (i > 5)
        {
            doc.write(1, 1, itth->second, boldFormat);
            doc.write(2, 1, tr("Расчётный период с %1 по %2")
                .arg(start.toString(Configuration::date_format))
                .arg(end.toString(Configuration::date_format))
            );

            doc.write(4, 1, tr("Class"), center(boldFormat));
            doc.write(4, 2, tr("Total"), center(boldFormat));
            doc.write(4, 3, tr("Subject name"), boldFormat);
            doc.mergeCells(CellRange(4, 1, 5, 1));
            doc.mergeCells(CellRange(4, 2, 5, 2));

            int j = 3;
            for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
            {
                if (!sections.contains(itss.key()))
                    continue;

                doc.write(4, j, itss.value().first, wrapFormat);
                doc.setRowHeight(4, 60);
                doc.mergeCells(CellRange(4, j, 4, j+1));
                doc.write(5, j, tr("Amount"));
                doc.write(5, j+1, tr("Summa"));
                j += 2;
            }
            if (i > 6)
            {
                doc.write(i+1, 2, QString("=sum(B7:B%1)").arg(i), moneyFormat);
            }
        }
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
        tr("Creating the director's report from %1 till %2")
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

        doc.write(1, 1, tr("%1 class").arg(className), boldFormat);
        doc.write(1, 2, tr("The report period is from %1 till %2")
            .arg(start.toString(Configuration::date_format))
            .arg(end.toString(Configuration::date_format))
        );
        doc.write(3, 1, tr("Full name"), center(boldFormat));
        doc.mergeCells(CellRange(3, 1, 5, 1));
        doc.setColumnWidth(1, 20);
        doc.write(3, 2, tr("Total"), center(boldFormat));
        doc.mergeCells(CellRange(3, 2, 5, 2));
        doc.write(3, 3, tr("Subject name"), center(boldFormat));
        doc.setRowHeight(4, 50);
        doc.setRowHeight(5, 32);

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

        int j = 2;
        for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
        {
            doc.write(4, ++j, itss.value().first, small(wrapFormat));
            doc.mergeCells(CellRange(4, j, 4, j+1));
            doc.write(5, j, tr("Amount"), wrapFormat);
            doc.setColumnWidth(j, 6);
            doc.write(5, ++j, tr("Summa"), wrapFormat);
            doc.setColumnWidth(j, 7);
        }
        doc.mergeCells(CellRange(3, 3, 3, j));

        int i = 5;
        for (auto student : sortedStudents)
        {
            doc.write(++i, 1, student.second);

            int j = 3;
            QStringList total;
            for (auto itss = acc.constBegin(); itss != acc.constEnd(); ++itss)
            {
                bool ok = false;
                double price = prices[itss.key()].toDouble(&ok);
                if (ok)
                {
                    doc.write(i, j+1,
                        QString("=%1*%2").arg(QXlsx::CellReference(i, j).toString()).arg(price),
                        small(moneyFormat)
                    );
                    total += QXlsx::CellReference(i, j+1).toString();
                } else
                {
                    qWarning() << "No price for" << itss.value().first;
                    doc.write(i, j+1, "?");
                }

                int sum = 0;
                for (int v : itss.value().second[student.first])
                    sum += v;
                doc.write(i, j, sum, small());
                j += 2;
            }
            doc.write(i, 2, total.join("+").prepend("="), moneyFormat);
        }
        doc.write(i+1, 2, QString("=SUM(B7:B%1)").arg(i), moneyFormat);
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