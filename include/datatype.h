#ifndef DATATYPE
#define DATATYPE

#include <string>
#include <vector>
#include <cmath>
#define MAX_ATTRIBUTE_NUM 100
#define maxStringlength  256
#define MIN_THETA 0.0001
#define MAX_INDEX_NUM 10

using namespace std;

struct table_entity_attribute
{
	string name;
	short type;  // int = 0 , float = 1, char = 2
	unsigned char type_size;
	short state; // normal attribute = 0 unique attribute = 1 unique attribute+index = 2
	bool is_display;
};

struct Index
{
    short attri_no;
	int attri_offset;
    string index_name;
};

class Data
{
public:
    short type;
	virtual ~Data() {};
};

class Datai : public Data
{
public:
	int x;

    Datai(int i):x(i){
        type = 0;
    };
	Datai() :x(0) {
		type = 0;
	};
	virtual ~Datai() {};
};

class Dataf : public Data
{
public:
	float x;

    Dataf(float f):x(f){
        type = 1;
    };
	Dataf() :x(0) {
		type = 1;
	};
	virtual ~Dataf() {};
};

class Datac : public Data
{
public:
	string x;
	unsigned char type_size;

    Datac(string c):x(c){
        type_size = c.length();
            type = 2;
    };
	Datac() :x("") {
		type_size = 0;
		type = 2;
	};
	virtual ~Datac() {};
};

typedef vector<Data*> Tuple;
typedef vector<Tuple> Tuples;

struct Table
{
	friend class CatalogManager;
	int blockNum;
	string table_name;
	short attribute_num;
	table_entity_attribute attribute[MAX_ATTRIBUTE_NUM];
	Tuples tuples;
	vector<Index> index;
	int unique_attri_offset[MAX_INDEX_NUM]; //not including indexed unique
	int length;

	Table();
	Table(const Table& ori);
	void get_attribute_size();
};

struct condition
{
	string entity;
	string op;
	string restrict;
};

int compare(void* a, void* b, int datatype);
int compare(Data* a, void* b, int datatype, int i);
int compare(void* a, void* b, int datatype, int i);

#endif