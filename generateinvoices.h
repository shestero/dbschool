//
// Created by shestero on 9/5/25.
//

#ifndef DBSCHOOL1_GENERATEINVOICES_H
#define DBSCHOOL1_GENERATEINVOICES_H

#include <QDialog>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class GenerateInvoices;
}

QT_END_NAMESPACE

class GenerateInvoices : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateInvoices(QWidget* parent = nullptr);
    ~GenerateInvoices() override;

private:
    Ui::GenerateInvoices* ui;
};


#endif //DBSCHOOL1_GENERATEINVOICES_H