#ifndef GSVIEWAPP_H
#define GSVIEWAPP_H

#include <QApplication>
#include <QtGui>

#define gsviewApplication \
    (static_cast<GSViewApp*>(QCoreApplication::instance()))

class GSViewApp : public QApplication
{
    Q_OBJECT
public:
    GSViewApp( int &argc, char **argv );
    static int exec();

signals:

public slots:
    void onStarted();

protected:
    bool event(QEvent *ev);

private:
    QString m_fileToOpen;

};

#endif // GSVIEWAPP_H
