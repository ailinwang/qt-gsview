#include "Document.h"

Document::Document()
{
    mu_ctx = nullptr;
    m_opened = false;
}

Document::~Document()
{
    if (mu_ctx != nullptr)
        CleanUp();
}

bool Document::Initialize()
{
    if (mu_ctx != nullptr)
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
        mu_ctx = nullptr;
    }
}

bool Document::OpenFile(const std::string fileName)
{
    status_t result = mu_ctx->OpenDocument((char *)fileName.c_str());
    if (result == S_ISOK)
    {
        m_opened = true;
        Setup();
        return true;
    }
    return false;
}

bool Document::Setup ()
{
    m_currentPage = 0;
    m_scaleFactor = 1.0;

    return true;
}

bool Document::GetCurrentPageSize (point_t *render_size)
{
    return GetPageSize(m_currentPage, render_size);
}

bool Document::GetPageSize (int page_num, point_t *render_size)
{
    int result = mu_ctx->MeasurePage(page_num, render_size);
    if (result != 0)
        return false;

    render_size->X *= m_scaleFactor;
    render_size->Y *= m_scaleFactor;

    return true;
}

bool Document::RenderCurrentPage (unsigned char *bmp_data, int bmp_width,
                    int bmp_height, bool flipy)
{
    return RenderPage (m_currentPage, bmp_data, bmp_width,
            bmp_height, m_scaleFactor, flipy);
}


bool Document::RenderPage (int page_num, unsigned char *bmp_data, int bmp_width,
                    int bmp_height, float scale, bool flipy)
{
    mu_ctx->RenderPage (page_num, bmp_data, bmp_width,
                       bmp_height, scale, flipy);
    return true;
}

int Document::GetPageCount()
{
    return mu_ctx->GetPageCount();
}

