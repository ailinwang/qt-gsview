
#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QPrinter>

namespace Ui {
class Window;
}

#include "Document.h"

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
    void clickedThumbnail();

private:
    Ui::Window *ui;

    void connectActions();
	void updateActions();

	void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void drawPage(int pageNumber);
    void setInitialSizeAndPosition();

    static void errorMessage(const std::string theTitle, const std::string theMessage);

    void exitFullScreen();
    bool handlePassword();
    void buildThumbnails();

    static int m_numWindows;

    QScrollArea *m_pageScrollArea = NULL;
    QScrollArea *m_thumbScrollArea = NULL;

    //  array of page images
    double m_scalePage = 1.0;
    QLabel *m_pageImages = NULL;

    //  array of thumbnail images
    bool m_thumbnailsBuilt = false;
    QPushButton *m_thumbnailImages = NULL;

    //  current page
    int m_currentPage = 0;

    //  additional toolbar widgets
    QLabel *m_pageNumber = NULL;
    QLabel *m_totalPages = NULL;

    //  document that contains all of the mupDF functionality
    Document *m_document = NULL;
};

#endif  //  WINDOW_H
