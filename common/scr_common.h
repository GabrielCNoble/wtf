#ifndef SCR_COMMON_H
#define SCR_COMMON_H


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
	
	SCRIPT_VAR_TYPE_STRUCT,
	
	SCRIPT_VAR_TYPE_ARRAY,
	SCRIPT_VAR_TYPE_STRING,
	SCRIPT_VAR_TYPE_HANDLE,
};


typedef struct
{
	char *var;
	int type;
	int sub_type;
	char *struct_name;
}script_var_t;

typedef struct
{
	void *module;
	void *init;
	void *main;
}script_exec_data_t;

struct script_t
{
	struct script_t *next;
	struct script_t *prev;
	
	int type;
	int flags;
		
	void *script_module;					/* opaque reference to asIScriptModule... */
	void *main_entry_point;					/* opaque reference to asIScriptFunction... */
	
	
	int (*get_data_callback)(struct script_t *script);
	void *(*setup_data_callback)(struct script_t *script, void *data);

	char *name;
	char *file_name;
	
	int update_count;
};




#endif
