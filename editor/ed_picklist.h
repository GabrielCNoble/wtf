#ifndef ED_PICKLIST_H
#define ED_PICKLIST_H


enum PICK_TYPE
{
	PICK_NONE = 0,
	PICK_SECTION,
	PICK_HANDLE,
	PICK_BRUSH,
	PICK_BRUSH_FACE,
	PICK_BRUSH_EDGE,
	PICK_LIGHT,
	PICK_ENTITY,
	PICK_SPAWN_POINT,
	PICK_UV_VERTEX,
	PICK_COLLIDER_PRIMITIVE,
	PICK_PORTAL,
	PICK_WAYPOINT,
	PICK_TRIGGER,

};

struct pick_record_t
{
	int type;
	int index0;
	void *pointer;
	int index1;
	int index2;
};

struct pick_list_t
{
	int record_count;
	int selection_section;
	int max_records;
	int last_selection_type;
	struct pick_record_t *records;
};


struct pick_list_t editor_CreatePickList();

void editor_DestroyPickList(struct pick_list_t *pick_list);

void editor_AddSelection(struct pick_record_t *record, struct pick_list_t *pick_list);

void editor_DropSelection(struct pick_record_t *record, struct pick_list_t *pick_list);

void editor_ClearSelection(struct pick_list_t *pick_list);

void editor_PushSelectionSection(struct pick_list_t *pick_list);

void editor_PopSelectionSection(struct pick_list_t *pick_list);

int editor_IsSelectionInList(struct pick_record_t *record, struct pick_list_t *pick_list);

int editor_IsLastSelection(struct pick_record_t *record, struct pick_list_t *pick_list);


#endif // ED_PICKLIST_H










