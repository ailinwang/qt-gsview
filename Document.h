#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "status.h"
#include "muctx.h"
#include "Page.h"

typedef unsigned char Byte;

class Document
{
public:
    Document();
    ~Document();

    bool Initialize ();
    void CleanUp ();

    bool OpenFile (const std::string fileName);
    bool isOpen() {return m_opened;}

    int  GetPageCount();
    bool GetPageSize (int page_num, double scale, point_t *render_size);
    bool RenderPage (int page_num, double scale, unsigned char *bmp_data, int bmp_width,
                        int bmp_height);

    bool RequiresPassword();
    bool ApplyPassword(const std::string password);

private:

    muctx *mu_ctx;
    bool m_opened;
    int m_pageCount;

    Page *m_pages;
    Page *m_thumbnails;
};

#endif // DOCUMENT_H
