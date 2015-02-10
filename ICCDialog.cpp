#include <QtWidgets>

#include "ICCDialog.h"
#include "ui_ICCDialog.h"

ICCDialog::ICCDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ICCDialog)
{
    ui->setupUi(this);
    ui->selectedProfile->setWordWrap(true);
}

ICCDialog::~ICCDialog()
{
    delete ui;
}

void ICCDialog::show()
{
    if (m_icc_path.isEmpty())
        ui->selectedProfile->setText(tr("None"));
    else
        ui->selectedProfile->setText(m_icc_path);

    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
}

void ICCDialog::on_selectButton_clicked()
{
    //  get the last-visited directory.  Default is desktop
    QSettings settings;
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString lastDir  = settings.value("LastOpenFileDir", desktopLocations.first()).toString();

    //  create a dialog for choosing a file
    QFileDialog dialog(NULL, tr("Choose a Color Profile"), lastDir);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("ICC Profile Files (*.icc *.icm)"));
    dialog.setWindowModality(Qt::ApplicationModal);

    //  show and run the dialog
    dialog.show();
    int result = dialog.exec();

    if (result == QDialog::Accepted)
    {
        //  remember the last-visited directory
        settings.setValue("LastOpenFileDir", dialog.directory().absolutePath());

        m_icc_path = dialog.selectedFiles().first();
        ui->selectedProfile->setText(m_icc_path);
    }

}
