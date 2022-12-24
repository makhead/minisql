#ifndef INDEXMANAGER_H
#define INDEXMANAGER_H
#include <string>
#include <iostream>
#include "datatype.h"
#include "Bplus.h"

using namespace std;

class IndexManager
{
	int col_type;
	string table_index_name;
public:
	IndexManager(string name, int col_type_in) : table_index_name(name), col_type(col_type_in) {}
	void CreateIndex();
	void Drop();
	void Insert(Data*& key,int offset);
	void Delete(Data*& key);
	int Find(Data*& key);
	vector<int> FindRange();
	// vector<int> FindRange(Data*& keystart, Data*& keyend);
	int index_exist();
};

#endif //INDEXMANAGER_H