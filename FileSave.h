#ifndef FILESAVE_H
#define FILESAVE_H

#include "Window.h"

class FileType {
public:
    QString filterName;
    QString filterType;
    QString filter;
    bool needsProfile;
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
    void saveWithProgress(QString options, QString src, QString dst);
    void setProgress (int val);
    void saveAsText (QString dst, int type);

    Window *m_window;
    QProcess *m_process = NULL;
    QProgressDialog *m_progressDialog;

    QString m_dst;
    QString m_tmp;

    QString m_icc_path;
};

#endif // FILESAVE_H
