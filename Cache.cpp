#ifndef _QT
#include "pch.h"
#endif
#include "Cache.h"
#include "list"

Cache::Cache(void)
{
	this->size = 0;
	this->head = NULL;
	this->tail = NULL;
}

Cache::~Cache(void)
{
}

void Cache::Empty(fz_context *mu_ctx)
{
	if (this != nullptr) {
		cache_entry_t *curr_entry = this->head;

		while (curr_entry != NULL)
		{
			cache_entry_t *old_entry = curr_entry;
			curr_entry = old_entry->next;
            fz_drop_display_list(mu_ctx, old_entry->dlist);
            fz_drop_page(mu_ctx, old_entry->page);
            delete old_entry;
		}
		this->size = 0;
		this->head = NULL;
		this->tail = NULL;
	}
}

void Cache::Add(int value, int width_in, int height_in, fz_display_list *dlist, 
                fz_context *mu_ctx, fz_page *page)
{
	std::lock_guard<std::mutex> lock(cache_lock);

	/* If full, then delete the tail */
	if (size >= MAX_DISPLAY_CACHE_SIZE)
	{
		cache_entry_t *curr_entry = this->tail;
		cache_entry_t *prev_entry = curr_entry->prev;

		if (prev_entry != NULL)
			prev_entry->next = NULL;
		else
			head = NULL;

		tail = prev_entry;

		/* Decrement the caches rc of this list */
		fz_drop_display_list(mu_ctx, curr_entry->dlist);
		delete curr_entry;
		size--;
	}

	/* Make a new entry and stick at head */
	cache_entry_t *new_entry = new cache_entry_t;

    new_entry->dlist = dlist;
    new_entry->page = page;
    new_entry->index = value;
	new_entry->width = width_in;
	new_entry->height = height_in;

	new_entry->prev = NULL;
	if (head == NULL)
	{
		new_entry->next = NULL;
		head = new_entry;
		tail = new_entry;
	}
	else
	{
		new_entry->next = head;
		head->prev = new_entry;
		head = new_entry;
	}
	size++;

    //  11/25/2014 - Fred and Michael think this unnecessarily increments
    //  the display list's reference counter
//    fz_keep_display_list(mu_ctx, new_entry->dlist);
}

fz_page *Cache::FindPage(int page_num)
{
    std::lock_guard<std::mutex> lock(cache_lock);
    cache_entry_t *curr_entry = this->head;

    while (curr_entry != NULL)
    {
        if (curr_entry->index == page_num)
            break;
        curr_entry = curr_entry->next;
    }

    if (curr_entry != NULL)
        return curr_entry->page;
    else
        return NULL;
}

fz_display_list* Cache::Use(int value, int *width_out, int *height_out, fz_context *mu_ctx)
{
    UNUSED(mu_ctx);

	std::lock_guard<std::mutex> lock(cache_lock);
	cache_entry_t *curr_entry = this->head;

	while (curr_entry != NULL)
	{
		if (curr_entry->index == value)
			break;
		curr_entry = curr_entry->next;
	}
	if (curr_entry != NULL)
	{
        //  11/25/2014 - Fred and Michael think this unnecessarily increments
        //  the display list's reference counter
//		fz_keep_display_list(mu_ctx, curr_entry->dlist);

		/* Move this to the front */
		if (curr_entry != head)
		{
			cache_entry_t *prev_entry = curr_entry->prev;
			cache_entry_t *next_entry = curr_entry->next;
			prev_entry->next = next_entry;

			if (next_entry != NULL)
				next_entry->prev = prev_entry;
			else
				tail = prev_entry;

			curr_entry->prev = NULL;
			curr_entry->next = head;
			head->prev = curr_entry;
			head = curr_entry;
		}
		*width_out = curr_entry->width;
		*height_out = curr_entry->height;
		return curr_entry->dlist;
	}
	else
		return NULL;
}
