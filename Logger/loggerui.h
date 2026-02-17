#ifndef LOGGERUI_H
#define LOGGERUI_H

#include <QWidget>

//namespace for Q generated UI classes
namespace Ui {
class LoggerUI;
}

//inherits QWidget classes functionality
class LoggerUI : public QWidget
{
    //macro, enables Q functionality(signal-slot)
    Q_OBJECT

public:
    explicit LoggerUI(QWidget *parent = nullptr);
    ~LoggerUI();

Q_SIGNALS:
    void connectButtonClicked();
    void disconnectButtonClicked();
    void startButtonClicked();
    void stopButtonClicked();
    void pauseButtonClicked();
    void resumeButtonClicked();
    void exitButtonClicked();
    void openButtonClicked();
    void closeButtonClicked();
    void showDataButtonClicked(const QString &address);
    void showLimitsButtonClicked();



private:
    Ui::LoggerUI *ui;
};

#endif // LOGGERUI_H
