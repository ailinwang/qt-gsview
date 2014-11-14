#ifndef EXTRACTPAGESDIALOG_H
#define EXTRACTPAGESDIALOG_H

#include <QDialog>

class Window;
class QProcess;
class QProgressDialog;

class device_t
{
public:
    int index;
    QString name;
    QString label;
    QString extension;
    QString command;
    QString paging;
};

namespace Ui {
class ExtractPagesDialog;
}

class ExtractPagesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractPagesDialog(QWidget *parent = 0);
    ~ExtractPagesDialog();

    void run (Window *win);

private slots:
    void on_cancelButton_clicked();
    void on_extractButton_clicked();
    void on_allButton_clicked();
    void on_evenButton_clicked();
    void on_oddButton_clicked();
    void on_noneButton_clicked();

    void onReadyReadStandardOutput();
    void onCanceled();
    void onFinished(int exitCode);

private:
    void doSave();
    void doSaveMupdf();
    void doSaveGs();
    void doSave2();
    void setProgress (int val);
    void startCommand(QString command);

    Ui::ExtractPagesDialog *ui;
    Window *m_window=NULL;
    QString m_options;
    QString m_resolution;
    device_t m_device;
    QString m_destination;
    QProcess *m_process=NULL;
    QProgressDialog *m_progressDialog=NULL;
    std::vector<QString> m_commands;
    int m_currentCommand;
    bool m_contiguous;
    int m_firstPage;
    int m_lastPage;
};

#endif // EXTRACTPAGESDIALOG_H
