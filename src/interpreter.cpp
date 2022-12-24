#include "interpreter.h"

using namespace std;

interpreter::interpreter() {}
interpreter::~interpreter() {}

ostream& operator<<(ostream& os, const interpreter& in)
{
	os << in.s;
	return os;
}

istream& operator>>(istream& is, interpreter& in)
{
    string temp;
	getline(is, temp, ';');
    for (int i = 0; i < temp.length(); i++)
	{
		if (temp[i] == '\n'||temp[i]=='\r'||temp[i]=='\t')
			temp[i] = ' ';
	}
    in.s=temp;
	return is;
}

bool interpreter::querydecode()
{
	API api;
	string word;
	string_proc << s;
	string_proc >> word;
	try {
		if(word == "create")
		{
			string_proc >> word;
			if(word == "table")
				CreateTable(api);
			else if(word == "index")
				CreateIndex(api);
		}
		else if(word == "drop")
		{
			string_proc >> word;
			if(word == "table")
				DropTable(api);
			else if(word == "index")
				DropIndex(api);
		}
		else if(word == "select")
			Select(api);
		else if(word == "insert")
			Insert(api);
		else if(word == "delete")
			DeleteData(api);
		else if(word == "quit")
		{
			cout << "Bye\n";
			return 1;
		}
		else if(word == "execfile")
			exec_file();
		else if(word == "save")
			saveblock();
		else
			cerr << "ERROR 12: SQL syntax Error!\n";
	} catch(int error_type) {
		switch (error_type)
		{
		case 1: cerr << "ERROR 1: create table syntax error, "
		<< "the bracket positions in your query are not correct\n"; break;
		case 2:	cerr << "ERROR 2: create table syntax error, "
		<< "you typed one more comma before the close bracket\n"; break;
		case 3:	cerr << "ERROR 3: create table syntax error, "
		<< "the extra feature of the attribute is not keyword \"unique\"\n"; break;
		case 4:	cerr << "ERROR 4: delete syntax error, "
		<< "the keyword \"where\" is not correct\n"; break;
		case 5:	cerr << "ERROR 5: delete syntax error, "
		<< "the conjunction between clauses is not the keyword \"and\"\n"; break;
		case 6:	cerr << "ERROR 6: create index syntax error, "
		<< "the keyword create index \"on\" is not correct\n"; break;
		case 7:	cerr << "ERROR 7: create index syntax error, "
		<< "the bracket positions in your query are not correct\n"; break;
		case 8:	cerr << "ERROR 8: select syntax error, "
		<< "the keyword \"from\" is not correct\n"; break;
		case 9:	cerr << "ERROR 9: select syntax error, "
		<< "the keyword \"where\" is not correct\n"; break;
		case 10: cerr << "ERROR 10: select syntax error, "
		<< "the conjunction between clauses is not the keyword \"and\"\n"; break;
		case 11: cerr << "ERROR 11: Failed to open file!\n"; break;
		case 13: cerr << "ERROR 13: The values you are inserting are illegal!\n"; break;
		case 14: cerr << "ERROR 14: Table you typed in doesn't exist!\n"; break;
		}
	}
	string_proc.str("");
	string_proc.clear();
	return 0;
}

void interpreter::Insert(API &api)
{
	string table_name, word;
	bool res;
	int pos, end;
	vector<int>uni_no;

	string_proc >> table_name;
	string_proc >> table_name;
	try {
		api.get_table(table_name);
		pos=static_cast<int>(s.find('('));
		s=s.substr(pos+1);	
		for(int i=0; i < current_table.attribute_num; i++)
		{
			if(current_table.attribute[i].state == 1)
				uni_no.push_back(i);
			switch (current_table.attribute[i].type)
			{
			case 0:
				res=If_int(s, raw_data[i].name);
				break;

			case 1:
				res=If_float(s, raw_data[i].name);
				break;

			case 2:
				res=If_char(s, current_table.attribute[i].type_size, raw_data[i].name);
				break;
			}
			if(!res)
				break;

			pos=static_cast<int>(s.find(','));
			end=static_cast<int>(s.size());
			if(pos == -1 || pos > end)
				break;
			s=s.substr(pos+1);
		}
		if(res == -1)
			throw 13;
		api.insert(uni_no);
	} catch(int error_type) {
		throw error_type;
	}
}

bool interpreter::If_int(string word, string& valid)
{
	int i=0;
	while(word[i] == ' ')
		word=word.substr(1);

	int pos;
	pos=static_cast<int>(word.find(','));
	if(pos == -1)
		pos=static_cast<int>(word.find(')'));
	word=word.substr(0, pos);
	for(int i=0; i<word.size(); i++)
	{
		if(!(word[i]>='0' && word[i]<='9'))
			return false;
	}
	valid=word;
	return true;
}

bool interpreter::If_char(string word, int char_size, string& valid)
{
	int i=0;
	int pos1, pos2;
	while(word[i] == ' ')
		word=word.substr(1);

	if(word[0] == '\'')
	{
		word=word.substr(1);
		pos1=static_cast<int>(word.find('\''));
	}
	else if(word[0] == '\"')
	{
		word=word.substr(1);
		pos1=static_cast<int>(word.find('\"'));
	}else return false;
	
	pos2=static_cast<int>(word.find(','));
	if(pos1 > pos2 && !(pos2 == -1))
		return false;
	else
	{
		word=word.substr(0, pos1);
		if(word.size() > char_size)
			return false;
		else
		{
			valid=word;
			return true;
		}
	}
}

bool interpreter::If_float(string word, string& valid)
{
	int i=0;
	while(word[i] == ' ')
		word=word.substr(1);

	int pos, dec_point_cnt=0;
	pos=static_cast<int>(word.find(','));
	if(pos == -1)
		pos=static_cast<int>(word.find(')'));
	word=word.substr(0, pos);
	for(int i=0; i<word.size(); i++)
	{
		if(word[i] == '.')
			dec_point_cnt++;
		if(!(word[i]>='0' && word[i]<='9' || word[i] == '.') || dec_point_cnt > 1)
			return false;
	}
	if(!dec_point_cnt)
		return false;

	valid = word;
	return true;	
}


void interpreter::CreateTable(API &api)
{
	int i, OpenBracketPos, CloseBracketPos,char_size;
	string table_struct;
	string tablename;
	string temp,temp1;
	string primary_key_name("");
	int num_of_col=0;
	s = s.substr(13, s.length() - 13);
	stringstream ss(s);
	ss >> tablename;
	for (i = 0; i < s.length(); i++)
	{
		if (s[i] == '(')
		{
			OpenBracketPos = i;
			break;
		}
	}
	for (i = s.length()-1; i > OpenBracketPos; i--)
	{
		if (s[i] == ')')
		{
			CloseBracketPos = i;
			break;
		}
	}
	if (OpenBracketPos >= CloseBracketPos)
	{
		throw 1;
	}
	s = s.substr(OpenBracketPos+1, CloseBracketPos- OpenBracketPos-1);
	if (s[s.length() - 1] == ',' || s[s.length() - 2] == ',')
	{
		throw 2;
	}
		else
	s = s + ',';
	ss.str(s);
	while (getline(ss, table_struct, ','))
	{
		stringstream ss1(table_struct);
		ss1 >> temp >> temp1;
		if (temp == "primary"&& temp1 == "key")
		{
			for (i = 0; i < table_struct.length(); i++)
			{
				if (table_struct[i] == '(')
					OpenBracketPos = i;
				else if (table_struct[i] == ')')
					CloseBracketPos = i;
			}
			table_struct = table_struct.substr(OpenBracketPos+1, CloseBracketPos - OpenBracketPos-1);
			ss1.str(table_struct);
			ss1>> primary_key_name;
		}
		else
		{
			raw_data[num_of_col].name = temp;
			if (temp1 == "int")
			{
				raw_data[num_of_col].type = 0;
				raw_data[num_of_col].type_size = -1;
			}
			else if (temp1 == "float")
			{
				raw_data[num_of_col].type = 1;
				raw_data[num_of_col].type_size = -1;
			}
			else if (temp1.substr(0, 4) == "char")
			{
				raw_data[num_of_col].type = 2;
				temp1 = temp1.substr(5, temp1.length() - 6);
				char_size = stoi(temp1);
				if (char_size > 255)
				{
					cout << "char size is too large, set to 255"<<endl;
					char_size = 255;
				}
				else if (char_size <= 0)
				{
					cout << "char size is too small, set to 1" << endl;
					char_size = 1;
				}
				raw_data[num_of_col].type_size = char_size;
			}
			if (ss1 >> temp)
				if (temp == "unique")
					raw_data[num_of_col].state = 1;
				else
					throw 3;
			else
				raw_data[num_of_col].state = 0;
			num_of_col++;
		}
		
	}
	api.create_table(tablename, num_of_col, primary_key_name);
}

void interpreter::DropTable(API &api)
{
	string table_name;
	s = s.substr(11, s.length() - 11);
	stringstream ss(s);
	ss >> table_name;
	api.drop_table(table_name);
}

void interpreter::DeleteData(API &api)
{
	s = s.substr(12, s.length() - 12);
	stringstream ss(s);
	string table_name;
	string op1,op2,op3,temp1;
	condition temp;
	vector<condition> conditions;
	ss >> table_name;
	ss >> temp1;
	if (temp1 != "where")
	{
		throw 4;
	}
	while (ss >> temp.entity>>temp.op>>temp.restrict)
	{
		conditions.push_back(temp);
		if (!(ss >> temp1))
			break;
		else
		{
			if (temp1 != "and")
			{
				throw 5;
			}
		}
		
	}
	where wc(conditions);
	try {
		api.get_table(table_name);
		api.delete_data(wc);
	} catch(int error_type) {
		throw error_type;
	}
}

void interpreter::CreateIndex(API &api)
{
	s = s.substr(13, s.length() - 13);
	stringstream ss(s);
	string index_name;
	string index_col;
	string temp;
	string table_name;
	ss >> index_name;
	ss >> temp;
	if (temp != "on")
		throw 6;

	ss >> table_name;
	int OpenBacketPos=0,CloseBracketPos=0,i;
	for (i = 0; i < s.length(); i++)
	{
		if (s[i] == '(')
			OpenBacketPos = i;
		else if (s[i] == ')')
			CloseBracketPos = i;
	}
	if (OpenBacketPos >= CloseBracketPos)
	{
		throw 7;
		return;
	}
	s = s.substr(OpenBacketPos + 1, CloseBracketPos - OpenBacketPos - 1);
	ss.str(s);
	ss >> index_col;
	try {
		api.get_table(table_name);
		api.create_index(table_name, index_col, index_name);
	} catch(int error_type) {
		throw error_type;
	}
}

void interpreter::DropIndex(API &api)
{
	string index_name, table_name;
	string temp;
	s = s.substr(11, s.length() - 11);
	stringstream ss(s);
	ss >> index_name;
	ss >> temp;
	if (temp != "on")
		throw 15;
	
	ss >> table_name;
	api.drop_index(table_name, index_name);
}

void interpreter::Select(API &api)
{
	int pos, i, j=0, flag=0;
	s = s.substr(7, s.length() - 7);
	stringstream ss(s);
	string table_name;
	string temp1;
	condition temp;
	vector<condition> conditions;
	vector<string> ShowEntities;
	vector<int>attri_proj;

	if (s[0] == '*')
	{
		flag = 1;
		ss >> temp1;
		ShowEntities.push_back(temp1);
		ss >> temp1;
		if (temp1 != "from")
		{
			throw 8;
		}
	}
	else
	{
		while (ss >> temp1)
		{
			if (temp1 == "from")
				break;
			if( temp1[temp1.size() - 1] == ',' )
				temp1 = temp1.substr(0, temp1.length() - 1);
			else
				temp1 = temp1.substr(0, temp1.length());
			ShowEntities.push_back(temp1);
		}
	}
	ss >> temp1;
	table_name = temp1;
	if (ss >> temp1)
	{
		if (temp1 != "where")
		{
			throw 9;
		}
		while (ss >> temp.entity >> temp.op >> temp.restrict)
		{
			conditions.push_back(temp);
			if (!(ss >> temp1))
				break;
			if (temp1 != "and")
			{
				throw 10;
			}
		}
	}
	where wc(conditions);
	try {
		api.get_table(table_name);

		if(flag)
			for(int i = 0; i < current_table.attribute_num; i++)
				attri_proj.push_back(i);
		else
		{
			i = ShowEntities.size();
			while(1)
			{
				if(j == i)
					break;

				for(int i = 0; i < current_table.attribute_num; i++)
				{
					if(current_table.attribute[i].name == ShowEntities[j])
					{
						attri_proj.push_back(i);
						j++;
						break;
					}
				}
			}
		}
		api.select(attri_proj, wc);
	} catch(int error_type) {
		throw error_type;
	}
}

void interpreter::exec_file()
{
	string file;
	string_proc >> file;
	ifstream in(file);
	if(!in.is_open())
		throw 11;
	interpreter run_file;
	while(in >> run_file)
	{
		while(run_file.s[0] == ' ')
			run_file.s = run_file.s.substr(1);
		run_file.querydecode();
	}
	cout << "File execution succeeded!\n";
}

void interpreter::saveblock()
{
	for (int i = 0; i < maxBlocks; i++)
		bf.writetoFile(i);
}