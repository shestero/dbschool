//
// Created by shestero on 8/27/25.
//

#ifndef DBSCHOOL1_NETWORKINTERACTION_H
#define DBSCHOOL1_NETWORKINTERACTION_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class NetworkInteraction: public QObject
{
    Q_OBJECT

public:
    NetworkInteraction(QObject *parent = nullptr);

    inline const QUrl api(QString key) const;

    QNetworkRequest request(const QUrl &url);
    QString syncReply(QNetworkReply* reply);

    QString getStudentsHash();
    void sendStudents();
    void sendTables();
    void receiveTables();
    void deleteTables(const QStringList& ids);


private:
    void startRequest(const QUrl &url);

private slots:
    void handleAuthentication(QNetworkReply *reply, QAuthenticator *authenticator);
    void handleFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};


#endif //DBSCHOOL1_NETWORKINTERACTION_H