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

    virtual void show();

    void setSeparatorFilter(QString val);
    void setFallbackFilter (QString val);

signals:

public slots:
    void onFilterSelected (const QString &filter);

private:
    QString m_separator;
    QString m_fallback;
};

#endif // FILESAVEDIALOG_H
