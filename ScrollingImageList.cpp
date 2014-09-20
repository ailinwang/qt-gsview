#include "Window.h"
#include "ScrollingImageList.h"
#include "QtUtil.h"

#include <QAbstractSlider>
#include <QVBoxLayout>
#include <QTimer>

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
bool ScrollingImageList::imagesBuilt() const
{
    return m_imagesBuilt;
}

void ScrollingImageList::setImagesBuilt(bool imagesBuilt)
{
    m_imagesBuilt = imagesBuilt;
}

void ScrollingImageList::buildImages()
{
    if (!imagesBuilt())
    {
        //  create an array of images
        int nPages = m_document->GetPageCount();
        m_images = new ImageWidget[nPages]();

        //  set up scrolling area
        QWidget* contentWidget = m_scrollArea->widget();
        contentWidget->setLayout(new QVBoxLayout(contentWidget));
        contentWidget->layout()->setContentsMargins(0,0,0,0);

        //  find max width of the pages
        int maxW = 0;
        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize (i, 1.0, &pageSize);
            int w = (int)pageSize.X;
            if (w>maxW)
                maxW = w;
        }

        //  calculate a scale factor based on the width of the left scroll area
        double theScale = 0.8 * double(m_scrollArea->width())/double(maxW);

        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize(i, theScale, &pageSize);

            m_images[i].setFixedWidth(pageSize.X);
            m_images[i].setFixedHeight(pageSize.Y);
//            m_images[i].setFlat(true);  //  because it's a button/  don't like
            m_images[i].setPage(i);
            m_images[i].setScale(theScale);
            m_images[i].setPageSize(pageSize);

            contentWidget->layout()->addWidget(&(m_images[i]));
        }

        //  I don't like this because 200 msec seems arbitrary.
        //  10 msec is too small.  There should be some sort of state
        //  I can monitor, or event I can receive.
        QTimer::singleShot(200, this, SLOT(imagesBuiltSlot()));
        setImagesBuilt(true);
    }
}

void ScrollingImageList::imagesBuiltSlot()
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

void ScrollingImageList::renderVisibleImages()
{
    int nPages = m_document->GetPageCount();

    for (int i=0; i<nPages; i++)
    {
        bool visible = !(m_images[i].visibleRegion().isEmpty());

        if (visible)
        {
            if (!m_images[i].rendered())
            {
                point_t pageSize = m_images[i].pageSize();

                //  render
                int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
                Byte *bitmap = new Byte[numBytes];
                m_document->RenderPage (i, m_images[i].scale(), bitmap, pageSize.X, pageSize.Y);

                //  copy to widget
                QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
                QPixmap pix = QPixmap::fromImage(*myImage);
                m_images[i].setPixmap(pix);
                m_images[i].setRendered(true);

                delete myImage;
                delete bitmap;
            }
            else
            {
                m_images[i].repaint();
            }
        }
    }
}

Document *ScrollingImageList::document() const
{
    return m_document;
}

void ScrollingImageList::setDocument(Document *document)
{
    m_document = document;
}

void ScrollingImageList::hilightImage(int nImage)
{
    if (!imagesBuilt())
        return;

    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        ImageWidget *t = &(m_images[i]);
        t->setSelected(i==nImage);
    }
}

