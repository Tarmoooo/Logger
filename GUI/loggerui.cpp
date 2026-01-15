#include "loggerui.h"
#include "ui_loggerui.h"

LoggerUI::LoggerUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoggerUI)
{
    ui->setupUi(this);
    connect(ui->connectQPushButton, &QPushButton::clicked, this, &LoggerUI::connectButtonClicked);
}

LoggerUI::~LoggerUI()
{
    delete ui;
}
