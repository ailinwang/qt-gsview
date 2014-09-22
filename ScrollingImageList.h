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

    Document *document() const {return m_document;}
    void setDocument(Document *document) {m_document = document;}

    void hilightImage(int nImage);
    void goToPage (int nPage, bool evenIfVisible=false);

    void zoom (double scale, int nPage);
    void annot (bool showAnnotations);

    void buildImages();

    void setScale(double scale) {m_scale=scale;}
    virtual double getScale();

    virtual bool clickable() {return false;}

protected:
    QScrollArea *getScrollArea() {return m_scrollArea;}

public slots:
    void imagesBuiltSlot();
    void sliderReleasedSlot();
    void valueChangedSlot(int val);

signals:
    void imagesReady();

private:
    void renderVisibleImages();
    void renderImage(int index);
    void delayedRender();
    bool isImageVisible(int nPage);
    void rebuild (int nPage);

    QScrollArea *m_scrollArea = NULL;
    ImageWidget *m_images = NULL;
    bool m_imagesBuilt = false;
    Document *m_document = NULL;
    double m_scale = 1.0;
    bool m_shown = false;
    bool m_showAnnotations = true;
};

#endif  //  SCROLLINGIMAGELIST_H
