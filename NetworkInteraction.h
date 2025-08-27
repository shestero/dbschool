//
// Created by shestero on 8/27/25.
//

#ifndef DBSCHOOL1_NETWORKINTERACTION_H
#define DBSCHOOL1_NETWORKINTERACTION_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkInteraction: public QObject
{
    Q_OBJECT

public:
    NetworkInteraction(QObject *parent = nullptr);

    inline const QUrl api(QString key) const;

    QString getStudentsHash();
    void sendStudents();
    void sendTables();
    void receiveTables();
    void deleteTables(const QStringList ids);

    void getRequest(const QUrl &url)
    {
        QNetworkRequest request(url);
        manager->get(request);
    }

private:
    void startRequest(const QUrl &url);

private slots:
    void handleAuthentication(QNetworkReply *reply, QAuthenticator *authenticator);
    void handleFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
};


#endif //DBSCHOOL1_NETWORKINTERACTION_H