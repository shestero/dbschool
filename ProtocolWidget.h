//
// Created by shestero on 9/8/25.
//

#ifndef DBSCHOOL1_QPROTOCOLWIDGET_H
#define DBSCHOOL1_QPROTOCOLWIDGET_H

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QProgressBar>
#include <QTextEdit>
#include <QWidget>



class ProtocolWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolWidget(QWidget* parent = nullptr);
    ~ProtocolWidget() override;

public slots:

    void progress_max(int max)
    {
        progressBar->setMaximum(max);
    }

    void progress(int p)
    {
        progressBar->setValue(p);
    }

    void appendLog(const QString& line)
    {
        textEdit->append(line);
    }

    void writeTimestamp(const QString& title = "")
    {
        textEdit->append(QString("<font color='blue'>%1 <b>%2</b></font>")
                         .arg(
                             QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")
                         )
                         .arg(title)
        );
    }

    void onCopy()
    {
        QApplication::clipboard()->setText(textEdit->toPlainText()); // обычный текст
    }

    void onClear()
    {
        textEdit->clear();
    }

private:
    QTextEdit* textEdit;
    QProgressBar* progressBar;
};


#endif //DBSCHOOL1_QPROTOCOLWIDGET_H