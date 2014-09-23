
#include <QtWidgets>
#include <QAbstractScrollArea>
#include <QAction>
#include <QTemporaryDir>

#include "Window.h"
#include "Printer.h"
#include "ui_Window.h"

#include "QtUtil.h"

//  temp folder stuff
QString tempFolderPath("");
QString gsAppPath("/Users/fredross-perry/Desktop/mac/gs");

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    //  create the temp path
    tempFolderPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    tempFolderPath += "/";
    tempFolderPath += "gsview/";
//    qDebug() << "Temp path is" << tempFolderPath;

    //  create the folder.
    QDir dir(tempFolderPath);
    if (!dir.exists())
        dir.mkpath(".");

    //  TODO: extract apps to there, then modify gsPath;



    //  set up the UI
    ui->setupUi(this);
    setupToolbar();

    //  create and set up the left-side scrolling area
    m_thumbnails = new ThumbnailList();
    m_thumbnails->setScrollArea(ui->leftScrollArea);
    m_thumbnails->hide();  //  initially hidden
    connect(m_thumbnails, SIGNAL(imagesReady()), this, SLOT(thumbnailsReady()));

    //  create and set up the right-side scrolling area
    m_pages = new PageList();
    m_pages->setScrollArea(ui->rightScrollArea);
    connect(m_pages, SIGNAL(imagesReady()), this, SLOT(pagesReady()));

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

    //  view menu
    connect(ui->actionZoom_In, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(ui->actionZoom_Normal, SIGNAL(triggered()), this, SLOT(normalSize()));

    connect(ui->actionPage_Up, SIGNAL(triggered()), this, SLOT(pageUp()));
    connect(ui->actionPage_Down, SIGNAL(triggered()), this, SLOT(pageDown()));
    connect(ui->actionHome, SIGNAL(triggered()), this, SLOT(homeSlot()));
    connect(ui->actionEnd, SIGNAL(triggered()), this, SLOT(endSlot()));

    connect(ui->actionThumbnails, SIGNAL(triggered()), this, SLOT(actionThumbnails()));
    connect(ui->actionFull_Screen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
    connect(ui->actionAnnotations, SIGNAL(triggered()), this, SLOT(toggleAnnotations()));
    ui->actionAnnotations->setText(tr("Hide &Annotations"));

    //  help menu
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(ui->actionGSView_Help, SIGNAL(triggered()), this, SLOT(help()));

    countWindowUp();
}

void Window::setupToolbar()
{
    //  we do this in code, rather than relying on Qt designer, because
    //  we need to insert non-actions (widgets)

    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSave);
    ui->toolBar->addAction(ui->actionPrint);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionHome);
    ui->toolBar->addAction(ui->actionPage_Up);
    ui->toolBar->addAction(ui->actionPage_Down);
    ui->toolBar->addAction(ui->actionEnd);
    m_pageNumber = new QLineEdit();
    m_pageNumber->setMaximumWidth(30);
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
    m_percentage->setMaximumWidth(30);
    connect ( m_percentage, SIGNAL(returnPressed()), SLOT(percentageEditReturnPressed()));
    QLabel *pct = new QLabel();  pct->setText(tr("%"));
    ui->toolBar->insertWidget(NULL, m_percentage);
    ui->toolBar->insertWidget(NULL, pct);

    ui->toolBar->addAction(ui->actionZoom_Normal);
    ui->toolBar->addAction(ui->actionFull_Screen);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionThumbnails);
    ui->toolBar->addAction(ui->actionAnnotations);
}

Window::~Window()
{
    countWindowDown();
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
                m_pages->zoom (m_scalePage, m_currentPage);
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
    while (!pwdValid)
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr(""), tr("Enter password:"), QLineEdit::Password,
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

    return true;
}

bool Window::OpenFile (QString path)
{
    //  handle .PS and .EPS by running ghostscript first
    QFileInfo fileInfo (path);
    if (fileInfo.suffix().toLower() == QString("ps") ||
        fileInfo.suffix().toLower() == QString("eps")   )
    {
        //  put the result into the temp folder
        QString newPath = tempFolderPath + fileInfo.fileName() + ".pdf";

        //  create a process to do the conversion
        QProcess *process = new QProcess(this);

        //  construct the command
        QString command = gsAppPath;
        command += " -P- -dSAFER -q -P- -dNOPAUSE -dBATCH -sDEVICE=pdfwrite ";
        command += "-sOutputFile=\"" + newPath + "\"";
        command += " -c .setpdfwrite ";
        command += "-f \"" + path + "\"";
//        qDebug("command is: %s", command.toStdString().c_str());

        //  do it, and wait
        process->start(command);
        process->waitForFinished();

        //  now open the temp file.
        return OpenFile2(newPath);
    }
    else
    {
        //  open directly
        return OpenFile2(path);
    }
}

bool Window::OpenFile2 (QString path)
{
    //  load a file by path

    //  open the doc
    bool result = m_document->OpenFile(path.toStdString());
    if (!result)
    {
        QtUtil::errorMessage("Error opening file", "");
        return false;
    }

    //  set the window title
    this->setWindowTitle(path);
    m_path = path;

    //  size and position
    setInitialSizeAndPosition();

    //  ask for password if required
    if (!handlePassword())
        return false;

    updateActions();

    //  set initial page number and count into the toolbar
    int nPages = m_document->GetPageCount();
    m_pageNumber->setText(QString::number(1));
    m_totalPages->setText(QString::number(nPages));

    //  set initial percentage
    m_percentage->setText(QString::number((int)(100*m_scalePage)));

    //  prepare thumbnails
    m_thumbnails->setDocument(m_document);

    //  prepare pages
    m_pages->setDocument(m_document);
    m_pages->setScale(m_scalePage);
    m_pages->buildImages();

    return true;
}

void Window::setInitialSizeAndPosition()
{
    //  calculate size and center
    QDesktopWidget desktop;
    int screenWidth  = desktop.screen()->width();
    int screenHeight = desktop.screen()->height();
    int width = screenWidth*4/5;
    int height = screenHeight*4/5;
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
    //  delete things that were allocated

    if (m_document != NULL)
    {
        m_document->CleanUp();
        delete m_document;
        m_document = NULL;
    }

    this->close();
}

void Window::saveAction()
{
    QMessageBox::information (this, "", "saving is not yet implemented.");
}

// static
void Window::open()
{
    //  create a dialog for choosing a file
    const QStringList desktopLocations = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    QFileDialog dialog(NULL, tr("Open File"),
                       desktopLocations.isEmpty() ? QDir::currentPath() : desktopLocations.first());
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    //  INFO: It's hard to debug in the Qt IDE when we use the native
    //  file dialog.
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Viewable Files (*.pdf *.xps *.cbz *.ps *.eps)"));

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

        //  hide the dialog and process events so it really
        //  disappears before the window appears.
        dialog.hide();
        qApp->processEvents();

        //  attempt to load the file
        newWindow->show();
        if (newWindow->OpenFile(dialog.selectedFiles().first()))
        {
            //  success
            qApp->setActiveWindow(newWindow);
            return;
        }
        newWindow->hide();

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
    if (numWindows()<=0)
        exit(0);
}

int Window::m_numWindows = 0;

void Window::print()
{
    Printer *p = new Printer();
    p->setWindow(this);
    p->print();
}

void Window::zoomIn()
{
    zoom(m_scalePage+m_zoomInc);
}

void Window::zoomOut()
{
    zoom(m_scalePage-m_zoomInc);
}

void Window::normalSize()
{
    zoom (1.0);
}

void Window::zoom (double newScale)
{
    m_scalePage = newScale;

    if (m_scalePage > m_maxScale)
        m_scalePage = m_maxScale;

    if (m_scalePage < m_minScale)
        m_scalePage = m_minScale;

    m_pages->zoom (m_scalePage, m_currentPage);
    m_percentage->setText(QString::number((int)(100*m_scalePage)));
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
    QString message = tr("short version") + tr(" Qt<br/>") + tr("copyright");
    QMessageBox::about(this, tr("About muPDF"), message);
}

void Window::help()
{
    //  TODO
    QString message = "put something here.";//tr((pdfapp_usage(this->gapp)));
    QMessageBox::about(this, tr("How to use muPDF"), message);
}

void Window::updateActions()
{
}

void Window::customEvent (QEvent *event)
{
    switch (event->type())
    {
    case ImageClickedEvent::IMAGE_CLICKED_EVENT:
        {
            int nPage = static_cast<ImageClickedEvent *>(event)->getPageNumber();
            goToPage (nPage);
        }
        break;

    default:
        QMainWindow::customEvent(event);
        break;
    }
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
    m_currentPage = nPage;
    m_pages->goToPage (nPage, true);

    m_thumbnails->hilightImage(nPage);
    m_thumbnails->goToPage(nPage);

    m_pageNumber->setText(QString::number(m_currentPage+1));
}

void Window::toggleAnnotations()
{
    if (m_showAnnotations)
    {
        m_showAnnotations = false;
        m_pages->annot (m_showAnnotations);
        if (ui->leftScrollArea->isVisible())
            m_thumbnails->annot (m_showAnnotations);
        ui->actionAnnotations->setText(tr("Show &Annotations"));
    }
    else
    {
        m_showAnnotations = true;
        m_pages->annot (m_showAnnotations);
        if (ui->leftScrollArea->isVisible())
            m_thumbnails->annot (m_showAnnotations);
        ui->actionAnnotations->setText(tr("Hide &Annotations"));
    }
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
