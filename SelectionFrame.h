#ifndef SELECTIONFRAME_H
#define SELECTIONFRAME_H

#include <QRubberBand>

class SelectionFrame : public QRubberBand
{
    Q_OBJECT
public:
    explicit SelectionFrame(QWidget *parent = 0);
    virtual void paintEvent(QPaintEvent *event);

signals:

public slots:

private:
};

#endif // SELECTIONFRAME_H
