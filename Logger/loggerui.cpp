#include "loggerui.h"
#include "ui_loggerui.h"

LoggerUI::LoggerUI(QWidget *parent)
    : QWidget(parent)//base class constructor call
    , ui(new Ui::LoggerUI)//Allocate a new generated UI helper object and store its pointer
{
    ui->setupUi(this);//Creates all widgets defined in loggerui.ui; Parents them to this (LoggerUI);Assigns pointers like connectQPushButton
    connect(ui->connectQPushButton, &QPushButton::clicked, this, &LoggerUI::connectButtonClicked);
    connect(ui->disconnectQPushButton, &QPushButton::clicked,this, &LoggerUI::disconnectButtonClicked);
    //connect(ui->disconnectQPushButton, &QPushButton::clicked,this, &LoggerUI::disconnectButtonClicked);
    //connect(ui->channelNameQLineEdit, &QLineEdit::textEdited,this, &LoggerUI::ButtonClicked);

    connect(ui->showDataQPushButton, &QPushButton::clicked,this,
            [this]() {QString address = ui->channelNameQLineEdit->text();
                emit showDataButtonClicked(address);});

}

LoggerUI::~LoggerUI()
{
    delete ui;
}
