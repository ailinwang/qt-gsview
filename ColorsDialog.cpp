#include "ColorsDialog.h"
#include "Window.h"
#include "Document.h"
#include "ui_ColorsDialog.h"

#include <QCheckBox>
#include <QGraphicsWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

ColorsDialog::ColorsDialog (Window *win, Document *doc, int page, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorsDialog)
{
    ui->setupUi(this);

    //  current doc and page
    m_window = win;
    m_document = doc;
    m_currentpage = page;

    //  get the separations for this page
    m_numSeps = m_document->getNumSepsOnPage(m_currentpage);
    m_checks = new QCheckBox *[m_numSeps];

    //  new layout for the dialog
    QVBoxLayout *vb = new QVBoxLayout;
    this->setLayout(vb);
    vb->setContentsMargins(0,10,0,0);

    //  add a row for each separation
    for (int i=0;i<m_numSeps;i++)
    {
        Separation sep = m_document->getSep(m_currentpage, i);
        bool disabled = m_document->sepDisabled(m_currentpage, i);

        //AddSeparationRow (i, vb, sep.rgba, sep.name);

        QHBoxLayout *hb = new QHBoxLayout();
        vb->addLayout(hb);
        hb->setContentsMargins(10,0,0,0);

        QLabel *label = new QLabel();
        label->setFixedSize(24,24);
        QString style;
        style.sprintf("QLabel { background:#%6.6x; border: 0px none;}", sep.rgba);
        label->setStyleSheet(style);
        hb->addWidget(label);

        QLabel *spacer = new QLabel();
        spacer->setFixedSize(14,2);
        hb->addWidget(spacer);

        QCheckBox *checkbox = new QCheckBox();
        checkbox->setText(QString("  ") + sep.name);
        hb->addWidget(checkbox);
        m_checks[i] = checkbox;
        checkbox->setChecked(!disabled);
    }

    //  ok and cancel buttons
    QHBoxLayout *hb = new QHBoxLayout();
    vb->addLayout(hb);

    QPushButton *cancel = new QPushButton(tr("Cancel"));
    hb->addWidget(cancel);
    connect(cancel, SIGNAL(clicked()), this, SLOT(on_cancelButton_clicked()));

    QPushButton *ok = new QPushButton(tr("OK"));
    hb->addWidget(ok);
    connect(ok, SIGNAL(clicked()), this, SLOT(on_okButton_clicked()));
}

ColorsDialog::~ColorsDialog()
{
    delete ui;
}

void ColorsDialog::show()
{
    resize(sizeHint());
    QDialog::show();
}

void ColorsDialog::on_okButton_clicked()
{
    for (int i=0;i<m_numSeps;i++)
    {
        m_document->controlSep(m_currentpage,i,!(m_checks[i]->isChecked()));
    }

    m_window->reRenderCurrentPage();

    this->hide();
    delete this;
}

void ColorsDialog::on_cancelButton_clicked()
{
    this->hide();
    delete this;
}
