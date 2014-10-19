#ifndef FILESAVE_H
#define FILESAVE_H

#include "Window.h"

class FileType {
public:
    QString filterName;
    QString filterType;
    QString filter;
};

class QProcess;
class QProgressDialog;

class FileSave : public QObject
{
    Q_OBJECT
public:

    FileSave(Window *win) {m_window = win;}
    void run();

private slots:
    void onReadyReadStandardOutput();
    void onCanceled();
    void onFinished(int exitCode);

private:
    void saveWithProgress(QString command);
    void setProgress (int val);

    Window *m_window;
    QProcess *m_process = NULL;
    QProgressDialog *m_progressDialog;
};

#endif // FILESAVE_H
