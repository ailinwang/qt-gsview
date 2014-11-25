#ifndef FILESAVEDIALOG_H
#define FILESAVEDIALOG_H

#include <QFileDialog>

class FileSaveDialog : public QFileDialog
{
    Q_OBJECT

public:    
    explicit FileSaveDialog (QWidget *parent = 0,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString());

signals:

public slots:
    void onFilterSelected (const QString &filter);

public:
    virtual void show();

};

#endif // FILESAVEDIALOG_H
