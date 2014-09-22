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

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

class Window : public QMainWindow
{
	Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

	static void open();
    bool OpenFile (QString path);

protected:
	void keyPressEvent(QKeyEvent* event);
    void customEvent(QEvent *event);

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

private:
    Ui::Window *ui;

	void updateActions();
    void setupToolbar();

    void setInitialSizeAndPosition();

    void enterFullScreen();
    void exitFullScreen();
    bool handlePassword();
    void goToPage(int nPage);
    void zoom(double scale);

    //  pages
    double m_scalePage = 1.0;
    PageList *m_pages = NULL;

    //  thumbnails
    ThumbnailList *m_thumbnails = NULL;

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

    const double m_minScale = 0.05;
    const double m_maxScale = 5;
    const double m_zoomInc  = 0.25;
};

#endif  //  WINDOW_H
