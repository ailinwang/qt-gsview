#include <QVBoxLayout>
#include <QMessageBox>

#include "ContentsList.h"
#include "Window.h"

ContentsList::ContentsList(QObject *parent) :
    QObject(parent)
{
}

QScrollArea *ContentsList::scrollArea() const
{
    return m_scrollArea;
}

void ContentsList::setScrollArea (QScrollArea *scrollArea)
{
    m_scrollArea = scrollArea;

//    QAbstractSlider *slider = (QAbstractSlider *) m_scrollArea->verticalScrollBar();
//    connect (slider, SIGNAL(sliderReleased()), this, SLOT(sliderReleasedSlot()));
//    connect (slider, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)));

//    //  install an event filter on the scrolling area.
//    QWidget* contentWidget = m_scrollArea->widget();
//    contentWidget->installEventFilter(this);
}

void ContentsList::show()
{
    if (NULL != m_scrollArea)
    {
        m_scrollArea->show();
    }
}

void ContentsList::hide()
{
    if (NULL != m_scrollArea)
        m_scrollArea->hide();
}

void ContentsList::build()
{
    if (m_list == NULL)
    {
        //  make the list widget.
        m_list = new QListWidget();
        QWidget* contentWidget = m_scrollArea->widget();
        contentWidget->setLayout(new QVBoxLayout(contentWidget));
        contentWidget->layout()->setContentsMargins(0,0,0,0);
        contentWidget->layout()->addWidget(m_list);

        connect (m_list, SIGNAL(itemClicked ( QListWidgetItem *)), this, SLOT(itemClicked ( QListWidgetItem *)));

        int nContentsItems = m_document->ComputeContents();
        for (int i=0; i<nContentsItems; i++)
        {
            ContentItem *item = m_document->GetContentItem(i);
            if (item != NULL)
            {
                QString s;
                s += QString::number(item->Page+1);
                s += QString("       ");
                s += QString(item->StringMargin.c_str());
//                s += QString("   ");
//                s += QString(item->StringOrig.c_str());
                m_list->addItem(s);
            }
        }
    }
}

void ContentsList::itemClicked(QListWidgetItem *item)
{
    QString text = item->text();
    QString firstWord = text.split(" ").at(0);
    int page = firstWord.toInt();

    ((Window *)m_scrollArea->window())->goToPage(page-1);

}
