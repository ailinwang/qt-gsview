
#include <QtWidgets>
#include <QAbstractScrollArea>
#include <QAction>
#include <QTemporaryDir>
#include <QNativeGestureEvent>

#include "Window.h"
#include "Printer.h"
#include "ui_Window.h"
#include "QtUtil.h"
#include "FileSave.h"
#include "AboutDialog.h"
#include "MessagesDialog.h"
#include "ExtractPagesDialog.h"
#include "ICCDialog2.h"
#include "ColorsDialog.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    //  set up the UI
    ui->setupUi(this);
    setupToolbar();
    m_setupComplete = true;

    m_fileSave = new FileSave(this);

    //  contents area initially hidden
    m_contents = new ContentsList();
    m_contents->setScrollArea(ui->contentsScrollArea);
    m_contents->hide();

    //  create and set up the left-side scrolling area
    m_thumbnails = new ThumbnailList();
    m_thumbnails->setScrollArea(ui->leftScrollArea);
    m_thumbnails->hide();  //  initially hidden
    connect(m_thumbnails, SIGNAL(imagesReady()), this, SLOT(thumbnailsReady()));

    //  create and set up the right-side scrolling area
    m_pages = new PageList(this);
    m_pages->setScrollArea(ui->rightScrollArea);
    connect(m_pages, SIGNAL(imagesReady()), this, SLOT(pagesReady()));
    connect(m_pages, SIGNAL(startRender()), this, SLOT(onStartPageRender()));
    connect(m_pages, SIGNAL(finishRender()), this, SLOT(onFinishPageRender()));

    ui->rightScrollArea->installEventFilter(this);

    //  create and initialize the mu Document
    m_document = new Document();
    m_document->Initialize();

    //  connect actions to slots
    //  file menu
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openAction()));
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(closeAction()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveAction()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->actionInfo, SIGNAL(triggered()), this, SLOT(fileInfo()));
    connect(ui->actionGhostscript_Messages, SIGNAL(triggered()), this, SLOT(ghostscriptMessages()));
    connect(ui->actionExract_Pages, SIGNAL(triggered()), this, SLOT(extractPages()));
    connect(ui->actionSave_Selection, SIGNAL(triggered()), this, SLOT(saveSelection()));

    //  edit menu
    connect(ui->actionCopy_Text, SIGNAL(triggered()), this, SLOT(copyText()));
    connect(ui->actionDeselect_Text, SIGNAL(triggered()), this, SLOT(deselectText()));
    connect(ui->actionSelect_All_Text, SIGNAL(triggered()), this, SLOT(selectAllText()));
    connect(ui->actionFindDialog, SIGNAL(triggered()), this, SLOT(onFindDialog()));
    connect(ui->actionStop_Finding, SIGNAL(triggered()), this, SLOT(stopFind()));
    connect(ui->actionCopy_Page, SIGNAL(triggered()), this, SLOT(copyPage()));

    //  view menu
    connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(ui->actionZoom_Normal, SIGNAL(triggered()), this, SLOT(normalSize()));
    connect(ui->actionFit_Page, SIGNAL(triggered()), this, SLOT(fitPage()));
    connect(ui->actionFit_Width, SIGNAL(triggered()), this, SLOT(fitWidth()));

    //  proofing menu
    QSignalMapper* signalMapper = new QSignalMapper (this) ;
    connect (ui->action72,   SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action96,   SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action150,  SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action300,  SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action600,  SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action1200, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    connect (ui->action2400, SIGNAL(triggered()), signalMapper, SLOT(map())) ;
    signalMapper->setMapping (ui->action72,   72) ;
    signalMapper->setMapping (ui->action96,   96) ;
    signalMapper->setMapping (ui->action150,  150) ;
    signalMapper->setMapping (ui->action300,  300) ;
    signalMapper->setMapping (ui->action600,  600) ;
    signalMapper->setMapping (ui->action1200, 1200) ;
    signalMapper->setMapping (ui->action2400, 72400) ;
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(doProof(int))) ;

    //  colors menu item
    connect(ui->actionColors, SIGNAL(triggered()), this, SLOT(doColors()));

    connect(ui->actionPage_Up, SIGNAL(triggered()), this, SLOT(pageUp()));
    connect(ui->actionPage_Down, SIGNAL(triggered()), this, SLOT(pageDown()));
    connect(ui->actionHome, SIGNAL(triggered()), this, SLOT(homeSlot()));
    connect(ui->actionEnd, SIGNAL(triggered()), this, SLOT(endSlot()));
    connect(ui->actionBack, SIGNAL(triggered()), this, SLOT(back()));
    connect(ui->actionForward, SIGNAL(triggered()), this, SLOT(forward()));

    connect(ui->actionThumbnails, SIGNAL(triggered()), this, SLOT(actionThumbnails()));
    connect(ui->actionFull_Screen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

    connect(ui->actionAnnotations, SIGNAL(triggered()), this, SLOT(toggleAnnotations()));
    ui->actionAnnotations->setChecked(true);

    connect(ui->actionContents, SIGNAL(triggered()), this, SLOT(toggleContents()));
    ui->actionContents->setChecked(false);

    connect(ui->actionLinks, SIGNAL(triggered()), this, SLOT(toggleLinks()));

    //  options menu
    connect(ui->actionOutput_Intents, SIGNAL(triggered()), this, SLOT(outputIntents()));
    connect(ui->actionAAHigh,         SIGNAL(triggered()), this, SLOT(setAA()));
    connect(ui->actionAAMediumHigh,   SIGNAL(triggered()), this, SLOT(setAA()));
    connect(ui->actionAAMedium,       SIGNAL(triggered()), this, SLOT(setAA()));
    connect(ui->actionAALow,          SIGNAL(triggered()), this, SLOT(setAA()));
    connect(ui->actionAANone,         SIGNAL(triggered()), this, SLOT(setAA()));

    ui->actionAAHigh->setData(QVariant(AA_HIGH));
    ui->actionAAMediumHigh->setData(QVariant(AA_MEDHIGH));
    ui->actionAAMedium->setData(QVariant(AA_MED));
    ui->actionAALow->setData(QVariant(AA_LOW));
    ui->actionAANone->setData(QVariant(AA_NONE));

    //  help menu
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
//    connect(ui->actionGSView_Help, SIGNAL(triggered()), this, SLOT(help()));
    connect(ui->actionUser_Guide, SIGNAL(triggered()), this, SLOT(help()));

    //  account for this window
    countWindow(1);

    //  set initial size and placement
    resize(QDesktopWidget().availableGeometry(this).size() * 0.90);
    setInitialSizeAndPosition();

    setupRecentActions();

    connect(&m_startSearchTimer, SIGNAL(timeout()), this, SLOT(onFind2()));
}

void Window::setupRecentActions()
{
    QAction* recentFileAction = 0;
    for(int i = 0; i < QtUtil::maxRecentFiles; i++)
    {
        recentFileAction = new QAction(this);
        recentFileAction->setVisible(false);
        QObject::connect(recentFileAction, SIGNAL(triggered()),
                                     this, SLOT(openRecent()));
        m_recentFileActionList.append(recentFileAction);
    }

    QMenu *fileMenu = ui->menuFile;
    QMenu *recentFilesMenu = fileMenu->addMenu(tr("Open Recent"));
    for(int i = 0; i < QtUtil::maxRecentFiles; i++)
        recentFilesMenu->addAction(m_recentFileActionList.at(i));
}

void Window::updateRecentActions()
{
    //  get the list
    QStringList recentFilePaths = QtUtil::getRecentFileList();

    //  hide all the menus
    for (int i = 0; i < QtUtil::maxRecentFiles; i++)
        m_recentFileActionList.at(i)->setVisible(false);

    //  set up and show
    for (int i = 0; i < recentFilePaths.length(); i++)
    {
        QString strippedName = QFileInfo(recentFilePaths.at(i)).fileName();
        m_recentFileActionList.at(i)->setText(strippedName);
        m_recentFileActionList.at(i)->setData(recentFilePaths.at(i));
        m_recentFileActionList.at(i)->setVisible(true);
    }
}

void Window::openRecent()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString path = action->data().toString();

        //  we were, try and load it
        Window *newWindow = new Window();
        newWindow->show();

        if (newWindow->OpenFile(path))
        {
            //  success
            return;
        }

        //  error
        newWindow->hide();
        delete newWindow;

        //  remove from the recent list.
        QtUtil::removeRecentFile(path);
        updateRecentActions();

        QMessageBox::information(NULL, tr(""), tr("Error opening %1").arg(path));
    }
}

void Window::countWindow(int val)
{
    m_numWindows += val;
    if (m_numWindows<0)
        m_numWindows = 0;

    bool bEnable = true;
    if (numWindows()<=0)
        bEnable = false;

    ui->actionSave->setEnabled(bEnable);
    ui->actionClose->setEnabled(bEnable);
    ui->actionPrint->setEnabled(bEnable);
    ui->actionInfo->setEnabled(bEnable);
    ui->actionExract_Pages->setEnabled(bEnable);
    ui->actionSave_Selection->setEnabled(bEnable);

    ui->actionCopy_Text->setEnabled(bEnable);
    ui->actionDeselect_Text->setEnabled(bEnable);
    ui->actionSelect_All_Text->setEnabled(bEnable);
    ui->actionFindDialog->setEnabled(bEnable);
    ui->actionCopy_Page->setEnabled(bEnable);

    ui->actionZoom_In->setEnabled(bEnable);
    ui->actionZoom_Out->setEnabled(bEnable);
    ui->actionZoom_Normal->setEnabled(bEnable);
    ui->actionFit_Page->setEnabled(bEnable);
    ui->actionFit_Width->setEnabled(bEnable);

    ui->actionPage_Up->setEnabled(bEnable);
    ui->actionPage_Down->setEnabled(bEnable);
    ui->actionHome->setEnabled(bEnable);
    ui->actionEnd->setEnabled(bEnable);

    ui->actionThumbnails->setEnabled(bEnable);
    ui->actionFull_Screen->setEnabled(bEnable);
    ui->actionAnnotations->setEnabled(bEnable);
    ui->actionContents->setEnabled(bEnable);
    ui->actionLinks->setEnabled(bEnable);

    ui->actionOutput_Intents->setEnabled(bEnable);
    ui->menuAntiAlias->setEnabled(bEnable);
}

void Window::setupToolbar()
{
    //  we do this in code, rather than relying on Qt designer, because
    //  we need to insert non-actions (widgets)

    int fieldWidth;

    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSave);
    ui->toolBar->addAction(ui->actionPrint);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionHome);
    ui->toolBar->addAction(ui->actionPage_Up);
    ui->toolBar->addAction(ui->actionPage_Down);
    ui->toolBar->addAction(ui->actionEnd);

    m_pageNumber = new QLineEdit();
    fieldWidth = m_pageNumber->fontMetrics().charWidth("5",0)*5+10;
    m_pageNumber->setMaximumWidth(fieldWidth);
    m_pageNumber->setMinimumWidth(fieldWidth);
    m_pageNumber->setAlignment(Qt::AlignRight);
    connect ( m_pageNumber, SIGNAL(returnPressed()), SLOT(pageEditReturnPressed()));

    m_totalPages = new QLabel();
    QLabel *slash = new QLabel();  slash->setText(tr("/"));
    ui->toolBar->insertWidget(NULL, m_pageNumber);
    ui->toolBar->insertWidget(NULL, slash);
    ui->toolBar->insertWidget(NULL, m_totalPages);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionZoom_In);
    ui->toolBar->addAction(ui->actionZoom_Out);

    m_percentage = new QLineEdit();
    fieldWidth = m_percentage->fontMetrics().charWidth("5",0)*4+10;
    m_percentage->setMaximumWidth(fieldWidth);
    m_percentage->setMinimumWidth(fieldWidth);
    connect ( m_percentage, SIGNAL(returnPressed()), SLOT(percentageEditReturnPressed()));
    QLabel *pct = new QLabel();  pct->setText(tr("%"));
    ui->toolBar->insertWidget(NULL, m_percentage);
    ui->toolBar->insertWidget(NULL, pct);

    ui->toolBar->addAction(ui->actionZoom_Normal);
    ui->toolBar->addAction(ui->actionFit_Page);
    ui->toolBar->addAction(ui->actionFit_Width);
    ui->toolBar->addAction(ui->actionFull_Screen);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionThumbnails);
//    ui->toolBar->addAction(ui->actionAnnotations);  //  hiding this unless we can find a better icon
    ui->toolBar->addAction(ui->actionContents);
    ui->toolBar->addAction(ui->actionLinks);

    //  searching

    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionFindDialog);
    m_search = new QLineEdit();
    fieldWidth = m_search->fontMetrics().averageCharWidth()*20;
    m_search->setMaximumWidth(fieldWidth);
    m_search->setMinimumWidth(fieldWidth);
    ui->toolBar->insertWidget(NULL, m_search);

    ui->toolBar->addAction(ui->actionFind_Previous);
    ui->toolBar->addAction(ui->actionFind_Next);

    ui->toolBar->widgetForAction(ui->actionFind_Previous)->setMinimumWidth(16);
    ui->toolBar->widgetForAction(ui->actionFind_Previous)->setMaximumWidth(16);
    ui->toolBar->widgetForAction(ui->actionFind_Next)->setMinimumWidth(16);
    ui->toolBar->widgetForAction(ui->actionFind_Next)->setMaximumWidth(16);

    m_searchLabel = new QLabel();  m_searchLabel->clear();
    ui->toolBar->insertWidget(NULL, m_searchLabel);
    connect ( m_search, SIGNAL(textChanged(const QString &)), SLOT(onFind()));
    connect(ui->actionFind_Next, SIGNAL(triggered()), this, SLOT(findNext()));
    connect(ui->actionFind_Previous, SIGNAL(triggered()), this, SLOT(findPrevious()));
}

Window::~Window()
{

}

void Window::back()
{
    if (m_pageHistoryIndex > 0)
    {
        m_historyUpdateAllowed = false;
        m_pageHistoryIndex--;
        goToPage(m_pageHistory.at(m_pageHistoryIndex));
        m_historyUpdateAllowed = true;
    }
}

void Window::forward()
{
    if (m_pageHistoryIndex < m_pageHistory.length()-1)
    {
        m_historyUpdateAllowed = false;
        m_pageHistoryIndex++;
        goToPage(m_pageHistory.at(m_pageHistoryIndex));
        m_historyUpdateAllowed = true;
    }
    else
    {
        pageDown();
    }
}

void Window::updatePageHistory(int nPage)
{
    if (m_historyUpdateAllowed)
    {
        //  if the current history page is the same, do nothing
        if (m_pageHistoryIndex>=0 && m_pageHistoryIndex<=m_pageHistory.length()-1)
            if (m_pageHistory.at(m_pageHistoryIndex) == nPage)
                return;

        //  remove history items that are after the current pointer
        int i = m_pageHistory.length()-1;
        while (i>m_pageHistoryIndex)
        {
            m_pageHistory.removeAt(i);
            i--;
        }

        //  add the page and increment the pointer.
        m_pageHistory.append(nPage);
        m_pageHistoryIndex = m_pageHistory.length()-1;
//        qDebug() << "new page = " << nPage << ", history length = " << m_pageHistory.length();
    }
}

void Window::pageEditReturnPressed()
{
    //  user typed Enter while in the page number field.
    //  go to the page.

    QString s = m_pageNumber->text();
    if (s.length()>0)
    {
        bool ok;
        int n = s.toInt(&ok);
        if (ok)
        {
            if (n>=1 && n<=m_document->GetPageCount())
            {
                //  edit field has a value, and it's an integer, and it's
                //  within limits.  Go ahead and change the page.
                goToPage(n-1);
                return;
            }
        }
    }

    //  something went wrong, so restore the correct value
    m_pageNumber->setText(QString::number(m_currentPage+1));
}

void Window::percentageEditReturnPressed()
{
    //  user typed Enter while in the percentage field.
    QString s = m_percentage->text();
    if (s.length()>0)
    {
        bool ok;
        int n = s.toInt(&ok);
        if (ok)
        {
            double f = double(n)/100;

            if ( f>=m_minScale && f<=m_maxScale )
            {
                m_scalePage = f;
                m_pages->zoom (m_scalePage);
                return;
            }
        }
    }

    //  restore the value
    m_percentage->setText(QString::number((int)(100*m_scalePage)));

}

void Window::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Home:
        homeSlot();
        break;

    case Qt::Key_End:
        endSlot();
        break;

    case Qt::Key_PageDown:
        pageDown();
        break;

    case Qt::Key_PageUp:
        pageUp();
        break;

    case Qt::Key_Escape:
        exitFullScreen();
        break;

    case Qt::Key_Backspace:
        back();
        break;

    default:
        break;
    }
}

bool Window::handlePassword()
{
    if (!m_document->RequiresPassword())
        return true;

    //  ask for password.  This will loop until the user enters a correct
    //  password, or the user cancels.

    bool pwdValid = false;
    QString text;
    while (!pwdValid)
    {
        bool ok;
        text = QInputDialog::getText(this, tr(""), tr("Enter password:"), QLineEdit::Password,
                                             QDir::home().dirName(), &ok);
        if (ok && !text.isEmpty())
        {
            pwdValid = m_document->ApplyPassword(text.toStdString());
        }
        else
        {
            //  user cancelled.
            return false;
        }
    }

    m_password = text;  //  save for later

    return true;
}

bool Window::OpenFile (QString path)
{
    QFileInfo fileInfo (path);

    //  set the file type and extension.  Used by the File INfo
    //  dialog.

    if (fileInfo.suffix().toLower() == QString("pdf") )
    {
        m_fileExtension = "PDF";
        m_fileType = tr("Portable Document Format");
    }
    if (fileInfo.suffix().toLower() == QString("epub") )
    {
        m_fileExtension = "EPUB";
        m_fileType = tr("Electronic Publishing");
    }
    else if (fileInfo.suffix().toLower() == QString("ps") )
    {
        m_fileExtension = "PS";
        m_fileType = "PostScript";
    }
    else if (fileInfo.suffix().toLower() == QString("xps") )
    {
        m_fileExtension = "XPS";
        m_fileType = "XPS";
    }
    else if (fileInfo.suffix().toLower() == QString("eps") )
    {
        m_fileExtension = "EPS";
        m_fileType = tr("Encapsulated Postscript");
    }
    else if (fileInfo.suffix().toLower() == QString("cbz") )
    {
        m_fileExtension = "CBZ";
        m_fileType = tr("Comic Book Archive");
    }
    else if (fileInfo.suffix().toLower() == QString("png") )
    {
        m_fileExtension = "PNG";
        m_fileType = tr("Portable Network Graphics Image");
    }
    else if (fileInfo.suffix().toLower() == QString("jpg") )
    {
        m_fileExtension = "JPG";
        m_fileType = tr("Joint Photographic Experts Group Image");
    }
    else
    {
        m_fileExtension = "UNKNOWN";
        m_fileType = tr("Unknown");
    }

    //  handle .PS and .EPS by running ghostscript first
    if (fileInfo.suffix().toLower() == QString("ps") ||
        fileInfo.suffix().toLower() == QString("eps")   )
    {
        //  put the result into the temp folder
        QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

        //  construct the command
        QString command = "\"" + QtUtil::getGsPath() + "\"";
        command += " -P- -dSAFER -q -P- -dNOPAUSE -dBATCH -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " -c .setpdfwrite ";
        command += "-f \"" + path + "\"";

        MessagesDialog::addMessage(command+"\n\n");

        //  create a process to do it, and wait
        m_gsProcess = new QProcess(this);
        m_gsProcess->setProcessChannelMode(QProcess::MergedChannels);
        connect (m_gsProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(printOutput()));
        m_gsProcess->start(command);
        m_gsProcess->waitForFinished();
        disconnect (m_gsProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(printOutput()));

        //  now open the temp file.
        return OpenFile2(newPath);
    }
    else
    {
        //  open directly
        return OpenFile2(path);
    }
}

void Window::printOutput()
{
    QByteArray byteArray = m_gsProcess->readAllStandardOutput();
    QStringList strLines = QString(byteArray).split("\n");

    foreach (QString line, strLines) {
        MessagesDialog::addMessage(line+"\n");
    }
}

bool Window::OpenFile2 (QString path)
{
    //  load a file by path

    //  open the doc
    bool result = m_document->OpenFile(path.toStdString());
    if (!result)
    {
        return false;
    }

    m_document->SetAA(8);

    //  set the window title
    QFile f(path);
    QFileInfo fileInfo(f.fileName());
    QString filename(fileInfo.fileName());
    this->setWindowTitle(filename);

    m_path = path;

    //  ask for password if required
    if (!handlePassword())
        return false;

    //  update recently opened files list
    QtUtil::addRecentFile(path);
    updateRecentActions();

    updateActions();

#ifdef _QT_MAC
        qApp->processEvents();
#else
        while (qApp->hasPendingEvents())
            qApp->processEvents();
#endif


    //  set initial page number and count into the toolbar
    int nPages = m_document->GetPageCount();
    m_pageNumber->setText(QString::number(1));
    m_totalPages->setText(QString::number(nPages));

    //  set initial percentage
    m_percentage->setText(QString::number((int)(100*m_scalePage)));

    //  prepare thumbnails
    m_thumbnails->setDocument(m_document);

    //  prepare contents
    m_contents->setDocument(m_document);

    //  prepare pages
    m_pages->setDocument(m_document);
    m_pages->setScale(m_scalePage);
    m_pages->buildImages();

    //  calculate an initial superScale

    point_t pageSize;
    m_document->GetPageSize(m_currentPage, 1.0, &pageSize);
    QDesktopWidget desktop;
    int swidth  = desktop.screen()->width() *0.90 ;
    double ratiow = swidth / pageSize.X;
    m_superScale = ratiow * 0.90;

//    if (ratioh<m_superScale)
//        m_superScale = ratioh;

//    QScreen *screen = QApplication::screens().at(0);
//    qreal dpi = (qreal)screen->physicalDotsPerInchX();
//    m_superScale = dpi / 72;  //  72 determined empirically

//    //  adjust if we're too big
//    point_t pageSize;
//    m_document->GetPageSize(m_currentPage, m_superScale, &pageSize);
//    if (pageSize.X > (m_pages->width()-m_scrollbarAllowance))
//        m_superScale = m_superScale * (m_pages->width()-m_scrollbarAllowance) / pageSize.X;

    //  TODO:  why does this engage the centering properly?
    //    m_superScale = 1.001;

    normalSize();

    updatePageHistory(0);

    m_isOpen = true;

    //  handle proofing menu items
    ui->menuProof->menuAction()->setVisible(!getIsProof());
    ui->actionColors->setVisible(getIsProof());

    return true;
}

void Window::setInitialSizeAndPosition()
{
    //  calculate size and center
    QDesktopWidget desktop;
    int screenWidth  = desktop.screen()->width();
    int screenHeight = desktop.screen()->height();
    int width = screenWidth*0.90;
    int height = screenHeight*0.90;
    int top  = screenHeight/10;
    int left = screenWidth/10;

    //  add/subtract random amounts to left and top

    int high = left+50;
    int low = left-50;
    left = qrand() % ((high + 1) - low) + low;

    high = top+50;
    low = top-50;
    top = qrand() % ((high + 1) - low) + low;

    //  move it!
    setGeometry(left, top, width, height);
}

void Window::openAction()
{
    Window::open();
}

void Window::quit()
{
    qApp->exit(0);
}

void Window::closeAction()
{
    this->close();
}

void Window::saveAction()
{
    //  do this in another class
    m_fileSave->run();
}

// static
void Window::open()
{
    //  get the last-visited directory.  Default is desktop
    QSettings settings;
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QString lastDir  = settings.value("LastOpenFileDir", desktopLocations.first()).toString();

    //  if the location does not exist (maybe it was deleted/moved),
    //  fall back to the desktop
    QFile *f = new QFile(lastDir);
    if (!f->exists())
        lastDir = desktopLocations.first();

    //  create a dialog for choosing a file
    QFileDialog dialog(qApp->activeWindow(), tr("Open File"),lastDir);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::DontUseNativeDialog, !USE_NATIVE_FILE_DIALOGS);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Viewable Files (*.pdf *.xps *.cbz *.ps *.eps *.epub)"));

    //  get current window count
    int windowCount = numWindows();

    //  create a window that will show the file.
    Window *newWindow = new Window();

    //  remember the currently active window
    QWidget *priorWindow = qApp->activeWindow();

    while (true)
    {
        //  show and run the dialog
        dialog.show();
        int result = dialog.exec();

        //  if the user cancelled, give up
        if (result != QDialog::Accepted)
            break;

        //  remember the last-visited directory
        settings.setValue("LastOpenFileDir", dialog.directory().absolutePath());

        //  show the window
        newWindow->show();
        qApp->setActiveWindow(newWindow);

        //  hide the dialog
        dialog.hide();

#ifdef _QT_MAC
        qApp->processEvents();
#else
        while (qApp->hasPendingEvents())
            qApp->processEvents();
#endif

        //  attempt to load the file
        if (newWindow->OpenFile(dialog.selectedFiles().first()))
        {
            //  success
            qApp->setActiveWindow(newWindow);
            return;
        }
        newWindow->hide();
        QMessageBox::critical(NULL, "", tr("Error opening file"));

        //  if this was the 2nd (or higher) open file,
        //  break out so we don't hit the dialog again.
        if (numWindows() >= 2)
            break;
    }

    //  user gave up, so delete the window we created.
    delete newWindow;

    //  restore prior active window
    if (NULL != priorWindow)
        qApp->setActiveWindow(priorWindow);

    //  if no windows are open, quit.
    if (windowCount==0)
    {
        qApp->processEvents();
        qApp->exit(0);
    }
}

int Window::m_numWindows = 0;

void Window::print()
{
    QApplication::restoreOverrideCursor();
    qApp->processEvents();

    Printer *p = new Printer();
    p->setWindow(this);
    p->print();
}

void Window::zoomIn()
{
    ui->actionZoom_Normal->setChecked(false);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(false);

    zoom(m_scalePage+m_zoomInc);
}

void Window::zoomOut()
{
    ui->actionZoom_Normal->setChecked(false);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(false);

    zoom(m_scalePage-m_zoomInc);
}

void Window::normalSize()
{
    ui->actionZoom_Normal->setChecked(true);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(false);

    zoom (1.0);
}

void Window::zoom (double newScale)
{
    m_scalePage = newScale;

    if (m_scalePage > m_maxScale)
        m_scalePage = m_maxScale;

    if (m_scalePage < m_minScale)
        m_scalePage = m_minScale;

    m_pages->zoom (m_scalePage*m_superScale);
    m_percentage->setText(QString::number((int)(100*(m_scalePage+.001))));  //  add a little bit for rounding
}

void Window::pageUp()
{
    if (m_currentPage>0)
    {
        m_currentPage -= 1;
        goToPage(m_currentPage);
    }
}

void Window::pageDown()
{
    int numPages = m_document->GetPageCount();
    if (m_currentPage+1<numPages)
    {
        m_currentPage += 1;
        goToPage(m_currentPage);
    }
}

void Window::helpAbout()
{
    AboutDialog *about = new AboutDialog(this);
    about->show();
}

void Window::help()
{
    //  open a file
#ifdef _QT_MAC
       QString path = QtUtil::getRealAppDirPath() + QString("../Resources/UserGuide.pdf");
#else
       QString path = QtUtil::getRealAppDirPath() + QString("UserGuide.pdf");
#endif

    //  we were, try and load it
    Window *newWindow = new Window();
    newWindow->show();

    if (newWindow->OpenFile(path))
    {
        //  success
        return;
    }

    //  error
    newWindow->hide();
    delete newWindow;

    //  remove from the recent list.
    QtUtil::removeRecentFile(path);

    QMessageBox::information(NULL, tr(""), tr("Error opening %1").arg(path));

}

void Window::updateActions()
{
}

void Window::customEvent (QEvent *event)
{
    if (event->type() == ImageClickedEvent::IMAGE_CLICKED_EVENT)
    {
        int nPage = static_cast<ImageClickedEvent *>(event)->getPageNumber();
        goToPage (nPage);
        return;
    }

    if (event->type() == LiveZoomEvent::LIVE_ZOOM_EVENT)
    {
        int command  = static_cast<LiveZoomEvent *>(event)->getCommand();
        double delta = static_cast<LiveZoomEvent *>(event)->getDelta();

        if (command == 1)
            startLiveZoom();
        else if (command == 2)
            doLiveZoom(delta);
        else if (command == 3)
            endLiveZoom();

        return;
    }

    QMainWindow::customEvent(event);
}

void Window::thumbnailsReady()
{
    //  hilight the current page
    m_thumbnails->hilightImage(m_currentPage);
    m_thumbnails->goToPage(m_currentPage);
}

void Window::pagesReady()
{
}

void Window::goToPage(int nPage)
{
    setCurrentPage(nPage);
    m_pages->goToPage (nPage, true);
}

void Window::setCurrentPage(int nPage, bool updateHistory /* =true */)
{
    m_currentPage = nPage;

    m_thumbnails->hilightImage(nPage);
    m_thumbnails->goToPage(nPage);

    m_pageNumber->setText(QString::number(m_currentPage+1));

    if (updateHistory)
        updatePageHistory(m_currentPage);
}

void Window::toggleAnnotations()
{
    if (m_showAnnotations)
    {
        m_showAnnotations = false;
        m_pages->annot (m_showAnnotations);
        if (ui->leftScrollArea->isVisible())
            m_thumbnails->annot (m_showAnnotations);
    }
    else
    {
        m_showAnnotations = true;
        m_pages->annot (m_showAnnotations);
        if (ui->leftScrollArea->isVisible())
            m_thumbnails->annot (m_showAnnotations);
    }
}

void Window::toggleLinks()
{
    if (m_showLinks)
    {
        m_showLinks = false;
        m_pages->links (m_showLinks);
    }
    else
    {
        m_showLinks = true;
        m_pages->links (m_showLinks);
    }
}

void Window::copyText()
{
    m_pages->copyText();
}

void Window::deselectText()
{
    m_pages->deselectText();
}

void Window::selectAllText()
{
    m_pages->selectAllText();
}

void Window::fitPage()
{
    ui->actionZoom_Normal->setChecked(false);
    ui->actionFit_Page->setChecked(true);
    ui->actionFit_Width->setChecked(false);

    double scale = getFitPageScale();

    //  zoom it
    zoom (scale);
}

double Window::getFitPageScale()
{
    //  get viewport size
    double height = m_pages->height();
    double width  = m_pages->width();

    //  get native size of current page
    point_t pageSize;
    m_document->GetPageSize(m_currentPage, m_superScale, &pageSize);
    double page_height = pageSize.Y;
    double page_width  = pageSize.X;
    page_width -= m_scrollbarAllowance;

    //  calculate zoom
    double height_scale = double(height) / page_height;
    double width_scale  = double(width)  / page_width;
    double scale = std::min(height_scale, width_scale);

    return scale;
}

void Window::fitWidth()
{
    ui->actionZoom_Normal->setChecked(false);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(true);

    double scale = getFitWidthScale();

    zoom (scale);
}

double Window::getFitWidthScale()
{
    //  calculate an initial superScale based on the window width.
    //  get native size of current page
    point_t pageSize;
    m_document->GetPageSize(m_currentPage, 1.0, &pageSize);
    int pw = m_pages->width();
    pw -= m_scrollbarAllowance;
    double scale = double(pw)  / pageSize.X;
    scale /= m_superScale;

    return scale;
}

static QString makeRow(QString label, QString value)
{
    QString strVar;

    strVar += "<tr>";
    strVar += "<td>";
    strVar += label;
    strVar += "</td>";
    strVar += "<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>";
    strVar += "<td>";
    strVar += value;
    strVar += "</td>";
    strVar += "</tr>";
    strVar += "";

    return strVar;
}

void Window::fileInfo()
{
    //  build an HTML string that is a table.

    QString strVar;

    strVar += "<table style=\"font-weight:normal;\">";
    strVar += "";

    strVar += makeRow(QString("File:"), getPath());

    QString docType = m_fileExtension;
    docType += " - ";
    docType += m_fileType;
    strVar += makeRow(tr("Document Type:"), docType);

    strVar += makeRow(tr("Pages:"), QString::number(m_document->GetPageCount()));

    strVar += makeRow(tr("Current Page:"), QString::number(m_currentPage+1));

    strVar += "</table>";

    //  show the message
    //  TODO: title is not working

    QSpacerItem* horizontalSpacer = new QSpacerItem(600, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QMessageBox msgBox(QMessageBox::Information, tr("Info"), strVar);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
}

void Window::toggleContents()
{
    if (!m_showContents)
    {
        //  do we have contents?
        int nContentsItems = m_document->ComputeContents();
        if (nContentsItems<=0)
        {
            ui->actionContents->setChecked(m_showContents);
            QMessageBox::information(NULL, tr(""), tr("No table of contents found."));
            return;
        }
    }

    m_showContents = !m_showContents;
    if (m_showContents)
        m_contents->show();
    else
        m_contents->hide();

    ui->actionContents->setChecked(m_showContents);

    //  if we're showing, get the contents and put them in the list
    if (m_showContents)
        m_contents->build();
}

void Window::setAA()
{
    //  first uncheck all the items.
    ui->actionAAHigh->setChecked(false);
    ui->actionAAMediumHigh->setChecked(false);
    ui->actionAAMedium->setChecked(false);
    ui->actionAALow->setChecked(false);
    ui->actionAANone->setChecked(false);

    //  now check the one that was sent
    ((QAction *)sender())->setChecked(true);

    //  get the value
    QVariant qval = ((QAction *)sender())->data();
    int val = qval.toInt();

    m_document->SetAA(val);
    m_pages->reRender();
}


void Window::reRenderCurrentPage()
{
    m_pages->reRender(m_currentPage);
}

void Window::ghostscriptMessages()
{
    m_messagesDialog = new MessagesDialog();
    m_messagesDialog->show();
}

void Window::onFind()
{
    //  terminate previous search
    stopSearch();

    //  pick this up in a bit to allow for rapid typing
//    m_startSearchTimer.stop();
//    m_startSearchTimer.start(750);
    onFind2();
}

void Window::onFind2()
{
    m_startSearchTimer.stop();

    //  start fresh
    m_searchLabel->setText(tr(""));
    m_searchLabel->repaint();
    m_pages->clearSearchText();
    m_searchLabel->clear();
    m_searchItems.clear();
    m_searchHits = 0;
    m_searchCounter = -1;

    //  get text to find
    QString text = m_search->text();

    if (text.length() > 0)
    {
        //  make a thread for searching, and a worker that runs in the thread.
        m_searchThread = new QThread;
        m_searchWorker = new SearchWorker(this, text);
        m_searchWorker->moveToThread(m_searchThread);

        //  connect worker and thread signals
        connect(m_searchThread, SIGNAL(started()),  m_searchWorker, SLOT(process()));
        connect(m_searchWorker, SIGNAL(finished()), m_searchThread, SLOT(quit()));
        connect(m_searchWorker, SIGNAL(finished()), m_searchWorker, SLOT(deleteLater()));
        connect(m_searchThread, SIGNAL(finished()), m_searchThread, SLOT(deleteLater()));

        //  connect worker signals to slots in this class
        connect(m_searchWorker, SIGNAL(finished()),        this, SLOT(searchFinished()));
        connect(m_searchWorker, SIGNAL(pageFinished(int, std::vector<SearchItem> *)), this, SLOT(searchPageFinished(int, std::vector<SearchItem> *)));

        //  start the thread
        m_searchThread->start();
    }

}

void Window::searchFinished()
{
    m_searchThread = NULL;
    updateSearchReport();
}

void Window::stopFind()
{
    if (NULL != m_searchThread)
        m_searchWorker->kill();
    m_searchThread = NULL;
    updateSearchReport();
}

void Window::searchPageFinished(int np, std::vector<SearchItem> *items)
{
    int nBefore = m_searchItems.size();

    if (items != NULL)
    {
        m_pages->setSearchText (np, items);
        m_searchHits += items->size();

        if (items->size()>0)
        {
            for (int i=0;i<(int)items->size();i++)
            {
                m_searchItems.push_back(items->at(i));
            }
        }

        updateSearchReport();

        //  skip to the first one
        if (nBefore<=0)
            findNext();
    }
}

void Window::updateSearchReport()
{
    QString text;

    if (m_searchHits > 0)
        text = QString::number(m_searchCounter+1)+tr("/")+QString::number(m_searchHits);
    else
        text = QString::number(0)+tr("/")+QString::number(0);

    if (m_searchThread && !m_searchThread->isFinished())
        text += " ...";

    m_searchLabel->setText(text);
    m_searchLabel->repaint();
}

void Window::goToSearchItem(int n)
{
    if (m_searchHits > 0)
        m_pages->hilightSearchText(&(m_searchItems.at(n)));
}

void Window::onFindDialog()
{
    //  as user for a string
    bool ok;
    QString text = QInputDialog::getText(this, NULL, tr("Text to find:"), QLineEdit::Normal, NULL, &ok);

    if (ok && !text.isEmpty())
    {
        //  setting the search field will trigger onFind(), so don't call it here.
        m_search->setText(text);
    }
}

QString Window::password() const
{
    return m_password;
}

void Window::setPassword(const QString &password)
{
    m_password = password;
}


void Window::findNext()
{
    if (m_searchCounter+1 < m_searchHits)
    {
        m_searchCounter++;
        updateSearchReport();
        goToSearchItem(m_searchCounter);
    }
}

void Window::findPrevious()
{
    if (m_searchCounter > 0)
    {
        m_searchCounter--;
        updateSearchReport();
        goToSearchItem(m_searchCounter);
    }
}

void Window::extractPages()
{
    //  create and show the dialog.
    //  it will delete itself when closed.

    m_extractDlg = new ExtractPagesDialog();
    m_extractDlg->run (this);

}

void Window::outputIntents()
{
    ICCDialog2 icc_dialog;
    icc_dialog.show();
    icc_dialog.exec();
}

void Window::copyPage()
{
    //  render a bitmap

    int nPage = m_currentPage;
    double scale = m_scalePage;

    //  TODO: what about scale/resolution?

    point_t pageSize;
    m_document->GetPageSize(nPage, scale, &pageSize);

    int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
    Byte *bitmap = new Byte[numBytes];
    m_document->RenderPage (nPage, scale, bitmap, pageSize.X, pageSize.Y, m_showAnnotations);
    QImage *image = new QImage(bitmap, (int)pageSize.X, (int)pageSize.Y, QImage::Format_ARGB32);

    //  copy to clipboard
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setImage(*image);

    if (NULL!=m_copiedImage)
        delete m_copiedImage;
    m_copiedImage = image;

    if (NULL!=m_copiedBitmap)
        delete m_copiedBitmap;
    m_copiedBitmap = bitmap;
}

void Window::saveSelection()
{
    //  if no area selected, error
    if (!m_pages->isAreaSelected())
    {
        QMessageBox::information(NULL, tr(""), tr("Nothing is selected."));
        return;
    }

    m_pages->saveSelection(m_fileSave);
}

void Window::homeSlot()
{
    goToPage(0);
}

void Window::endSlot()
{
    int numPages = m_document->GetPageCount();
    goToPage(numPages-1);
}

void Window::actionThumbnails()
{
    if (ui->leftScrollArea->isVisible())
    {
        m_thumbnails->hide();
        ui->actionThumbnails->setChecked(false);
    }
    else
    {
        m_thumbnails->show();
        ui->actionThumbnails->setChecked(true);
        m_thumbnails->buildImages();
    }
}

void Window::toggleFullScreen()
{
    if (windowState() != Qt::WindowFullScreen)
        enterFullScreen();
    else
        exitFullScreen();
}

void Window::exitFullScreen()
{
    setWindowState(Qt::WindowNoState);
    ui->actionFull_Screen->setText(tr("Enter &Full Screen"));
}

void Window::enterFullScreen()
{
    setWindowState(Qt::WindowFullScreen);
    ui->actionFull_Screen->setText(tr("Exit &Full Screen"));
}

bool Window::eventFilter(QObject *object, QEvent *e)
{
    UNUSED(object);

    QNativeGestureEvent *gesture = dynamic_cast<QNativeGestureEvent*>(e);
    if (gesture != NULL)
    {
        //  pinch-zooming should put us in normal zoom mode
        ui->actionZoom_Normal->setChecked(true);
        ui->actionFit_Page->setChecked(false);
        ui->actionFit_Width->setChecked(false);

        if (gesture->gestureType()==Qt::BeginNativeGesture)
        {
            QApplication::postEvent(this, new LiveZoomEvent(1,0));
        }

        if (gesture->gestureType()==Qt::EndNativeGesture)
        {
            QApplication::postEvent(this, new LiveZoomEvent(3,0));
        }

        if (gesture->gestureType()==Qt::ZoomNativeGesture)
        {
            double delta = 0.05;
            if (m_scalePage<1)
                delta = 0.025;
            if (gesture->value()<0)
                delta = -delta;

            QApplication::postEvent(this, new LiveZoomEvent(2,delta));
        }

        return true;  //  we're consuming these
    }

    return m_pages->onEvent(e);
}

void Window::stopSearch()
{
    if (NULL != m_searchThread)
    {
        m_searchWorker->kill();

        //  the thread just terminated will shut down normally, but will no longer
        //  deliver new resuts to this thread.
    }

    m_searchLabel->setText(tr(""));
    m_searchLabel->repaint();
}

void Window::closeEvent(QCloseEvent *event)
{
    UNUSED(event);

    //  don't do this twice
    if (!m_isOpen)
        return;
    m_isOpen = false;

    //  terminate previous search thread
    stopSearch();

    //  delete things that were allocated
    if (NULL!=m_copiedImage)
        delete m_copiedImage;
    m_copiedImage = NULL;

    if (NULL!=m_copiedBitmap)
        delete m_copiedBitmap;
    m_copiedBitmap = NULL;

    m_thumbnails->cleanup();
    m_pages->cleanup();

    if (m_document != NULL)
    {
        m_document->CleanUp();
        delete m_document;
        m_document = NULL;
    }

    delete m_fileSave;
    delete m_contents;
    delete m_thumbnails;
    delete m_pages;

    countWindow(-1);
}

void Window::changeEvent(QEvent *e)
{
#ifdef _QT_MAC

    //  for mac, we want to hide the (menu bar and) tool bar when
    //  we're full-screen.
    //  because we use the native menu bar, we don't need to hide it.

    if (m_protectRecursion)
        return;
    m_protectRecursion = true;

    if (ui->toolBar && m_setupComplete)
    {
        if (isFullScreen())
        {
            ui->toolBar->setVisible(false);
            //ui->menubar->setVisible(false);
        }
        else
        {
            ui->toolBar->setVisible(true);
            //ui->menubar->setVisible(true);
        }
    }

    m_protectRecursion = false;

#endif

    if ( e->type() == QEvent::ActivationChange)
    {
        updateRecentActions();
    }

}

void Window::resizeEvent(QResizeEvent *event)
{
    if (!m_isOpen)
        return;

    if (ui->actionFit_Page->isChecked())
    {
        double delta = getFitPageScale() - m_scalePage;
        QApplication::postEvent(this, new LiveZoomEvent(2,delta));
    }
    else if (ui->actionFit_Width->isChecked())
    {
        double delta = getFitWidthScale() - m_scalePage;
        QApplication::postEvent(this, new LiveZoomEvent(2,delta));
    }

    QMainWindow::resizeEvent(event);
}

void Window::wheelZoomIn()
{
    //  wheel-zooming should put us in normal zoom mode
    ui->actionZoom_Normal->setChecked(true);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(false);

    double delta = 0.05;
    if (m_scalePage<1)
        delta = 0.025;

    QApplication::postEvent(this, new LiveZoomEvent(2,delta));
}

void Window::wheelZoomOut()
{
    //  wheel-zooming should put us in normal zoom mode
    ui->actionZoom_Normal->setChecked(true);
    ui->actionFit_Page->setChecked(false);
    ui->actionFit_Width->setChecked(false);

    double delta = -0.05;
    if (m_scalePage<1)
        delta = -0.025;

    QApplication::postEvent(this, new LiveZoomEvent(2,delta));
}

void Window::onLiveZoomTimer()
{
    m_zoomTimer->stop();
    endLiveZoom();
}

void Window::startLiveZoom()
{
    m_isLiveZooming = true;

    //  create the timer
    if (m_zoomTimer == NULL)
    {
        m_zoomTimer = new QTimer(this);
        m_zoomTimer->stop();
        connect(m_zoomTimer, SIGNAL(timeout()), this, SLOT(onLiveZoomTimer()));
    }

    int startPage = -1;
    if (ui->actionFit_Page->isChecked())
    {
        startPage = currentPage();
        goToPage(startPage);
    }

    m_pages->startLiveZoom(startPage);

    m_lastLiveZoomTime = QDateTime::currentMSecsSinceEpoch();

    //  start the timer
    m_zoomTimer->stop();
    m_zoomTimer->start(m_zoomTimerVal);
}

void Window::doLiveZoom(double delta)
{
    //  call start the first time (resize, wheel)
    if (!m_isLiveZooming)
        startLiveZoom();

    //  move the timer into the future
    m_zoomTimer->stop();
    m_zoomTimer->start(m_zoomTimerVal);

    //  now do it
    m_scalePage = m_scalePage+delta;
    if (m_scalePage > m_maxScale)
        m_scalePage = m_maxScale;
    if (m_scalePage < m_minScale)
        m_scalePage = m_minScale;

    m_pages->zoomLive(m_scalePage*m_superScale);
    m_percentage->setText(QString::number((int)(100*(m_scalePage+.001))));  //  add a little bit for rounding
}

void Window::endLiveZoom()
{
    m_isLiveZooming = false;

    if (ui->actionFit_Page->isChecked())
    {
        fitPage();
        goToPage(currentPage());
    }
    else if (ui->actionFit_Width->isChecked())
    {
        fitWidth();
    }
    else
    {
        zoom(m_scalePage);
        m_pages->endLiveZoom();
    }
}
bool Window::getIsProof() const
{
    return m_isProof;
}

void Window::setIsProof(bool isProof)
{
    m_isProof = isProof;
}

void Window::doWorkInIdle()
{
}

void Window::doProof(int resolution)
{
    //  paths
    QString originalPath = getPath();

    //  if the original is xps, convert to pdf first.
    if (QtUtil::extensionFromPath(originalPath)==QString("xps"))
    {
        //  put the result into the temp folder
        QFileInfo fileInfo (originalPath);
        QString newPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".pdf";

        //  construct the command
        QString command = "\"" + QtUtil::getGxpsPath() + "\"";
        command += " -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " -dNOPAUSE \"" + originalPath + "\"";

        //  create a process to do it, and wait
        QProcess *process = new QProcess(this);
        process->start(command);
        process->waitForFinished();

        originalPath = newPath;
    }

    QFileInfo fileInfo (originalPath);
    QString outPath = QtUtil::getTempFolderPath() + fileInfo.fileName() + ".gproof";

    //  make the .gproof file
    bool result = this->m_document->MakeProof(originalPath.toStdString(),
                                              outPath.toStdString(),
                                              resolution);
    if (result)
    {
        Window *newWindow = new Window();
        newWindow->setIsProof(true);
        newWindow->show();
        if (newWindow->OpenFile(outPath))
        {
            //  success
            return;
        }

        //  error
        newWindow->hide();
        delete newWindow;
    }

    //  if we get here, there's been an error
    QMessageBox::information(NULL, tr(""), tr("Error proofing %1").arg(originalPath));
}

void Window::doColors()
{
    ColorsDialog *dlg = new ColorsDialog(this, m_document, m_currentPage);
    dlg->show();
}

void Window::onStartPageRender()
{
    //  page rendering is beginning
    QApplication::setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents();
}

void Window::onFinishPageRender()
{
    //  page rendering has finished.
    QApplication::restoreOverrideCursor();
    qApp->processEvents();
}

//----------------------------------------------------------------------------

SearchWorker::SearchWorker(Window *window, QString text)
{
    m_window = window;
    m_searchText = text;
}

SearchWorker::~SearchWorker()
{
}

void SearchWorker::kill()
{
    m_window->document()->AbortTextSearch();
    m_killed=true;
}

void SearchWorker::process()
{
    if (m_killed)
        return;

    int nPages = m_window->document()->GetPageCount();

    for (int i = 0; i < nPages; ++i)
    {
        //  killed?
        if (m_killed)
            break;

        //  do the searching
        std::vector<SearchItem> *items = m_window->document()->SearchText (
                    i, (char*)m_searchText.toStdString().c_str());

        if (!m_killed)
            emit pageFinished (i, items);
    }

    //  done!
    if (!m_killed)
        emit finished();
}
