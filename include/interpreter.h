#ifndef INTERPRETER
#define INTERPRETER

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <exception>

#include "datatype.h"
#include "API.h"

extern struct table_entity_attribute raw_data[MAX_ATTRIBUTE_NUM];
extern struct Table current_table;

using namespace std;

class interpreter {
    string s;
    stringstream string_proc;
public:
    interpreter();
    ~interpreter();

    bool querydecode();

    void Insert(API &api);
    void CreateTable(API &api);
	void DropTable(API &api);
	void DeleteData(API &api);
	void CreateIndex(API &api);
	void DropIndex(API &api);
	void Select(API &api);
    void exec_file();
    void saveblock();
    
    bool If_int(string word, string& valid);
    bool If_char(string word, int char_size, string& valid);
    bool If_float(string word, string& valid);

    friend istream&
        operator>>(istream& is, interpreter& in);
    friend ostream&
        operator<<(ostream& os, const interpreter& in);
};

#endif