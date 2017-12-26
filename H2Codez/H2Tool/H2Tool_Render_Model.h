#pragma once
#include<iostream>
const char DFBT[] = "dfbt";

const __int8 FINS[0x10] = { 0x46 ,0x4E, 0x49, 0x53 ,0x00 ,0x00, 0x00, 0x00, 0x01 ,0x00, 0x00, 0x00 ,0x2C, 0x00, 0x00 ,0x00 };//denotes starting of Section BLOCK's child BLOCKs section
const __int8 TCES[0x10] = { 0x54, 0x43 ,0x45, 0x53 ,0x01,0x00,0x00, 0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x6C ,0x00 ,0x00 ,0x00 };//used to denote the starting of Section DATA BLOCK's child BLOCKs section
const __int8 TADP[0x10] = { 0x54 ,0x41 ,0x44 ,0x50 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x38 ,0x00 ,0x00 ,0x00 };//used to denote the ending of Section DATA BLOCK's child BLOCKs section
const __int8 KOLB[0x10] = { 0x4B ,0x4F ,0x4C ,0x42 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x28 ,0x00 ,0x00 ,0x00 };//denotes ending of Section BLOCK's child BLOCKs section

struct dfbt
{
	long long padding;
	DWORD block_count;
	DWORD block_size;
};
struct dfbt_list
{
	dfbt* DFBT;
	dfbt_list* next_header;
};
struct tag_data_struct
{
	char* tag_data;
	DWORD size;
};
//a class to take and convert contents of sbsp render_data to mode render_data
class sbsp_mode
{
public :
	sbsp_mode(char*, DWORD);
	void Add_sbps_DATA(tag_data_struct* args[], DWORD count);
	tag_data_struct* Get_Tag_DATA();
private:
	char* mode_data;
	DWORD mode_size;

	void Add_Header(char* header_data);
	void Add_declaration(char* tail_data);

	void Add_Clusters(char* sbsp_data, DWORD sbsp_size);
	void Add_Cluster_DATA(char*, DWORD, DWORD&);

	dfbt_list* List_block_headers(char* tag_data, DWORD size);
	dfbt* Get_dfbt_from_size(dfbt_list*,DWORD block_size,DWORD start_off);

	void Copy_Cluster_BLOCK(char* dest, char* src);
	void Copy_Cluster_DATA_BLOCK(char* dest, char* src);
	void Copy_Parts_BLOCK(char* dest, char* src);
};