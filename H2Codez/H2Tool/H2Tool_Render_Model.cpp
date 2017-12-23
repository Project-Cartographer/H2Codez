#include "stdafx.h"
#include"H2Tool_Render_Model.h"
#include<cstdarg>


sbsp_mode::sbsp_mode(char* mode_data, DWORD mode_size):mode_size(mode_size)
{
	this->mode_data = new char[mode_size];
	memcpy(this->mode_data, mode_data, mode_size);
}
tag_data_struct* sbsp_mode::Get_Tag_DATA()
{
	tag_data_struct* ret = new tag_data_struct();

	ret->tag_data = mode_data;
	ret->size = mode_size;

	return ret;
}
/*
the arguments should be of the form
argument list should be of the from (count,tag_data_struct1*,tag_data_struct2*........)
*/
void sbsp_mode::Add_sbps_DATA(DWORD count, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, count);
	
	tag_data_struct* sbsp_data_struct;
	char* sbsp_data;
	DWORD sbsp_size;

	//first we add all the Clusters
	for (int i = 0;i < count;i++)
	{		
		sbsp_data_struct = (tag_data_struct*)va_arg(arg_ptr, DWORD);
		sbsp_data = sbsp_data_struct->tag_data;
		sbsp_size = sbsp_data_struct->size;
		
		Add_Clusters(sbsp_data, sbsp_size);
	}

	//next we go for Cluster_DATA
	va_start(arg_ptr, count);

	for (int i = 0;i < count;i++)
	{
		sbsp_data_struct = (tag_data_struct*)va_arg(arg_ptr, DWORD);
		sbsp_data = sbsp_data_struct->tag_data;
		sbsp_size = sbsp_data_struct->size;
		
		dfbt_list* sbsp_dfbt_list = List_block_headers(sbsp_data, sbsp_size);
		dfbt* sbsp_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x318, 0x0);
		DWORD* pcluster_count = (DWORD*)((DWORD)sbsp_header + 0x10 + 0xD4);

		DWORD mem_start = (DWORD)sbsp_data;//a variable that is the deciding factor in selecting the correct field to be copied

		dfbt* cluster_header = Get_dfbt_from_size(sbsp_dfbt_list, 0xD8, 0x0);
		//the number of cluster_count,the number of seperate declaration of Cluster_DATA header(cuz each Cluster can have a max of ONE Cluster DATA)
		for (int j = 0;j < *pcluster_count;j++)
		{
			DWORD cluster_data_count = *(DWORD*)((DWORD)cluster_header + 0x10 + 0x28 + 0x2C + i * 0xD8);

			//We have to write the starting of the Child_BLOCKS
			Add_declaration((char*)FINS);

			if (cluster_data_count ==1)
			{
				DWORD old_mode_size = mode_size;
				//Time to write a new_header for Cluster_DATA_block
				dfbt* cluster_data_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x6C, mem_start);
				Add_Header((char*)cluster_data_header);

				//fix the header a bit
				*(DWORD*)(mode_data + old_mode_size + 0x4) = 0x1;
				//and the size
				*(DWORD*)(mode_data + old_mode_size + 0xC) = 0xB4;
				//Time to copy the Cluster_DATA and its child_BLOCKS
				Add_Cluster_DATA(sbsp_data, sbsp_size, mem_start);
			}

			//we have to write the end of the child_BLOCKS
			Add_declaration((char*)KOLB);
		}

	}

}
void sbsp_mode::Add_Clusters(char* sbsp_data, DWORD sbsp_size)
{
	dfbt_list* sbsp_dfbt_list = List_block_headers(sbsp_data, sbsp_size);
	dfbt_list* mode_dfbt_list = List_block_headers(mode_data, mode_size);

	dfbt* sbsp_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x318, 0x0);

	if (sbsp_header !=nullptr)
	{
		DWORD* pcluster_count = (DWORD*)((DWORD)sbsp_header + 0x10 + 0xD4);

		dfbt* mode_header = Get_dfbt_from_size(mode_dfbt_list, 0xB8, 0x0);
		DWORD* psection_count = (DWORD*)((DWORD)mode_header + 0x10 + 0x30);

		dfbt* cluster_header = Get_dfbt_from_size(sbsp_dfbt_list, 0xD8, 0x0);

		if (cluster_header !=nullptr)
		{
			DWORD new_mode_size;

			if (*psection_count == 0)
				new_mode_size = mode_size + 0x10 + (*pcluster_count) * 0x68;
			else new_mode_size = mode_size + (*pcluster_count) * 0x68;

			char* new_mode_data = new char[new_mode_size];

			memcpy(new_mode_data, mode_data, mode_size);

			DWORD section_count_off = (DWORD)psection_count - (DWORD)mode_data;

			if (*psection_count == 0)
			{
				//we create a new dfbt header and add stuff accordingly
				memcpy(new_mode_data + mode_size, cluster_header, 0x10);//copied the cluster BLOCK header
				//the header already contains the count

				//fix the cluster BLOCK header size a bit
				*(DWORD*)(new_mode_data + mode_size + 0x4) = 0x0;//fix a difference in dfbt declarations
				*(DWORD*)(new_mode_data + mode_size + 0xC) = 0x68;//fix the size

				//time to copy clusters
				for (int i = 0;i < *pcluster_count;i++)
					Copy_Cluster_BLOCK(new_mode_data + mode_size + 0x10 + i * 0x68, (char*)((DWORD)cluster_header + 0x10 + i * 0xD8));
			}
			else
			{
				//we just add stuff and update the count
				//update dfbt header count
				dfbt* section_header = Get_dfbt_from_size(mode_dfbt_list, 0x68, 0x0);
				section_header->block_count += *pcluster_count;

				//add stuff
				for (int i = 0;i < *pcluster_count;i++)
					Copy_Cluster_BLOCK(new_mode_data + mode_size + i * 0x68, (char*)((DWORD)cluster_header + 0x10 + i * 0xD8));
			}
			//Update the count in the new_mode_data
			*(DWORD*)(new_mode_data + section_count_off) = *psection_count + cluster_header->block_count;

			//update stuff
			delete[] mode_data;
			mode_data = new_mode_data;
			mode_size = new_mode_size;
		}
	}
}
dfbt_list* sbsp_mode::List_block_headers(char* tag_data,DWORD size)
{	
	//first dfbt starts at 0x40
	dfbt* my_dfbt = (dfbt*)((DWORD)tag_data + 0x40);

	dfbt_list* ret = new dfbt_list();
	ret->DFBT = my_dfbt;
	ret->next_header = nullptr;

	dfbt_list* current_header_in_action = ret;

	for (int i = 0x50;i < size;i++)
	{
		if (*(tag_data + i) == DFBT[0])
		{
			if (*(tag_data + i + 1) == DFBT[1])
			{
				if (*(tag_data + i + 2) == DFBT[2])
				{
					if (*(tag_data + i + 3) == DFBT[3])
					{
						//well found another dfbt
						dfbt* my_dfbt = (dfbt*)((DWORD)tag_data + i);

						dfbt_list* temp = new dfbt_list();
						temp->DFBT = my_dfbt;
						temp->next_header = nullptr;
						
						current_header_in_action->next_header = temp;

						current_header_in_action = temp;
					}
				}
			}
		}
	}
	return ret;
}

dfbt* sbsp_mode::Get_dfbt_from_size(dfbt_list* list,DWORD block_size,DWORD mem_start)
{
	while (list)
	{
		if ((list->DFBT->block_size == block_size) && (((DWORD)list->DFBT) > mem_start))
			return list->DFBT;

		list = list->next_header;
	}
	return nullptr;
}
void sbsp_mode::Copy_Cluster_BLOCK(char* dest, char* src)
{
	*(long*)(dest) = (long)0x0;//padding for geometry classification(4 bytes)
	memcpy(dest + 0x4, src, 0x2c);//copying global_geometry_section_info_struct_block
	*(__int16*)(dest + 0x4 + 0x2C) = 0xFFFF;//rigidNode 2bytes
	*(__int16*)(dest + 0x4 + 0x2C + 0x2) = 0x0;//Flags 2bytes
	memcpy(dest + 0x4 + 0x2C + 0x2 + 0x2, src + 0x2C + 0x28, 0xC);//copying cluster_data BLOCK declaration
	memcpy(dest + 0x4 + 0x2C + 0x2 + 0x2 + 0xC, src + 0x2C, 0x28);//copying BLOCK_info struct

	*(DWORD*)(dest + 0x18) = 0x0;//setting the count of global_geometry_compression_info_block to zero as i am not intrested in copying it
}
void sbsp_mode::Add_Cluster_DATA(char* sbsp_data, DWORD sbsp_size, DWORD& mem_start)
{

	dfbt_list* sbsp_dfbt_list = List_block_headers(sbsp_data, sbsp_size);
	dfbt* cluster_DATA_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x6C, mem_start);

	DWORD cluster_DATA_count = cluster_DATA_header->block_count;

	DWORD new_mode_size = mode_size + cluster_DATA_count * 0xB4;
	char* new_mode_data = new char[new_mode_size];
	memcpy(new_mode_data, mode_data, mode_size);


	for (int i = 0;i < cluster_DATA_count;i++)
	{
		Copy_Cluster_DATA_BLOCK(new_mode_data + mode_size + i * 0xB4, (char*)cluster_DATA_header + 0x10 + i * 0x6C);
	}

	delete[] mode_data;
	mode_data = new_mode_data;
	mode_size = new_mode_size;

	Add_declaration((char*)TCES);//write the beginning of the child_BLOCKS

	//now time to copy the child_BLOCKs
	mem_start = (DWORD)cluster_DATA_header;

	DWORD parts_count = *(DWORD*)((DWORD)cluster_DATA_header + 0x10);

	if (parts_count > 0)
	{
		dfbt* parts_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x48, mem_start);
		
		Add_Header((char*)parts_header);

		DWORD new_mode_size = mode_size + 0x48 * parts_count;
		char* new_mode_data = new char[new_mode_size];

		memcpy(new_mode_data, mode_data, mode_size);

		for (int i = 0;i < parts_count;i++)
			Copy_Parts_BLOCK(new_mode_data + mode_size + i * 0x48, (char*)parts_header + 0x10 + i * 0x48);

		delete[] mode_data;
		mode_data = new_mode_data;
		mode_size = new_mode_size;

		mem_start = (DWORD)parts_header;
	}
	//time to copy sub_parts;
	DWORD sub_parts_count = *(DWORD*)((DWORD)cluster_DATA_header + 0x10 + 0xC);
	if (sub_parts_count > 0)
	{
		dfbt* sub_parts_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x8, mem_start);

		Add_Header((char*)sub_parts_header);

		DWORD new_mode_size = mode_size + 0x8 * sub_parts_count;
		char* new_mode_data = new char[new_mode_size];

		memcpy(new_mode_data, mode_data, mode_size);
		memcpy(new_mode_data + mode_size, (char*)sub_parts_header + 0x10, 0x8 * sub_parts_count);

		delete[] mode_data;
		mode_data = new_mode_data;
		mode_size = new_mode_size;		

		mem_start = (DWORD)sub_parts_header;
	}
	//time to copy RAW_vertex
	DWORD RAW_vertex_count = *(DWORD*)((DWORD)cluster_DATA_header + 0x10 + 0x24);
	if (RAW_vertex_count > 0)
	{
		dfbt* RAW_vertex_header = Get_dfbt_from_size(sbsp_dfbt_list, 0xC4, mem_start);

		Add_Header((char*)RAW_vertex_header);

		DWORD new_mode_size = mode_size + 0xC4 * RAW_vertex_count;
		char* new_mode_data = new char[new_mode_size];

		memcpy(new_mode_data, mode_data, mode_size);
		memcpy(new_mode_data + mode_size, (char*)RAW_vertex_header + 0x10, 0xC4 * RAW_vertex_count);

		delete[] mode_data;
		mode_data = new_mode_data;
		mode_size = new_mode_size;

		mem_start = (DWORD)RAW_vertex_header;
	}
	//Copy strip_index BLOCK
	DWORD strip_index_count = *(DWORD*)((DWORD)cluster_DATA_header + 0x10 + 0x30);
	if (strip_index_count > 0)
	{
		dfbt* strip_index_header = Get_dfbt_from_size(sbsp_dfbt_list, 0x2, mem_start);

		Add_Header((char*)strip_index_header);

		DWORD new_mode_size = mode_size + 0x2 * strip_index_count;
		char* new_mode_data = new char[new_mode_size];

		memcpy(new_mode_data, mode_data, mode_size);
		memcpy(new_mode_data + mode_size, (char*)strip_index_header + 0x10, 0x2 * strip_index_count);

		delete[] mode_data;
		mode_data = new_mode_data;
		mode_size = new_mode_size;

		mem_start = (DWORD)strip_index_header;
	}
	Add_declaration((char*)TADP);//write the end of the child_BLOCKS

}
void sbsp_mode::Copy_Cluster_DATA_BLOCK(char* dest, char* src)
{
	//Copy the Global geometry struct
	memcpy(dest, src, 0x6C);
	//set the Visibility Bounds count to zero
	*(DWORD*)(dest + 0x18) = 0x0;
	//setting Visibility mope DATA count to 0x0
	*(DWORD*)(dest + 0x3C) = 0x0;
	//have to set the MOPP Reorder Table count to 0x0
	*(DWORD*)(dest + 0x50) = 0x0;
	//Vertex Buffer BLOCK
	*(DWORD*)(dest + 0x5C) = 0x0;
	//write rest of the bytes to 0
	memset(dest + 0x6C, 0x0, 0x48);
	/*
	You will be required to create it manually through guerrilla
	//and lastly create a node_map BLOCK
	*(DWORD*)(dest + 0xA4) = 0x1;
	*/
}
void sbsp_mode::Add_Header(char* header_data)
{
	DWORD new_mode_size = mode_size + 0x10;
	char* new_mode_data = new char[new_mode_size];

	memcpy(new_mode_data, mode_data, mode_size);
	memcpy(new_mode_data + mode_size, (void*)header_data, 0x10);

	delete[] mode_data;
	mode_data = new_mode_data;
	mode_size = new_mode_size;

}
void sbsp_mode::Add_declaration(char* tail_data)
{
	DWORD new_mode_size = mode_size + 0x10;
	char* new_mode_data = new char[new_mode_size];

	memcpy(new_mode_data, mode_data, mode_size);
	memcpy(new_mode_data + mode_size, tail_data, 0x10);

	delete[] mode_data;
	mode_data = new_mode_data;
	mode_size = new_mode_size;
}
void sbsp_mode::Copy_Parts_BLOCK(char* dest, char* src)
{
	memcpy(dest, src, 0x48);
	*(DWORD*)(dest + 0x2) = 0x3;//fix some flags
}