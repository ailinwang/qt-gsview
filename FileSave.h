#ifndef FILESAVE_H
#define FILESAVE_H

#include "Window.h"

class FileType {
public:
    QString filterName;
    QString filterType;
};

class FileSave
{
public:
    FileSave(Window *win) {m_window = win;}
    void run();

private:
    Window *m_window;

};

#endif // FILESAVE_H
