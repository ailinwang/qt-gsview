#ifndef FILESAVEDIALOG_H
#define FILESAVEDIALOG_H

#include <QFileDialog>
#include "ui_filedialogextension.h"

class FileSaveDialog : public QFileDialog
{
    Q_OBJECT

public:    
    explicit FileSaveDialog (QWidget *parent = 0,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString());

    virtual void show();

    void setSeparatorFilter(QString val);
    void setFallbackFilter (QString val);

signals:

public slots:
    void onFilterSelected (const QString &filter);

private:
    QString m_separator;
    QString m_fallback;

    //  for getting the options UI from a form
    Ui::Form *ui;
};

#endif // FILESAVEDIALOG_H
