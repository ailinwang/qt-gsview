#ifndef MUDOCUMENT_H
#define MUDOCUMENT_H

#include "status.h"
#include "muctx.h"

typedef unsigned char Byte;

class muDocument
{
public:
    muDocument();
    ~muDocument();

    bool Initialize ();
    void CleanUp ();

    bool OpenFile (const std::string fileName);

    bool GetCurrentPageSize (point_t *render_size);
    bool GetPageSize (int page_num, point_t *render_size);

    bool RenderCurrentPage (unsigned char *bmp_data, int bmp_width,
                        int bmp_height, bool flipy);
    bool RenderPage (int page_num, unsigned char *bmp_data, int bmp_width,
                        int bmp_height, float scale, bool flipy);

    bool isOpen() {return m_opened;}

    int GetPageCount();

private:
    bool Setup ();

    muctx *mu_ctx;
    int m_currentPage;
    float m_scaleFactor;
    bool m_opened;
};

#endif // MUDOCUMENT_H
