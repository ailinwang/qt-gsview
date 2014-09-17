#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <QPushButton>

class Thumbnail : public QPushButton
{
    Q_OBJECT
public:
    explicit Thumbnail(QWidget *parent = 0);

    void setPage(int nPage) {m_pageNumber=nPage;}
    void setWindow (void *pWin) {m_window=pWin;}

signals:

public slots:
    void clicked();

private:
    int m_pageNumber = 0;
    void *m_window = NULL;

};

#endif // THUMBNAIL_H
