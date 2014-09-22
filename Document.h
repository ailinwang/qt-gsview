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
                        int bmp_height, bool showAnnotations);

    bool RequiresPassword();
    bool ApplyPassword(const std::string password);

private:

    muctx *mu_ctx = NULL;
    bool m_opened = false;
    int m_pageCount = 0;

//    Page *m_pages = NULL;
//    Page *m_thumbnails = NULL;
};

#endif // DOCUMENT_H
