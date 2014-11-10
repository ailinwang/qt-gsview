#include <QFileDialog>
#include <QPushButton>
#include <QDebug>
#include <QGridLayout>

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
    //  user changed filter.  show the profiles UI
}
