#include <QtWidgets>

#include "Thumbnail.h"
#include "Window.h"

Thumbnail::Thumbnail(QWidget *parent) :
    QPushButton(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(clicked()));
}

void Thumbnail::clicked()
{
    Window *w = (Window *) m_window;
    w->clickedThumb(m_pageNumber);
}
