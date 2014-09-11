#ifndef MUDOCUMENT_H
#define MUDOCUMENT_H

#include "status.h"
#include "muctx.h"

typedef char Byte;

class muDocument
{
public:
    muDocument();
    status_t Initialize();
    void CleanUp();
    status_t OpenFile(std::string fileName);
    int RenderPage (int page_num,
                    char * bmp_data,
                    int bmp_width,
                    int bmp_height,
                    double scale,
                    bool flipy);

private:
    muctx *mu_ctx;
};

#endif // MUDOCUMENT_H
