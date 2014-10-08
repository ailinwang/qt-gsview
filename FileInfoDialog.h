#ifndef FILEINFODIALOG_H
#define FILEINFODIALOG_H

#include <QDialog>

namespace Ui {
class FileInfoDialog;
}

class FileInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileInfoDialog(QWidget *parent = 0);
    ~FileInfoDialog();

private:
    Ui::FileInfoDialog *ui;
};

#endif // FILEINFODIALOG_H
