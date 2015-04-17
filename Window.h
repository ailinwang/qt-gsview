#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPrinter>
#include <QTimer>

namespace Ui {
class Window;
}

#include "Document.h"
#include "ImageWidget.h"
#include "PageList.h"
#include "ThumbnailList.h"
#include "ContentsList.h"
#include "ExtractPagesDialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

class FileSave;
class MessagesDialog;

class SearchWorker : public QObject {
    Q_OBJECT

public:
    SearchWorker(Window *window, QString text);
    ~SearchWorker();
    void kill();

public slots:
    void process();

signals:
    void pageFinished(int pageNumber, std::vector<SearchItem> *items);
    void finished();

private:
    Window *m_window;
    bool m_killed = false;
    QString m_searchText;
};

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
    void setCurrentPage(int nPage, bool updateHistory=true);

    QString password() const;
    void setPassword(const QString &password);

    int currentPage() {return m_currentPage;}

    void changeEvent(QEvent *);
    void resizeEvent(QResizeEvent *event);

protected:
    void keyPressEvent(QKeyEvent* event);
    void customEvent(QEvent *event);
    bool eventFilter(QObject *object, QEvent *e);
    void closeEvent(QCloseEvent *event);

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
    void onFind();
    void onFind2();
    void onFindDialog();
    void findNext();
    void findPrevious();
    void extractPages();
    void outputIntents();
    void copyPage();
    void printOutput();
    void back();
    void forward();
    void onResizeTimer();
    void openRecent();
    void searchFinished();
    void searchPageFinished(int np, std::vector<SearchItem> *items);
    void stopFind();

public slots:
    void saveSelection();

private:
    Ui::Window *ui;

	void updateActions();
    void setupToolbar();

    void setInitialSizeAndPosition();

    void enterFullScreen();
    void exitFullScreen();
    bool handlePassword();

    void zoom(double scale, bool resizing);

    void updateSearchReport();
    void goToSearchItem(int n);

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
    QLineEdit *m_search = NULL;

    //  document that contains all of the mupDF functionality
    Document *m_document = NULL;

    //  counting open windows
    static int m_numWindows;
    static int numWindows() {return m_numWindows;}
    void countWindow(int val);
    void enableMenus();

    bool m_showAnnotations = true;
    bool m_showLinks = false;
    bool m_showContents = false;

    const double m_minScale = 0.05;
    const double m_maxScale = 5;
    const double m_zoomInc  = 0.25;
    const int m_scrollbarAllowance = 20;

    QString m_path = NULL;

    QString m_fileExtension;
    QString m_fileType;
    FileSave *m_fileSave;

    MessagesDialog *m_messagesDialog=NULL;

    int m_searchHits = 0;
    int m_searchCounter = 0;
    QLabel *m_searchLabel = NULL;
    std::vector<SearchItem> m_searchItems;
    SearchWorker *m_searchWorker = NULL;
    QThread *m_searchThread = NULL;
    void stopSearch();
    QTimer m_startSearchTimer;

    ExtractPagesDialog *m_extractDlg=NULL;

    Byte *m_copiedBitmap = NULL;
    QImage *m_copiedImage = NULL;

    QString m_password;  //  the one the user supplied when the file was opened

    bool m_isOpen = false;

    bool m_protectRecursion = false;

    QProcess *m_gsProcess;

    bool m_setupComplete = false;

    double m_superScale = 1.0;

    //  data for the update history stack
    QList<int> m_pageHistory;
    int m_pageHistoryIndex = -1;
    void updatePageHistory(int nPage);
    bool m_historyUpdateAllowed = true;

    QTimer *m_resizetimer= NULL;
    bool m_isResizing = false;

    //  recent file stuff
    QList<QAction*> m_recentFileActionList;
    void setupRecentActions();
    void updateRecentActions();
};

#endif  //  WINDOW_H
