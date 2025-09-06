//
// Created by shestero on 8/27/25.
//

#include "NetworkInteraction.h"

#include "Configuration.h"
#include "ProtocolDialog.h"
#include "mainwindow.h"

#include <QApplication>
#include <QAuthenticator>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QMessageBox>
#include <QNetworkAccessManager>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include "Worker.h"

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

QNetworkRequest NetworkInteraction::request(const QUrl& url)
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

    return request;
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

QString NetworkInteraction::syncReply(QNetworkReply* reply)
{
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // This line waits for the reply to finish

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->errorString();
        reply->deleteLater(); // Clean up the reply object
        return {};
    }

    QByteArray data = reply->readAll();
    qDebug() << "Data received:" << data;
    reply->deleteLater(); // Clean up the reply object
    return {data.toStdString().c_str() };
}

void NetworkInteraction::startRequest(const QUrl &url)
{
    QNetworkRequest request = this->request(url);
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
    QNetworkRequest request = this->request(url);
    QNetworkReply* reply = manager->get(request);
    auto hash = syncReply(reply);
    return hash;
}

bool NetworkInteraction::renameToBak(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        qWarning() << "Файл не существует:" << filePath;
        return false;
    }

    // Новый путь с тем же именем, но расширением .bak
    QString newFilePath = info.path() + "/" + info.completeBaseName() + ".bak";

    if (QFile::exists(newFilePath)) {
        QFile::remove(newFilePath);  // если уже есть .bak — удаляем
    }

    if (!QFile::rename(filePath, newFilePath)) {
        qWarning() << "Ошибка переименования в" << newFilePath;
        return false;
    }

    qDebug() << "Файл переименован в:" << newFilePath;
    return true;
}

bool NetworkInteraction::deleteFile(const QString &filePath)
{
    if (QFile::exists(filePath)) {
        if (QFile::remove(filePath)) {
            qDebug() << "Файл удалён:" << filePath;
            return true;
        }
        qDebug() << "Не удалось удалить файл:" << filePath;
    } else {
        qDebug() << "Файл не найден:" << filePath;
    }
    return false;
}

void NetworkInteraction::sendStudents()
{
    auto url = api("students");
}

void NetworkInteraction::sendTable(const QString& file_name)
{
    auto url = api(QString("attendance/%1").arg(file_name));
    QNetworkRequest request = this->request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    QString filename = QString("attendance/outbox/%1").arg(file_name);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file " << file_name;
        emit appendLog(tr("Cannot open file %1 !").arg(file_name));
        return;
    }

    // Отправка PUT-запроса
    QNetworkReply *reply = manager->put(request, &file);
    QString response = syncReply(reply);
    // syncReply блокирует; важно не грохнуть объект file до того, как запрос отработал до конца
    qDebug() << "response = " << response;

    if (response == "OK")
    {
        emit appendLog(tr("\t- file sent"));
        if (!renameToBak(filename))
        {
            // не удалось переименовать в .bak - просто удаляем
            if (deleteFile(filename))
            {
                emit appendLog(tr("\t- file deleted"));
            } else
            {
                emit appendLog(tr("\t- Cannot delete the file!!"));
            }
        }
    }
}

void NetworkInteraction::sendTables(const QStringList& files)
{
    ProtocolDialog* progress = createProgress(tr("Sending new blanks to the servers"));
    progress->progress_max(files.size());
    int i = 0;
    for (const auto &file : files) {
        qDebug() << file;
        progress->appendLog(file); // emit appendLog(file);
        sendTable(file);
        progress->progress(++i);

        // Обновить GUI
        QApplication::processEvents(); // QCoreApplication::processEvents();
        // или QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
    }
}


bool badFileName(const QString& file) {
    int dots = 0;
    for (QChar c : file) {
        if (!(c.isLetterOrNumber() || c=='.' || c=='-' || c=='_')) {
            return true;
        }
        if (c=='.')
            if (++dots > 1)
                return true;
    }
    return false;
}

MainWindow* mainWindow()
{
    MainWindow* mainWindow = nullptr;
    QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget* widget : topLevelWidgets) {
        // Check if the widget is a QMainWindow and cast it
        mainWindow = qobject_cast<MainWindow*>(widget);
        if (mainWindow) {
            break;
        }
    }
    return mainWindow;
}

void NetworkInteraction::receiveTables()
{
    auto url = api("attendances/outbox");
    auto request = this->request(url);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QNetworkReply *reply = manager->get(request);
    QString response = syncReply(reply);
    QGuiApplication::restoreOverrideCursor();
    if (response.isEmpty())
    {
        qWarning() << "Empty response";
        QMessageBox::critical(
            mainWindow(),
            tr("Receiving the attendance tables"),
            tr("Cannot read the file list from server"));
        return;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(response.toUtf8());
    if (jsonDocument.array().isEmpty())
    {
        qWarning() << "Empty array response";
        QMessageBox::critical(
            mainWindow(),
            tr("Receiving the attendance tables"),
            tr("No filled files for load"));
        return;
    }

    ProtocolDialog* dialog = createProgress(tr("Receiving filled blanks"));
    dialog->progress_max(jsonDocument.array().size());

    int i = 0;
    for (const QJsonValue &item : jsonDocument.array())
    {
        i++;
        if (!item.isString())
            continue;

        QString fileName = item.toString();
        if (badFileName(fileName))
        {
            qWarning() << "Bad file name:" << fileName << " (skipped)";
            dialog->appendLog(tr("File name %1 considered bad; it's skipped!").arg(fileName));
            continue;
        }

        auto url = api(QString("attendance/outbox/") + fileName);
        auto request = this->request(url);
        QNetworkReply *reply = manager->get(request);
        QString response = syncReply(reply);
        if (response.isEmpty())
        {
            qWarning() << "Cannot read file" << fileName;
            dialog->appendLog(tr("Cannot read file %1 from server!").arg(fileName));
            continue;
        }

        QString outputName = QString("attendance/inbox/") + fileName;
        QFile file(outputName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream(&file) << response;
            file.close();
        } else {
            qCritical() << "Cannot save file" << outputName;
            dialog->appendLog(tr("Cannot save file %1!").arg(outputName));
        }

        this->deleteTables(QStringList() << fileName); // TODO: not optimal!

        dialog->progress(i);
        dialog->appendLog(fileName);
    }
}

void NetworkInteraction::deleteTables(const QStringList& ids)
{
    auto url = api("attendance");

    // TODO
}

ProtocolDialog* NetworkInteraction::createProgress(const QString& title)
{
    MainWindow* parent = mainWindow();
    auto dialog = new ProtocolDialog(title, parent);
    dialog->show();
    if (parent)
    {
        connect(parent->network, &NetworkInteraction::appendLog, dialog, &ProtocolDialog::appendLog);
    }
    QThread* thread = new QThread(dialog);
    dialog->moveToThread(thread);
    thread->start();

    return dialog;
}




