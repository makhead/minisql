#ifndef API_H
#define API_H
#include "CatalogManager.h"
#include "RecordManager.h"
#include "IndexManager.h"

using namespace std;

class API {
public:
    void get_table(string& table_name);
    void create_table(string& table_name, int& num_of_col, string& primary_key);
    void drop_table(string& table_name);
    void delete_data(where& wc);
    void create_index(string& table_name, string& attri_name, string& index_name);
    void select(vector<int>& attri_proj, where& wc);
    void insert(vector<int>& uni_no);
    void drop_index(string& table_name, string& index_name);
};

#endif