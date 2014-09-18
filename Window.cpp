
#include <QtWidgets>
#include <QAbstractScrollArea>
#include <QAction>
#include <QPrintDialog>

#include "Window.h"
#include "ui_Window.h"

#include "QtUtil.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    setupToolbar();

    //  hide the thumbnails to start
    ui->leftScrollArea->hide();

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

    connect(ui->actionThumbnails, SIGNAL(triggered()), this, SLOT(actionThumbnails()));
    connect(ui->actionFull_Screen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

    //  help menu
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    connect(ui->actionGSView_Help, SIGNAL(triggered()), this, SLOT(help()));

    //  count me
    m_numWindows++;
}

void Window::setupToolbar()
{
    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSave);
    ui->toolBar->addAction(ui->actionPrint);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionPage_Up);
    ui->toolBar->addAction(ui->actionPage_Down);

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
    ui->toolBar->addAction(ui->actionZoom_Normal);

    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->actionThumbnails);

}

Window::~Window()
{
    //  un-count me
    if (m_numWindows>0)
        m_numWindows--;
}

void Window::pageEditReturnPressed()
{
    QString s = m_pageNumber->text();
    if (s.length()>0)
    {
        bool ok;
        int n = s.toInt(&ok);
        if (ok)
        {
            if (n>=1 && n<=m_document->GetPageCount())
            {
                goToPage(n-1, true);
            }
        }
    }
}

void Window::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
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
    //  load a file by path

    //  TODO handle .PS and .EPS by running ghostscript first

    //  open the doc
    bool result = m_document->OpenFile(path.toStdString());
    if (!result)
    {
        Window::errorMessage("Error opening file", "");
        return false;
    }

    //  set the name in this window
    setWindowFilePath (path);

    //  size and position
    setInitialSizeAndPosition();

    if (!handlePassword())
        return false;

    //  set up scrolling area
    m_pageScrollArea = ui->rightScrollArea;
    QWidget* contentWidget =ui->rightScrollAreaWidgetContents;
    contentWidget->setLayout(new QVBoxLayout(contentWidget));
    contentWidget->layout()->setContentsMargins(0,0,0,0);

    //  create an array of page images
    int nPages = m_document->GetPageCount();
    m_pageImages = new QLabel[nPages]();    

    for (int i=0; i<nPages; i++)
    {
        point_t pageSize;
        m_document->GetPageSize(i,m_scalePage, &pageSize);
        m_pageImages[i].setFixedWidth(pageSize.X);
        m_pageImages[i].setFixedHeight(pageSize.Y);

        m_pageImages[i].setBackgroundRole(QPalette::Dark);
        contentWidget->layout()->addWidget(&(m_pageImages[i]));
    }

    //  draw first page
    drawPage(0);

    updateActions();

    //  set initial page number and count into the toolbar
    m_pageNumber->setText(QString::number(1));
    m_totalPages->setText(QString::number(nPages));

    return true;
}

void Window::setInitialSizeAndPosition()
{
    //  size and center
    QDesktopWidget desktop;
    int screenWidth  = desktop.screen()->width();
    int screenHeight = desktop.screen()->height();
    int width = screenWidth*4/5;
    int height = screenHeight*4/5;
    int top  = screenHeight/10;
    int left = screenWidth/10;
    setGeometry(left, top, width, height);
}

void Window::errorMessage(const std::string theTitle, const std::string theMessage)
{
    QMessageBox::critical(NULL, QString(theTitle.c_str()), QString(theMessage.c_str()));
}

void Window::drawPage(int pageNumber)
{
    //  doc must be valid and open
    if (m_document == NULL || !m_document->isOpen())
        return;

    //  compute page size
    point_t pageSize;
    m_document->GetPageSize(pageNumber,m_scalePage, &pageSize);
    int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;

    //  render
    Byte *bitmap = new Byte[numBytes];
    m_document->RenderPage(pageNumber, m_scalePage, bitmap, pageSize.X, pageSize.Y);

    //  copy to window
    QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
    m_pageImages[pageNumber].setPixmap(QPixmap::fromImage(*myImage));
    m_pageImages[pageNumber].adjustSize();

    delete myImage;
    delete bitmap;
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

    //  It's hard to debug in the Qt IDE when we use the native
    //  file dialog.  SO don't for now.
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Viewable Files (*.pdf *.xps *.cbz)"));

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

int Window::numWindows()
{
    //  i was hoping Qt would keep track of open windows.
    return m_numWindows;
}

void Window::print()
{
    //  get the printer
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    dialog->hide();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    while(qApp->hasPendingEvents())
        qApp->processEvents();

    //  get scale factor based on printer's resolution
    double scalePrint = printer.resolution() / 72;

    //  figure out printing range
    int fromPage = 1;
    int toPage = m_document->GetPageCount();
    if (printer.fromPage()>0)
        fromPage = printer.fromPage();
    if (printer.toPage()>0)
        toPage = printer.toPage();
    if (toPage>m_document->GetPageCount())
        toPage = m_document->GetPageCount();

    //  those were 1-based, so subtract
    fromPage -= 1;
    toPage -= 1;

    //  begin printing
    QPainter *painter = new QPainter();
    painter->begin(&printer);

    int numPages = toPage-fromPage+1;
    QProgressDialog progress("Printing", "Cancel", 0, numPages, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    while(qApp->hasPendingEvents())
        qApp->processEvents();

    bool cancelled = false;

    //  for each page
    int page = fromPage;
    while (page <= toPage)
    {
        progress.setValue(page-fromPage+1);
        QString message; message.sprintf("Printing %d of %d ...", page, numPages);
        progress.setLabelText(message);
        while(qApp->hasPendingEvents())
            qApp->processEvents();

        if (progress.wasCanceled())
        {
            cancelled = true;
            break;
        }

        //  if not the first page, start a new page
        if (page != fromPage)
            printer.newPage();

        //  compute page size
        point_t pageSize;
        m_document->GetPageSize(page, scalePrint, &pageSize);

        //  render a bitmap
        int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
        Byte *bitmap = new Byte[numBytes];
        m_document->RenderPage(page, scalePrint, bitmap, pageSize.X, pageSize.Y);

        //  copy to printer
        QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
        painter->drawImage(0, 0, *myImage);

        delete myImage;
        delete bitmap;

        page++;
    }

    progress.hide();
    while(qApp->hasPendingEvents())
        qApp->processEvents();

    QApplication::restoreOverrideCursor();
    while(qApp->hasPendingEvents())
        qApp->processEvents();

    //  end printing
    if (cancelled)
    {
        QMessageBox::information(this, "", "Printing was cancelled.");
    }
    else
    {
        QMessageBox::information(this, "", "Printing is complete.");
    }
    painter->end();

    //  don't need the painter any more
    delete painter;
    painter = NULL;
}

void Window::zoomIn()
{
    QMessageBox::information (this, "", "zooming is not yet implemented.");
}

void Window::zoomOut()
{
    QMessageBox::information (this, "", "zooming is not yet implemented.");
}

void Window::normalSize()
{
    QMessageBox::information (this, "", "zooming is not yet implemented.");
}

void Window::pageUp()
{
    if (m_currentPage>0)
    {
        m_currentPage -= 1;
        goToPage(m_currentPage, true);
    }
}

void Window::pageDown()
{
    int numPages = m_document->GetPageCount();
    if (m_currentPage+1<numPages)
    {
        m_currentPage += 1;
        goToPage(m_currentPage, true);
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

void Window::buildThumbnails()
{
    if (!m_thumbnailsBuilt)
    {
        //  create an array of thumbnail images
        int nPages = m_document->GetPageCount();
        m_thumbnailImages = new Thumbnail[nPages]();

        //  set up scrolling area
        QWidget* contentWidget = ui->leftScrollAreaWidgetContents;
        contentWidget->setLayout(new QVBoxLayout(contentWidget));
        contentWidget->layout()->setContentsMargins(0,0,0,0);

        //  find max width of the pages
        int maxW = 0;
        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize (i, 1.0, &pageSize);
            int w = (int)pageSize.X;
            if (w>maxW)
                maxW = w;
        }

        //  calculate a scale factor based on the width of the left scroll area
        double scaleThumbnail = 0.8 * double(ui->leftScrollArea->width())/double(maxW);

        for (int i=0; i<nPages; i++)
        {
            point_t pageSize;
            m_document->GetPageSize(i, scaleThumbnail, &pageSize);
            m_thumbnailImages[i].setFixedWidth(pageSize.X+10);
            m_thumbnailImages[i].setFixedHeight(pageSize.Y+10);
            m_thumbnailImages[i].setFlat(true);

            m_thumbnailImages[i].setPage(i);
            m_thumbnailImages[i].setWindow(this);

            contentWidget->layout()->addWidget(&(m_thumbnailImages[i]));

            //  render
            int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
            Byte *bitmap = new Byte[numBytes];
            m_document->RenderPage (i, scaleThumbnail, bitmap, pageSize.X, pageSize.Y);

            //  copy to widget
            QImage *myImage = QtUtil::QImageFromData (bitmap, (int)pageSize.X, (int)pageSize.Y);
//            m_thumbnailImages[i].setPixmap(QPixmap::fromImage(*myImage));
//            m_thumbnailImages[i].adjustSize();

            QPixmap pix = QPixmap::fromImage(*myImage);
            QIcon icon(pix);
            m_thumbnailImages[i].setIcon(icon);
            m_thumbnailImages[i].setIconSize(pix.size());

            delete myImage;
            delete bitmap;
        }

        m_thumbnailsBuilt = true;

        //  hilight the current page
        hilightThumb(m_currentPage);
    }
}

void Window::hilightThumb(int nPage)
{
    if (!m_thumbnailsBuilt)
        return;

    int nPages = m_document->GetPageCount();
    for (int i=0; i<nPages; i++)
    {
        Thumbnail *t = &(m_thumbnailImages[i]);
        if (i==nPage)
            t->setStyleSheet("border:4px solid #149B44;");  //  forest green
        else
            t->setStyleSheet("border:0px");
    }
}

void Window::clickedThumb (int nPage)
{    
    hilightThumb(nPage);
    goToPage(nPage, false);
}

void Window::goToPage(int nPage, bool hilight)
{
    m_currentPage = nPage;
    drawPage (m_currentPage);

    //  scroll to top of page
    QRect r = m_pageImages[m_currentPage].geometry();
    int scrollTo = r.top()-10;
    if (scrollTo<0)
        scrollTo = 0;
    m_pageScrollArea->verticalScrollBar()->setValue(scrollTo);
    m_pageNumber->setText(QString::number(m_currentPage+1));

    //  hilight the thumb
    if (hilight)
        hilightThumb(m_currentPage);
}

void Window::actionThumbnails()
{
    if (ui->leftScrollArea->isVisible())
    {
        ui->leftScrollArea->hide();
        ui->actionThumbnails->setChecked(false);
    }
    else
    {
        ui->leftScrollArea->show();
        ui->actionThumbnails->setChecked(true);
        buildThumbnails();
    }
}

void Window::toggleFullScreen()
{
    if (windowState() != Qt::WindowFullScreen)
    {
        setWindowState(Qt::WindowFullScreen);
        ui->actionFull_Screen->setText(tr("Exit &Full Screen"));
    }
    else
    {
        setWindowState(Qt::WindowNoState);
        ui->actionFull_Screen->setText(tr("Enter &Full Screen"));
    }
}

void Window::exitFullScreen()
{
    setWindowState(Qt::WindowNoState);
    ui->actionFull_Screen->setText(tr("Enter &Full Screen"));
}

