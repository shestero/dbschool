//
// Created by shestero on 9/5/25.
//

#ifndef DBSCHOOL1_WORKER_H
#define DBSCHOOL1_WORKER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <functional>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker()
    {
        QThread* thread = new QThread(this);
        qDebug() << "Worker's thread: " << thread;
        moveToThread(thread);

        connect(thread, &QThread::finished, this, &QObject::deleteLater);
        connect(this, &Worker::taskFinished, thread, &QThread::quit);
    }

public slots:
    void runTask(std::function<void()> task) {
        if (task) {
            qDebug() << "Running task in thread:" << QThread::currentThread();
            task();
            emit taskFinished();
        }
    }

    signals:
        void taskFinished();
};

#endif //DBSCHOOL1_WORKER_H