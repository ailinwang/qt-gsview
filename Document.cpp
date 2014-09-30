#include "Document.h"

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
    if (num_items == 0 || num_items == E_FAIL)
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
    mu_ctx->RenderPage (page_num, bmp_data, bmp_width,
                       bmp_height, scale, false, showAnnotations);
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

