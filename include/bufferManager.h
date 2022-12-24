#pragma once
#include"datatype.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <ctime>
#define maxBlocks 1000
#define blockSize 4096
#define Deleted '#'
#define notEmpty 1
#define Empty  '#'

extern class bufferBlock blocks[maxBlocks];
extern struct Table current_table;

class bufferBlock
{
public:
	//position
	unsigned int blockpos; //n-th block
	unsigned int pos_in_block;//position in the block
	//status
	std::string fname;
	char vals[blockSize + 1];
	bool dirty; //是否修改过
	bool valid;
	//LRU
	unsigned int last_vis_time;
	//file
	unsigned int offset;
	//
	//func
	void init();  //initiallize
	//value get
	string getvals(int start, int end);
	//initiallize the block
	bufferBlock()
	{
		init();
	}
};

class buffermanager
{
public:
	buffermanager();
	~buffermanager();
	friend class RecordManager;
	friend class CataManager;
	
	void writetoFile(int num);
	void read(string fname, int offset, int num);
	void write(int num);
	void used(int num);
	unsigned int blockNum(std::string fname, int offset);
	unsigned int find_Empty_Buffer(std::string fname);//like above but can't change 
	unsigned int inflate(string fname);
	void invalid(string fname);
	int find_assign_buffer(string fname, int offset);
	bufferBlock find_Insertable_pos();
	unsigned int getaBlock(string fname, int offset);
	void changeblock();
};