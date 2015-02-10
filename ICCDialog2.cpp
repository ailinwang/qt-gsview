#include <QtWidgets>
#include <QSettings>

#include "ICCDialog2.h"
#include "ui_ICCDialog2.h"

QString ICCDialog2::m_rgbProfile;
QString ICCDialog2::m_cmykProfile;
QString ICCDialog2::m_grayProfile;

//  don't translate these
#define RGB_SETTINGS_KEY  "RGB Profile"
#define CMYK_SETTINGS_KEY "CMYK Profile"
#define GRAY_SETTINGS_KEY "Gray Profile"

ICCDialog2::ICCDialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ICCDialog2)
{
    ui->setupUi(this);
}

ICCDialog2::~ICCDialog2()
{
    delete ui;
}

void ICCDialog2::show()
{
    //  get previous values from QSettings
    QSettings settings;
    m_rgbProfile  = settings.value(RGB_SETTINGS_KEY, QString("")).toString();
    m_cmykProfile = settings.value(CMYK_SETTINGS_KEY, QString("")).toString();
    m_grayProfile = settings.value(GRAY_SETTINGS_KEY, QString("")).toString();

    refreshUI();

    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
}

void ICCDialog2::on_grayButton_clicked()
{
    QString result = askForProfile();

    if (!result.isEmpty())
        m_grayProfile = result;

    refreshUI();
}

void ICCDialog2::on_rgbButton_clicked()
{
    QString result = askForProfile();

    if (!result.isEmpty())
        m_rgbProfile = result;

    refreshUI();
}

void ICCDialog2::on_cmykButton_clicked()
{
    QString result = askForProfile();

    if (!result.isEmpty())
        m_cmykProfile = result;

    refreshUI();
}

void ICCDialog2::on_okButton_clicked()
{
    //  save current values using QSettings
    QSettings settings;
    settings.setValue(RGB_SETTINGS_KEY,  m_rgbProfile);
    settings.setValue(CMYK_SETTINGS_KEY, m_cmykProfile);
    settings.setValue(GRAY_SETTINGS_KEY, m_grayProfile);

    close();
}

void ICCDialog2::refreshUI()
{    
    QString styleEmpty("QLabel { color:red; border: 1px solid; border-color: red;}");
    QString styleNotEmpty("QLabel { color:black; border: 0px none;}");
    QString emptyText(tr("Not Set"));

    ui->rgbLabel->setStyleSheet(styleNotEmpty);
    if (m_rgbProfile.isEmpty())
    {
        ui->rgbLabel->setText(emptyText);
        ui->rgbLabel->setStyleSheet(styleEmpty);
    }
    else
        ui->rgbLabel->setText(m_rgbProfile);

    ui->cmykLabel->setStyleSheet(styleNotEmpty);
    if (m_cmykProfile.isEmpty())
    {
        ui->cmykLabel->setText(emptyText);
        ui->cmykLabel->setStyleSheet(styleEmpty);
    }
    else
        ui->cmykLabel->setText(m_cmykProfile);

    ui->grayLabel->setStyleSheet(styleNotEmpty);
    if (m_grayProfile.isEmpty())
    {
        ui->grayLabel->setText(emptyText);
        ui->grayLabel->setStyleSheet(styleEmpty);
    }
    else
        ui->grayLabel->setText(m_grayProfile);
}

QString ICCDialog2::askForProfile()
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

        return dialog.selectedFiles().first();
    }

    return QString("");
}
