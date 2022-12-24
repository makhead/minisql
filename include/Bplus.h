#ifndef _BPLUS_H
#define _BPLUS_H
#define HeaderOffset 16
#define PointerOffset (maxchildren * index_length[col_type])
// #include <fstream>
#include <iostream>
#include <cstring>
#include "datatype.h"
#include "bufferManager.h"

extern class buffermanager bf;
extern class bufferBlock blocks[maxBlocks];
enum NodeState{Root,Internal,Leaf};

const int index_length[3] = { 4,4,256 };

class Bpnode
{
public:
	int state;   // root = 0 , internal = 1, leaf = 2
	int nodeAddr;
	int nowChildren;
	int fatherAddr;
	int blocknum;
	void* data;
	int* pointer;
public:
	void init(int col_type, int maxchildren);
	int insert(Data* key, int offset, int col_type); //leaf
	int insert(void* key, int offset, int col_type); // internal
	void deleteRecord(Data* key, int col_type);//leaf
	void deleteRecord(void* key, int col_type);//internal
	void updateInternalKey(Data* originKey, Data*updateKey, int col_type);
	void cleannode(int numofnode,int&rootpos,int col_type,int maxchildren, string table_name);
	//============================================================================================
	void mergeLeaf(Bpnode& node, int col_type, int maxchildren, string table_name); // merge two leaf
	void mergeInternal(Bpnode& node, int col_type, string table_name, int maxchildren); // merge two internal
	int rightBorrowOK(int col_type, string table_name, int maxchildren);
	int leftBorrowOK(int col_type, string table_name, int maxchildren);
	int rightMergeOK(int col_type, string table_name, int maxchildren);
	int leftMergeOK(int col_type, string table_name, int maxchildren);
	void rightBorrowInternal (int col_type, string table_name, int maxchildren);
	void rightBorrowLeaf(int col_type, string table_name, int maxchildren);
	void leftBorrowInternal(int col_type, string table_name, int maxchildren);
	void leftBorrowLeaf(int col_type, string table_name, int maxchildren);
	//=============================================================================================
	void splitLeafnode(Bpnode& splitnode, int col_type, int maxchildren);
	void splitInternalnode(Bpnode& splitnode, int col_type, int maxchildren, string table_name);
	//==========================================================================================
	void getNode(int nodeAddr_in, string table_name, int col_type, int maxchildren);
	void saveNode(int nodeAddr_in, int col_type, int maxchildren);
	void freeNode(int col_type);
};

int deleteBack(int curAddr, Data* key, int col_type, int& maxchildren, int& numofnode, int& rootpos, string table_name);
int insertBack(int curAddr, Data* key, int col_type, int offset, int& maxchildren, int& numofnode, int& rootpos, string table_name);

class BpTree
{
public:
	string table_name;
	int col_type;
	int maxchildren;
	int numofnode;
	int rootpos;
	void insert(Data* key,int offset,string table_name);
	int find(Data* key); // init the table name first
	vector<int> RangeFind(Data* key1, Data* key2);
	void deletenode(Data* key);	
	void create(int col_type_in, string table_name);
	void Bpsave();
	BpTree(string tablename_in,int col_type_in);
};

#endif