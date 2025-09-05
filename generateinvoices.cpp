//
// Created by shestero on 9/5/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GenerateInvoices.h" resolved

#include "generateinvoices.h"
#include "ui_generateinvoices.h"

#include <QTableView>


GenerateInvoices::GenerateInvoices(QWidget* parent) :
    QDialog(parent), ui(new Ui::GenerateInvoices)
{
    ui->setupUi(this);

    if (QTableView *table = ui->calendarWidget->findChild<QTableView*>()) {
        table->hide();
    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GenerateInvoices::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GenerateInvoices::reject);
}

GenerateInvoices::~GenerateInvoices()
{
    delete ui;
}