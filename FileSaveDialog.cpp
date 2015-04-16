#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QApplication>

#include "FileSaveDialog.h"
#include "ui_filedialogextension.h"

FileSaveDialog::FileSaveDialog (QWidget *parent,
                            const QString &caption,
                            const QString &directory,
                            const QString &filter) : QFileDialog(parent, caption, directory, filter)
{
    //  connect to signal when user changes filter
    connect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));

    //  bring in the options form
    ui = new Ui::Form();
    ui->setupUi(this);
}

void FileSaveDialog::onFilterSelected(const QString &filter)
{
    //  user changed filter.
    //  prevent them from using the separator
    //  Qt: is there a better way to do this?
    if (filter.compare(m_separator)==0)
    {
        disconnect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));
        this->selectNameFilter(m_fallback);
        connect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));
    }

//    if (filter.contains("*.ps"))
//    {
//        ui->verticalLayoutWidget->setVisible(true);
//    }
//    else
//    {
//        ui->verticalLayoutWidget->setVisible(false);
//    }


}

void FileSaveDialog::show()
{
    //  customize the dialog
    this->layout()->addWidget(ui->verticalLayoutWidget);
    ui->verticalLayoutWidget->setVisible(false);

    //  show it
    QFileDialog::show();
}

void FileSaveDialog::setSeparatorFilter(QString val)
{
    m_separator = val;
}

void FileSaveDialog::setFallbackFilter(QString val)
{
    m_fallback = val;
}
