#ifndef CATALOGMANAGER
#define CATALOGMANAGER

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

#include "bufferManager.h"
#include "IndexManager.h"
#include "datatype.h"

extern struct table_entity_attribute raw_data[MAX_ATTRIBUTE_NUM];
extern struct Table current_table;
extern class buffermanager bf;
extern class bufferBlock blocks[maxBlocks];

using namespace std;

class CatalogManager {
public:
    void get_table(string table_name);
    void create_table(string table_name, int num_of_col, string primary_key);
    bool table_exist(string table_name);
    void create_index(string table_name, int attri_no, string index_name);
    void drop_table(string table_name);
    void drop_index();
    void update_block(const string& file);
};

#endif