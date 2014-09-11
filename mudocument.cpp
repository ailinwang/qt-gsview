#include "mudocument.h"

muDocument::muDocument()
{
    mu_ctx = nullptr;
}

status_t muDocument::Initialize()
{
    if (mu_ctx != nullptr)
        return E_FAILURE;  //  already inited

    mu_ctx = new muctx();
    status_t result = mu_ctx->InitializeContext();
    return result;
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

status_t muDocument::OpenFile(std::string fileName)
{
    status_t result = mu_ctx->OpenDocument((char *)fileName.c_str());
    return result;
}

