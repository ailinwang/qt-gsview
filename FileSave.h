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
class QBasicTimer;

class FileSave : public QObject
{
    Q_OBJECT
public:
//    explicit Printer(QObject *parent = 0);

    FileSave(Window *win) {m_window = win;}
    void run();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void onReadyReadStandardOutput();
    void onCanceled();

private:
    bool saveWithProgress(QString command);

    Window *m_window;
    QProcess *m_process = NULL;
    QProgressDialog *m_progressDialog;
    QBasicTimer *m_timer;
};

#endif // FILESAVE_H
