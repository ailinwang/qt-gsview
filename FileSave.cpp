#include "FileSave.h"

#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

#include "QtUtil.h"

FileType fileTypes[] = {
    {"PDF",             "pdf" , "" },
    {"Linearizded PDF", "pdf" , "" },
    {"PDF 1.3",         "pdf" , "" },
    {"PDF/A-1 RGB",     "pdf" , "" },
    {"PDF/A-1 CMYK",    "pdf" , "" },
    {"PDF/A-2 RGB",     "pdf" , "" },
    {"PDF/A-2 CMYK",    "pdf" , "" },
    {"PDF/X-3 Gray",    "pdf" , "" },
    {"PDF/X-3 CMYK",    "pdf" , "" },
    {"PCL-XL",          "bin" , "" },
    {"XPS",             "xps" , "" },
    {"Text",            "txt" , "" },
    {"HTML",            "html", "" },
    {"XML",             "xml" , "" }
};
int numTypes = 14;

void FileSave::run()
{
    //  where is the desktop?
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString desktop = desktopLocations.first();

    //  what are the file types?
    QString types;
    int i;
    for (i=0;i<numTypes;i++)
    {
        QString theFilter = fileTypes[i].filterName + QString(" (*.") + fileTypes[i].filterType + QString(")");
        fileTypes[i].filter = theFilter;
        if (i>0)
            types += ";;";
        types += theFilter;
    }

    //  set up the dialog
    QFileDialog dialog(m_window, "Save", desktop);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    //  INFO: It's hard to debug in the Qt IDE when we use the native file dialog.
    dialog.setOption(QFileDialog::DontUseNativeDialog, false);
    dialog.setNameFilter(types);

    //  get the name
    dialog.show();
    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        QString fileName = dialog.selectedFiles().first();
        QString filter = dialog.selectedNameFilter();
        QString filterExt = QtUtil::extensionFromFilter(filter);
        QString fileExt = QtUtil::extensionFromPath(fileName);

        //  if the user gave no extension, use the filter
        if (fileExt.length()==0)
        {
            fileExt = filterExt;
            fileName += ".";
            fileName += filterExt;
        }

        //  if the extensions don't match, that's an error
        if (fileExt != filterExt)
        {
            QMessageBox::information (m_window, "", "you supplied a filename extension that does not match the format.");
            return;
        }

        //  find the matching type from the filter
        int index = -1;
        for (i=0;i<numTypes;i++)
        {
            if (filter==fileTypes[i].filter)
            {
                index = i;
                break;
            }
        }

        //  now do the save

        if (index==0)
        {
            //  regular PDF - just copy the file
            //  overwrite - the dialog will have asked the user this question
            //  so it's ok to do it
            QString original = m_window->getPath();
            if (QFile::exists(fileName))
                QFile::remove(fileName);
            QFile::copy(original, fileName);
        }
        else if (index==1)
        {
            //  linearized PDF


        }
        else
        {
            //  NYI
            QString message = "saving is not yet implemented.<br/><br/>";
            message += "saving " + fileName + "<br/>as " + QString::number(index+1) + ". " + filter;
            QMessageBox::information (m_window, "", message);
        }
    }

}
