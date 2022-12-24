#include "datatype.h"

Table current_table;
table_entity_attribute raw_data[MAX_ATTRIBUTE_NUM];

void Table::get_attribute_size()
{
    length=0;
    for(int i=0; i < attribute_num; i++)
    {
        switch (attribute[i].type)
        {
        case 0:
            length += sizeof(int);
            break;
        case 1:
            length += sizeof(float);
            break;
        case 2:
            length += attribute[i].type_size + 1;
            break;
        }
    }
}

Table::Table() {}

Table::Table(const Table& ori)
{
    attribute_num = ori.attribute_num;
    for(int i=0; i < attribute_num; i++)
    {
		attribute[i].name = ori.attribute[i].name;
		attribute[i].type = ori.attribute[i].type;
		attribute[i].type_size = ori.attribute[i].type_size;
		attribute[i].state = ori.attribute[i].state;
		attribute[i].is_display = ori.attribute[i].is_display;
	}
}

int compare(void* a, void* b, int datatype)
{
	if (datatype == 0)
	{
		int t1 = *(int*)a;
		int t2 = *(int*)b;
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
	else if (datatype == 1)
	{
		float t1 = *(float*)a;
		float t2 = *(float*)b;
		if(abs(t1 - t2) < MIN_THETA)
			return 0;
		else if(t1 < t2)
			return -1;
		else
			return 1;
	}
	else
	{
		string t1 = *(string*)a;
		string t2((char*)b);
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
}

int compare(void* a, void* b, int datatype,int i)
{
	if (datatype == 0)
	{
		int t1 = *(int*)a;
		int t2 = ((int*)b)[i];
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
	else if (datatype == 1)
	{
		float t1 = *(float*)a;
		float t2 = ((float*)b)[i];
		if(abs(t1 - t2) < MIN_THETA)
			return 0;
		else if(t1 < t2)
			return -1;
		else
			return 1;
	}
	else
	{
		string t1 = *(string*)a;
		string t2((char*)b + 256*i);
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
}

int compare(Data* a, void* b, int datatype, int i)
{
	if (datatype == 0)
	{
		int t1 = ((Datai*)a)->x;
		int t2 = ((int*)b)[i];
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
	else if (datatype == 1)
	{
		float t1 = ((Dataf*)a)->x;
		float t2 = ((float*)b)[i];
		if(abs(t1 - t2) < MIN_THETA)
			return 0;
		else if(t1 < t2)
			return -1;
		else
			return 1;
	}
	else
	{
		string t1 = ((Datac*)a)->x;
		string t2((char*)b + 256*i);
		if (t1 < t2)
			return -1;
		else if (t1 == t2)
			return 0;
		else
			return 1;
	}
}