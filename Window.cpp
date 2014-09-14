
#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include "Window.h"

Window::Window()
{
    //  create and initialize the document
    m_document = new Document();
    m_document->Initialize();

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();

    numWindows++;
}

Window::~Window()
{
    if (numWindows>0)
        numWindows--;
}

void Window::keyPressEvent(QKeyEvent* event)
{
    if (event->text().length() > 0)  // modifiers come thru as ""
    {
//        QChar qc = event->text().at(0);
//        int c = qc.toLatin1();
//        //  TODO:  key input
    }
}

bool Window::OpenFile (QString path)
{
    //  load a file by path

    //  TODO handle .PS and .EPS by running ghostscript first

    //  TODO: handle password

    //  open the doc
    bool result = m_document->OpenFile(path.toStdString());
    if (!result)
    {
        Window::errorMessage("Error opening file", "");
        return false;
    }

    //  set the name in this window
    setWindowFilePath (path);

    //  signal a resize
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    //  draw
    drawCurrentPage();

    return true;
}

void Window::errorMessage(const std::string theTitle, const std::string theMessage)
{
    QMessageBox::critical(NULL, QString(theTitle.c_str()), QString(theMessage.c_str()));
}

QImage * imageFromData(unsigned char *samples, int w, int h)
{
    QImage *myImage = new QImage(w, h, QImage::Format_ARGB32);

    int index = 0;
    for (int row=0; row <h ;row++)
    {
        for (int col=0; col <w ;col++)
        {
            //  image comes in as RGBA
            unsigned char red   = samples[index];
            unsigned char green = samples[index+1];
            unsigned char blue  = samples[index+2];
            unsigned char alpha = samples[index+3];
            index+= 4;
            uint pixel = red + (green<<8) + (blue<<16) + (alpha<<24);

            myImage->setPixel (col, row, pixel);
        }
    }

    return myImage;
}

void Window::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void Window::drawCurrentPage()
{
    if (m_document == NULL)
        return;
    if (!m_document->isOpen())
        return;

    //  compute page size
    point_t pageSize;
    m_document->GetCurrentPageSize(&pageSize);
    int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;

    //  render
    Byte *bitmap = new Byte[numBytes];
    m_document->RenderCurrentPage(bitmap, pageSize.X, pageSize.Y, false);

    //  copy to window
    QImage *myImage = imageFromData(bitmap, (int)pageSize.X, (int)pageSize.Y);
    imageLabel->setPixmap(QPixmap::fromImage(*myImage));
    delete myImage;
    delete bitmap;
    imageLabel->adjustSize();
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
            //success
            return;
        }
        newWindow->hide();
    }

    //  user gave up, so delete the window we created.
    delete newWindow;

    //  if no windows are open, quit.
    if (newWindow<=0)
    {
        exit(0);
    }
}

int Window::numWindows = 0;

void Window::print()
{
    //  get a high-res printer
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    dialog->hide();

//    while(qApp->hasPendingEvents())
//        qApp->processEvents();

    //  remember current page and zoom
    double oldZoom = m_document->GetZoom();
    int oldPage = m_document->GetPageNumber();

    //  set new zoom based on printer's resolution
    double newZoom = printer.resolution() / 72;
    m_document->SetZoom(newZoom);

    //  figure out printing range
    int fromPage = 1;
    int toPage = m_document->GetPageCount();
    if (printer.fromPage()>0)
        fromPage = printer.fromPage();
    if (printer.toPage()>0)
        toPage = printer.toPage();
    if (toPage>m_document->GetPageCount())
        toPage = m_document->GetPageCount();

    //  begin printing
    QPainter *painter = new QPainter();
    painter->begin(&printer);

//    int numPages = toPage-fromPage+1;
//    QProgressDialog progress("Printing", "Cancel", 0, numPages, this);
//    progress.setWindowModality(Qt::WindowModal);
//    progress.show();
//    while(qApp->hasPendingEvents())
//        qApp->processEvents();

    bool cancelled = false;

    //  for each page
    int page = fromPage;
    while (page <= toPage)
    {
//        progress.setValue(page-fromPage+1);
//        QString message; message.sprintf("Printing %d of %d ...", page, numPages);
//        progress.setLabelText(message);
//        while(qApp->hasPendingEvents())
//            qApp->processEvents();

//        if (progress.wasCanceled())
//        {
//            cancelled = true;
//            break;
//        }

        //  if not the first page, start a new page
        if (page != fromPage)
            printer.newPage();

        //  compute page size
        point_t pageSize;
        m_document->GetCurrentPageSize(&pageSize);

        //  render a bitmap
        int numBytes = (int)pageSize.X * (int)pageSize.Y * 4;
        Byte *bitmap = new Byte[numBytes];
        m_document->RenderCurrentPage(bitmap, pageSize.X, pageSize.Y, false);

        //  copy to printer
        QImage *myImage = imageFromData(bitmap, (int)pageSize.X, (int)pageSize.Y);
        painter->drawImage(0, 0, *myImage);
        delete myImage;
        delete bitmap;

        page++;
    }

//    progress.hide();
//    while(qApp->hasPendingEvents())
//        qApp->processEvents();

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

    //  restore the original zoom and page
    m_document->SetZoom(oldZoom);
    m_document->SetPageNumber(oldPage);

    //  restore the original page
    drawCurrentPage();
}

void Window::zoomIn()
{
//    pdfapp_onkey (gapp , '+');
}

void Window::zoomOut()
{
//    pdfapp_onkey (gapp , '-');
}

void Window::normalSize()
{
//    pdfapp_zoom_normal (gapp);
}

void Window::pageUp()
{
//    pdfapp_onkey (gapp , ',');
}

void Window::pageDown()
{
//    pdfapp_onkey (gapp , '.');
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
}

//extern "C" void winfullscreen(pdfapp_t *gapp, int state)
//{
//    qDebug("called winfullscreen. %d", state);
//    Window *Window = (Window *) gapp->userdata;
//    if (state==1)
//    {
//        Window->setWindowState(Qt::WindowFullScreen);
//    }
//    else
//    {
//        Window->setWindowState(Qt::WindowNoState);
//    }
//}

