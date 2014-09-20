#ifndef SCROLLINGIMAGELIST_H
#define SCROLLINGIMAGELIST_H


#include <QObject>
#include <QScrollArea>

#include "ui_Window.h"
#include "Thumbnail.h"
#include "Document.h"

class ScrollingImageList : public QObject
{
    Q_OBJECT

public:
    explicit ScrollingImageList(QObject *parent = 0);

    QScrollArea *scrollArea() const;
    void setScrollArea(QScrollArea *scrollArea);

    void show();
    void hide();

    bool imagesBuilt() const;
    void setImagesBuilt(bool imagesBuilt);

    void buildImages();

    Document *document() const;
    void setDocument(Document *document);

    void hilightImage(int nImage);

public slots:
    void imagesBuiltSlot();
    void sliderReleasedSlot();

signals:
    void imagesReady();

private:
    void renderVisibleImages();

    QScrollArea *m_scrollArea = NULL;
    Thumbnail *m_images = NULL;
    bool m_imagesBuilt = false;
    Document *m_document = NULL;
};

#endif  //  SCROLLINGIMAGELIST_H
