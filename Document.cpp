#include "Document.h"

extern "C" {
#include "mupdf/pdf-tools.h"
}

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
    CleanupTextBlocks();

    if (mu_ctx != NULL)
    {
        mu_ctx->CleanUp();
        delete mu_ctx;
        mu_ctx = NULL;
    }
}

void Document::CleanupTextBlocks()
{
    for (int i=0; i<m_pageCount ;i++)
    {
        if (!m_block_list[i].empty())
        {
            int num_blocks = m_block_list[i].size();
            for (int kk = 0; kk < num_blocks; kk++)
            {
                TextBlock *block = (m_block_list[i].at(kk));
                int num_lines = block->line_list->size();
                for (int jj = 0; jj < num_lines; jj++)
                {
                    TextLine *line = (block->line_list->at(jj));
                    int num_chars = line->char_list->size();
                    for (int ii = 0; ii < num_chars; ii++)
                    {
                        TextCharacter *textchar = (line->char_list->at(ii));
                        delete textchar;
                    }
                    delete line;
                }
                delete block;
            }
        }
        m_block_list[i].clear();
    }
    delete [] m_block_list;
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
    m_pageLinks = new PageLinks[m_pageCount];  //  MEMORY

    //  allocate an array of text block lists
    m_block_list = new std::vector<TextBlock *>[m_pageCount];  //  MEMORY

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
        Link *new_link = new Link();  //  MEMORY
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
    if (!m_block_list[page_num].empty())
        return;

    int width;
    int height;
    int num_blocks;
    fz_text_page *text;
    void *text_ptr = (void*)mu_ctx->CreateDisplayListText (page_num, &width, &height, &text, &num_blocks, false);
    if (text_ptr==NULL)
        return;

//    m_block_list[page_num].clear();

    if (num_blocks>0)
    {
        //  for each block
        for (int kk = 0; kk < num_blocks; kk++)
        {
            double top_x = 0, top_y = 0, height = 0, width = 0;

            //  get next block
            int num_lines = mu_ctx->GetTextBlock (text, kk, &top_x, &top_y, &height, &width);

            //  init the block
            TextBlock *block = new TextBlock(page_num, kk);  //  MEMORY
            block->X = top_x;
            block->Y = top_y;
            block->Width = width;
            block->Height = height;
            block->line_list = new std::vector<TextLine *>();  //  MEMORY

            //  add block to the block list
            m_block_list[page_num].push_back(block);

            //  for each line
            for (int jj = 0; jj < num_lines; jj++)
            {
                //  get next line
                int num_chars = mu_ctx->GetTextLine (text, kk, jj, &top_x, &top_y, &height, &width);

                //  init line
                TextLine *line = new TextLine(page_num, kk, jj);  //  MEMORY
                line->X = top_x;
                line->Y = top_y;
                line->Width = width;
                line->Height = height;
                line->char_list = new std::vector<TextCharacter *>();  //  MEMORY

                //  add to the block's line list
                block->line_list->push_back(line);

                for (int mm = 0; mm < num_chars; mm++)
                {
                    int character = mu_ctx->GetTextCharacter(text, kk, jj, mm, &top_x,
                        &top_y, &height, &width);

                    TextCharacter *textchar = new TextCharacter(page_num, kk, jj, mm);  //  MEMORY
                    textchar->X = top_x;
                    textchar->Y = top_y;
                    textchar->Width = width;
                    textchar->Height = height;
                    textchar->character = static_cast<char>(character);

                    //  add to the line
                    line->char_list->push_back(textchar);
                }
            }
        }
    }
}

unsigned int Document::ComputeContents()
{
    //  if we did this already
    if (m_content_items != NULL)
        return m_content_items->size();

    //  generate the contents
    //  We get back a standard smart pointer from muctx interface
    sh_vector_content content_smart_ptr_vec(new std::vector<sh_content>());
    int has_content;
    mutex_lock.lock();
    has_content = mu_ctx->GetContents(content_smart_ptr_vec);
    mutex_lock.unlock();

    //  sorry, none.
    if (!has_content)
        return 0;

    //  make a new vector
    m_content_items = new std::vector<ContentItem *>();

    //  copy the content items
    unsigned int num_items = content_smart_ptr_vec->size();
    for (unsigned int k = 0; k < num_items; k++)
    {
        ContentItem *new_content = new ContentItem();
        sh_content muctx_content = content_smart_ptr_vec->at(k);
        new_content->Page = muctx_content->page;
        new_content->StringMargin = muctx_content->string_margin;
        new_content->StringOrig = muctx_content->string_orig;
        m_content_items->push_back(new_content);
    }
    return num_items;
}

ContentItem *Document::GetContentItem (unsigned int item_num)
{
    if (m_content_items == NULL)
        return NULL;
    if (item_num+1 > m_content_items->size())
        return NULL;

    return m_content_items->at(item_num);
}

void Document::SetAA(int level)
{
    if (mu_ctx != NULL)
        mu_ctx->SetAA(level);
}

void Document::PDFExtract (const char *infile, const char *outfile,
                           const char *password, bool has_password,
                           bool linearize, int num_pages, int *pages)
{
    //  This function uses muTool to extract pages to a file.

    //  make a page list.  If none was supplied, make a range
    //  that includes all the pages.
    std::string pagelist = "";
    if (num_pages>0)
    {
        for (int i=0; i<num_pages; i++)
        {
            if (i>0)
                pagelist += ",";
            pagelist += std::to_string(pages[i]+1);
        }
    }
    else
    {
        pagelist += "1-" + std::to_string(GetPageCount());
    }

    //  count the arguments
    int argc = 3;  //  "clean", input file, output file
    if (has_password)
        argc += 2;
    if (linearize)
        argc += 1;
    argc += 1;  //  always give pages

    //  allocate argument array
    char **argv = new char*[argc];
    int pos = 0;

    //  build the argument list
    static char clean[] = "clean";
    argv[pos++] = clean;
    if (has_password)
    {
        static char dashp[] = "-p";
        argv[pos++] = dashp;
        argv[pos++] = (char *)password;
    }
    if (linearize)
    {
        static char dashl[] = "-l";
        argv[pos++] = dashl;
    }
    argv[pos++] = (char *)infile;
    argv[pos++] = (char *)outfile;
    argv[pos++] = (char *)pagelist.c_str();
    argv[pos] = NULL;  //  done

    //  run the tool
    int result = pdfclean_main (argc, argv);
    UNUSED(result);

    //  clean up
    delete(argv);
}

std::string Document::GetText(int page_num, int type)
{
    if (mu_ctx != NULL)
        return mu_ctx->GetText(page_num, type);

    return "";
}

void Document::SavePage(char *filename, int pagenum, int resolution, int type, bool append)
{
    if (mu_ctx != NULL)
        mu_ctx->SavePage(filename, pagenum, resolution, type, append);
}

std::vector<SearchItem> *Document::SearchText (int page_num, char *textToFind)
{
    if (mu_ctx != NULL)
    {
        //  get the hits
        sh_vector_text text_smart_ptr_vec(new std::vector<sh_text>());
        int hits = mu_ctx->GetTextSearch(page_num, textToFind, text_smart_ptr_vec);

        //  reformat them.
        std::vector<SearchItem> *items = new std::vector<SearchItem>();
        for (int i=0; i<hits; i++)
        {
            sh_text result = text_smart_ptr_vec->at(i);

            SearchItem *item = new SearchItem();
            item->pageNumber = page_num;
            item->left = result->upper_left.X;
            item->top = result->upper_left.Y;
            item->right = result->lower_right.X;
            item->bottom = result->lower_right.Y;

            items->push_back(*item);
        }

        return items;
    }

    return NULL;
}
