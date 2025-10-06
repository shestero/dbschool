//
// Created by shestero on 8/6/25.
//

#ifndef CSVTABLEMODEL_H
#define CSVTABLEMODEL_H

#include <QStandardItemModel>
#include <QMap>

class TableView;

class CSVTableModel : public QStandardItemModel {
Q_OBJECT

public:
    explicit CSVTableModel(QObject *parent, const QString& file_name);
    explicit CSVTableModel(QObject *parent, const QString& file_name, int fkColumn, CSVTableModel *pFkModel);

    ~CSVTableModel() override;

    Qt::ItemFlags flags ( const QModelIndex & index ) const override;

    //int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    //QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const QString file_name;

    void load();

    QMap<int, QString> dictionary(int titleCol = 1) const;
    QMap<int, int> codeDictionary(int titleCol = 1) const;

protected:
    QMap<int, QMap<int, QString>> foreign;
    QMap<int, QMap<QString, int>> back;

public slots:
    void save();

};


#endif //CSVTABLEMODEL_H
