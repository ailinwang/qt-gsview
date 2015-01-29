#ifndef PRINTER_H
#define PRINTER_H

#include "Window.h"
#ifdef USE_CUPS
#include <cups/cups.h>
#endif

#include <QObject>

//  here is a class to be used in a QThread

class PrintWorker : public QObject {
    Q_OBJECT

public:
    PrintWorker(Window *window, QPrinter *printer, QString pageRange, bool rotate, double userScale);
    ~PrintWorker();
    void kill() {m_killed=true;}

public slots:
    void process();

signals:
    void pagePrinted(int pageNumber);
    void finished();

private:

    QPrinter *m_printer;
    QString m_printRange;
    Window *m_window;
    bool m_killed = false;
    bool m_rotate = false;
    double m_userScale = 1.0;
};

class QProgressDialog;

class Printer : public QObject
{
    Q_OBJECT
public:
    explicit Printer(QObject *parent = 0);

    void print();
    void setWindow (Window *win) {m_window=win;}

    void pdfPrint (QPrinter *printer, QString path, QString pageRange, int copies, bool landscape, bool rotate, double userScale, QPair<QString,QSizeF> paperSize);
    void bitmapPrint (QPrinter *printer, QString pageRange, int copies, bool landscape, bool rotate, double userScale, QPair<QString,QSizeF> paperSize);

    static QList<int> listFromRange(QString rangeList, int maxPage);

signals:

public slots:
    void pagePrinted(int nPage);
    void printFinished();
    void onCanceled();

private:
    void setProgress (int val);

    QPrinter *m_printer = NULL;
    Window *m_window = NULL;
    int m_jobID = 0;
    QThread* m_printThread = NULL;
    PrintWorker *m_printWorker = NULL;
    QProgressDialog *m_progress = NULL;
    int m_pagesToPrint = 0;
    bool m_killed = false;
};

#endif // PRINTER_H
