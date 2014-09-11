#include "mudocument.h"

muDocument::muDocument()
{
    mu_ctx = nullptr;
}

muDocument::~muDocument()
{
    if (mu_ctx != nullptr)
        CleanUp();
}

bool muDocument::Initialize()
{
    if (mu_ctx != nullptr)
        return false;  //  already inited

    mu_ctx = new muctx();
    status_t result = mu_ctx->InitializeContext();
    if (result == S_ISOK)
        return true;
    return false;
}

void muDocument::CleanUp()
{
    if (mu_ctx != NULL)
    {
        mu_ctx->CleanUp();
        delete mu_ctx;
        mu_ctx = nullptr;
    }
}

bool muDocument::OpenFile(const std::string fileName)
{
    status_t result = mu_ctx->OpenDocument((char *)fileName.c_str());
    if (result == S_ISOK)
        return true;
    return false;
}

bool muDocument::ProcessFile ()
{
    return false;
}
