#include "MessagesDialog.h"
#include "ui_MessagesDialog.h"

#include <QString>

QString MessagesDialog::m_messages;

MessagesDialog::MessagesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessagesDialog)
{
    ui->setupUi(this);

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(onClear()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOK()));
}

MessagesDialog::~MessagesDialog()
{
    delete ui;
}

void MessagesDialog::addMessage(QString message)
{
    m_messages += message;
}

void MessagesDialog::onClear()
{
    ui->plainTextEdit->clear();
    m_messages = "";
}

void MessagesDialog::onOK()
{
    hide();
}

void MessagesDialog::showEvent(QShowEvent *event)
{
    ui->plainTextEdit->setPlainText(m_messages);
    QDialog::showEvent(event);
}
