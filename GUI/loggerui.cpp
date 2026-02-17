#include "loggerui.h"
#include "ui_loggerui.h"


LoggerUI::LoggerUI(QWidget *parent)
    : QWidget(parent)//base class constructor call
    , ui(new Ui::LoggerUI)//Allocate a new generated UI helper object and store its pointer
{
    ui->setupUi(this);//Creates all widgets defined in loggerui.ui; Parents them to this (LoggerUI);Assigns pointers like connectQPushButton
    connect(ui->connectQPushButton, &QPushButton::clicked, this, &LoggerUI::connectButtonClicked);
    connect(ui->startQPushButton, &QPushButton::clicked,this, &LoggerUI::startButtonClicked);
    connect(ui->stopQPushButton, &QPushButton::clicked,this, &LoggerUI::stopButtonClicked);
    connect(ui->disconnectQPushButton, &QPushButton::clicked,this, &LoggerUI::disconnectButtonClicked);
    connect(ui->pauseQPushButton, &QPushButton::clicked,this, &LoggerUI::pauseButtonClicked);
    connect(ui->resumeQPushButton, &QPushButton::clicked,this, &LoggerUI::resumeButtonClicked);
    connect(ui->openQPushButton, &QPushButton::clicked,this, &LoggerUI::openButtonClicked);

    connect(ui->showDataQPushButton, &QPushButton::clicked,this, [this](){
        Q_EMIT showDataButtonClicked(ui->channelNameQLineEdit->text());
    });

    connect(ui->showLimitQPushButton, &QPushButton::clicked,this, [this](){
        Q_EMIT showLimitsButtonClicked(ui->pointNameQLineEdit->text());
    });


}


LoggerUI::~LoggerUI()
{
    delete ui;
}

void LoggerUI::onSendMeasurement(const MeasurementData &data)
{
    QDateTime qtTime =
        QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(
                data.timep.time_since_epoch()
                ).count()
            );
    QString timestamp =
        qtTime.toString("dd-MM-yyyy HH:mm:ss");


    QString valueStr;

    if (auto pVal = std::get_if<int>(&data.value)) {

        valueStr = QString::number(*pVal);
    }
    else if (auto pVal = std::get_if<double>(&data.value)) {

        valueStr = QString::number(*pVal, 'f', 2);
    }
    QString shownData = QString("%1 %2 %3 %4")
                            .arg(timestamp)
                            .arg(QString::fromStdString(data.channelName))
                            .arg(QString::fromStdString(data.pointName))
                            .arg(valueStr);
    ui->plainTextEdit->appendPlainText(shownData);

}


void LoggerUI::onShowData(ChannelMap& data)
{
    ui->plainTextEdit->clear();

    if (data.empty()) {
        ui->plainTextEdit->appendPlainText("No results found.");
        return;
    }

    for (auto const& [channelName, sensorMap] : data) {
        ui->plainTextEdit->appendPlainText(QString("Channel: %1")
                                               .arg(QString::fromStdString(channelName)));

        for (auto const& [sensorName, history] : sensorMap) {
            ui->plainTextEdit->appendPlainText(QString("Sensor: %1")
                                                   .arg(QString::fromStdString(sensorName)));

            for (auto const& [val, time] : history) {

                auto s = std::chrono::time_point_cast<std::chrono::seconds>(time);
                QDateTime qTime = QDateTime::fromSecsSinceEpoch(s.time_since_epoch().count());
                QString timeStr = qTime.toString("dd-MM-yyyy HH:mm:ss");

                QString valueStr;
                if (auto pVal = std::get_if<int>(&val)) {

                    valueStr = QString::number(*pVal);
                }
                else if (auto pVal = std::get_if<double>(&val)) {
                    valueStr = QString::number(*pVal, 'f', 2);
                }

                ui->plainTextEdit->appendPlainText(QString("%1 Value: %2")
                                                       .arg(timeStr, valueStr));
            }
        }
    }

}

