//
// Created by shestero on 9/10/25.
//

#ifndef DBSCHOOL1_TOOLBARCALENDARWIDGET_H
#define DBSCHOOL1_TOOLBARCALENDARWIDGET_H

#include <QCalendarWidget>


class ToolBarCalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    explicit ToolBarCalendarWidget(QWidget* parent = nullptr);

};


#endif //DBSCHOOL1_TOOLBARCALENDARWIDGET_H