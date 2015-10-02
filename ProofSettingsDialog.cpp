#include <QFileDialog>

#include "QtUtil.h"

#include "ProofSettingsDialog.h"
#include "ui_ProofSettingsDialog.h"

QList<QString> ProofSettingsDialog::m_softProfiles;
QList<QString> ProofSettingsDialog::m_printProfiles;

ProofSettingsDialog::ProofSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProofSettingsDialog)
{
    ui->setupUi(this);

    //  set the window title
    setWindowTitle(tr("Proof Settings"));

    //  add previously selected profiles
    for (int i=0; i<m_softProfiles.count(); ++i)
    {
        QString path = m_softProfiles[i];
        QString name = QFileInfo(path).baseName();
        ui->softProfileComboBox->addItem(name, path);
        //ui->softProfileComboBox->setCurrentText(name);
    }

    for (int i=0; i<m_printProfiles.count(); ++i)
    {
        QString path = m_printProfiles[i];
        QString name = QFileInfo(path).baseName();
        ui->printProfileComboBox->addItem(name, path);
        //ui->printProfileComboBox->setCurrentText(name);
    }

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
        return QString("");

    QVariant qval = ui->printProfileComboBox->currentData();
    return qval.toString();
}

QString ProofSettingsDialog::getDisplayProfile()
{
    int index = ui->softProfileComboBox->currentIndex();
    if (index==0)
        return QString("");

    QVariant qval = ui->softProfileComboBox->currentData();
    return qval.toString();
}

QFileDialog *ProofSettingsDialog::makeICCDialog(QString title)
{
    QString lastDir = QtUtil::getLastOpenFileDir();
    QFileDialog *dialog = new QFileDialog(NULL, title, lastDir);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setNameFilter(tr("ICC Profile Files (*.icc *.icm)"));
    dialog->setWindowModality(Qt::ApplicationModal);
    return dialog;
}

void ProofSettingsDialog::on_choosePrintProfileButton_clicked()
{    
    QFileDialog *dialog = makeICCDialog(tr("Choose a Print Color Profile"));
    dialog->show();
    int result = dialog->exec();

    if (result == QDialog::Accepted)
    {
        //  get name and path of chosen file
        QString path = dialog->selectedFiles().at(0);
        QString name = QFileInfo(path).baseName();

        //  add it if it's not in the list
        int index = ui->printProfileComboBox->findText(name);
        if (index < 0)
        {
            ui->printProfileComboBox->addItem(name, path);
            ui->printProfileComboBox->setCurrentText(name);
            m_printProfiles.append(path);
        }
    }
}

void ProofSettingsDialog::on_chooseSoftProfileButton_clicked()
{
    QFileDialog *dialog = makeICCDialog(tr("Choose a Soft Color Profile"));
    dialog->show();
    int result = dialog->exec();

    if (result == QDialog::Accepted)
    {
        //  get name and path of chosen file
        QString path = dialog->selectedFiles().at(0);
        QString name = QFileInfo(path).baseName();

        //  add it if it's not in the list
        int index = ui->printProfileComboBox->findText(name);
        if (index < 0)
        {
            ui->softProfileComboBox->addItem(name, path);
            ui->softProfileComboBox->setCurrentText(name);
            m_softProfiles.append(path);
        }
    }
}
