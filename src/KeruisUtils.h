#pragma once
#include <QMainWindow>

class KeruisUtils : public QMainWindow {
    Q_OBJECT

public:
    KeruisUtils(QWidget* parent = nullptr);
    ~KeruisUtils();

    void contextMenuEvent(QContextMenuEvent *event) override {

    }

private:
};