#ifndef CONTENTSLIST_H
#define CONTENTSLIST_H

#include <QObject>
#include <QScrollArea>
#include <QListWidget>

#include "Document.h"

class ContentsList : public QObject
{
    Q_OBJECT
public:
    explicit ContentsList(QObject *parent = 0);

    QScrollArea *scrollArea() const;
    void setScrollArea(QScrollArea *scrollArea);

    void show();
    void hide();

    Document *document() const {return m_document;}
    void setDocument(Document *document) {m_document = document;}

    void build();

signals:

public slots:

private:
    QScrollArea *m_scrollArea = NULL;
    Document *m_document = NULL;
    QListWidget *m_list = NULL;

};

#endif // CONTENTSLIST_H
