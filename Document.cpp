#include "Document.h"

Document::Document()
{
    mu_ctx = NULL;
    m_opened = false;
    m_pageCount = 0;
    m_pages = NULL;
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
    if (result == S_ISOK)
    {
        m_opened = true;
        m_pageCount = mu_ctx->GetPageCount();

        //  allocate an array of pages
        m_pages = new Page[m_pageCount];

        return true;
    }
    return false;
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

bool Document::RenderPage (int page_num, double scale, unsigned char *bmp_data, int bmp_width,
                    int bmp_height)
{
    mu_ctx->RenderPage (page_num, bmp_data, bmp_width,
                       bmp_height, scale, false);
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

