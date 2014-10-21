#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPrinter>

namespace Ui {
class Window;
}

#include "Document.h"
#include "ImageWidget.h"
#include "PageList.h"
#include "ThumbnailList.h"
#include "ContentsList.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

class FileSave;
class MessagesDialog;

class Window : public QMainWindow
{
	Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

	static void open();
    bool OpenFile (QString path);
    bool OpenFile2 (QString path);

    Document *document() {return m_document;}
    bool getShowAnnotations() {return m_showAnnotations;}
    QString getPath() {return m_path;}
    void goToPage(int nPage);

protected:
	void keyPressEvent(QKeyEvent* event);
    void customEvent(QEvent *event);
    bool eventFilter(QObject *object, QEvent *e);

private slots:
    void openAction();
    void closeAction();
    void saveAction();
    void print();
	void zoomIn();
	void zoomOut();
	void normalSize();
	void helpAbout();
    void help();
	void pageUp();
	void pageDown();
    void toggleFullScreen();
    void quit();
    void actionThumbnails();
    void pageEditReturnPressed();
    void percentageEditReturnPressed();
    void thumbnailsReady();
    void pagesReady();
    void toggleAnnotations();
    void homeSlot();
    void endSlot();
    void toggleLinks();
    void copyText();
    void deselectText();
    void selectAllText();
    void fitPage();
    void fitWidth();
    void fileInfo();
    void toggleContents();
    void setAA();
    void ghostscriptMessages();

private:
    Ui::Window *ui;

	void updateActions();
    void setupToolbar();

    void setInitialSizeAndPosition();

    void enterFullScreen();
    void exitFullScreen();
    bool handlePassword();

    void zoom(double scale);

    //  pages
    double m_scalePage = 1.0;
    PageList *m_pages = NULL;

    //  thumbnails
    ThumbnailList *m_thumbnails = NULL;

    //  contents
    ContentsList *m_contents = NULL;

    //  current page
    int m_currentPage = 0;

    //  additional toolbar widgets
    QLineEdit *m_pageNumber = NULL;
    QLabel *m_totalPages = NULL;
    QLineEdit *m_percentage = NULL;

    //  document that contains all of the mupDF functionality
    Document *m_document = NULL;

    //  counting open windows
    static int m_numWindows;
    static int numWindows() {return m_numWindows;}
    static void countWindowUp() {m_numWindows++;}
    static void countWindowDown() {if (m_numWindows>0) m_numWindows--;}

    bool m_showAnnotations = true;
    bool m_showLinks = false;
    bool m_showContents = false;

    const double m_minScale = 0.05;
    const double m_maxScale = 5;
    const double m_zoomInc  = 0.25;

    QString m_path = NULL;

    QString m_fileExtension;
    QString m_fileType;
    FileSave *m_fileSave;

    MessagesDialog *m_messagesDialog=NULL;
};

#endif  //  WINDOW_H
