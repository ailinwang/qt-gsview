#pragma once

#include <memory>
#include <functional>
#include <vector>

#ifndef _QT
#include <windows.h>
#else
#include <pthread.h>
#include <string>
#endif

#include "status.h"
#include "Cache.h"

extern "C" {
#include "mupdf/fitz.h"
#ifndef _QT
#include "mupdf/pdf-tools.h"
#endif
}


#define MAX_SEARCH 500

enum { SVG_OUT, PNM_OUT, PCL_OUT, PWG_OUT };
enum { HTML = 0, XML, TEXT };

typedef struct point_s
{
	double X;
	double Y;
} point_t;

/* Links */
typedef struct document_link_s
{
	link_t type;
	point_t upper_left;
	point_t lower_right;
	std::unique_ptr<char[]> uri;
	int page_num;
} document_link_t;
#define sh_link std::shared_ptr<document_link_t>
#define sh_vector_link std::shared_ptr<std::vector<sh_link>>

/* Text Search */
typedef struct text_search_s
{
	point_t upper_left;
	point_t lower_right;
} text_search_t;
#define sh_text std::shared_ptr<text_search_t>
#define sh_vector_text std::shared_ptr<std::vector<sh_text>>

/* Content Results */
typedef struct content_s
{
	int  page;
    std::string string_orig;
	std::string string_margin;
} content_t;
#define sh_content std::shared_ptr<content_t>
#define sh_vector_content std::shared_ptr<std::vector<sh_content>>

#ifdef _WINRT_DLL
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

typedef struct win_stream_struct_s
{
	IRandomAccessStream^ stream;
	unsigned char public_buffer[4096];
} win_stream_struct;
#else
typedef struct win_stream_struct_s
{
	char* stream;
} win_stream_struct;
#endif

/* separations */
typedef struct separation_s
{
    const char *name;
    unsigned int cmyk;
    unsigned int rgba;
} separation_t;

class muctx
{
private:

#ifdef _QT
    pthread_mutex_t mu_criticalsec[FZ_LOCK_MAX];
#else
	CRITICAL_SECTION mu_criticalsec[FZ_LOCK_MAX];
#endif

#ifdef _WINRT_DLL
	win_stream_struct win_stream;
#endif
	fz_locks_context mu_locks;
	fz_context *mu_ctx;
	fz_document *mu_doc;
	fz_outline *mu_outline;
	fz_rect mu_hit_bbox[MAX_SEARCH];
	void FlattenOutline(fz_outline *outline, int level,
						sh_vector_content contents_vec);
    Cache *page_cache;
    Cache *annot_cache;
    Cache *text_cache;

    fz_cookie *m_search_cookie;

public:
	muctx(void);
	~muctx(void);
	void CleanUp(void);
	int GetPageCount();
	status_t InitializeContext();

    status_t RenderPage(int page_num, unsigned char *bmp_data, int bmp_width,
                        int bmp_height, float scale, bool flipy);

    status_t RenderPage(int page_num, unsigned char *bmp_data, int bmp_width,
                        int bmp_height, float scale, bool flipy, bool includeAnnotations);

    status_t RenderPageMT(void *dlist, void *a_dlist, int page_width, int page_height,
							unsigned char *bmp_data, int bmp_width, int bmp_height,
							float scale, bool flipy, bool tile, point_t top_left,
							point_t bottom_right);
	fz_display_list* CreateDisplayList(int page_num, int *width, int *height);
	fz_display_list * CreateDisplayListText(int page_num, int *width,
        int *height, fz_text_page **text, int *length);
	fz_display_list * CreateAnnotationList(int page_num);
	int MeasurePage(int page_num, point_t *size);
	point_t MeasurePage(fz_page *page);
	unsigned int GetLinks(int page_num, sh_vector_link links_vec);
	void SetAA(int level);
	int GetTextSearch(int page_num, char* needle, sh_vector_text texts_vec);
    void AbortTextSearch();
	int GetContents(sh_vector_content contents_vec);
	std::string GetText(int page_num, int type);
	void ReleaseText(void *text);
	bool RequiresPassword(void);
	bool ApplyPassword(char* password);
	status_t SavePage(char *filename, int pagenum, int resolution, int type,
		bool append);

    int GetTextBlock (void *page, int block_num,
        double *top_x, double *top_y, double *height, double *width);
    int GetTextLine(void *page, int block_num, int line_num,
        double *top_x, double *top_y, double *height, double *width);
    int GetTextCharacter(void *page, int block_num, int line_num,
        int item_num, double *top_x, double *top_y, double *height, double *width);

    void freeText(fz_text_page *text);

    status_t MakeProof(char *infile, char *outfile, int resolution);

#ifdef _WINRT_DLL
	status_t InitializeStream(IRandomAccessStream^ readStream, char *ext);
#else
	status_t OpenDocument(char *filename);
#endif

    int getNumSepsOnPage (int page_num);
    status_t getSep (int page_num, int sep, separation_t*separation);
    status_t controlSep (int page_num, int nsep, bool disable);
    bool sepDisabled(int page_num, int nsep);
};
