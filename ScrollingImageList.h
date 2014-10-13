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

    Document *document() const {return m_document;}
    void setDocument(Document *document) {m_document = document;}

    void hilightImage(int nImage);
    void goToPage (int nPage, bool evenIfVisible=false);

    void zoom (double scale, int nPage);
    void annot (bool showAnnotations);
    void links (bool showLinks);

    void buildImages();

    void setScale(double scale) {m_scale=scale;}
    virtual double getScale();

    virtual bool thumbnails() {return false;}

    bool eventFilter(QObject *target, QEvent *event);

    void reRender();

protected:
    QScrollArea *getScrollArea() {return m_scrollArea;}
    ImageWidget *images() {return m_images;}

public slots:
    void sliderReleasedSlot();
    void valueChangedSlot(int val);
    void onImagesReady2();

signals:
    void imagesReady();

private:
    void renderVisibleImages();
    void renderImage(int index);
    bool isImageVisible(int nPage);
    void rebuild (int nPage);
    void onImagesReady();

    QScrollArea *m_scrollArea = NULL;
    ImageWidget *m_images = NULL;
    bool m_imagesBuilt = false;
    Document *m_document = NULL;
    double m_scale = 1.0;
    bool m_showAnnotations = true;
    bool m_showLinks = false;

    bool m_zooming = false;
    double m_zoomRatio;
};

#endif  //  SCROLLINGIMAGELIST_H
