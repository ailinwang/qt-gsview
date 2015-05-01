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

    virtual void zoom (double scale);
    void annot (bool showAnnotations);
    void links (bool showLinks);

    void buildImages();

    void setScale(double scale) {m_scale=scale;}
    virtual double getScale();

    virtual bool thumbnails() {return false;}

    bool eventFilter(QObject *target, QEvent *event);

    void reRender();

    void cleanup();

    virtual void onScrollChange() {}
    virtual void onSliderReleased() {}
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}

    QScrollArea *getScrollArea() {return m_scrollArea;}

    bool isSliderDown();

    virtual void wheelZoomIn() {}
    virtual void wheelZoomOut() {}

    virtual void setBackgroundColor() {}

    void zoomLive (double scale);
    void startLiveZoom(int page);
    void endLiveZoom();

protected:
    ImageWidget *images() {return m_images;}
    Document *m_document = NULL;
    bool isImageVisible(int nPage);

public slots:
    void sliderReleasedSlot();
    void valueChangedSlot(int val);
    void onRenderTimer();
    virtual void onScrollingTimer() {}

signals:
    void imagesReady();

private:
    void renderVisibleImages(bool lowRes=false);
    void renderImage(int index, bool lowRes);
    void onImagesReady();
    void renderVisibleImagesLow();

    QScrollArea *m_scrollArea = NULL;
    ImageWidget *m_images = NULL;
    bool m_imagesBuilt = false;
    double m_scale = 1.0;
    bool m_showAnnotations = true;
    bool m_showLinks = false;
    QTimer *m_rendertimer = NULL;

    double start_scale;
    int start_page;
    bool liveScrolling = false;
};

#endif  //  SCROLLINGIMAGELIST_H
