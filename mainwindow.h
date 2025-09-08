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


class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class NetworkInteraction;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

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
};


#endif //MAINWINDOW_H
