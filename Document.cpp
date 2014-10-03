#include "Document.h"

#include <QString>
#include <QPainter>
#include <QRect>
#include <QPoint>
#include <QBrush>
#include <QColor>

Document::Document()
{
}

Document::~Document()
{
    if (mu_ctx != NULL)
        CleanUp();
}

bool Document::Initialize()
{
    if (mu_ctx != NULL)
        return false;  //  already inited

    mu_ctx = new muctx();
    status_t result = mu_ctx->InitializeContext();
    if (result == S_ISOK)
        return true;

    return false;
}

void Document::CleanUp()
{
    if (mu_ctx != NULL)
    {
        mu_ctx->CleanUp();
        delete mu_ctx;
        mu_ctx = NULL;
    }
}

bool Document::OpenFile(const std::string fileName)
{
    status_t result = mu_ctx->OpenDocument((char *)fileName.c_str());
    if (result != S_ISOK)
        return false;

    //  return false if there are no pages.
    //  this can happen if a file is incorrectly typed,
    //  that is, if a txt file is masquerading as a pdf file.
    m_pageCount = mu_ctx->GetPageCount();
    if (m_pageCount<=0)
        return false;

    m_opened = true;

    //  allocate an array of page links
    m_pageLinks = new PageLinks[m_pageCount];

//        //  allocate an array of pages
//        m_pages = new Page[m_pageCount];

    //  allocate an array of text block lists
    m_block_list = new QVector<TextBlock>[m_pageCount];

    return true;
}

bool Document::GetPageSize (int page_num, double scale, point_t *render_size)
{
    int result = mu_ctx->MeasurePage(page_num, render_size);
    if (result != 0)
        return false;

    render_size->X *= scale;
    render_size->Y *= scale;

    return true;
}

Link * Document::GetLink(int page_num, int link_num)
{
    int numItems = ComputeLinks(page_num);
    if (numItems<=0)
        return NULL;
    if (link_num>=numItems)  //  zero-based
        return NULL;

    return &(m_pageLinks[page_num].links.at(link_num));
}

int Document::ComputeLinks(int page_num)
{
    //  see if links for this page were already done
    if (m_pageLinks[page_num].processed)
        return m_pageLinks[page_num].links.size();

    //  get the links
    sh_vector_link link_smart_ptr_vec(new std::vector<sh_link>());
    mutex_lock.lock();
    int num_items = mu_ctx->GetLinks(page_num, link_smart_ptr_vec);
    mutex_lock.unlock();

    //  no links found for this page
    if (num_items == 0 || num_items == E_FAILURE)
        return 0;

    //  run thru the list and repack
    for (int k = 0; k < num_items; k++)
    {
        Link *new_link = new Link();
        if (new_link == nullptr)
            return 0;

        sh_link muctx_link = link_smart_ptr_vec->at(k);

        new_link->top    = muctx_link->upper_left.Y;
        new_link->left   = muctx_link->upper_left.X;
        new_link->bottom = muctx_link->lower_right.Y;
        new_link->right  = muctx_link->lower_right.X;

        new_link->PageNum = muctx_link->page_num;
        new_link->Type = muctx_link->type;

        if (new_link->Type == LINK_URI)
        {
            new_link->Uri = muctx_link->uri.get();
        }

        //  add to this page's link list
        m_pageLinks[page_num].links.push_back(*new_link);
    }

    m_pageLinks[page_num].processed = true;

    return num_items;
}

bool Document::RenderPage (int page_num, double scale, unsigned char *bmp_data, int bmp_width,
                    int bmp_height, bool showAnnotations)
{

//    mu_ctx->RenderPage (page_num, bmp_data, bmp_width,
//                       bmp_height, scale, false, showAnnotations);

    //  bmp_data has already been allocated by the caller.
    if (bmp_data == NULL)
    {
        return false;
    }

    mutex_lock.lock();

    void *dlist = NULL;
    int page_height;
    int page_width;
    dlist = (void*) mu_ctx->CreateDisplayList (page_num, &page_width, &page_height);

    void *annotlist = NULL;
    if (showAnnotations)
        annotlist = (void*) mu_ctx->CreateAnnotationList (page_num);

    mutex_lock.unlock();

    if (dlist == NULL)
    {
        return false;
    }

    status_t code = mu_ctx->RenderPageMT (dlist, annotlist, page_width, page_height,
                                    &(bmp_data[0]), bmp_width, bmp_height,
                                    scale, false, false,
                                    { double(0), double(0) },
                                    { double(page_width), double(page_height) });
    if (code != S_ISOK)
    {
        return false;
    }

    return true;
}

int Document::GetPageCount()
{
    return m_pageCount;
}

bool Document::RequiresPassword()
{
    return mu_ctx->RequiresPassword();
}

bool Document::ApplyPassword(const std::string password)
{
    bool ok = mu_ctx->ApplyPassword((char *)password.c_str());
    return ok;
}

void Document::ComputeTextBlocks (int page_num)
{
    int width;
    int height;
    int num_blocks;
    fz_text_page *text;
    void *text_ptr = (void*)mu_ctx->CreateDisplayListText (page_num, &width, &height, &text, &num_blocks, false);

    m_block_list[page_num].clear();
    //  TODO: optimize

    if (num_blocks>0)
    {
        //  for each block
        for (int kk = 0; kk < num_blocks; kk++)
        {
            double top_x = 0, top_y = 0, height = 0, width = 0;

            //  get next block
            int num_lines = mu_ctx->GetTextBlock (text, kk, &top_x, &top_y, &height, &width);

            //  init the block
            TextBlock *block = new TextBlock();
            block->X = top_x;
            block->Y = top_y;
            block->Width = width;
            block->Height = height;
            block->PageNumber = page_num;
            block->line_list = new QVector<TextLine>();

            //  add block to the block list
            m_block_list[page_num].push_back(*block);

            //  for each line
            for (int jj = 0; jj < num_lines; jj++)
            {
                //  get next line
                int num_chars = mu_ctx->GetTextLine (text, kk, jj, &top_x, &top_y, &height, &width);

                //  init line
                TextLine *line = new TextLine();
                line->X = top_x;
                line->Y = top_y;
                line->Width = width;
                line->Height = height;
                line->char_list = new QVector<TextCharacter>();

                //  add to the block's line list
                block->line_list->push_back(*line);

                for (int mm = 0; mm < num_chars; mm++)
                {
                    int character = mu_ctx->GetTextCharacter(text, kk, jj, mm, &top_x,
                        &top_y, &height, &width);

                    TextCharacter *textchar = new TextCharacter();
                    textchar->X = top_x;
                    textchar->Y = top_y;
                    textchar->Width = width;
                    textchar->Height = height;
                    textchar->character = static_cast<char>(character);

                    //  add to the
                    line->char_list->push_back(*textchar);
                }
            }
        }
    }
}

void Document::HilightBlocks (double scale, int pageNumber, QPainter *painter)
{
    bool drawBlocks = false;
    bool drawLines = true;
    bool drawChars = false;

    int num_blocks = m_block_list[pageNumber].length();
    for (int kk = 0; kk < num_blocks; kk++)
    {
        TextBlock block = m_block_list[pageNumber].at(kk);

        if (drawBlocks)
        {
            QRect brect ( QPoint(scale*block.X,scale*block.Y),
                          QPoint(scale*(block.X+block.Width),scale*(block.Y+block.Height)));
            painter->fillRect(brect, QBrush(QColor("#506EB3E8")));
        }

        int num_lines = block.line_list->length();
        for (int jj = 0; jj < num_lines; jj++)
        {
            TextLine line = block.line_list->at(jj);

            if (drawLines)
            {
                QRect lrect ( QPoint(scale*line.X,scale*line.Y),
                              QPoint(scale*(line.X+line.Width),scale*(line.Y+line.Height)));
                painter->setPen(QPen(QColor("#ff0000"), 1));
                painter->drawRect(lrect);
            }

            int num_chars = line.char_list->length();
            for (int ii = 0; ii < num_chars; ii++)
            {
                TextCharacter theChar = line.char_list->at(ii);

                if (drawChars)
                {
                    QRect crect ( QPoint(scale*theChar.X,scale*theChar.Y),
                                  QPoint(scale*(theChar.X+theChar.Width),scale*(theChar.Y+theChar.Height)));
                    painter->setPen(QPen(QColor("#0000ff"), 1));
                    painter->drawRect(crect);
                }
            }
        }
    }

}
