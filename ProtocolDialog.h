//
// Created by shestero on 8/30/25.
//

#ifndef DBSCHOOL1_PROTOCOLDIALOG_H
#define DBSCHOOL1_PROTOCOLDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>

class ProtocolDialog : public QDialog {
    Q_OBJECT
public:
    ProtocolDialog(const QString& title, QWidget *parent = nullptr);

    void progress_max(int max)
    {
        progressBar->setMaximum(max);
    }

    void progress(int p)
    {
        progressBar->setValue(p);
    }

public slots:
    void appendLog(const QString& line) {
        textEdit->append(line);
    }

private:
    QTextEdit *textEdit;
    QProgressBar* progressBar;
};

#endif //DBSCHOOL1_PROTOCOLDIALOG_H