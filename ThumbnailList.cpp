#include "ThumbnailList.h"

ThumbnailList::ThumbnailList()
{
}

double ThumbnailList::getScale()
{
    //  calculate a scale factor based on the width of the left scroll area

    int nPages = document()->GetPageCount();

    //  find max width of the pages
    int maxW = 0;
    for (int i=0; i<nPages; i++)
    {
        point_t pageSize;
        document()->GetPageSize (i, 1.0, &pageSize);
        int w = (int)pageSize.X;
        if (w>maxW)
            maxW = w;
    }

    double theScale = 0.8 * double(getScrollArea()->width())/double(maxW);

    return theScale;
}
