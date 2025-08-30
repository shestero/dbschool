//
// Created by shestero on 8/6/25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QTableWidget>

#include "NetworkInteraction.h"


class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onCreateAttendanceTables();
    void onSendAttendanceTables();
    void onReceiveAttendanceTables();
    void onRefreshStudentTable();
    void onRegularChecks();

signals:
    void sendMessage(const QString &msg);

private:
    NetworkInteraction* network_interaction;
    QAction* sendAction;
};


#endif //MAINWINDOW_H
