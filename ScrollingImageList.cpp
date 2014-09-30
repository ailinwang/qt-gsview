#include "Window.h"
#include "ScrollingImageList.h"
#include "QtUtil.h"

#include <QAbstractSlider>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>
#include <QDebug>

ScrollingImageList::ScrollingImageList(QObject *parent) :
    QObject(parent)
{
}

QScrollArea *ScrollingImageList::scrollArea() const
{
    return m_scrollArea;
}

void ScrollingImageList::setScrollArea(QScrollArea *scrollArea)
{
    m_scrollArea = scrollArea;

    QAbstractSlider *slider = (QAbstractSlider *) m_scrollArea->verticalScrollBar();
    connect (slider, SIGNAL(sliderReleased()), this, SLOT(sliderReleasedSlot()));
    connect (slider, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)));

    //  install an event filter on the scrolling area.
    QWidget* contentWidget = m_scrollArea->widget();
    contentWidget->installEventFilter(this);
}

void ScrollingImageList::show()
{
    if (NULL != m_scrollArea)
        m_scrollArea->show();
}

void ScrollingImageList::hide()
{
    if (NULL != m_scrollArea)
        m_scrollArea->hide();
}

double ScrollingImageList::getScale()
{
    return m_scale;
}

void ScrollingImageList::buildImages()
{
    if (!m_imagesBuilt)
    {
        //  create an array of images
        int nPages = m_document->GetPageCount();
        m_images = new ImageWidget[nPages]();

        //  set up scrolling area
        QWidget* contentWidget = m_scrollArea->widget();
        contentWidget->setLayout(new QVBoxLayout(contentWidget));
        contentWidget->layout()->setContentsMargins(0,0,0,0);

        double theScale = getScale();
        setScale(theScale);

        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize(i, theScale, &pageSize);

            m_images[i].setFixedWidth(pageSize.X);
            m_images[i].setFixedHeight(pageSize.Y);
            m_images[i].setPage(i);
            m_images[i].setScale(theScale);
            m_images[i].setPageSize(pageSize);
            m_images[i].setBackgroundRole(QPalette::Dark);
            m_images[i].setClickable(clickable());

            contentWidget->layout()->addWidget(&(m_images[i]));
        }

        m_imagesBuilt = true;
    }
}

void ScrollingImageList::annot (bool showAnnotations)
{
    m_showAnnotations = showAnnotations;
    reRender();
}

void ScrollingImageList::links (bool showLinks)
{
    m_showLinks = showLinks;
    reRender();
}

void ScrollingImageList::reRender()
{
    //  just re-render any images that are currently rendered
    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        if (m_images[i].rendered())
            renderImage(i);
    }
}

void ScrollingImageList::rebuild (int nPage)
{
    //  resize and clear  all the images and mark as not rendered
    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        point_t pageSize;
        m_document->GetPageSize(i, m_scale, &pageSize);

        m_images[i].setFixedWidth(pageSize.X);
        m_images[i].setFixedHeight(pageSize.Y);
        m_images[i].setScale(m_scale);
        m_images[i].setPageSize(pageSize);

//        m_images[i].clear();

        //  when we get here, it's probably because we're zooming.
        //  so, first just substitute a scaled version of the old pixmap.
        //  then later, when the rendering tkes place, it will be replaced
        //  with a better version.  But in the meantime, the zooming
        //  will appear instantaneously.

        const QPixmap *pm = m_images[i].pixmap();
        if (pm)
        {
            QPixmap scaledPixmap = pm->scaled(m_images[i].size(), Qt::KeepAspectRatio);
            m_images[i].setPixmap(scaledPixmap);
        }

        m_images[i].setRendered(false);
        m_images[i].setBackgroundRole(QPalette::Dark);
    }

    //  go to the given page and render visible.
    goToPage(nPage);
}

void ScrollingImageList::zoom (double theScale, int nPage)
{
    if (theScale != m_scale)
    {
        //  set the new value
        m_scale = theScale;

        //  do the rebuild
        rebuild (nPage);
    }
}


void ScrollingImageList::onImagesReady()
{
    //  draw the images that can be seen
    renderVisibleImages();

    //  notify that we're ready
    emit imagesReady();
}

void ScrollingImageList::sliderReleasedSlot()
{
    renderVisibleImages();
}

void ScrollingImageList::valueChangedSlot(int val)
{
    QAbstractSlider *slider = (QAbstractSlider *) m_scrollArea->verticalScrollBar();
    if (!slider->isSliderDown())
        renderVisibleImages();
}

void ScrollingImageList::renderImage(int i)
{
    point_t pageSize = m_images[i].pageSize();

    //  render
    int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
    Byte *bitmap = new Byte[numBytes];
    m_document->RenderPage (i, m_images[i].scale(), bitmap, pageSize.X, pageSize.Y, m_showAnnotations);

    //  copy to widget
    QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
    QPixmap pix = QPixmap::fromImage(*myImage);
    m_images[i].setPixmap(pix);

    //  get the links and put them in the image.
    m_images[i].setShowLinks(m_showLinks);

    m_images[i].setRendered(true);

    delete myImage;
    delete bitmap;
}

void ScrollingImageList::renderVisibleImages()
{
    int nPages = m_document->GetPageCount();

    for (int i=0; i<nPages; i++)
    {
        if (isImageVisible(i))
        {
            if (!m_images[i].rendered())
            {
                renderImage(i);
            }
            else
            {
                m_images[i].repaint();
            }
        }
    }
}

void ScrollingImageList::hilightImage(int nImage)
{
    if (!m_imagesBuilt)
        return;

    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        ImageWidget *t = &(m_images[i]);
        t->setSelected(i==nImage);
    }
}

bool ScrollingImageList::isImageVisible(int nPage)
{
    QRegion visibleRegion = m_images[nPage].visibleRegion();

    if (visibleRegion.isEmpty())
        return false;

    //  I forget why I did this.
//    QRect rect = visibleRegion.boundingRect();
//    if (rect.height() < m_images[nPage].height()*0.20)
//        return false;

    return true;
}

void ScrollingImageList::goToPage (int nPage, bool evenIfVisible)
{
    if (!m_imagesBuilt)
        return;

    //  if the current page is in view, do nothing.
    if (!evenIfVisible && isImageVisible(nPage))
        return;

    //  scroll to top of page
    QRect r = m_images[nPage].geometry();
    int scrollTo = r.top()-10;  //  so we see a little margin above
    if (scrollTo<0)
        scrollTo = 0;
    m_scrollArea->verticalScrollBar()->setValue(scrollTo);
}

bool ScrollingImageList::eventFilter(QObject *target, QEvent *event)
{
    //  process the event
    bool result = QObject::eventFilter(target, event);

    //  if it was a layout, render visible
    if (event->type() == QEvent::LayoutRequest)
    {
        onImagesReady();
    }

    //  done
    return result;
}

