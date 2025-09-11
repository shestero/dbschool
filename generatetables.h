//
// Created by shestero on 9/5/25.
//

#ifndef DBSCHOOL1_GENERATETABLES_H
#define DBSCHOOL1_GENERATETABLES_H

// TODO: DELETE

#include <QDialog>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class GenerateTables;
}

QT_END_NAMESPACE

class GenerateTables : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateTables(const QDate& start, QWidget* parent = nullptr);
    ~GenerateTables() override;

private:
    Ui::GenerateTables* ui;
};


#endif //DBSCHOOL1_GENERATETABLES_H