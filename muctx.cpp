#ifndef _QT
#pragma once
#endif

#ifndef _QT
#include "pch.h"
#endif
#include "muctx.h"
#include "Cache.h"
#include <mutex>

/* This class interfaces to mupdf API with minimal windows objects
 * (other than the file streaming stuff) */
//static int textlen(fz_text_page *page);

#ifdef _WINRT_DLL
// Attempt to use t.wait()
//#include <ppltasks.h>
//using namespace concurrency;
/* File streaming set up for WinRT */
static int win_next_file(fz_stream *stm, int len)
{
	void *temp = stm->state;
	win_stream_struct *state = reinterpret_cast <win_stream_struct*> (temp);
	IRandomAccessStream^ Stream = state->stream;
	unsigned char *buf = state->public_buffer;
	unsigned long long curr_pos = Stream->Position;
	unsigned long long length = Stream->Size;
	DataReader^ local_reader = ref new DataReader(Stream);
	if (local_reader == nullptr)
		return 0;

	// This does not work here.  mupdf is not set up to wait for win_next_file
	// to complete in an ansyn manner 
	//auto t = create_task(local_reader->LoadAsync(len));
	//t.wait();
	DataReaderLoadOperation^ result = local_reader->LoadAsync(len);
	while (result->Status != AsyncStatus::Completed) {
	}
	result->GetResults();

	/* First see what is available */
	int curr_len2 = local_reader->UnconsumedBufferLength;
	if (curr_len2 < len)
		len = curr_len2;

	/* And make sure that we have enough room */
	if (len > sizeof(state->public_buffer))
		len = sizeof(state->public_buffer);

	Platform::Array<unsigned char>^ arrByte = ref new Platform::Array<unsigned char>(len);
	if (arrByte == nullptr)
		return 0;
	local_reader->ReadBytes(arrByte);

	memcpy(buf, arrByte->Data, len);
	local_reader->DetachStream();

	stm->rp = buf;
	stm->wp = buf + len;
	stm->pos += len;
	if (len == 0)
		return EOF;
	return *stm->rp++;
}

static void win_seek_file(fz_stream *stm, int offset, int whence)
{
	void *temp = stm->state;
	win_stream_struct *stream = reinterpret_cast <win_stream_struct*> (temp);
	IRandomAccessStream^ Stream = stream->stream;
	unsigned long long curr_pos = Stream->Position;
	unsigned long long length = Stream->Size;
	unsigned long long n;

	if (whence == SEEK_END)
	{
		n = length + offset;
	}
	else if (whence == SEEK_CUR)
	{
		n = curr_pos + offset;
	}
	else if (whence == SEEK_SET)
	{
		n = offset;
	}
	Stream->Seek(n);
	curr_pos = Stream->Position;
	stm->pos = n;
	stm->wp = stm->rp;
}

static void win_close_file(fz_context *ctx, void *state)
{
	win_stream_struct *win_stream = reinterpret_cast <win_stream_struct*> (state);
	IRandomAccessStream^ stream = win_stream->stream;
	delete stream;
}

status_t muctx::InitializeStream(IRandomAccessStream^ readStream, char *ext)
{
	win_stream.stream = readStream;
	fz_stream *mu_stream = fz_new_stream(mu_ctx, 0, win_next_file, win_close_file, NULL);
	mu_stream->seek = win_seek_file;
	mu_stream->state = reinterpret_cast <void*> (&win_stream);

	/* Now lets see if we can open the file */
	fz_try(mu_ctx)
	{
		mu_doc = fz_open_document_with_stream(mu_ctx, ext, mu_stream);
	}
	fz_always(mu_ctx)
	{
		fz_close(mu_stream);
	}
	fz_catch(mu_ctx)
	{
		return E_FAILURE;
	}
	return S_ISOK;
}
#else

status_t muctx::OpenDocument(char *filename)
{
    fz_try(mu_ctx)
    {
        this->mu_doc = fz_open_document(mu_ctx, filename);
    }
    fz_catch(mu_ctx)
    {
        return E_FAILURE;
    }
	return S_ISOK;
}

#endif

/* mutext functions see mupdf readme for details */
static void lock_mutex(void *user, int lock)
{
#ifdef _QT
    pthread_mutex_t *mutex = (pthread_mutex_t *) user;
    if (pthread_mutex_lock(&mutex[lock]) != 0)
    {
        //  TODO
        //  fail("pthread_mutex_lock()");
    }

#else
	LPCRITICAL_SECTION locks = (LPCRITICAL_SECTION)user;
	EnterCriticalSection(&locks[lock]);
#endif
}

static void unlock_mutex(void *user, int lock)
{
#ifdef _QT
    pthread_mutex_t *mutex = (pthread_mutex_t *) user;
    if (pthread_mutex_unlock(&mutex[lock]) != 0)
    {
        //  TODO
        //  fail("pthread_mutex_unlock()");
    }

#else
    LPCRITICAL_SECTION locks = (LPCRITICAL_SECTION)user;
	LeaveCriticalSection(&locks[lock]);
#endif
}

void muctx::CleanUp(void)
{
	fz_free_outline(mu_ctx, mu_outline);
	fz_close_document(mu_doc);
    page_cache->Empty(mu_ctx);
    annot_cache->Empty(mu_ctx);
    text_cache->Empty(mu_ctx);
    fz_free_context(mu_ctx);

	delete page_cache;
    delete annot_cache;
    delete text_cache;

    annot_cache = NULL;
    page_cache = NULL;
    text_cache = NULL;

    this->mu_ctx = NULL;
	this->mu_doc = NULL;
	this->mu_outline = NULL;
}

void muctx::SetAA(int level)
{
	fz_set_aa_level(mu_ctx, level);
}

/* Set up the context, mutex and cookie */
status_t muctx::InitializeContext()
{
	int i;

	/* Get the mutexes set up */
	for (i = 0; i < FZ_LOCK_MAX; i++)
    {
#ifdef _QT
        pthread_mutex_init (&(mu_criticalsec[i]), NULL);
#else
		InitializeCriticalSectionEx(&mu_criticalsec[i], 0, 0);
#endif
    }
	mu_locks.user = &mu_criticalsec[0];
	mu_locks.lock = lock_mutex;
	mu_locks.unlock = unlock_mutex;

	/* Allocate the context */
	this->mu_ctx = fz_new_context(NULL, &mu_locks, FZ_STORE_DEFAULT);
	if (this->mu_ctx == NULL)
	{
		return E_OUTOFMEM;
	}
	else
	{
		fz_register_document_handlers(this->mu_ctx);
		return S_ISOK;
	}
}

/* Initializer */
muctx::muctx(void)
{
	mu_ctx = NULL;
	mu_doc = NULL;
	mu_outline = NULL;
	page_cache = new Cache();
    annot_cache = new Cache();
    text_cache = new Cache();
}

/* Destructor */
muctx::~muctx(void)
{
	fz_free_outline(mu_ctx, mu_outline);
	fz_close_document(mu_doc);
	page_cache->Empty(mu_ctx);
    annot_cache->Empty(mu_ctx);
    text_cache->Empty(mu_ctx);
    fz_free_context(mu_ctx);

	mu_ctx = NULL;
	mu_doc = NULL;
	mu_outline = NULL;

	delete page_cache;
	page_cache = NULL;

    delete annot_cache;
    annot_cache = NULL;

    delete text_cache;
    text_cache = NULL;
}

/* Return the documents page count */
int muctx::GetPageCount()
{
	if (this->mu_doc == NULL)
		return -1;
	else
		return this->mu_doc->count_pages(this->mu_doc);
}

/* Get page size */
int muctx::MeasurePage(int page_num, point_t *size)
{
	fz_rect rect;
	fz_page *page = NULL;
	fz_rect *bounds;

	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		bounds = fz_bound_page(mu_doc, page, &rect);
		size->X = bounds->x1 - bounds->x0;
		size->Y = bounds->y1 - bounds->y0;
	}
	fz_always(mu_ctx)
	{
		fz_free_page(mu_doc, page);
	}
	fz_catch(mu_ctx)
	{
        return E_FAILURE;
	}
	return 0;
}

/* Get page size */
point_t muctx::MeasurePage(fz_page *page)
{
	point_t pageSize;
	fz_rect rect;
	fz_rect *bounds;

	bounds = fz_bound_page(mu_doc, page, &rect);
	pageSize.X = bounds->x1 - bounds->x0;
	pageSize.Y = bounds->y1 - bounds->y0;

	return pageSize;
}

void muctx::FlattenOutline(fz_outline *outline, int level,
			  sh_vector_content contents_vec)
{
	char indent[8*4+1];
	if (level > 8)
		level = 8;
	memset(indent, ' ', level * 4);
	indent[level * 4] = 0;

	std::string indent_str = indent;
	std::string str_indent;

	while (outline)
	{
		if (outline->dest.kind == FZ_LINK_GOTO)
		{
			int page = outline->dest.ld.gotor.page;
			if (page >= 0 && outline->title)
			{
				/* Add to the contents std:vec */
				sh_content content_item(new content_t());
				content_item->page = page;
				content_item->string_orig = outline->title;
				str_indent = content_item->string_orig;
				str_indent.insert(0, indent_str);
				content_item->string_margin = str_indent;
				contents_vec->push_back(content_item);
			}
		}
		FlattenOutline(outline->down, level + 1, contents_vec);
		outline = outline->next;
	}
}

int muctx::GetContents(sh_vector_content contents_vec)
{
	fz_outline *root = NULL;
	int has_content = 0;

	fz_var(root);
	fz_try(mu_ctx)
	{
		root = fz_load_outline(mu_doc);
		if (root != NULL)
		{
			has_content = 1;
			FlattenOutline(root, 0, contents_vec);
		}
	}
	fz_always(mu_ctx)
	{
		fz_free_outline(mu_ctx, root);
	}
	fz_catch(mu_ctx)
	{
        return E_FAILURE;
	}
	return has_content;
}

int muctx::GetTextSearch(int page_num, char* needle, sh_vector_text texts_vec)
{
	fz_page *page = NULL;
	fz_text_sheet *sheet = NULL;
	fz_device *dev = NULL;
	fz_text_page *text = NULL;
	int hit_count = 0;
	int k;

	fz_var(page);
	fz_var(sheet);
	fz_var(dev);
	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		sheet = fz_new_text_sheet(mu_ctx);
		text = fz_new_text_page(mu_ctx);
		dev = fz_new_text_device(mu_ctx, sheet, text);
		fz_run_page(mu_doc, page, dev, &fz_identity, NULL);
		fz_free_device(dev);  /* Why does this need to be done here?  Seems odd */
		dev = NULL;
		hit_count = fz_search_text_page(mu_ctx, text, needle, mu_hit_bbox, nelem(mu_hit_bbox));

		for (k = 0; k < hit_count; k++)
		{
			sh_text text_search(new text_search_t());
			text_search->upper_left.X = mu_hit_bbox[k].x0;
			text_search->upper_left.Y = mu_hit_bbox[k].y0;
			text_search->lower_right.X = mu_hit_bbox[k].x1;
			text_search->lower_right.Y = mu_hit_bbox[k].y1;
			texts_vec->push_back(text_search);
		}
	}
	fz_always(mu_ctx)
	{
		fz_free_page(mu_doc, page);
		fz_free_device(dev);
		fz_free_text_sheet(mu_ctx, sheet);
		fz_free_text_page(mu_ctx, text);
	}
	fz_catch(mu_ctx)
	{
        return E_FAILURE;
	}
	return hit_count;
}

/* Get the links and pack into a smart pointer structure */
unsigned int muctx::GetLinks(int page_num, sh_vector_link links_vec)
{
	fz_page *page = NULL;
	fz_link *links = NULL;
//	int k = 0;
	unsigned int num_links = 0;

	fz_var(page);
	fz_var(links);
	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		links = fz_load_links(mu_doc, page);

		fz_link *curr_link = links;
		if (curr_link != NULL)
		{
			/* Get our smart pointer structure filled */
			while (curr_link != NULL)
			{
				fz_rect curr_rect = curr_link->rect;
				sh_link link(new document_link_t());

				link->upper_left.X = curr_rect.x0;
				link->upper_left.Y = curr_rect.y0;
				link->lower_right.X = curr_rect.x1;
				link->lower_right.Y = curr_rect.y1;

				switch (curr_link->dest.kind)
				{
				case FZ_LINK_GOTO:

					link->type = LINK_GOTO;
					link->page_num = curr_link->dest.ld.gotor.page;
					break;

				case FZ_LINK_URI:
				{
					int lenstr = strlen(curr_link->dest.ld.uri.uri);
					std::unique_ptr<char[]> uri(new char[lenstr + 1]);
#ifdef _QT
                    strncpy(uri.get(), curr_link->dest.ld.uri.uri, lenstr + 1);
#else
					strcpy_s(uri.get(), lenstr + 1, curr_link->dest.ld.uri.uri);
#endif
					link->uri.swap(uri);
					link->type = LINK_URI;
					break;
				}

				default:
					link->type = NOT_SET;

				}
				links_vec->push_back(link);
				curr_link = curr_link->next;
				num_links += 1;
			}
		}
	}
	fz_always(mu_ctx)
	{
		fz_free_page(mu_doc, page);
		fz_drop_link(mu_ctx, links);
	}
	fz_catch(mu_ctx)
	{
        return E_FAILURE;
	}
	return num_links;
}

fz_display_list * muctx::CreateAnnotationList(int page_num)
{
	fz_device *dev = NULL;
	fz_page *page = NULL;
	int width, height;

	/* First see if we have this one in the cache */
	fz_display_list *dlist = annot_cache->Use(page_num, &width, &height, mu_ctx);
	if (dlist != NULL)
		return dlist;

	/* Apparently not, lets go ahead and create and add to cache */
	fz_var(dev);
	fz_var(page);
	fz_var(dlist);

	fz_try(mu_ctx)
	{
		fz_annot *annot;
		page = fz_load_page(mu_doc, page_num);
		annot = fz_first_annot(mu_doc, page);
		if (annot != NULL)
		{
			/* Create display list */
			dlist = fz_new_display_list(mu_ctx);
			dev = fz_new_list_device(mu_ctx, dlist);

			for (annot = fz_first_annot(mu_doc, page); annot; annot = fz_next_annot(mu_doc, annot))
				fz_run_annot(mu_doc, page, annot, dev, &fz_identity, NULL);
			annot_cache->Add(page_num, 0, 0, dlist, mu_ctx);
		}
	}
	fz_always(mu_ctx)
	{
		fz_free_device(dev);
		fz_free_page(mu_doc, page);
	}
	fz_catch(mu_ctx)
	{
		fz_drop_display_list(mu_ctx, dlist);
		return NULL;
	}
	return dlist;
}

fz_display_list * muctx::CreateDisplayList(int page_num, int *width, int *height)
{
	fz_device *dev = NULL;
	fz_page *page = NULL;
	point_t page_size;

	/* First see if we have this one in the cache */
	fz_display_list *dlist = page_cache->Use(page_num, width, height, mu_ctx);
	if (dlist != NULL)
		return dlist;

	/* Apparently not, lets go ahead and create and add to cache */
	fz_var(dev);
	fz_var(page);
	fz_var(dlist);

	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);

		/* Create a new list */
		dlist = fz_new_display_list(mu_ctx);
		dev = fz_new_list_device(mu_ctx, dlist);
		fz_run_page_contents(mu_doc, page, dev, &fz_identity, NULL);
		page_size = MeasurePage(page);
		*width = page_size.X;
		*height = page_size.Y;
		/* Add it to the cache and set that it is in use */
		page_cache->Add(page_num, *width, *height, dlist, mu_ctx);
	}
	fz_always(mu_ctx)
	{
		fz_free_device(dev);
		fz_free_page(mu_doc, page);
	}
	fz_catch(mu_ctx)
	{
		fz_drop_display_list(mu_ctx, dlist);
		return NULL;
	}
	return dlist;
}

/* A special version which will create the display list AND get the information
   that we need for various text selection tasks */
fz_display_list * muctx::CreateDisplayListText(int page_num, int *width, int *height,
    fz_text_page **text_out, int *length, bool useCache)
{
	fz_text_sheet *sheet = NULL;
	fz_text_page *text = NULL;
	fz_device *dev = NULL;
	fz_device *textdev = NULL;
	fz_page *page = NULL;

	point_t page_size;
	*length = 0;

    /* First see if we have this one in the cache */
    fz_display_list *dlist = NULL;
    //if (useCache)
    {
        dlist = text_cache->Use(page_num, width, height, mu_ctx);
        if (dlist != NULL)
            return dlist;
    }

	/* Apparently not, lets go ahead and create and add to cache */
	fz_var(dev);
	fz_var(textdev);
	fz_var(page);
	fz_var(dlist);
	fz_var(sheet);
	fz_var(text);

	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		sheet = fz_new_text_sheet(mu_ctx);
        text = fz_new_text_page(mu_ctx);

		/* Create a new list */
        dlist = fz_new_display_list(mu_ctx);
		dev = fz_new_list_device(mu_ctx, dlist);

		/* Deal with text device */
		textdev = fz_new_text_device(mu_ctx, sheet, text);
		fz_run_page(mu_doc, page, textdev, &fz_identity, NULL);

		*length = text->len;
		fz_free_device(textdev);
		textdev = NULL;
		*text_out = text;

		fz_run_page_contents(mu_doc, page, dev, &fz_identity, NULL);
		page_size = MeasurePage(page);
		*width = page_size.X;
		*height = page_size.Y;
		/* Add it to the cache and set that it is in use */
        //if (useCache)
            text_cache->Add(page_num, *width, *height, dlist, mu_ctx);
	}
	fz_always(mu_ctx)
	{
		fz_free_device(dev);
		fz_free_page(mu_doc, page);
		fz_free_text_sheet(mu_ctx, sheet);
        fz_drop_display_list(mu_ctx, dlist);
	}
	fz_catch(mu_ctx)
	{
		fz_drop_display_list(mu_ctx, dlist);
		return NULL;
	}
	return dlist;
}

/* Render display list bmp_data buffer.  No lock needed for this operation */
status_t muctx::RenderPageMT(void *dlist, void *a_dlist, int page_width, int page_height, 
							 unsigned char *bmp_data, int bmp_width, int bmp_height,
							 float scale, bool flipy, bool tile, point_t top_left,
							 point_t bottom_right)
{
    UNUSED(page_width);
    UNUSED(tile);
    UNUSED(bottom_right);

	fz_device *dev = NULL;
	fz_pixmap *pix = NULL;
	fz_matrix ctm, *pctm = &ctm;
	fz_context *ctx_clone = NULL;
	fz_display_list *display_list = (fz_display_list*) dlist; 
	fz_display_list *annot_displaylist = (fz_display_list*) a_dlist;

	ctx_clone = fz_clone_context(mu_ctx);

	fz_var(dev);
	fz_var(pix);
	fz_var(display_list);
	fz_var(annot_displaylist);

	fz_try(ctx_clone)
	{
		pctm = fz_scale(pctm, scale, scale);
		/* Flip on Y. */
		if (flipy) 
		{
			ctm.f = (float) page_height * ctm.d;
			ctm.d = -ctm.d;
			ctm.f += top_left.Y;
		}
		else
		{
			ctm.f -= top_left.Y;
		}
		ctm.e -= top_left.X;

		pix = fz_new_pixmap_with_data(ctx_clone, fz_device_bgr(ctx_clone),
										bmp_width, bmp_height, bmp_data);
		fz_clear_pixmap_with_value(ctx_clone, pix, 255);
		dev = fz_new_draw_device(ctx_clone, pix);
		fz_run_display_list(display_list, dev, pctm, NULL, NULL);
		if (annot_displaylist != NULL)
			fz_run_display_list(annot_displaylist, dev, pctm, NULL, NULL);

	}
	fz_always(ctx_clone)
	{
		fz_free_device(dev);
		fz_drop_pixmap(ctx_clone, pix);
        fz_drop_display_list(ctx_clone, display_list);
        fz_drop_display_list(ctx_clone, annot_displaylist);
	}
	fz_catch(ctx_clone)
	{
		fz_free_context(ctx_clone);
		return E_FAILURE;
	}
	fz_free_context(ctx_clone);
	return S_ISOK;
}

/* Render page_num to size width by height into bmp_data buffer.  Lock needed. */
status_t muctx::RenderPage(int page_num, unsigned char *bmp_data, int bmp_width,
                           int bmp_height, float scale, bool flipy)
{
    return RenderPage(page_num, bmp_data, bmp_width, bmp_height, scale, flipy, true);
}

status_t muctx::RenderPage(int page_num, unsigned char *bmp_data, int bmp_width,
                           int bmp_height, float scale, bool flipy, bool includeAnnotations)
{
	fz_device *dev = NULL;
	fz_pixmap *pix = NULL;
	fz_page *page = NULL;
	fz_matrix ctm, *pctm = &ctm;
	point_t page_size;

	fz_var(dev);
	fz_var(pix);
	fz_var(page);

	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		page_size = MeasurePage(page);
		pctm = fz_scale(pctm, scale, scale);
		/* Flip on Y */
		if (flipy)
		{
			ctm.f = bmp_height;
			ctm.d = -ctm.d;
		}
		pix = fz_new_pixmap_with_data(mu_ctx, fz_device_bgr(mu_ctx), bmp_width, 
										bmp_height, bmp_data);
		fz_clear_pixmap_with_value(mu_ctx, pix, 255);
		dev = fz_new_draw_device(mu_ctx, pix);
//		fz_run_page(mu_doc, page, dev, pctm, NULL);
        fz_run_page_contents(mu_doc, page, dev, pctm, NULL);

        if (includeAnnotations)
        {
            fz_annot *annot;
            for (annot = fz_first_annot(mu_doc, page); annot; annot = fz_next_annot(mu_doc, annot))
//                fz_run_annot(mu_doc, page, annot, dev, &fz_identity, NULL);
                  fz_run_annot(mu_doc, page, annot, dev, pctm, NULL);
        }
	}
	fz_always(mu_ctx)
	{
		fz_free_device(dev);
		fz_drop_pixmap(mu_ctx, pix);
		fz_free_page(mu_doc, page);
	}
	fz_catch(mu_ctx)
	{
		return E_FAILURE;
	}
	return S_ISOK;
}

bool muctx::RequiresPassword(void)
{
	return fz_needs_password(mu_doc) != 0;
}

bool muctx::ApplyPassword(char* password)
{
	return fz_authenticate_password(mu_doc, password) != 0;
}

std::string muctx::GetText(int page_num, int type)
{
	fz_output *out = NULL;
	fz_device *dev = NULL;
	fz_page *page = NULL;
	fz_text_sheet *sheet = NULL;
	fz_text_page *text = NULL;
	fz_buffer *buf = NULL;
	std::string output;

	fz_var(dev);
	fz_var(page);
	fz_var(sheet);
	fz_var(text);
	fz_var(buf);
	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		sheet = fz_new_text_sheet(mu_ctx);
		text = fz_new_text_page(mu_ctx);
		dev = fz_new_text_device(mu_ctx, sheet, text);
		fz_run_page(mu_doc, page, dev, &fz_identity, NULL);
		fz_free_device(dev);
		dev = NULL;
		fz_analyze_text(mu_ctx, sheet, text);
		buf = fz_new_buffer(mu_ctx, 256);
		out = fz_new_output_with_buffer(mu_ctx, buf);
		if (type == HTML)
		{
			fz_print_text_page_html(mu_ctx, out, text);
		}
		else if (type == XML)
		{
			fz_print_text_page_xml(mu_ctx, out, text);
		}
		else
		{
			fz_print_text_page(mu_ctx, out, text);
		}
		output = std::string(((char*)buf->data));
	}
	fz_always(mu_ctx)
	{
		fz_free_device(dev);
		fz_free_page(mu_doc, page);
		fz_free_text_sheet(mu_ctx, sheet);
		fz_free_text_page(mu_ctx, text);
		fz_drop_buffer(mu_ctx, buf);
	}
	fz_catch(mu_ctx)
	{
		return nullptr;
	}
	return output;
}

void muctx::ReleaseText(void *text)
{
	fz_text_page *text_page = (fz_text_page*) text;
	fz_free_text_page(mu_ctx, text_page);
}

/* To do: banding */
status_t muctx::SavePage(char *filename, int page_num, int resolution, int type,
	bool append)
{
	float zoom;
	fz_matrix ctm;
	fz_rect bounds, tbounds;
	FILE *file = NULL;
	fz_output *out = NULL;
	fz_device *dev = NULL;
	int width, height;
	fz_display_list *dlist = NULL;
	fz_display_list *annot_dlist = NULL;
	fz_page *page = NULL;
//	bool valid = true;
	fz_pixmap *pix = NULL;
	fz_irect ibounds;

	fz_var(dev);
	fz_var(page);
	fz_var(dlist);
	fz_var(annot_dlist);
	fz_var(pix);

	fz_try(mu_ctx)
	{
		page = fz_load_page(mu_doc, page_num);
		fz_bound_page(mu_doc, page, &bounds);
		zoom = resolution / 72;
		fz_scale(&ctm, zoom, zoom);
		tbounds = bounds;
		fz_transform_rect(&tbounds, &ctm);
		fz_round_rect(&ibounds, &tbounds);

		/* First see if we have this one in the cache */
		dlist = page_cache->Use(page_num, &width, &height, mu_ctx);
		annot_dlist = annot_cache->Use(page_num, &width, &height, mu_ctx);

		if (type == SVG_OUT)
		{
			file = fopen(filename, "wb");
			if (file == NULL)
				fz_throw(mu_ctx, FZ_ERROR_GENERIC, "cannot open file '%s'", filename);
			out = fz_new_output_with_file(mu_ctx, file);

			dev = fz_new_svg_device(mu_ctx, out, tbounds.x1 - tbounds.x0, tbounds.y1 - tbounds.y0);
			if (dlist != NULL)
				fz_run_display_list(dlist, dev, &ctm, &tbounds, NULL);
			else
				fz_run_page(mu_doc, page, dev, &ctm, NULL);
			if (annot_dlist != NULL)
				fz_run_display_list(annot_dlist, dev, &ctm, &tbounds, NULL);
			else
			{
				fz_annot *annot;
				for (annot = fz_first_annot(mu_doc, page); annot; annot = fz_next_annot(mu_doc, annot))
					fz_run_annot(mu_doc, page, annot, dev, &fz_identity, NULL);
			}
		}
		else
		{
			pix = fz_new_pixmap_with_bbox(mu_ctx, fz_device_rgb(mu_ctx), &ibounds);
			fz_pixmap_set_resolution(pix, resolution);
			fz_clear_pixmap_with_value(mu_ctx, pix, 255);
			dev = fz_new_draw_device(mu_ctx, pix);
			if (dlist != NULL)
				fz_run_display_list(dlist, dev, &ctm, &tbounds, NULL);
			else
				fz_run_page(mu_doc, page, dev, &ctm, NULL);
			if (annot_dlist != NULL)
				fz_run_display_list(annot_dlist, dev, &ctm, &tbounds, NULL);
			else
			{
				fz_annot *annot;
				for (annot = fz_first_annot(mu_doc, page); annot; annot = fz_next_annot(mu_doc, annot))
					fz_run_annot(mu_doc, page, annot, dev, &fz_identity, NULL);
			}
			switch (type)
			{
				case PNM_OUT:
					fz_write_pnm(mu_ctx, pix, filename);
					break;
				case PCL_OUT: /* This can do multi-page */
					fz_pcl_options options;
					fz_pcl_preset(mu_ctx, &options, "ljet4");
					fz_write_pcl(mu_ctx, pix, filename, append, &options);
					break;
				case PWG_OUT: /* This can do multi-page */
					fz_write_pwg(mu_ctx, pix, filename, append, NULL);
					break;
			}
		}
	}
	fz_always(mu_ctx)
	{
		if (pix != NULL)
			fz_drop_pixmap(mu_ctx, pix);
		fz_free_device(dev);
		fz_free_page(mu_doc, page);
		if (dlist != NULL)
			fz_drop_display_list(mu_ctx, dlist);
		if (out != NULL)
		{
			fz_close_output(out);
			fclose(file);
		}
	}
	fz_catch(mu_ctx)
	{
		return E_FAILURE;
	}
	return S_ISOK;
}
int muctx::GetTextBlock (void *page, int block_num,
    double *top_x, double *top_y, double *height, double *width)
{
    fz_text_page *text = (fz_text_page*) page;
    fz_text_block *block;

    if (text->blocks[block_num].type != FZ_PAGE_BLOCK_TEXT)
        return 0;
    block = text->blocks[block_num].u.text;

    *top_x = block->bbox.x0;
    *top_y = block->bbox.y0;
    *height = block->bbox.y1 - *top_y;
    *width = block->bbox.x1 - *top_x;
    return block->len;
}

/* Information about a line of text */
int muctx::GetTextLine(void *page, int block_num, int line_num,
    double *top_x, double *top_y, double *height, double *width)
{
    int len = 0;
    fz_text_block *block;
    fz_text_line line;
    fz_text_span *span;
    fz_text_page *text = (fz_text_page*)page;

    block = text->blocks[block_num].u.text;
    line = block->lines[line_num];

    *top_x = line.bbox.x0;
    *top_y = line.bbox.y0;
    *height = line.bbox.y1 - *top_y;
    *width = line.bbox.x1 - *top_x;

    for (span = line.first_span; span; span = span->next)
    {
        len += span->len;
    }
    return len;
}

/* Information down to the character level */
int muctx::GetTextCharacter(void *page, int block_num, int line_num,
    int item_num, double *top_x, double *top_y, double *height, double *width)
{
    fz_text_block *block;
    fz_text_line line;
    fz_text_span *span;
    fz_text_page *text = (fz_text_page*)page;
    fz_char_and_box cab;
    int index = item_num;

    block = text->blocks[block_num].u.text;
    line = block->lines[line_num];

    span = line.first_span;
    while (index >= span->len)
    {
        index = index - span->len;  /* Reset to start of next span */
        span = span->next;  /* Get next span */
    }

    cab.c = span->text[index].c;
    fz_text_char_bbox(&(cab.bbox), span, index);
    *top_x = cab.bbox.x0;
    *top_y = cab.bbox.y0;
    *height = cab.bbox.y1 - *top_y;
    *width = cab.bbox.x1 - *top_x;

    return cab.c;
}

void muctx::freeText(fz_text_page *text)
{
    fz_free_text_page(mu_ctx, text);
}

