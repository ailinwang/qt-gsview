#include "FileInfoDialog.h"
#include "ui_FileInfoDialog.h"

FileInfoDialog::FileInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileInfoDialog)
{
    ui->setupUi(this);
}

FileInfoDialog::~FileInfoDialog()
{
    delete ui;
}
