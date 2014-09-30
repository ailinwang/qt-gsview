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

int Document::GetLinks(int page_num)
{
    sh_vector_link link_smart_ptr_vec(new std::vector<sh_link>());
    mutex_lock.lock();
    unsigned int num_items = mu_ctx->GetLinks(page_num, link_smart_ptr_vec);
    mutex_lock.unlock();



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

