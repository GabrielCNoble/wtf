#ifndef SCR_COMMON_H
#define SCR_COMMON_H


#define SCRIPT_NAME_MAX_LEN 24

#define MAX_SCRIPT_CONTEXTS 128

enum SCRIPT_ACCESS_MASK
{
	SCRIPT_ACCESS_MASK_BASE = 1,
	SCRIPT_ACCESS_MASK_NAVIGATION_SYSTEM = 1 << 1,
	SCRIPT_ACCESS_MASK_PARTICLE_SYSTEM = 1 << 2,
	SCRIPT_ACCESS_MASK_PHYSICS_SYSTEM = 1 << 3,
	SCRIPT_ACCESS_MASK_ALL_SYSTEMS = SCRIPT_ACCESS_MASK_BASE | SCRIPT_ACCESS_MASK_NAVIGATION_SYSTEM | SCRIPT_ACCESS_MASK_PARTICLE_SYSTEM | SCRIPT_ACCESS_MASK_PHYSICS_SYSTEM,
};

enum SCRIPT_FLAGS
{
	SCRIPT_FLAG_NOT_COMPILED = 1,
	SCRIPT_FLAG_EXECUTING = 1 << 1,
};

enum SCRIPT_TYPE
{
	SCRIPT_TYPE_NONE,
	SCRIPT_TYPE_PARTICLE_SYSTEM,
	SCRIPT_TYPE_ENTITY,
	SCRIPT_TYPE_AI,
	SCRIPT_TYPE_WORLD,
	SCRIPT_TYPE_GAME,
	SCRIPT_TYPE_EVENT,
};

enum SCRIPT_VAR_TYPE
{
	SCRIPT_VAR_TYPE_NONE = 0,

	SCRIPT_VAR_TYPE_INT8,
	SCRIPT_VAR_TYPE_INT16,
	SCRIPT_VAR_TYPE_INT32,
	SCRIPT_VAR_TYPE_INT64,

	SCRIPT_VAR_TYPE_FLOAT,
	SCRIPT_VAR_TYPE_DOUBLE,

	SCRIPT_VAR_TYPE_VEC2T,
	SCRIPT_VAR_TYPE_VEC3T,
	SCRIPT_VAR_TYPE_VEC4T,

	SCRIPT_VAR_TYPE_MAT3T,

	SCRIPT_VAR_TYPE_STRUCT,

	SCRIPT_VAR_TYPE_ARRAY,
	SCRIPT_VAR_TYPE_STRING,
	SCRIPT_VAR_TYPE_HANDLE,
	SCRIPT_VAR_TYPE_COMPONENT,
};


enum SCR_SCRIPT_PROP_TYPE
{
    SCR_SCRIPT_PROP_TYPE_INT,
    SCR_SCRIPT_PROP_TYPE_FLOAT,
    SCR_SCRIPT_PROP_TYPE_VEC2_T,
    SCR_SCRIPT_PROP_TYPE_VEC3_T,
    SCR_SCRIPT_PROP_TYPE_VEC4_T,
    SCR_SCRIPT_PROP_TYPE_MAT2_T,
    SCR_SCRIPT_PROP_TYPE_MAT3_T,
    SCR_SCRIPT_PROP_TYPE_MAT4_T,
};

struct script_prop_t
{
	char *name;
	int size;
	int type;
	void *memory;
};




enum SCR_SCRIPT_TIMER_STATUS
{
    SCR_SCRIPT_TIMER_STATUS_IDLE,
    SCR_SCRIPT_TIMER_STATUS_RUNNING,
    SCR_SCRIPT_TIMER_STATUS_TIMED_OUT
};


struct script_timer_t
{
    char *name;
    int timeout;            /* in ms... */
    int status;
};


/*typedef struct
{
	char *var;
	int type;
	int sub_type;
	char *struct_name;
}script_var_t;*/




enum SCRIPT_ARG_TYPE
{
	SCRIPT_ARG_TYPE_BYTE = 0,
	SCRIPT_ARG_TYPE_WORD,
	SCRIPT_ARG_TYPE_DWORD,
	SCRIPT_ARG_TYPE_QWORD,
	SCRIPT_ARG_TYPE_FLOAT,
	SCRIPT_ARG_TYPE_DOUBLE,
	SCRIPT_ARG_TYPE_ADDRESS,
};



struct script_arg_t
{
	int type;

	union
	{
		char byte_arg;
		short word_arg;
		int dword_arg;
		long int qword_arg;
		float float_arg;
		double double_arg;
		void *address_arg;
	}arg;
};

struct script_execution_data_t
{
	char data[64];
};


#define MAX_SCRIPT_ARGS 8

struct script_entry_point_t
{
	void *entry_point;
	int arg_count;
	struct script_arg_t args[MAX_SCRIPT_ARGS];
};

#define MAX_SCRIPT_ENTRY_POINTS 16

struct script_context_t
{
	void *execution_context;
	struct script_execution_data_t execution_data;

	int entry_point_count;
	struct script_entry_point_t entry_points[MAX_SCRIPT_ENTRY_POINTS];
};



struct script_t
{
	struct script_t *next;
	struct script_t *prev;

	int type;
	int flags;

	void *script_module;					/* opaque reference to asIScriptModule... */
	void *main_entry_point;					/* opaque reference to asIScriptFunction... */


	int (*get_data_callback)(struct script_t *script);
	void (*reload_callback)();
	void *(*setup_data_callback)(struct script_t *script, void *data);

	char *name;
	char *file_name;

	int update_count;
};




#endif
