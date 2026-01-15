#ifndef LOGGERUI_H
#define LOGGERUI_H

#include <QWidget>

namespace Ui {
class LoggerUI;
}

class LoggerUI : public QWidget
{
    Q_OBJECT

public:
    explicit LoggerUI(QWidget *parent = nullptr);
    ~LoggerUI();

Q_SIGNALS:
    void connectButtonClicked();


private:
    Ui::LoggerUI *ui;
};

#endif // LOGGERUI_H
