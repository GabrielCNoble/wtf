#include "script.h"
#include "scr_math.h"
#include "scr_types.h"
#include "vector.h"
#include "memory.h"
#include "particle.h"
#include "player.h"
#include "physics.h"
#include "scr_entity.h"
#include "input.h"

#include "path.h"

#include <math.h>

#include <stdio.h>

extern "C++"
{
#include "angelscript.h"
}
asIScriptEngine *scr_virtual_machine = NULL;

int scr_max_contexts = 0;
int scr_contexts_stack_top = -1;
asIScriptContext **scr_contexts_stack = NULL;



struct script_t *scr_scripts = NULL;
struct script_t *scr_last_script = NULL;


/* from r_main.c */
extern int r_frame;
extern int r_width;
extern int r_height;

/* from particle.c */
extern int ps_frame;



script_var_t particle_system_script_vars[] = 
{
	{"ps_particle_positions", SCRIPT_VAR_TYPE_ARRAY, SCRIPT_VAR_TYPE_VEC4T, NULL},
	{"ps_particle_frames", SCRIPT_VAR_TYPE_ARRAY, SCRIPT_VAR_TYPE_INT32, NULL},
	{"ps_particles", SCRIPT_VAR_TYPE_ARRAY, SCRIPT_VAR_TYPE_STRUCT, "particle_t"},
	{"ps_particle_system", SCRIPT_VAR_TYPE_HANDLE, SCRIPT_VAR_TYPE_STRUCT, "particle_system_t"},	
};




#ifdef __cplusplus
extern "C"
{
#endif

 
void script_MessageCallback(const asSMessageInfo *info, void *parm)
{
	char *type = "ERR";
	
	if(info->type == asMSGTYPE_WARNING)
	{
		type = "WARNING";
	}
	else if(info->type == asMSGTYPE_INFORMATION)
	{
		type = "INFORMATION";
	}
	
	printf("%s: line: %d col: %d\nsection: %s\n%s\n", type, info->row, info->col, info->section, info->message);
}


int script_Init()
{
	
	int i;
	
	scr_virtual_machine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	scr_virtual_machine->SetMessageCallback(asFUNCTION(script_MessageCallback), NULL, asCALL_CDECL);
	
	script_RegisterTypesAndFunctions();
	
	scr_max_contexts = 128;
	
	scr_contexts_stack = (asIScriptContext **)memory_Malloc(sizeof(asIScriptContext *) * scr_max_contexts, "script_Init");
	
	for(i = 0; i < scr_max_contexts; i++)
	{
		scr_contexts_stack[i] = scr_virtual_machine->CreateContext();
	}
	
	scr_contexts_stack_top = scr_max_contexts - 1;
	
	return 1;
}

void script_Finish()
{
	scr_virtual_machine->ShutDownAndRelease();
	
	memory_Free(scr_contexts_stack);
}

void script_DebugPrint()
{
	printf("debug print\n");
}

void script_RegisterTypesAndFunctions()
{
	/*
	===============================================
	===============================================
	===============================================
	*/
	
	/* every script can see those functions... */
	scr_virtual_machine->SetDefaultAccessMask(SCRIPT_ACCESS_MASK_BASE);
	
	//scr_virtual_machine->RegisterGlobalFunction("double sin(double _X)", asFUNCTION(sin), asCALL_CDECL);
	//scr_virtual_machine->RegisterGlobalFunction("double cos(double _X)", asFUNCTION(cos), asCALL_CDECL);
	//scr_virtual_machine->RegisterGlobalFunction("double tan(double _X)", asFUNCTION(tan), asCALL_CDECL);
	//scr_virtual_machine->RegisterGlobalFunction("double sqrt(double _X)", asFUNCTION(sqrt), asCALL_CDECL);
	//scr_virtual_machine->RegisterGlobalFunction("double pow(double _X, double _Y)", asFUNCTION(pow), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("float randfloat()", asFUNCTION(randfloat), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void script_DebugPrint()", asFUNCTION(script_DebugPrint), asCALL_CDECL);
	
	scr_virtual_machine->RegisterGlobalProperty("const int r_frame", &r_frame);
	scr_virtual_machine->RegisterGlobalProperty("const int ps_frame", &ps_frame);
	
	
	
	/*
	===============================================
	===============================================
	===============================================
	*/
	
	
	scr_virtual_machine->RegisterEnum("MOUSE_FLAGS");
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_LEFT_BUTTON_CLICKED", MOUSE_LEFT_BUTTON_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_LEFT_BUTTON_JUST_CLICKED", MOUSE_LEFT_BUTTON_JUST_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_LEFT_BUTTON_JUST_RELEASED", MOUSE_LEFT_BUTTON_JUST_RELEASED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_LEFT_BUTTON_DOUBLE_CLICKED", MOUSE_LEFT_BUTTON_DOUBLE_CLICKED);
	
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_RIGHT_BUTTON_CLICKED", MOUSE_RIGHT_BUTTON_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_RIGHT_BUTTON_JUST_CLICKED", MOUSE_RIGHT_BUTTON_JUST_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_RIGHT_BUTTON_JUST_RELEASED", MOUSE_RIGHT_BUTTON_JUST_RELEASED);
	
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_MIDDLE_BUTTON_CLICKED", MOUSE_MIDDLE_BUTTON_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_MIDDLE_BUTTON_JUST_CLICKED", MOUSE_MIDDLE_BUTTON_JUST_CLICKED);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_MIDDLE_BUTTON_JUST_RELEASED", MOUSE_MIDDLE_BUTTON_JUST_RELEASED);
	
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_WHEEL_UP", MOUSE_WHEEL_UP);
	scr_virtual_machine->RegisterEnumValue("MOUSE_FLAGS", "MOUSE_WHEEL_DOWN", MOUSE_WHEEL_DOWN);
	
	
	scr_virtual_machine->RegisterEnum("KEYBOARD_FLAGS");
	scr_virtual_machine->RegisterEnumValue("KEYBOARD_FLAGS", "KEY_PRESSED", KEY_PRESSED);
	scr_virtual_machine->RegisterEnumValue("KEYBOARD_FLAGS", "KEY_JUST_PRESSED", KEY_JUST_PRESSED);
	scr_virtual_machine->RegisterEnumValue("KEYBOARD_FLAGS", "KEY_JUST_RELEASED", KEY_JUST_RELEASED);
	

	scr_virtual_machine->RegisterGlobalFunction("int input_GetKeyStatus(int key)", asFUNCTION(input_GetKeyStatus), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("int input_GetMouseButton(int button)", asFUNCTION(input_GetMouseButton), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("int input_RegisterKey(int key)", asFUNCTION(input_RegisterKey), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("int input_UnregisterKey(int key)", asFUNCTION(input_UnregisterKey), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void input_GetMouseDelta(float &out dx, float &out dy)", asFUNCTION(input_GetMouseDelta), asCALL_CDECL);
	
	
	
	
	scr_virtual_machine->RegisterEnum("SDL_Scancode");
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_A", SDL_SCANCODE_A);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_B", SDL_SCANCODE_B);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_C", SDL_SCANCODE_C);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_D", SDL_SCANCODE_D);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_E", SDL_SCANCODE_E);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_F", SDL_SCANCODE_F);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_G", SDL_SCANCODE_G);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_H", SDL_SCANCODE_H);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_I", SDL_SCANCODE_I);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_J", SDL_SCANCODE_J);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_K", SDL_SCANCODE_K);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_L", SDL_SCANCODE_L);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_M", SDL_SCANCODE_M);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_N", SDL_SCANCODE_N);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_O", SDL_SCANCODE_O);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_P", SDL_SCANCODE_P);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_Q", SDL_SCANCODE_Q);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_R", SDL_SCANCODE_R);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_S", SDL_SCANCODE_S);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_T", SDL_SCANCODE_T);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_U", SDL_SCANCODE_U);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_V", SDL_SCANCODE_V);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_W", SDL_SCANCODE_W);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_X", SDL_SCANCODE_X);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_Y", SDL_SCANCODE_Y);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_Z", SDL_SCANCODE_Z);
	
	
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_1", SDL_SCANCODE_1);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_2", SDL_SCANCODE_2);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_3", SDL_SCANCODE_3);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_4", SDL_SCANCODE_4);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_5", SDL_SCANCODE_5);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_6", SDL_SCANCODE_6);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_7", SDL_SCANCODE_7);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_8", SDL_SCANCODE_8);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_9", SDL_SCANCODE_9);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_0", SDL_SCANCODE_0);
	
	
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_RETURN", SDL_SCANCODE_RETURN);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_SPACE", SDL_SCANCODE_SPACE);
	
	
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_LSHIFT", SDL_SCANCODE_LSHIFT);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_RSHIFT", SDL_SCANCODE_RSHIFT);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_LCTRL", SDL_SCANCODE_LCTRL);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_RCTRL", SDL_SCANCODE_RCTRL);
	
	
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_UP", SDL_SCANCODE_UP);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_DOWN", SDL_SCANCODE_DOWN);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_LEFT", SDL_SCANCODE_LEFT);
	scr_virtual_machine->RegisterEnumValue("SDL_Scancode", "SDL_SCANCODE_RIGHT", SDL_SCANCODE_RIGHT);
	
	
	
	
	
	
	
	

	
	/*
	===============================================
	===============================================
	===============================================
	*/
	
	scr_virtual_machine->RegisterObjectType("vec2_t", sizeof(vec2_t), asOBJ_VALUE | asOBJ_APP_PRIMITIVE);
	scr_virtual_machine->RegisterObjectProperty("vec2_t", "float x", asOFFSET(vec2_t, x));
	scr_virtual_machine->RegisterObjectProperty("vec2_t", "float y", asOFFSET(vec2_t, y));
	scr_virtual_machine->RegisterObjectBehaviour("vec2_t", asBEHAVE_CONSTRUCT, "void Constructor()", asFUNCTION(vec2_constructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec2_t", asBEHAVE_CONSTRUCT, "void Constructor(float x, float y)", asFUNCTION(vec2_constructor_init), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec2_t", asBEHAVE_DESTRUCT, "void Destructor()", asFUNCTION(vec2_destructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec2_t", "float get_opIndex(int index)", asFUNCTION(vec_get_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec2_t", "void set_opIndex(int index, float value)", asFUNCTION(vec_set_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec2_t", "vec2_t &opAssign(vec2_t &in other)", asFUNCTION(vec2_op_assign), asCALL_CDECL_OBJFIRST);
	
	scr_virtual_machine->RegisterObjectType("vec3_t", sizeof(vec3_t), asOBJ_VALUE | asOBJ_APP_PRIMITIVE);
	scr_virtual_machine->RegisterObjectProperty("vec3_t", "float x", asOFFSET(vec3_t, x));
	scr_virtual_machine->RegisterObjectProperty("vec3_t", "float y", asOFFSET(vec3_t, y));
	scr_virtual_machine->RegisterObjectProperty("vec3_t", "float z", asOFFSET(vec3_t, z));
	scr_virtual_machine->RegisterObjectBehaviour("vec3_t", asBEHAVE_CONSTRUCT, "void Constructor()", asFUNCTION(vec3_constructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec3_t", asBEHAVE_CONSTRUCT, "void Constructor(float x, float y, float z)", asFUNCTION(vec3_constructor_init), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec3_t", asBEHAVE_DESTRUCT, "void Destructor()", asFUNCTION(vec3_destructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "float get_opIndex(int index)", asFUNCTION(vec_get_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "void set_opIndex(int index, float value)", asFUNCTION(vec_set_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "vec3_t &opAssign(vec3_t &in other)", asFUNCTION(vec3_op_assign), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "vec3_t &opAdd(vec3_t &in other)", asFUNCTION(vec3_op_add), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "vec3_t &opSub(vec3_t &in other)", asFUNCTION(vec3_op_sub), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec3_t", "vec3_t &opMul(float factor)", asFUNCTION(vec3_op_mul), asCALL_CDECL_OBJFIRST);
	
	
	scr_virtual_machine->RegisterObjectType("vec4_t", sizeof(vec4_t), asOBJ_VALUE | asOBJ_APP_PRIMITIVE);
	scr_virtual_machine->RegisterObjectProperty("vec4_t", "float x", asOFFSET(vec4_t, x));
	scr_virtual_machine->RegisterObjectProperty("vec4_t", "float y", asOFFSET(vec4_t, y));
	scr_virtual_machine->RegisterObjectProperty("vec4_t", "float z", asOFFSET(vec4_t, z));
	scr_virtual_machine->RegisterObjectProperty("vec4_t", "float w", asOFFSET(vec4_t, w));
	scr_virtual_machine->RegisterObjectBehaviour("vec4_t", asBEHAVE_CONSTRUCT, "void Constructor()", asFUNCTION(vec4_constructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec4_t", asBEHAVE_CONSTRUCT, "void Constructor(float x, float y, float z, float w)", asFUNCTION(vec4_constructor_init), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("vec4_t", asBEHAVE_DESTRUCT, "void Destructor()", asFUNCTION(vec4_destructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec4_t", "float get_opIndex(int index)", asFUNCTION(vec_get_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec4_t", "void set_opIndex(int index, int value)", asFUNCTION(vec_set_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("vec4_t", "vec4_t &opAssign(vec4_t &in other)", asFUNCTION(vec4_op_assign), asCALL_CDECL_OBJFIRST);
	
	
	
	
	scr_virtual_machine->RegisterObjectType("mat3_t", sizeof(mat3_t), asOBJ_VALUE | asOBJ_APP_PRIMITIVE);
	scr_virtual_machine->RegisterObjectProperty("mat3_t", "vec3_t r0", asOFFSET(mat3_t, r0));
	scr_virtual_machine->RegisterObjectProperty("mat3_t", "vec3_t r1", asOFFSET(mat3_t, r1));
	scr_virtual_machine->RegisterObjectProperty("mat3_t", "vec3_t r2", asOFFSET(mat3_t, r2));
	scr_virtual_machine->RegisterObjectBehaviour("mat3_t", asBEHAVE_CONSTRUCT, "void Constructor()", asFUNCTION(mat3_constructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("mat3_t", asBEHAVE_CONSTRUCT, "void Constructor(vec3_t &in r0, vec3_t &in r1, vec3_t &in r2)", asFUNCTION(mat3_constructor_init), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("mat3_t", asBEHAVE_DESTRUCT, "void Destructor()", asFUNCTION(mat3_destructor), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("mat3_t", "vec3_t &get_opIndex(int index)", asFUNCTION(mat3_get_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("mat3_t", "void set_opIndex(int index, vec3_t &in value)", asFUNCTION(mat3_set_op_index), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("mat3_t", "mat3_t &opAssign(mat3_t &in other)", asFUNCTION(mat3_op_assign), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("mat3_t", "vec3_t &opMul(vec3_t &in other)", asFUNCTION(mat3_op_mul), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectMethod("mat3_t", "void identity()", asFUNCTION(mat3_identity), asCALL_CDECL_OBJFIRST);
	
	
	scr_virtual_machine->RegisterGlobalFunction("float dot(vec3_t &in vec_a, vec3_t &in vec_b)", asFUNCTION(vec3_dot), asCALL_CDECL);
	
	/*
	===============================================
	===============================================
	===============================================
	*/ 
	
	scr_virtual_machine->RegisterObjectType("array<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE);
	 
	scr_virtual_machine->RegisterObjectBehaviour("array<T>", asBEHAVE_ADDREF, "void AddRef()", asFUNCTION(script_generic_array_AddRef), asCALL_CDECL_OBJFIRST);
	scr_virtual_machine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASE, "void Release()", asFUNCTION(script_generic_array_Release), asCALL_CDECL_OBJFIRST);
	
	scr_virtual_machine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ Constructor(int &in sub_type)", asFUNCTION(script_generic_array_Constructor), asCALL_CDECL);
	scr_virtual_machine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ Constructor_Sized(int &in sub_type, int size)", asFUNCTION(script_generic_array_Constructor_Sized), asCALL_CDECL);
	
	scr_virtual_machine->RegisterObjectMethod("array<T>", "T &opIndex(int index)", asFUNCTION(script_generic_array_ElementAt), asCALL_CDECL_OBJFIRST);
	//scr_virtual_machine->RegisterObjectMethod("array<T>", "int Count()", asFUNCTION(script_generic_array_Count), asCALL_CDECL_OBJFIRST);
	
	/*
	===============================================
	===============================================
	===============================================
	*/
	
	//scr_virtual_machine->SetDefaultAccessMask(SCRIPT_ACCESS_MASK_PARTICLE_SYSTEM);
	
	/* particle system stuff... */
	scr_virtual_machine->RegisterObjectType("particle_t", sizeof(particle_t), asOBJ_VALUE | asOBJ_POD);
	scr_virtual_machine->RegisterObjectProperty("particle_t", "int life", asOFFSET(particle_t, life));
	scr_virtual_machine->RegisterObjectProperty("particle_t", "vec3_t velocity", asOFFSET(particle_t, velocity));
	
	scr_virtual_machine->RegisterObjectType("particle_system_t", sizeof(particle_system_t ), asOBJ_REF | asOBJ_NOCOUNT);
	scr_virtual_machine->RegisterObjectBehaviour("particle_system_t", asBEHAVE_FACTORY, "particle_system_t@ Factory()", asFUNCTION(script_DummyDefaultConstructor), asCALL_CDECL);
	//scr_virtual_machine->RegisterObjectType("particle_system_t", 0, asOBJ_REF | asOBJ_NOCOUNT);
	
	//scr_virtual_machine->RegisterObjectBehaviour("particle_system_t", asBEHAVE_CONSTRUCT, "void Constructor()", asFUNCTION(script_DummyConstructor), asCALL_CDECL_OBJFIRST);
	//scr_virtual_machine->RegisterObjectBehaviour("particle_system_t", asBEHAVE_DESTRUCT, "void Destructor()", asFUNCTION(script_DummyDestructor), asCALL_CDECL_OBJFIRST);
	
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 particle_count", asOFFSET(particle_system_t, particle_count));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 max_particles", asOFFSET(particle_system_t, max_particles));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 max_life", asOFFSET(particle_system_t, max_life));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 max_frame", asOFFSET(particle_system_t, max_frame));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 respawn_time", asOFFSET(particle_system_t, respawn_time));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int16 respawn_countdown", asOFFSET(particle_system_t, respawn_countdown));
	scr_virtual_machine->RegisterObjectProperty("particle_system_t", "int spawn_frame", asOFFSET(particle_system_t, spawn_frame));
	
	
	/*
	===============================================
	===============================================	
	===============================================	
	*/
	
	scr_virtual_machine->RegisterObjectType("entity_handle_t", sizeof(struct entity_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	
	
	scr_virtual_machine->RegisterGlobalFunction("void entity_Jump(float jump_force)", asFUNCTION(entity_ScriptJump), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void entity_Move(vec3_t &in direction)", asFUNCTION(entity_ScriptMove), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void entity_FindPath(vec3_t &in to)", asFUNCTION(entity_ScriptFindPath), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void entity_GetWaypointDirection(vec3_t &out direction)", asFUNCTION(entity_ScriptGetWaypointDirection), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("mat3_t &entity_GetOrientation()", asFUNCTION(entity_ScriptGetOrientation), asCALL_CDECL);
	scr_virtual_machine->RegisterGlobalFunction("void entity_Rotate(vec3_t &in axis, float angle, int set)", asFUNCTION(entity_ScriptRotate), asCALL_CDECL);
}

/*
=========================================================
=========================================================
=========================================================
*/

void *script_PopScriptContext()
{
	void *context = NULL;
	
	if(scr_contexts_stack_top >= 0)
	{
		context = scr_contexts_stack[scr_contexts_stack_top];
		scr_contexts_stack_top--;
	}
	
	return context;
}

void script_PushScriptContext(void *script_context)
{
	scr_contexts_stack_top++;
	scr_contexts_stack[scr_contexts_stack_top] = (asIScriptContext *)script_context;
}


/*
=========================================================
=========================================================
=========================================================
*/

struct script_t *script_CreateScript(char *file_name, char *script_name, int script_type_size, int (*get_data_callback)(struct script_t *script), void *(*setup_data_callback)(struct script_t *script, void *data))
{
	script_t *script;
	/*int size;
	
	
	switch(type)
	{
		default:
		case SCRIPT_TYPE_NONE:
			size = sizeof(script_t);
		break;
		
		case SCRIPT_TYPE_PARTICLE_SYSTEM:
			size = sizeof(particle_system_script_t);
		break;
		
		case SCRIPT_TYPE_AI:
			size = sizeof(struct ai_script_t);
		break;
	}*/
	
	script = (struct script_t *)memory_Malloc(script_type_size, "script_CreateScript");
	
	script->next = NULL;
	script->prev = NULL;
	script->flags = 0;
	//script->type = type;
	
	script->main_entry_point = NULL;
	script->script_module = NULL;
	script->setup_data_callback = setup_data_callback;
	script->get_data_callback = get_data_callback;
	
	script->name = memory_Strdup(script_name, "script_CreateScript");
	script->file_name = memory_Strdup(file_name, "script_CreateScript");
	
	if(!scr_scripts)
	{
		scr_scripts = script;
	}
	else
	{
		scr_last_script->next = script;
		script->prev = scr_last_script;
	}
		
	scr_last_script = script;
	
	return script;
	
}

struct script_t *script_LoadScript(char *file_name, char *script_name, int script_type_size, int (*get_data_callback)(struct script_t *script), void *(*setup_data_callback)(struct script_t *script, void *data))
{
	script_t *script = NULL;
	particle_system_script_t *ps_script;
	//script_t temp_script;
	FILE *file;
	asIScriptModule *module = NULL;
	char *script_text;
	//unsigned int script_file_size;
	
	
	script_text = script_LoadScriptSource(file_name);
	
	if(script_text)
	{
		script = script_CreateScript(file_name, script_name, script_type_size, get_data_callback, setup_data_callback);
		script_CompileScriptSource(script_text, script);		
	}
	
	return script;
}

void script_ReloadScript(script_t *script)
{
	
}


char *script_LoadScriptSource(char *file_name)
{
	//script_t *script;
	FILE *file;
	asIScriptModule *module;
	asIScriptModule *temp_module;
	char *script_text = NULL;
	unsigned int script_file_size;
	int rtrn;
	
	file = path_TryOpenFile(file_name); 
	
	if(!file)
	{
		printf("script_LoadScript: couldn't find script file [%s]\n", file_name);
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	script_file_size = ftell(file);
	rewind(file);
	
	script_text = (char *)memory_Malloc(script_file_size + 1, "script_LoadScript");
	fread(script_text, script_file_size, 1, file);
	fclose(file);
	
	script_text[script_file_size] = '\0';
	
	return script_text;
	
	//rtrn = script_CompileScriptSource(script_text, script_name, script_module, script);
	
	//memory_Free(script_text);
	//return rtrn;
}

int script_CompileScriptSource(char *source, struct script_t *script)
{
	asIScriptModule *module;
	asIScriptModule *temp_module;
	
	static char error_log[1024];
	
	particle_system_script_t *ps_script;
	struct ai_script_t *ai_script;
	
	int success = 0;

	module = scr_virtual_machine->GetModule("temp_module", asGM_ALWAYS_CREATE);
	module->AddScriptSection(script->name, source);
	module->SetAccessMask(SCRIPT_ACCESS_MASK_ALL_SYSTEMS);
	
	if(module->Build() < 0)
	{
		if(!script->script_module)
		{
			/* if this script doesn't have a 
			previous succesfully compiled module,
			mark it as unusable... */
			script->flags |= SCRIPT_FLAG_NOT_COMPILED;
		}
		
		module->Discard();
		
		return 0;
	}
	else
	{
		
		//if(script->script_module)
		if(script->script_module)
		{
			temp_module = (asIScriptModule *)script->script_module;
			temp_module->Discard();
		}
		
		success = 1;
		
		module->SetName(script->name);
		script->script_module = module;
		script->flags &= ~SCRIPT_FLAG_NOT_COMPILED;
		
		//script->main_entry_point = module->GetFunctionByName("main");
		script->main_entry_point = script_GetFunctionAddress("main", script);
		
		if(script->get_data_callback)
		{
			success = script->get_data_callback(script);
		}
		else
		{
			printf("script_CompileScriptSource: script [%s] has not get data callback\n", script->name);
		}
		
		/*switch(script->type)
		{
			case SCRIPT_TYPE_PARTICLE_SYSTEM:
				ps_script = (particle_system_script_t *)script;
				
				ps_script->particle_array = script_GetGlobalVarAddress("ps_particles", script);
				ps_script->particle_frame_array = script_GetGlobalVarAddress("ps_particle_frames", script);
				ps_script->particle_position_array = script_GetGlobalVarAddress("ps_particle_positions", script);
				ps_script->particle_system = script_GetGlobalVarAddress("ps_particle_system", script);
				ps_script->init = script_GetFunctionAddress("ps_init", script);
				
				if(!ps_script->particle_array)
				{
					printf("script_CompileScriptSource: script [%s] is missing global var [array<particle_t> ps_particles]\n", script->name);
					success = 0;
				}
				
				if(!ps_script->particle_frame_array)
				{
					printf("script_CompileScriptSource: script [%s] is missing global var [array<int> ps_particle_frames]\n", script->name);
					success = 0;
				}
				
				if(!ps_script->particle_position_array)
				{
					printf("script_CompileScriptSource: script [%s] is missing global var [array<vec4_t> ps_particle_positions]\n", script->name);
					success = 0;
				}
				
				if(!ps_script->particle_system)
				{
					printf("script_CompileScriptSource: script [%s] is missing global var [particle_system_t@ ps_particle_system]\n", script->name);
					success = 0;
				}
				
				if(!ps_script->init)
				{
					printf("script_CompileScriptSource: script [%s] is missing function [void ps_init()]\n", script->name);
					success = 0;
				}		
			break;
			
			case SCRIPT_TYPE_AI:
				ai_script = (struct ai_script_t *)script;
				ai_script->ai_controller = script_GetGlobalVarAddress("controller", script);
				
				if(!ai_script->ai_controller)
				{
					success = 0;
				}
				
			break;
		}*/
		
		if(!success)
		{
			ps_script->script.flags |= SCRIPT_FLAG_NOT_COMPILED;
		}
		
	}
	
	return success;
}


void script_ReloadScripts()
{
	script_t *script;
	char *script_source;
	script = scr_scripts;
	
	while(script)
	{
		script_source = script_LoadScriptSource(script->file_name);
		script_CompileScriptSource(script_source, script);
		script = script->next;
	}
	
}



void script_ExecuteScriptImediate(script_t *script, void *data)
{
	asIScriptFunction *entry_point;
	asIScriptContext *context;
	
	if(script)
	{
		if(script->flags & SCRIPT_FLAG_NOT_COMPILED)
		{
			printf("script_ExecuteScriptImediate: script [%s] is not compiled\n", script->name);
			return;
		}
		
		context = (asIScriptContext *)script_PopScriptContext();
		
		if(!context)
		{
			printf("script_ExecuteScriptImediate: couldn't allocate a context for script [%s]", script->name);
			return;
		}
		
		if(script->setup_data_callback)
		{
			entry_point = (asIScriptFunction *)script->setup_data_callback(script, data);
		}
		else
		{
			printf("script_ExecuteScriptImediate: script [%s] has no setup data callback. Executing main entry point\n", script->name);
			entry_point = (asIScriptFunction *)script->main_entry_point;
		}
		
		if(entry_point)
		{
			context->Prepare(entry_point);
			context->Execute();
			//context->Unprepare();
		}
		else
		{
			printf("script_ExecuteScriptImediate: invalid entry point for script [%s]\n", script->name);
		}
		
		script_PushScriptContext(context);	
	}
}


void *script_GetGlobalVarAddress(char *var, script_t *script)
{
	asIScriptModule *module;
	
	module = (asIScriptModule *)script->script_module;
	
	return module->GetAddressOfGlobalVar(module->GetGlobalVarIndexByName(var));
}


void *script_GetFunctionAddress(char *function, script_t *script)
{
	asIScriptModule *module;
	
	module = (asIScriptModule *)script->script_module;
	
	return module->GetFunctionByName(function);
}


#ifdef __cplusplus
}
}
}
#endif







