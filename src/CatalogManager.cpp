#include "CatalogManager.h"

void CatalogManager::get_table(string table_name)
{
    if(!table_exist(table_name))
        throw 14;

    current_table.table_name = table_name;
    string file = "Table_" + table_name + "_schema";
    int No = bf.getaBlock(file, 0), i;
    char* begin = blocks[No].vals;
    char temp[maxStringlength];
    char save[256];
    int pos = 0;

    memcpy(&current_table.attribute_num, &begin[pos], sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&i, &begin[pos], sizeof(int));
    current_table.index.resize(i);
    pos = pos + sizeof(int);
    memcpy(&current_table.blockNum, &begin[pos], sizeof(int));
    pos = pos + sizeof(int);
    memcpy(current_table.unique_attri_offset, &begin[pos], sizeof(int)*MAX_INDEX_NUM);
    pos = pos + sizeof(int)*MAX_INDEX_NUM;

    int pos1 = pos;
    for(i=0; i < current_table.attribute_num; i++)
    {
        while(begin[pos1]!='\0')
            pos1 += sizeof(char);
        memcpy(&temp, &begin[pos], (pos1-pos+1)*sizeof(char));
        current_table.attribute[i].name = temp;
        pos = pos1 + sizeof(char);
        pos1 = pos;
    }
    
    for(i=0;i<current_table.attribute_num;i++){
        memcpy(&current_table.attribute[i].type, &begin[pos], sizeof(short));
        pos = pos + sizeof(short);
    }

    for(i=0;i<current_table.attribute_num;i++){
        memcpy(&current_table.attribute[i].type_size, &begin[pos], sizeof(char));
        pos = pos + sizeof(char);
    }
    
    for(i=0;i<current_table.attribute_num;i++){
        memcpy(&current_table.attribute[i].state, &begin[pos], sizeof(short));
        pos = pos + sizeof(short);
    }
    
    for(i=0;i<current_table.index.size();i++){
        memcpy(&current_table.index[i].attri_no, &begin[pos], sizeof(short));
        pos = pos + sizeof(short);
    }

    pos1 = pos;
    for(i=0;i<current_table.index.size();i++){
        while(begin[pos1]!='\0')
            pos1 += 1;
        memcpy(save, &begin[pos], (pos1-pos+1)*sizeof(char));
        current_table.index[i].index_name = save;
        pos = pos1 + sizeof(char);
        pos1 = pos;
    }

    for(i=0;i<current_table.index.size();i++){
        memcpy(&current_table.index[i].attri_offset, &begin[pos], sizeof(int));
        pos = pos + sizeof(int);
    }
    
    bf.used(No);
}

void CatalogManager::create_table(string table_name, int num_of_col, string primary_key)
{
    if(table_exist(table_name))
        cout << "The table has already existed!\n";
    int i, size;
    string file = "Table_" + table_name + "_schema";
    ofstream out(file);
    out.close();
    
    current_table.table_name = table_name;
    current_table.attribute_num = num_of_col;
    for(i=0; i < num_of_col; i++)
    {
        current_table.attribute[i].name = raw_data[i].name;
		current_table.attribute[i].type = raw_data[i].type;
		current_table.attribute[i].type_size = raw_data[i].type_size;
		current_table.attribute[i].state = raw_data[i].state;
		current_table.attribute[i].is_display = raw_data[i].is_display;
    }

    if(primary_key.size())
    {
        size=1;
        Index pri_index;
        for(i=0; i < current_table.attribute_num; i++)
        {
            if(current_table.attribute[i].name == primary_key)
            {
                IndexManager Index_m(table_name + "_pri_" + primary_key, current_table.attribute[i].type);
	            Index_m.CreateIndex();

                pri_index.attri_no = i;
                pri_index.index_name = "pri_" + primary_key;
                pri_index.attri_offset = size;
                current_table.index.push_back(pri_index);
                current_table.attribute[i].state = 2;
                break;
            }
            else switch (current_table.attribute[i].type)
            {
            case 0:
                size += sizeof(int);
                break;
            case 1:
                size += sizeof(float);
                break;
            case 2:
                size += current_table.attribute[i].type_size + 1;
                break;
            }
        }
    }

    size=1;
    memset(current_table.unique_attri_offset, -1, sizeof(int)*MAX_INDEX_NUM);
    for(int i=0, j=0; i < current_table.attribute_num; i++)
    {
        if(current_table.attribute[i].state == 1)
            current_table.unique_attri_offset[j++] = size;
        switch (current_table.attribute[i].type)
        {
        case 0:
            size += sizeof(int);
            break;
        case 1:
            size += sizeof(float);
            break;
        case 2:
            size += current_table.attribute[i].type_size + 1;
            break;
        }
    }
    update_block(file);
    
    // API api; call record create tuple for this table (ready)
    // api.CreateTable(*t);
}

bool CatalogManager::table_exist(string table_name)
{
    table_name = "Table_" + table_name + "_schema";
    ifstream in(table_name);
    if(in.fail())
        return false;
    else
    {
        in.close();
        return true;
    }
}

void CatalogManager::drop_table(string table_name)
{
    if(!table_exist(table_name))
        cout << "ERROR in drop_table: No table named " << table_name << endl;
    bf.invalid("Table_" + table_name + "_schema");
    // API api; call record del
    // api.DropTable(*tb);
    if(!remove(("Table_" + table_name + "_schema").c_str()))
    	cout << "Table drop succeeded" << endl;
	else
		cout << "Table drop failed" << endl;
}

void CatalogManager::create_index(string table_name, int attri_no, string index_name)
{
    int j;
    Index new_index;
    for(j=0; j<current_table.index.size(); j++)
        if(attri_no == current_table.index[j].attri_no)
            break;
    if(j<current_table.index.size())
        cout << "Index already exists on this attribute!\n";
    
    for(j=0; j<current_table.index.size(); j++)
        if(current_table.index[j].index_name == index_name)
            break;
    if(j<current_table.index.size())
        cout << "Index name has been used!\n";
    
    new_index.attri_no = attri_no;
    new_index.index_name = index_name;
    current_table.index.push_back(new_index);
    current_table.attribute[attri_no].state = 2;
    
    string file = "Table_" + table_name + "_schema";
    update_block(file);
}

void CatalogManager::update_block(const string& file)
{
    int No = bf.getaBlock(file, 0);

    char* begin = blocks[No].vals;
    int pos = 0, i;
    
    memcpy(&begin[pos], &current_table.attribute_num, sizeof(int));
    pos = pos + sizeof(int);
    i = current_table.index.size();
    memcpy(&begin[pos], &i, sizeof(int));
    pos = pos + sizeof(int);
    
    int bn = 0;
    memcpy(&begin[pos], &bn, sizeof(int));
    pos = pos + sizeof(int);
    memcpy(&begin[pos], current_table.unique_attri_offset, sizeof(int)*MAX_INDEX_NUM);
    pos = pos + sizeof(int)*MAX_INDEX_NUM;
    
    for(i=0; i < current_table.attribute_num; i++){
        memcpy(&begin[pos], current_table.attribute[i].name.data(), current_table.attribute[i].name.length()*sizeof(char));
        pos = pos + (int)current_table.attribute[i].name.length()*sizeof(char);
        memcpy(&begin[pos], "\0", sizeof(char));
        pos += sizeof(char);
    }
    
    for(i=0; i < current_table.attribute_num; i++) {
        memcpy(&begin[pos], &current_table.attribute[i].type, sizeof(short));
        pos = pos + sizeof(short);
    }

    for(i=0; i < current_table.attribute_num; i++) {
        memcpy(&begin[pos], &current_table.attribute[i].type_size, sizeof(char));
        pos = pos + sizeof(char);
    }
    
    for(i=0; i < current_table.attribute_num; i++) {
        memcpy(&begin[pos], &current_table.attribute[i].state, sizeof(short));
        pos = pos + sizeof(short);
    }
    
    for(i=0; i < current_table.index.size(); i++) {
        memcpy(&begin[pos], &current_table.index[i].attri_no, sizeof(short));
        pos = pos + sizeof(short);
    }

    for(i=0; i < current_table.index.size(); i++) {
        memcpy(&begin[pos], current_table.index[i].index_name.data(), current_table.index[i].index_name.length()*sizeof(char));
        pos = pos + (int)current_table.index[i].index_name.length()*sizeof(char);
        memcpy(&begin[pos], "\0", sizeof(char));
        pos += sizeof(char);
    }

    for(i=0; i < current_table.index.size(); i++) {
        memcpy(&begin[pos], &current_table.index[i].attri_offset, sizeof(int));
        pos = pos + sizeof(int);
    }

    bf.write(No);
}

void CatalogManager::drop_index()
{
    /*
    Table* temp = getTable(tname);
    try{
        temp->dropindex(iname);
        drop_table(tname);
        create_table(tname, temp->attr, temp->primary, temp->index);
        delete temp;
    }
    catch(TableException e1){
        delete temp;
        throw e1;
    }*/
}

