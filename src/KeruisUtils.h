#pragma once
#include <QMainWindow>
#include "RedialMenu/RedialMenu.h"

class KeruisUtils : public QMainWindow {
    Q_OBJECT

public:
    KeruisUtils(QWidget* parent = nullptr);
    ~KeruisUtils();

    void contextMenuEvent(QContextMenuEvent *event) override {
        auto *menu = new RadialMenu(this);
        menu->move(event->globalPos() - QPoint(menu->width()/2, menu->height()/2));
        connect(menu, &RadialMenu::segmentClicked, this, [](int index) {
            qDebug() << "Segment clicked:" << index;
            // TODO: 根据 index 执行具体功能
        });
        menu->show();
    }

private:
    RadialMenu m_radialMenu;
};