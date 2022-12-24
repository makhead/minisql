#include <fstream>
#include "interpreter.h"
using namespace std;

extern struct table_entity_attribute raw_data[MAX_ATTRIBUTE_NUM];
extern struct Table current_table;
extern class buffermanager bf;
extern class bufferBlock blocks[maxBlocks];

int main()
{
    interpreter inpt;
    bool t;
    char c;
    cout << "minisql> ";
    while(1)
    {
        cin >> inpt;
        t = inpt.querydecode();
        if(t)
            break;
        else
            cin.ignore(1);
    }
    bf.~buffermanager();
}