
#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QPrinter>

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
    Window();
    ~Window();

	static void open();
    bool OpenFile (QString path);

protected:
	void keyPressEvent(QKeyEvent* event);
//	void resizeEvent(QResizeEvent* event);

private slots:
    void openAction();
    void closeAction();
    void print();
	void zoomIn();
	void zoomOut();
	void normalSize();
	void helpAbout();
    void helpUsage();

	void pageUp();
	void pageDown();

    void toggleFullScreen();

private:
	void createActions();
	void createMenus();
	void updateActions();

	void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void drawPage(int pageNumber);
    void setInitialSizeAndPosition();

    static void errorMessage(const std::string theTitle, const std::string theMessage);

    void exitFullScreen();

    QAction *openAct;
    QAction *closeAct;
    QAction *printAct;
	QAction *exitAct;
	QAction *zoomInAct;
	QAction *zoomOutAct;
	QAction *normalSizeAct;
	QAction *aboutAct;
    QAction *usageAct;
    QAction *fullScreenAct;

	QAction *pageUpAct;
	QAction *pageDownAct;

	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *helpMenu;

    static int m_numWindows;

    QScrollArea *m_scrollArea = NULL;
    QWidget *m_contentWidget = NULL;

    //  array of page images
    QLabel *m_pageImages = NULL;

    //  array of thumbnail images
    QLabel *m_thumbnailImages = NULL;

    //  current page
    int m_currentPage = 0;

    Document *m_document = NULL;
};

#endif  //  WINDOW_H
