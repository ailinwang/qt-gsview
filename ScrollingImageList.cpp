#include "Window.h"
#include "ScrollingImageList.h"
#include "QtUtil.h"

#include <QAbstractSlider>
#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>
#include <QDebug>

static bool inGoToPage = false;

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

    //  create a timer for rendering
    m_rendertimer = new QTimer(this);
    m_rendertimer->stop();
    connect(m_rendertimer, SIGNAL(timeout()), this, SLOT(onRenderTimer()));
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

        //  set the initial scale value
        double theScale = getScale();
        setScale(theScale);

        //  configure and add the images
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
            m_images[i].setThumbnail(thumbnails());
            m_images[i].setDocument(document());

            contentWidget->layout()->addWidget(&(m_images[i]));
            contentWidget->layout()->setAlignment(&(m_images[i]), Qt::AlignHCenter);
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
    //  just re-render any images that are currently rendered
    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        if (m_images[i].rendered())
        {
            m_images[i].setShowLinks(showLinks);
            m_images[i].repaint();
        }
    }
}

void ScrollingImageList::reRender()
{
    //  mark all pages un-rendered,
    //  then render the visible ones

    int nPages = m_document->GetPageCount();

    for (int i=0; i<nPages; i++)
        m_images[i].setRendered(false);

    renderVisibleImages();

}

void ScrollingImageList::cleanup()
{
    if (m_imagesBuilt)
    {
        int nPages = m_document->GetPageCount();
        for (int i=0; i<nPages; i++)
            m_images[i].cleanup();

        delete []m_images;
    }
}

void ScrollingImageList::zoom (double theScale)
{
    if (theScale != m_scale)
    {
        double zoomRatio = theScale/m_scale;
        int nPages = m_document->GetPageCount();

        //  estimate where the vertical slider should go and send it there.
        QAbstractSlider *vslider = (QAbstractSlider *) m_scrollArea->verticalScrollBar();
        int oldVal = vslider->value();
        int newVal = oldVal * zoomRatio;

        //  hide the scroll area widget while we do the rest to avoid flickering
        m_scrollArea->widget()->hide();

        vslider->setValue(newVal);

        //  set the new scale value
        m_scale = theScale;

        //  resize all the images and mark them as not rendered
        int maxW = 0;
        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize(i, m_scale, &pageSize);

            if (pageSize.X>maxW)
                maxW = pageSize.X;

            m_images[i].setFixedWidth(pageSize.X);
            m_images[i].setFixedHeight(pageSize.Y);
            m_images[i].setScale(m_scale);
            m_images[i].setPageSize(pageSize);
            m_images[i].setRendered(false);
            m_images[i].setBackgroundRole(QPalette::Dark);

            //  first just substitute a scaled version of the old pixmap.
            //  then later, when the rendering takes place, it will be replaced
            //  with a better version.  But in the meantime, the zooming
            //  will appear instantaneously.

            const QPixmap *pm = m_images[i].pixmap();
            if (pm)
            {
                QPixmap scaledPixmap = pm->scaled(m_images[i].size(), Qt::KeepAspectRatio);
                m_images[i].setPixmap(scaledPixmap);
            }

            //  render if it will become visible later.
//            if (i>=minVis && i<=maxVis)
//                renderImage(i);
        }

        //  now show the scroll area again
        //  and render
        qApp->processEvents();
        m_scrollArea->widget()->show();
        qApp->processEvents();
        renderVisibleImages();
        qApp->processEvents();

        //  center the slider
        QAbstractSlider *hslider = (QAbstractSlider *) m_scrollArea->horizontalScrollBar();
        hslider->setValue((maxW-hslider->size().width())/2);

        emit imagesReady();
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
    onSliderReleased();
}

bool ScrollingImageList::isSliderDown()
{
    QAbstractSlider *slider = (QAbstractSlider *) m_scrollArea->verticalScrollBar();
    return slider->isSliderDown();
}

void ScrollingImageList::valueChangedSlot(int val)
{
    UNUSED(val);

    if (!isSliderDown())
    {
        //  probably clicked in the trough
        //  render immediately
        renderVisibleImages();
    }
    else
    {
        //  we're scrolling
        renderVisibleImagesLow();
    }

    if (!inGoToPage)
        onScrollChange();
}

void ScrollingImageList::renderVisibleImagesLow()
{
    int nPages = m_document->GetPageCount();

    for (int i=0; i<nPages; i++)
    {
        if (isImageVisible(i))
        {
            if (!m_images[i].rendered())
                m_images[i].render(m_showAnnotations, m_showLinks, true);
        }
    }

    //  renderVisibleImagesLow() is called more-or-less continuously during scrolling.
    //  Stopping the timer and restarting it defers high-res rendering
    //  until the scrolling stops.

    m_rendertimer->stop();
    m_rendertimer->start(250);
}

void ScrollingImageList::onRenderTimer()
{
    renderVisibleImages();
}

void ScrollingImageList::renderImage(int i, bool lowRes)
{
    m_images[i].render(m_showAnnotations, m_showLinks, lowRes);
}

void ScrollingImageList::renderVisibleImages(bool lowRes /* =false*/)
{
    int nPages = m_document->GetPageCount();

    for (int i=0; i<nPages; i++)
    {
        if (isImageVisible(i))
        {
            if (!m_images[i].rendered())
            {
                renderImage(i, lowRes);
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

    inGoToPage = true;
    m_scrollArea->verticalScrollBar()->setValue(scrollTo);
    inGoToPage = false;

    qApp->processEvents();
    this->renderVisibleImages();
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

