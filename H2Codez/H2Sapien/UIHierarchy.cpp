#include "UIHierarchy.h"
#include "Tags\ScenarioTag.h"
#include "util\Patches.h"

// cause corruptio is a pain
constexpr intptr_t safety_margin = 0x100;

typedef datum(__cdecl* get_tag_for_extractor)();

struct tag_field_extractor_factory
{
	uint32_t vtable = 0x8025D4; // &tag_field_extractor_factory::`vftable'
	get_tag_for_extractor get_tag_func;
	blam_tag tag_type;

	tag_field_extractor_factory(blam_tag _tag_type, get_tag_for_extractor _get_tag_func) :
		get_tag_func(_get_tag_func),
		tag_type(_tag_type)
	{};
};
CHECK_STRUCT_SIZE(tag_field_extractor_factory, 0xC);

struct hierarchy_list_entry
{
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	char nothing[safety_margin] = {'n', 'o', 't'};
};

void insert_post_handler(void* map, hierarchy_list_entry* entry, blam_tag key)
{
	typedef hierarchy_list_entry* __fastcall post_handlers__insert(void* thisptr, int, hierarchy_list_entry* a2, uint32_t* key);
	auto post_handlers__insert_impl = reinterpret_cast<post_handlers__insert*>(0x424780);
	post_handlers__insert_impl(map, 0, entry, &key.i_data);
}

struct hierarchy_entry_info
{
	blam_tag tag;
	editor_string name;
	int field_8 = 0;
	int field_C = 0;
	char nothing[safety_margin] = { 'n', 'o', 't' };
	hierarchy_entry_info(editor_string _name, blam_tag _tag) :
		tag(_tag),
		name(_name)
	{};
};
CHECK_STRUCT_SIZE(hierarchy_entry_info, 0x10 + safety_margin);

struct block_element_field_extractor_factory
{
	uint32_t vtable = 0x801E68; // &block_element_field_extractor_factory::`vftable
	int32_t field_4 = NONE;
	char nothing[safety_margin] = { 'n', 'o', 't' };
};

enum ObjectType : uint32_t
{
	Biped = 0,
	Vehicle = 1,
	Weapon = 2,
	Equipment = 3,
	Garbage = 4,
	Projectile = 5,
	Scenery = 6,
	Machine = 7,
	Control = 8,
	LightFixture = 9,
	SoundScenery = 10,
	Crate = 11,
	Creature = 12,
};

/*
	This function barely works, leaks memory, don't touch it or it will crash
*/
void add_object_hierarchy(void* map, blam_tag tag, editor_string name, intptr_t offset, ObjectType type)
{
	hierarchy_list_entry entry;
	insert_post_handler(map, &entry, tag);
	entry.field_0 = *reinterpret_cast<int*>(0xA66AC8); // g_tool_implementation
	entry.field_4 = *reinterpret_cast<int*>(0xA66ACC); // g_tool_implementation::counted_base

	// something to do with counted base
	typedef LONG __fastcall sub_40BC10(void* thisptr);
	auto sub_40BC10_impl = reinterpret_cast<sub_40BC10*>(0x40BC10);
	sub_40BC10_impl(*reinterpret_cast<void**>(0xA66ACC)); // g_tool_implementation::counted_base

	// functions or something
	typedef void* __fastcall boost__counted_base__field_extractor_factory__ctor(void* thisptr, int, block_element_field_extractor_factory* a2);
	auto boost__counted_base__field_extractor_factory__ctor__impl = reinterpret_cast<boost__counted_base__field_extractor_factory__ctor*>(0x47E640);
	typedef void __fastcall sub_47E440(void* thisptr);
	auto sub_47E440_impl = reinterpret_cast<sub_47E440*>(0x47E440);
	typedef void __fastcall sub_47E5E0(void* thisptr, int, hierarchy_list_entry* a2);
	auto sub_47E5E0_impl = reinterpret_cast<sub_47E5E0*>(0x47E5E0);

	int data[8 + (safety_margin / sizeof(int))]; // don't care enough to make a struct
	sub_47E440_impl(&data);
	data[0] = 0xA668D8;
	data[7] = 236;

	auto block_factory = new block_element_field_extractor_factory;

	boost__counted_base__field_extractor_factory__ctor__impl(&data, 0, block_factory);

	sub_47E5E0_impl(&data, 0, &entry);

	typedef void __fastcall scenario_object_node_information_factory__ctor(void* thisptr, int, int a2, void* a3, void* a4, hierarchy_entry_info* a5);
	auto scenario_object_node_information_factory__ctor__impl = reinterpret_cast<scenario_object_node_information_factory__ctor*>(0x47E970);

	hierarchy_entry_info info(name, tag);

	typedef void __fastcall scenario_block_field_extractor_factory__ctor(void* thisptr, int, int offset, int a3, int a4);
	auto scenario_block_field_extractor_factory__ctor__impl = reinterpret_cast<scenario_block_field_extractor_factory__ctor*>(0x482720);

	auto extractor = new char[0x38u + safety_margin];
	scenario_block_field_extractor_factory__ctor__impl(extractor, 0, offset, type, 0);

	// this is likely what actually breaks
	auto memory_leak = new char[28 + safety_margin]; // memory was made to be leaked
	scenario_object_node_information_factory__ctor__impl(memory_leak, 0, 0xA66A10, extractor, &data, &info);

	// cleanup??

	typedef void __fastcall sub_47E3D0(void *thisptr);
	auto sub_47E3D0_impl = reinterpret_cast<sub_47E3D0*>(0x47E3D0);
	typedef void __fastcall Boost__counted_base__free(intptr_t thisptr);
	auto Boost__counted_base__free__impl = reinterpret_cast<Boost__counted_base__free*>(0x40B820);

	sub_47E3D0_impl(&data);
	Boost__counted_base__free__impl(entry.field_4);
}

static void __fastcall create_custom_objects_section(void* map_base)
{
	add_object_hierarchy(map_base, 'bipd', "Bipeds", offsetof(scnr_tag, bipeds), ObjectType::Biped);
	add_object_hierarchy(map_base, 'vehi', "Vehicles", offsetof(scnr_tag, vehicles), ObjectType::Vehicle);
	add_object_hierarchy(map_base, 'weap', "Weapon", offsetof(scnr_tag, weapons), ObjectType::Weapon);
	add_object_hierarchy(map_base, 'eqip', "Equipment", offsetof(scnr_tag, equipment), ObjectType::Equipment);
	add_object_hierarchy(map_base, 'cret', "Creatures", offsetof(scnr_tag, creatures), ObjectType::Creature);
}

static void ASM_FUNC hierarchy_objects_section_create_hook()
{
	__asm
	{
		lea     ecx, [esp + 0x30]
		call    create_custom_objects_section

		// replaced code
		lea     ecx, [esp + 0x18]
		push    ecx

		// return
		mov     ecx, 0x4133C2
		jmp     ecx
	}
}

void H2SapienPatches::add_custom_hierarchy_entries()
{
	WriteJmp(0x4133BD, hierarchy_objects_section_create_hook);
}
