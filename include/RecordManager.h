#ifndef RECORD_MANAGER
#define RECORD_MANAGER
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "datatype.h"
#include "bufferManager.h"
#include "IndexManager.h"

extern struct Table current_table;
extern struct table_entity_attribute raw_data[MAX_ATTRIBUTE_NUM];
extern class buffermanager bf;
extern class bufferBlock blocks[maxBlocks];

using namespace std;

typedef enum {
	eq, leq, l, geq, g, neq
} iswhere;

struct cond {
	Data* dataw;
	iswhere flag;
	string entity;
};

struct where {
	vector<cond>conditions;
	int cnumber;

	where(vector<condition>& prerequisite);
	~where();
};

class RecordManager {
public:
	void create_table();
	void insert(vector<int>& uni_no);
	void select(vector<int>& attri_proj, where& wc);
	void display(Table& out_table, vector<int>& attri_proj);
	bool tuplesatisfied(Tuple& temp, int(&index)[MAX_ATTRIBUTE_NUM], where& wc);
	void SelectProject(Table& out_table, vector<int>& attri_proj);
	void Delete(where& wc);
	void DropTable(string table_name);
	void CreateIndex(string table_name, int attri_no, string index_name);
	bool Search_with_Index();
	bool Search_without_Index(vector<int>& uni_no);
};

#endif