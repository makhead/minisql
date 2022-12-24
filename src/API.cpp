#include "API.h"

void API::get_table(string& table_name) {
    try {
        CatalogManager Catalog_m;

        Catalog_m.get_table(table_name);
        current_table.get_attribute_size();
    } catch(int error_type) {
        throw error_type;
    }
}

void API::create_table(string& table_name, int& num_of_col, string& primary_key) {
    CatalogManager Catalog_m;
    RecordManager Record_m;

    Catalog_m.create_table(table_name, num_of_col, primary_key);
	Record_m.create_table();
}

void API::drop_table(string& table_name) {
    try {
        CatalogManager Catalog_m;
        RecordManager Record_m;
        get_table(table_name);
        for(int i=0; i<current_table.index.size(); i++)
        {
            IndexManager Index_m(current_table.table_name + '_' + current_table.index[i].index_name, 0);
            Index_m.Drop();
        }
        Catalog_m.drop_table(table_name);
        Record_m.DropTable(table_name);
        cout << "Drop process suceeded!\n";
    } catch(int error_type) {
        throw error_type;
    }
}

void API::create_index(string& table_name, string& attri_name, string& index_name) {
    try {
        CatalogManager Catalog_m;
        RecordManager Record_m;
        int attri_no;

        for(attri_no = 0; attri_no < current_table.attribute_num; attri_no++)
            if(current_table.attribute[attri_no].name == attri_name)
                break;
        if(attri_no == current_table.attribute_num)
            cout << "No attribute named " << attri_name << endl;

        Catalog_m.create_index(table_name, attri_no, index_name);
        Record_m.CreateIndex(table_name, attri_no, index_name);
    } catch(int error_type) {
        throw error_type;
    }
}

void API::select(vector<int>& attri_proj, where& wc) {
    RecordManager Record_m;

    Record_m.select(attri_proj, wc);
}

void API::delete_data(where& wc) {
    RecordManager Record_m;

    Record_m.Delete(wc);
}

void API::insert(vector<int>& uni_no) {
    try {
        RecordManager Record_m;

        Record_m.insert(uni_no);
    } catch(int error_type) {
        throw error_type;
    }
}

void API::drop_index(string& table_name, string& index_name)
{
    CatalogManager Catalog_m;
    get_table(table_name);
    for(int i=0; i<current_table.index.size(); i++)
        if(current_table.index[i].index_name == index_name)
            current_table.index.erase(current_table.index.begin()+i);
    Catalog_m.update_block("Table_" + table_name + "_schema");

    IndexManager Index_m(table_name + '_' + index_name, 0);
    Index_m.Drop();
}