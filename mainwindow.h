//
// Created by shestero on 8/6/25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QCalendarWidget>
#include <QMainWindow>
#include <QTableWidget>

#include "NetworkInteraction.h"
#include "ProtocolWidget.h"
#include "toolbarcalendarwidget.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class NetworkInteraction;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // [ studends map, ss_id => st_id  => дата => сумма ]
    QPair<QMap<int, QString>, QMap<int, QPair<QString, QMap<int, QMap<QDate, int>>>>>
        scan(const QDate& date_start, const QDate& date_end);

public slots:
    void onCreateAttendanceTables();
    void onSendAttendanceTables();
    void onReceiveAttendanceTables();
    void onRefreshStudentTable();
    void onIssueInvoices();
    void onReportForTeacher();
    void onReportForDirector();
    void onRegularChecks();

signals:
    void sendMessage(const QString& msg);

private:
    QString calculateSha256Hash(const QString& filePath);

    NetworkInteraction* network;
    QAction* sendAction;

    QCalendarWidget* reportCalendarWidget;
    ProtocolWidget* logger;
};


#endif //MAINWINDOW_H
