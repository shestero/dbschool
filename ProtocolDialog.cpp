#include "ProtocolDialog.h"

#include <QFontDatabase>
#include <QVBoxLayout>

ProtocolDialog::ProtocolDialog(const QString& title, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Execution: %1").arg(title));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(textEdit = new QTextEdit(this));
    layout->addWidget(progressBar = new QProgressBar(this));

    textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textEdit->setReadOnly(true);

    writeTimestamp();

    resize(640, 400);
}