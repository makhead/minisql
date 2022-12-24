#include "IndexManager.h"
using namespace std;

void IndexManager::CreateIndex()//empty tree
{
	if (index_exist())
	{
		cout << "Index already exist!\n";
		return;
	}
	string filename = "Table_" + table_index_name + "_bptree";
	ofstream out(filename);
	out.close();
	filename = "Table_" + table_index_name + "_index";
	ofstream out2(filename);
	out2.close();

	current_table.blockNum += 2;
	bf.changeblock();
	BpTree Bp(table_index_name, col_type);
	Bp.create(col_type, table_index_name);
}

int IndexManager::index_exist()
{
	string index_file_name = "Table_" + table_index_name + "_index";
	ifstream in(index_file_name);
	if (in.fail())
		return false;
	else
	{
		in.close();
		return true;
	}
}

void IndexManager::Drop()
{
	string file1 = "Table_" + table_index_name + "_index";
	string file2 = "Table_" + table_index_name + "_bptree";
	bf.invalid(file1);
	bf.invalid(file2);
	if (remove(file1.c_str()) == 0 && remove(file2.c_str()) == 0)
		cout << "Index drop succeeded" << endl;
	else
		cout << "Index drop failed" << endl;
	return;
}

int IndexManager::Find(Data*& key)
{
	if (!index_exist())
	{
		cout << "No such index!\n";
		return -1;
	}
	BpTree Bp(table_index_name, col_type);
	return Bp.find(key);
}

// vector<int> IndexManager::FindRange(Data*& keystart, Data*& keyend)
vector<int> IndexManager::FindRange()
{
	vector<int>ret = {-1};
	if (!index_exist())
	{
		cout << "No such index!\n";
		return ret;
	}
	Data *a, *b;
	BpTree Bp(table_index_name, col_type);

	a = new Datac("caomincheng");
	b = new Datac("wy");
	ret = Bp.RangeFind(a, b);
	delete a;
	delete b;
	return ret;
}

void IndexManager::Insert(Data*& key,int offset)
{
	if (!index_exist())
	{
		cout << "No such index!\n";
		return;
	}
	if(Find(key) != -1)
	{
		cout << "index already exist" << endl;
		return;
	}
	BpTree Bp(table_index_name, col_type);
	Bp.insert(key, offset, table_index_name);
	Bp.Bpsave();
}

void IndexManager::Delete(Data*& key)
{
	if (!index_exist())
	{
		cout << "No such index!\n";
		return;
	}
	if (Find(key) == -1)
	{
		cout << "The value you want to delete is not found!\n";;
		return;
	}
	BpTree Bp(table_index_name, col_type);
	Bp.deletenode(key);
	Bp.Bpsave();
}