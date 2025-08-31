#include "ProtocolDialog.h"

#include <QVBoxLayout>

ProtocolDialog::ProtocolDialog(const QString& title, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Execution: %1").arg(title));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(textEdit = new QTextEdit(this));
    layout->addWidget(progressBar = new QProgressBar(this));

    textEdit->setReadOnly(true);

    resize(500, 300);
}