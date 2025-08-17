//
// Created by shestero on 8/7/25.
//

#ifndef FOREIGNKEYDELEGATE_H
#define FOREIGNKEYDELEGATE_H

#include <QStyledItemDelegate>
#include "csvtablemodel.h"


class ForeignKeyDelegate : public QStyledItemDelegate
{
   Q_OBJECT // Required for moc

public:
   explicit ForeignKeyDelegate(CSVTableModel* foreign, QObject *parent = nullptr)
   : QStyledItemDelegate(parent), pFkModel(foreign) {}

   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   void setEditorData(QWidget *editor, const QModelIndex &index) const override;

public slots:
   //void commitAndCloseEditor();

private:
   CSVTableModel* pFkModel;
};



#endif //FOREIGNKEYDELEGATE_H
