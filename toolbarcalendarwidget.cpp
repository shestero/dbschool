//
// Created by shestero on 9/10/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ToolBarCalendarWidget.h" resolved

#include "toolbarcalendarwidget.h"

#include <QTableView>

ToolBarCalendarWidget::ToolBarCalendarWidget(QWidget* parent) :
    QCalendarWidget(parent)
{
    setMaximumSize(220, 30);
    setLocale(QLocale(QLocale::Russian, QLocale::Russia));
    setHorizontalHeaderFormat(QCalendarWidget::NoHorizontalHeader);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setGridVisible(false);
    setNavigationBarVisible(true);
    setDateEditEnabled(true);
    setAutoFillBackground(false);

    setStyleSheet(R"(
        QCalendarWidget {
            background: transparent;
        }

        QWidget#qt_calendar_navigationbar {
            background-color: white;       /* фон */
        }
        QToolButton {
            background: transparent;       /* кнопки прозрачные */
            color: black;                  /* цвет текста/стрелок */
        }
        QToolButton:hover {
            background: lightgray;         /* фон при наведении */
            color: black;                  /* текст остаётся чёрным */
        }
    )");
    if (QTableView *table = findChild<QTableView*>()) { // hook
        table->hide();
    }
    setContentsMargins(5, 0, 5, 0);
}
