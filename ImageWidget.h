#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QLabel>
#include <QEvent>

#include "muctx.h"
#include "Document.h"

class Document;
class Link;
class TextLine;

//  custom event that's sent when we are clicked
class ImageClickedEvent : public QEvent
{
public:
    ImageClickedEvent (const int pageNumber): QEvent(IMAGE_CLICKED_EVENT), m_pageNumber(pageNumber) {}
    int getPageNumber() const {return m_pageNumber;}
    static const QEvent::Type IMAGE_CLICKED_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
private:
    int m_pageNumber;
};

class ImageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = 0);

    virtual void paintEvent(QPaintEvent *);

    void setPage(int nPage) {m_pageNumber=nPage;}

    bool rendered() const {return m_rendered;}

    //  use devicePixelRatio() for Retina support
    double scale2() const {return m_scale;}
    double scale() const {return m_scale * devicePixelRatio();}
    void setScale(double scale) {m_scale = scale;}

    point_t pageSize() const {return m_pageSize;}
    void setPageSize(const point_t &pageSize){m_pageSize = pageSize;}

    bool selected() const {return m_selected;}
    void setSelected(bool selected);

    bool thumbnail() const {return m_thumbnail;}
    void setThumbnail(bool val) {m_thumbnail = val;}

    bool showLinks() const {return m_showLinks;}
    void setShowLinks(bool showLinks) {m_showLinks = showLinks;}

    void setDocument(Document *doc) {m_document=doc;}

    void mouseMoveEvent( QMouseEvent * event );

    int pageNumber() {return m_pageNumber;}

    void clearSelection();
    void addToSelection(TextLine *line, int selBegin=-1, int selEnd=-1);
    void removeFromSelection(TextLine *line);
    QString selectedText();
    void selectAllText();

    void setSearchText (std::vector<SearchItem> *items);
    void clearSearchText();
    void hilightSearchText(SearchItem *item);

    void cleanup();

    void render (bool showAnnotations, bool showLinks, bool lowRes);
    void setRendered(bool rendered);

    void onMouseRelease(QEvent *event);

    void setSelectingArea(bool val) {m_selectingArea=val;};

    bool isInLink() {return m_inLink;}

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:

    void deleteImageData();

    void HilightBlocks (QPainter *painter, double scale, int pageNumber,
                        bool drawBlocks, bool drawLines, bool drawChars);

    int m_pageNumber = 0;
    bool m_rendered = false;
    double m_scale = 1.0;
    point_t m_pageSize;
    bool m_selected = false;
    bool m_thumbnail = false;
    bool m_showLinks = false;
    Document *m_document = NULL;
    Link *m_mouseInLink = NULL;
    std::vector<TextLine *> m_selected_lines;

    std::vector<SearchItem> *m_searchItems = NULL;
    SearchItem *m_hilightedItem = NULL;

    Byte *m_bitmap = NULL;
    QImage *m_image = NULL;
    QPixmap *m_pixmap = NULL;

    bool m_selectingArea = false;

    bool m_inLink = false;
};

#endif // IMAGEWIDGET_H
