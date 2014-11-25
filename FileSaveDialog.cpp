#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QApplication>

#include "FileSaveDialog.h"

FileSaveDialog::FileSaveDialog (QWidget *parent,
                            const QString &caption,
                            const QString &directory,
                            const QString &filter) : QFileDialog(parent, caption, directory, filter)
{
    //  connect to signal when user changes filter
    connect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));
}

void FileSaveDialog::onFilterSelected(const QString &filter)
{
    //  user changed filter.
    //  prevent them from using the separator
    if (filter.compare(m_separator)==0)
    {
        disconnect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));
        this->selectNameFilter(m_fallback);
        connect (this, SIGNAL(filterSelected(const QString &)), this, SLOT(onFilterSelected(const QString &)));
    }
}

void FileSaveDialog::show()
{
    //  customize the dialog
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
