#include "ed_picklist.h"
#include "c_memory.h"

#include <stdlib.h>
#include <string.h>


struct pick_list_t editor_CreatePickList()
{
    struct pick_list_t pick_list;

    pick_list.max_records = 128;
    pick_list.record_count = 0;
    pick_list.last_selection_type = PICK_NONE;
    pick_list.records = memory_Calloc(sizeof(struct pick_record_t ), pick_list.max_records);

    return pick_list;
}

void editor_DestroyPickList(struct pick_list_t *pick_list)
{
    if(pick_list)
    {
        if(pick_list->records)
        {
            memory_Free(pick_list->records);
        }
    }
}


void editor_AddSelection(struct pick_record_t *record, struct pick_list_t *pick_list)
{
	struct pick_record_t *records;
	struct pick_record_t *new_records;
	int i;

	int total_records = 0;

	if(record->type == PICK_NONE)
	{
		return;
	}

	/* try to drop this selection, to make sure
	there's just one selection per object... */
	editor_DropSelection(record, pick_list);


	if(pick_list->record_count >= pick_list->max_records)
	{
	    total_records = pick_list->max_records;
	    records = pick_list->records;

	    for(i = 0; i < pick_list->selection_section; i++)
        {
            total_records += records[-1].index0;
            records -= records[-1].index0;
        }

		new_records = memory_Malloc(sizeof(struct pick_record_t) * (total_records + 256));
		memcpy(new_records, records, sizeof(struct pick_record_t) * total_records);
		memory_Free(records);

		pick_list->max_records = total_records + 256;
		pick_list->records = new_records;

        if(pick_list->selection_section)
        {
            for(i = 0; i < total_records; i++)
            {
                if(new_records[i].type == PICK_SECTION)
                {
                    pick_list->max_records -= new_records[i].index0;
                    pick_list->records += new_records[i].index0;
                }
            }
        }
	}

	pick_list->records[pick_list->record_count] = *record;
	pick_list->record_count++;
	pick_list->last_selection_type = record->type;
}



void editor_DropSelection(struct pick_record_t *record, struct pick_list_t *pick_list)
{
	int i;
	int c;
	int j;
	int k;

	struct pick_record_t *records = pick_list->records;

	if(record->type == PICK_NONE)
	{
		return;
	}

	c = pick_list->record_count;

	for(i = 0; i < c; i++)
	{
		if(record->type == records[i].type)
		{
			if(record->type == PICK_BRUSH)
			{
				if(record->pointer == records[i].pointer)
				{
					goto _move_selections;
				}

				continue;
			}

			if(record->index0 == records[i].index0)
			{
				_move_selections:

				for(j = i; j < c - 1; j++)
				{
					records[j] = records[j + 1];
				}

				pick_list->record_count--;
				break;
			}
		}
	}

	pick_list->last_selection_type = records[pick_list->record_count - 1].type;
}

void editor_ClearSelection(struct pick_list_t *pick_list)
{
    int i;

    for(i = 0; i < pick_list->selection_section; i++)
    {
        pick_list->max_records += pick_list->records[-1].index0;
        pick_list->records -= pick_list->records[-1].index0;
    }

    pick_list->selection_section = 0;
	pick_list->record_count = 0;
	pick_list->last_selection_type = PICK_NONE;
}

void editor_PushSelectionSection(struct pick_list_t *pick_list)
{
    struct pick_record_t section;

    if(pick_list)
    {
        section.type = PICK_SECTION;
        /* the section delimiter keeps the record count of the section
        it delimits, including itself in the list. That's why the record
        count gets incremented here... */
        section.index0 = pick_list->record_count + 1;


        editor_AddSelection(&section, pick_list);
        pick_list->records += pick_list->record_count;
        pick_list->max_records -= pick_list->record_count;
        pick_list->record_count = 0;

        pick_list->selection_section++;
    }
}

void editor_PopSelectionSection(struct pick_list_t *pick_list)
{
    struct pick_record_t section;

    if(pick_list)
    {
        if(pick_list->selection_section)
        {
            pick_list->selection_section--;
            section = pick_list->records[-1];

            pick_list->records -= section.index0;
            pick_list->max_records += section.index0;

            /* section.index0 is the record count of the previous selection
            section, which includes the section delimiter selection. So, we
            subtract one from the record count here, effectively dropping the
            the section delimiter from the list... */
            pick_list->record_count = section.index0 - 1;
        }
    }
}

int editor_IsSelectionInList(struct pick_record_t *record, struct pick_list_t *pick_list)
{
    int i;

    if(record->type == PICK_BRUSH)
    {
        for(i = 0; i < pick_list->record_count; i++)
        {
            if(record->pointer == pick_list->records[i].pointer)
            {
                return 1;
            }
        }
    }
    else
    {
        for(i = 0; i < pick_list->record_count; i++)
        {
            if(record->index0 == pick_list->records[i].index0)
            {
                return 1;
            }
        }
    }

    return 0;
}


int editor_IsLastSelection(struct pick_record_t *record, struct pick_list_t *pick_list)
{
    if(pick_list->record_count)
    {
        if(record->type == pick_list->records[pick_list->record_count - 1].type)
        {
            if(record->type == PICK_BRUSH)
            {
                if(record->pointer == pick_list->records[pick_list->record_count - 1].pointer)
                {
                    return 1;
                }
            }
            else
            {
                if(record->index0 == pick_list->records[pick_list->record_count - 1].index0)
                {
                    return 1;
                }
            }
        }

    }

    return 0;
}





