#include "ProtocolDialog.h"

#include <QVBoxLayout>

ProtocolDialog::ProtocolDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Протокол операции");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);

    resize(500, 300);
}