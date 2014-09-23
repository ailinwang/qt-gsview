#ifndef PRINTER_H
#define PRINTER_H

#include "Window.h"
#include <cups/cups.h>
#include <QTimer>

#include <QObject>

class Printer : public QObject
{
    Q_OBJECT
public:
    explicit Printer(QObject *parent = 0);

    void print();
    void setWindow (Window *win) {m_window=win;}

    void pdfPrint (QPrinter *printer, QString path, int fromPage, int toPage);
    void bitmapPrint (QPrinter *printer, int fromPage, int toPage);

signals:

public slots:
    void monitor();

private:
    Window *m_window = NULL;
    int m_jobID = 0;
    QTimer *m_monitor = NULL;
};

#endif // PRINTER_H
