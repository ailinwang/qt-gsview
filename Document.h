#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <vector>

#include "status.h"
#include "muctx.h"

class QPainter;

typedef unsigned char Byte;

enum
{
    AA_HIGH = 8,
    AA_MEDHIGH = 6,
    AA_MED = 4,
    AA_LOW = 2,
    AA_NONE = 0
};

class ContentItem
{
public:
    std::string StringOrig;
    std::string StringMargin;
    int Page = 0;
};

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
    TextCharacter(int pageNum, int blockNum, int lineNum, int charNum)
        {PageNumber=pageNum;BlockNumber=blockNum;LineNumber=lineNum;CharNumber=charNum;}
    int PageNumber;
    int BlockNumber;
    int LineNumber;
    int CharNumber;
    char character;
};

class TextLine : public Block
{
public:
    TextLine(int pageNum, int blockNum, int lineNum)
        {PageNumber=pageNum;BlockNumber=blockNum;LineNumber=lineNum;}
    int PageNumber;
    int BlockNumber;
    int LineNumber;
    std::vector<TextCharacter> *char_list;
};

class TextBlock : public Block
{
public:
    TextBlock(int pageNum, int blockNum)
        {PageNumber=pageNum;BlockNumber=blockNum;}
    int PageNumber;
    int BlockNumber;
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

    std::vector<TextBlock> *blockList() {return m_block_list;}

    unsigned int ComputeContents();
    ContentItem *GetContentItem(unsigned int item_num);

    void SetAA(int level);

private:

    muctx *mu_ctx = NULL;
    bool m_opened = false;
    int m_pageCount = 0;
    std::mutex mutex_lock;

    PageLinks *m_pageLinks = NULL;

    std::vector<TextBlock> *m_block_list = NULL;

    std::vector<ContentItem *> *m_content_items = NULL;
};

#endif // DOCUMENT_H
