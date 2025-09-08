//
// Created by shestero on 9/8/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_QProtocolWidget.h" resolved

#include "ProtocolWidget.h"

#include <QFontDatabase>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>


ProtocolWidget::ProtocolWidget(QWidget* parent) :
    QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel(tr("Log:")));
    headerLayout->addStretch();
    QPushButton* copyBtn = new QPushButton(
        QApplication::style()->standardIcon(QStyle::SP_FileDialogContentsView), tr("Copy"));
    QPushButton* clearBtn = new QPushButton(
        QApplication::style()->standardIcon(QStyle::SP_DialogResetButton), tr("Clear"));
    headerLayout->addWidget(copyBtn);
    headerLayout->addWidget(clearBtn);
    layout->addLayout(headerLayout);
    layout->addWidget(textEdit = new QTextEdit(this));
    layout->addWidget(progressBar = new QProgressBar(this));
    //layout->setMargin(5);

    textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textEdit->setReadOnly(true);

    connect(copyBtn, &QPushButton::clicked, this, &ProtocolWidget::onCopy);
    connect(clearBtn, &QPushButton::clicked, this, &ProtocolWidget::onClear);

    writeTimestamp(tr("Program started"));
}

ProtocolWidget::~ProtocolWidget()
{
}