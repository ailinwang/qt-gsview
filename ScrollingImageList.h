#ifndef SCROLLINGIMAGELIST_H
#define SCROLLINGIMAGELIST_H


#include <QObject>
#include <QScrollArea>

#include "ui_Window.h"
#include "ImageWidget.h"
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
    void goToPage (int nPage);

    void setScale(double scale) {m_scale=scale;}
    void zoom(double scale, int nPage);

public slots:
    void imagesBuiltSlot();
    void sliderReleasedSlot();
    void valueChangedSlot(int val);

signals:
    void imagesReady();

private:
    void renderVisibleImages();

    QScrollArea *m_scrollArea = NULL;
    ImageWidget *m_images = NULL;
    bool m_imagesBuilt = false;
    Document *m_document = NULL;
    double m_scale = -1;
};

#endif  //  SCROLLINGIMAGELIST_H
