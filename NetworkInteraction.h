//
// Created by shestero on 8/27/25.
//

#ifndef DBSCHOOL1_NETWORKINTERACTION_H
#define DBSCHOOL1_NETWORKINTERACTION_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

#include "ProtocolDialog.h"

class NetworkInteraction: public QObject
{
    Q_OBJECT

public:
    NetworkInteraction(QObject *parent = nullptr);

    inline const QUrl api(QString key) const;

    QNetworkRequest request(const QUrl &url);
    QString syncReply(QNetworkReply* reply);

    QString getStudentsHash();
    bool sendStudents();
    QString getTeachersHash();
    bool sendTeachers();

    void sendTable(const QString& file_name);
    void sendTables(const QStringList& files);
    void receiveTables();
    QStringList deleteTables(const QStringList& ids);

private:
    bool renameToBak(const QString& file_info);
    bool deleteFile(const QString &filePath);

    void startRequest(const QUrl &url);

private slots:
    void handleAuthentication(QNetworkReply *reply, QAuthenticator *authenticator);
    void handleFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;

    ProtocolDialog* createProgress(const QString& title);

signals:
    void appendLog(const QString& line);
};


#endif //DBSCHOOL1_NETWORKINTERACTION_H