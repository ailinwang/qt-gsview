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

    bool GetCurrentPageSize (point_t *render_size);
    bool GetPageSize (int page_num, point_t *render_size);

//    bool RenderCurrentPage (unsigned char *bmp_data, int bmp_width,
//                        int bmp_height, bool flipy);
    bool RenderPage (int page_num, unsigned char *bmp_data, int bmp_width,
                        int bmp_height, bool flipy);

    bool isOpen() {return m_opened;}

    int GetPageCount();

    void SetZoom (double theZoom) {m_scaleFactor = theZoom;}
    double GetZoom() {return m_scaleFactor;}

    void SetPageNumber(int thePage) {m_currentPage=thePage;}
    int GetPageNumber() {return m_currentPage;}

private:

    muctx *mu_ctx;
    int m_currentPage;
    float m_scaleFactor;
    bool m_opened;
    int m_pageCount;
    Page *m_pages;
};

#endif // DOCUMENT_H
