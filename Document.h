#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <vector>

#include "status.h"
#include "muctx.h"
#include "Page.h"

class QPainter;

typedef unsigned char Byte;

class Block
{
public:
    double Height;
    double Width;
    double X;
    double Y;
};

class TextCharacter : public Block
{
public:
    char character;
};

class TextLine : public Block
{
public:
    std::vector<TextCharacter> *char_list;
};

class TextBlock : public Block
{
public:
    int PageNumber;
    std::vector<TextLine> *line_list;
};

class Link
{
public:
    int Type = NOT_SET;
    std::string Uri;
    int PageNum = -1;

    int top = -1;
    int bottom = -1;
    int left = -1;
    int right = -1;
};

class PageLinks
{
public:
    bool processed = false;
    std::vector<Link> links;
};

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

    int ComputeLinks (int page_num);
    Link *GetLink(int page_num, int link_num);

    void ComputeTextBlocks (int page_num);

    void HilightBlocks (double scale, int pageNumber, QPainter *painter);

private:

    muctx *mu_ctx = NULL;
    bool m_opened = false;
    int m_pageCount = 0;
    std::mutex mutex_lock;

    PageLinks *m_pageLinks = NULL;

//    Page *m_pages = NULL;
//    Page *m_thumbnails = NULL;

    std::vector<TextBlock> *m_block_list = NULL;
};

#endif // DOCUMENT_H
