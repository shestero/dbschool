//
// Created by shestero on 8/7/25.
//

#include "ForeignKeyDelegate.h"
#include "CustomComboBoxSortFilterProxyModel.h"
#include <QComboBox>
#include <QDebug>

QWidget *ForeignKeyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    qDebug() << "createEditor";

    if (index.data().canConvert<int>()) {
        QString currentText = index.data(Qt::EditRole).toString();
        int currentIndex = index.data(Qt::UserRole).toInt();

        QComboBox* editor = new QComboBox(parent);
        CustomComboBoxSortFilterProxyModel *proxyModel = new CustomComboBoxSortFilterProxyModel(editor);
        proxyModel->setSourceModel(editor->model());
        editor->model()->setParent(proxyModel); // Reparent the original model to prevent deletion
        editor->setModel(proxyModel);

        bool selected = false;
        for (int r = 0; r < pFkModel->rowCount(); r++)
        {
            bool ok = false;
            int index = pFkModel->item(r)->text().toInt(&ok);
            if (ok)
            {
                QString text = pFkModel->item(r, 1)->text();
                editor->addItem(text, index);

                if (currentIndex == index)
                {
                    editor->setCurrentText(text);
                    selected = true;
                }
            }
        }

        if (!selected)
        {
            qDebug() << "Warning: item with selectedIndex=" << currentIndex << " was not found!";

            editor->addItem(currentText, currentIndex);
            editor->setCurrentText(currentText);
        }

        //connect(editor, &QComboBox::editingFinished, this, &ForeignKeyDelegate::commitAndCloseEditor);
        editor->model()->sort(0, Qt::AscendingOrder);
        return editor;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ForeignKeyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qDebug() << "setEditorData";
    //return QStyledItemDelegate::setEditorData(editor, index); // todo

    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        // get the index of the text in the combobox that matches the current value of the itenm
        QString currentText = index.data(Qt::EditRole).toString();
        int idx = index.data(Qt::UserRole).toInt();
        int cbIndex = cb->findText(currentText);
        qDebug() << "setEditorData: cbIndex=" << cbIndex << ", index=" << idx << ", text=" << currentText;
        // if it is valid, adjust the combobox
        if(cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

/*
void ForeignKeyDelegate::commitAndCloseEditor()
{
    QComboBox *editor = qobject_cast<QComboBox *>(sender());
    qDebug() << "currentText=" << editor->currentText();
}
*/