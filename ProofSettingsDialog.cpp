#include <QFileDialog>

#include "QtUtil.h"

#include "ProofSettingsDialog.h"
#include "ui_ProofSettingsDialog.h"

ProofSettingsDialog::ProofSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProofSettingsDialog)
{
    ui->setupUi(this);
}

ProofSettingsDialog::~ProofSettingsDialog()
{
    delete ui;
}

int ProofSettingsDialog::getResolution()
{
    //  get the selected resolution.  It's the first word
    //  in the selected item's text
    QString text = ui->resolutionComboBox->currentText();
    QStringList parts = text.split(" ");
    QString tres = parts.at(0);
    bool ok;
    int res = tres.toInt(&ok);
    if (ok)
        return res;

    return 0;
}

QString ProofSettingsDialog::getPrintProfile()
{
    int index = ui->printProfileComboBox->currentIndex();
    if (index==0)
        return ui->printProfileComboBox->currentText();

    QVariant qval = ui->printProfileComboBox->currentData();
    return qval.toString();
}

QString ProofSettingsDialog::getSoftProfile()
{
    int index = ui->softProfileComboBox->currentIndex();
    if (index==0)
        return ui->softProfileComboBox->currentText();

    QVariant qval = ui->softProfileComboBox->currentData();
    return qval.toString();
}

void ProofSettingsDialog::on_choosePrintProfileButton_clicked()
{
    QString lastDir = QtUtil::getLastOpenFileDir();
    QFileDialog dialog(NULL, tr("Choose a Print Color Profile"), lastDir);
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
        //  get name and path of chosen file
        QString path = dialog.selectedFiles().at(0);
        QFileInfo fileInfo (path);
        QString name = fileInfo.baseName();

        //  add it if it's not in the list
        int index = ui->printProfileComboBox->findText(name);
        if (index < 0)
        {
            ui->printProfileComboBox->addItem(name, path);
            ui->printProfileComboBox->setCurrentText(name);
        }
    }
}

void ProofSettingsDialog::on_chooseSoftProfileButton_clicked()
{
    QString lastDir = QtUtil::getLastOpenFileDir();
    QFileDialog dialog(NULL, tr("Choose a Print Color Profile"), lastDir);
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
        //  get name and path of chosen file
        QString path = dialog.selectedFiles().at(0);
        QFileInfo fileInfo (path);
        QString name = fileInfo.baseName();

        //  add it if it's not in the list
        int index = ui->printProfileComboBox->findText(name);
        if (index < 0)
        {
            ui->softProfileComboBox->addItem(name, path);
            ui->softProfileComboBox->setCurrentText(name);
        }
    }
}
