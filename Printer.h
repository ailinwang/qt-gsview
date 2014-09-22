#ifndef PRINTER_H
#define PRINTER_H

#include "Window.h"

class Printer
{
public:
    Printer(Window *win);

    void print();

private:
    Window *m_window = NULL;
};

#endif // PRINTER_H
