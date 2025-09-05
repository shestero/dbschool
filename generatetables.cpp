//
// Created by shestero on 9/5/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_GenerateTables.h" resolved

#include "generatetables.h"
#include "ui_generatetables.h"

#include <QDialogButtonBox>

GenerateTables::GenerateTables(const QDate& start, QWidget* parent) :
    QDialog(parent), ui(new Ui::GenerateTables)
{
    ui->setupUi(this);

    ui->calendarStart->setSelectedDate(start);
    ui->calendarEnd->setSelectedDate(start.addDays(30));
    ui->calendarStart->setMinimumDate(start);
    ui->calendarEnd->setMinimumDate(start);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GenerateTables::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GenerateTables::reject);

    connect(ui->calendarStart, &QCalendarWidget::selectionChanged, this, [this]() {
        QDate startDate = ui->calendarStart->selectedDate();
        // выставляем минимально допустимую дату конца
        ui->calendarEnd->setMinimumDate(startDate);
    });
    connect(ui->calendarEnd, &QCalendarWidget::selectionChanged, this, [this]() {
        QDate endDate = ui->calendarEnd->selectedDate();
        // выставляем максимально допустимую дату старта
        ui->calendarStart->setMaximumDate(endDate);
    });
}

GenerateTables::~GenerateTables()
{
    delete ui;
}