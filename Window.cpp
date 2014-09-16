
#include <QtWidgets>
#include <QAbstractScrollArea>
#include <QPrintDialog>

#include "Window.h"
#include "QtUtil.h"

Window::Window()
{
    //  create and initialize the document
    m_document = new Document();
    m_document->Initialize();

    //  set up menus
    createActions();
    createMenus();

    m_currentPage = 0;

    m_numWindows++;
}

Window::~Window()
{
    if (m_numWindows>0)
        m_numWindows--;
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

    //  ask for password
    if (m_document->RequiresPassword())
    {
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
    }

    //  create scrolling area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidgetResizable(true);
    setCentralWidget(scrollArea);
    m_scrollArea = scrollArea;

    //  inside, create a box with a vertical layout
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setObjectName("m_contentWidget");
    contentWidget->setLayout(new QVBoxLayout(contentWidget));
    scrollArea->setWidget(contentWidget);
    m_contentWidget = contentWidget;

    //  create an array of page images
    int nPages = m_document->GetPageCount();
    m_pageImages = new QLabel[nPages]();
    for (int i=0;i<nPages;i++)
    {
        m_pageImages[i].setBackgroundRole(QPalette::Dark);
        contentWidget->layout()->addWidget(&(m_pageImages[i]));
    }

    //  draw first page
    drawPage(0);

    updateActions();

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
            return;
        }
        newWindow->hide();

        //  if this was the 2nd (or higher) open file,
        //  break out so we don't hit the dialog again.
        if (m_numWindows >= 2)
            break;
    }

    //  user gave up, so delete the window we created.
    delete newWindow;

    //  if no windows are open, quit.
    if (newWindow<=0)
    {
        exit(0);
    }
}

int Window::m_numWindows = 0;

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
    qDebug("zoom in");
}

void Window::zoomOut()
{
    qDebug("zoom out");
}

void Window::normalSize()
{
    qDebug("zoom normal");
}

void Window::pageUp()
{
    if (m_currentPage>0)
    {
        m_currentPage -= 1;
        drawPage(m_currentPage);
        while (qApp->hasPendingEvents())
            qApp->processEvents();

        //  scroll to top of page
        QRect r = m_pageImages[m_currentPage].geometry();
        int scrollTo = r.top()-10;
        if (scrollTo<0)
            scrollTo = 0;
        m_scrollArea->verticalScrollBar()->setValue(scrollTo);
        qDebug ("page up %d %d", m_currentPage, scrollTo);

    }
}

void Window::pageDown()
{
    int numPages = m_document->GetPageCount();
    if (m_currentPage+1<numPages)
    {
        m_currentPage += 1;
        drawPage(m_currentPage);
        while (qApp->hasPendingEvents())
            qApp->processEvents();

        //  scroll to top of page
        QRect r = m_pageImages[m_currentPage].geometry();
        int scrollTo = r.top()-10;
        if (scrollTo<0)
            scrollTo = 0;
        m_scrollArea->verticalScrollBar()->setValue(scrollTo);
        qDebug ("page up %d %d", m_currentPage, scrollTo);
    }
}

void Window::helpAbout()
{
    QString message = tr("short version") + tr(" Qt<br/>") + tr("copyright");
    QMessageBox::about(this, tr("About muPDF"), message);
}

void Window::helpUsage()
{
    //  TODO
    QString message = "put something here.";//tr((pdfapp_usage(this->gapp)));
    QMessageBox::about(this, tr("How to use muPDF"), message);
}

void Window::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openAction()));

    closeAct = new QAction(tr("&Close..."), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeAction()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcut(tr("Ctrl+P"));
    printAct->setEnabled(true);
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoo"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(helpAbout()));

    usageAct = new QAction(tr("&Usage"), this);
    connect(usageAct, SIGNAL(triggered()), this, SLOT(helpUsage()));

    pageUpAct = new QAction(tr("Page &Up"), this);
    pageUpAct->setEnabled(false);
    pageUpAct->setShortcut(tr("Ctrl+U"));
    connect(pageUpAct, SIGNAL(triggered()), this, SLOT(pageUp()));

    pageDownAct = new QAction(tr("Page &Down"), this);
    pageDownAct->setEnabled(false);
    pageDownAct->setShortcut(tr("Ctrl+D"));
    connect(pageDownAct, SIGNAL(triggered()), this, SLOT(pageDown()));

    fullScreenAct = new QAction(tr("Enter &Full Screen"), this);
    fullScreenAct->setEnabled(false);
    fullScreenAct->setShortcut(tr("Ctrl+F"));
    connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
}

void Window::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(closeAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(pageUpAct);
    viewMenu->addAction(pageDownAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fullScreenAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(usageAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(helpMenu);
}

void Window::updateActions()
{
    printAct->setEnabled(true);

    zoomInAct->setEnabled(true);
    zoomOutAct->setEnabled(true);
    normalSizeAct->setEnabled(true);

    pageUpAct->setEnabled(true);
    pageDownAct->setEnabled(true);

    fullScreenAct->setEnabled(true);
}

void Window::toggleFullScreen()
{
    if (windowState() != Qt::WindowFullScreen)
    {
        setWindowState(Qt::WindowFullScreen);
        fullScreenAct->setText(tr("Exit &Full Screen"));
    }
    else
    {
        setWindowState(Qt::WindowNoState);
        fullScreenAct->setText(tr("Enter &Full Screen"));
    }
}

void Window::exitFullScreen()
{
    setWindowState(Qt::WindowNoState);
    fullScreenAct->setText(tr("Enter &Full Screen"));
}

