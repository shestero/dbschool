//
// Created by shestero on 8/27/25.
//

#include "NetworkInteraction.h"
#include "Configuration.h"

#include <QAuthenticator>
#include <QObject>
#include <QDebug>
#include <qt5/QtCore/qobject.h>
#include <qt5/QtNetwork/qnetworkaccessmanager.h>

NetworkInteraction::NetworkInteraction(QObject* parent):
    QObject(parent),
    manager(new QNetworkAccessManager(parent))
{
    connect(manager, &QNetworkAccessManager::authenticationRequired,
            this, &NetworkInteraction::handleAuthentication);
    //connect(manager, &QNetworkAccessManager::finished, this, &NetworkInteraction::handleFinished);
}

void NetworkInteraction::handleAuthentication(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply);
    qDebug() << "Authentication required for" << authenticator->realm();
    authenticator->setUser("username");
    authenticator->setPassword("password");
}

void NetworkInteraction::handleFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Success:" << reply->readAll();
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    //reply->deleteLater();
    //QCoreApplication::quit();
}

//#include <QNetworkAccessManager>
//#include <QNetworkReply>
#include <QFuture>
#include <QFutureInterface>
#include <QFutureWatcher>

QFuture<QString> replyToFuture(QNetworkReply *reply) {
    QFutureInterface<QString> iface;
    iface.reportStarted();

    QObject::connect(reply, &QNetworkReply::finished, [reply, iface]() mutable {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "reply size = " << reply->size();
            QByteArray data = reply->readAll();
            qDebug() << "data size = " << data.size();
            const QString result = QString(data.toStdString().c_str());
            qDebug() << "from replyToFuture:" << result;
            iface.reportResult(result);
        } else {
            // Можно завести QFuture<QByteArray, QNetworkReply::NetworkError> и т.д.
            // либо сигнализировать об ошибке кастомно
            qDebug() << "Network error:" << reply->errorString();
            iface.reportResult(QString());
        }
        iface.reportFinished();
        reply->deleteLater();
    });

    return iface.future();
}

void NetworkInteraction::startRequest(const QUrl &url)
{
    QNetworkRequest request(url);

    // Concatenate username and password with a colon and encode to base64
    Configuration config;
    const QString username = QString(config.login.c_str());
    const QString password = QString(config.password.c_str());
    QString credentials = username + ":" + password;
    QByteArray encodedData = credentials.toLocal8Bit().toBase64();
    QString headerData = "Basic " + encodedData;

    // Set the Authorization header
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    QNetworkReply* reply = manager->get(request);

    /*
    connect(reply, &QNetworkReply::finished, this, [=]() {
        // Process the reply data
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            const QString result = QString(data.toStdString().c_str());
            qDebug() << "get data (in lambda): " << result;
        } else {
            qDebug() << "Network error:" << reply->errorString();
            // promise->setException(ApiException {message});
        }
        reply->deleteLater(); // Schedule for safe deletion
    });
    */

    QFuture<QString> fut = replyToFuture(reply);

    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this); // 'this' is the parent object for memory management
    connect(watcher, &QFutureWatcher<QString>::finished, [watcher]() {
        // Code to execute when the future finishes
        auto result = watcher->result();
        qDebug() << "from future: " << result;
    });
    watcher->setFuture(fut);
}

const QUrl NetworkInteraction::api(QString key) const
{
    QString url =
        QString(Configuration().teach_server.c_str()) + "/api/" + key;
    qDebug() << "url=" << url;

    return QUrl(url);
}

QString NetworkInteraction::getStudentsHash()
{
    auto url = api("students/hash");
    startRequest(url);
    return "unknown"; // todo
}

void NetworkInteraction::sendStudents()
{
    auto url = api("students");
}

void NetworkInteraction::sendTables()
{
    auto url = api("attendance");
}

void NetworkInteraction::receiveTables()
{
    auto url = api("attendance");
}

void NetworkInteraction::deleteTables(const QStringList ids)
{
    auto url = api("attendance");
}






