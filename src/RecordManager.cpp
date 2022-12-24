#include "RecordManager.h"

where::where(vector<condition>& prerequisite) {
	cond t;
	cnumber = prerequisite.size();
	for (int i = 0, pos; i < prerequisite.size(); i++)
	{
		t.entity = prerequisite[i].entity;
		switch (prerequisite[i].op[0])
		{
		case '=': t.flag = eq; break;

		case '<':
			switch (prerequisite[i].op[1])
			{
			case '>': t.flag = neq; break;
			case '=': t.flag = leq; break;
			default: t.flag = l; break;
			}
			break;

		case '>':
			switch (prerequisite[i].op[1])
			{
			case '=': t.flag = geq; break;
			default: t.flag = g; break;
			}
			break;
		default:
			cout << "Error in RecordManager in function is satisified!" << endl;
			break;
		}

		if (prerequisite[i].restrict[0] == '\'')
		{
			pos = prerequisite[i].restrict.find('\'', 2);
			t.dataw = new Datac(prerequisite[i].restrict.substr(1, pos-1));
		}
		else
		{
			if (prerequisite[i].restrict.find('.') != -1)
				t.dataw = new Dataf(stof(prerequisite[i].restrict));
			else
				t.dataw = new Datai(stoi(prerequisite[i].restrict));
		}
		conditions.push_back(t);
	}
}

where::~where()
{
	for(int i=0; i<conditions.size(); i++)
		delete conditions[i].dataw;
}

void RecordManager::create_table()
{
    string file = "Table_" + current_table.table_name + "_tuple";
    ofstream out(file);
	out.close();
	current_table.blockNum = 1;
	bf.changeblock();
}

void RecordManager::insert(vector<int>& uni_no)
{
	if(!Search_with_Index() || !Search_without_Index(uni_no))
	{
		cout << "Found same unique value! Insertion failed!\n";// should be throw here
		return;
	}
	char* begin;
	int pos = 0;//当前的插入位置
	int ti;
	int i, j;
	float tj;

	begin = new char[current_table.length];
    for(int i=0; i < current_table.attribute_num; i++)
    {
        switch (current_table.attribute[i].type)
        {
        case 0:
            ti = stoi(raw_data[i].name);
            memcpy(begin + pos, &ti, sizeof(int));
			pos += sizeof(int);
            break;
        
        case 1:
            tj = stof(raw_data[i].name);
            memcpy(begin + pos, &tj, sizeof(float));
			pos += sizeof(float);
            break;

        case 2:
            memcpy(begin + pos, raw_data[i].name.c_str(), current_table.attribute[i].type_size);
			pos += current_table.attribute[i].type_size;
			begin[pos++] = '\0';
            break;
        }
    }

    bufferBlock iPos = bf.find_Insertable_pos();//获取插入位置

	for (i = 0; i < current_table.attribute_num; i++)
	{
		if (current_table.attribute[i].state == 2)
		{
			/* find the alias */
			for(j = 0; j < current_table.index.size(); j++)
				if(current_table.index[j].attri_no == i)
					break;

			IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[i].type);
			Data* temp;
			switch (current_table.attribute[i].type)
			{
				case 0: temp = new Datai(stoi(raw_data[i].name)); break;
				case 1: temp = new Dataf(stof(raw_data[i].name)); break;
				case 2: temp = new Datac(raw_data[i].name); break;
			}
			Index_m.Insert(temp, iPos.pos_in_block);
			delete temp;
		}
	}

	blocks[iPos.blockpos].vals[iPos.pos_in_block] = notEmpty;
	memcpy(&(blocks[iPos.blockpos].vals[iPos.pos_in_block + 1]), begin, current_table.length);

    bf.write(iPos.blockpos);
	delete[] begin;
}

void RecordManager::select(vector<int>& attri_proj, where& wc)
{
	 Table out_table(current_table);
	 Tuple temp;
	 string stringRow;
	 string file = "Table_" + current_table.table_name + "_tuple";
	 const int recordNum = blockSize / (current_table.length + 1);
	 int ti;
	 float tj;
	 char tk[maxStringlength];
	 int index[MAX_ATTRIBUTE_NUM], j=0;
	 int inPos = -2;
	 int No;

	if(wc.cnumber)
	{
		memset(index, -1, sizeof(index));
		while(1)
		{
			if(j == wc.cnumber)
				break;

			for(int i = 0; i < current_table.attribute_num; i++)
			{
				if(current_table.attribute[i].name == wc.conditions[j].entity)
				{
					index[j++] = i;
					break;
				}
			}
		}
	}

	for (int i = 0; i < wc.cnumber; i++)
	{
		if (wc.conditions[i].flag == eq)
		{
			for (int j = 0; j < current_table.index.size(); j++)
			{
				if (current_table.index[j].attri_no == index[i])
				{
					int k = current_table.index[j].attri_no;
					IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[k].type);
					Data* temp2 =  wc.conditions[i].dataw;
					inPos = Index_m.Find(temp2);
					break;
				}
			}
			if (inPos != -2) {
				break;
			}
		}

	}// inPos != -2 used index, inPos == -2 index is not used
	if (inPos != -2) {
		if(inPos != -1)
		{
			for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
			{
				No = bf.getaBlock(file, blockOffset);
				char *pdata = blocks[No].vals + inPos;
				int c_pos = 1;
				for (int attr_index = 0; attr_index < current_table.attribute_num; attr_index++)
				{
					switch (current_table.attribute[attr_index].type)
					{
					case 0:
						memcpy(&ti, &(pdata[c_pos]), sizeof(int));
						c_pos += sizeof(int);
						temp.push_back(new Datai(ti));
						break;
					case 1:
						memcpy(&tj, &(pdata[c_pos]), sizeof(float));	
						c_pos += sizeof(float);
						temp.push_back(new Dataf(tj));
						break;
					case 2:
						int strLen = current_table.attribute[attr_index].type_size + 1;
						memcpy(tk, &(pdata[c_pos]), strLen);
						c_pos += strLen;
						temp.push_back(new Datac(tk));
						break;
					}
				}
				if (tuplesatisfied(temp, index, wc))
				{
					out_table.tuples.push_back(temp);
					display(out_table, attri_proj);
					return;
				}
				for (int i = 0; i < temp.size(); i++)
				switch (current_table.attribute[i].type)
				{
					case 0: delete (Datai*)temp[i]; break;
					case 1: delete (Dataf*)temp[i]; break;
					case 2: delete (Datac*)temp[i]; break;
				}
				temp.clear();
			}
		}
		display(out_table, attri_proj);
		return;
	}

	 for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
	 {
		No = bf.getaBlock(file, blockOffset);
		 for (register int offset = 0; offset < recordNum; offset++)
		 {
			 int position = offset * (current_table.length + 1);
			 stringRow = blocks[No].getvals(position, position + (current_table.length + 1));
			 if (stringRow[0] == Empty)
				 continue;//该行是空的
			 int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			 for (int attr_index = 0; attr_index < current_table.attribute_num; attr_index++)
			 {
				 switch (current_table.attribute[attr_index].type)
				 {
				 case 0:
					 memcpy(&ti, &(stringRow.c_str()[c_pos]), sizeof(int));
					 c_pos += sizeof(int);
					 temp.push_back(new Datai(ti));
					 break;
				 case 1:
					 memcpy(&tj, &(stringRow.c_str()[c_pos]), sizeof(float));
					 c_pos += sizeof(float);
					 temp.push_back(new Dataf(tj));
					 break;
				 case 2:
					 int strLen = current_table.attribute[attr_index].type_size + 1;
					 memcpy(tk, &(stringRow.c_str()[c_pos]), strLen);
					 c_pos += strLen;
					 temp.push_back(new Datac(tk));
					 break;
				 }
			 }

			if (!wc.cnumber || tuplesatisfied(temp, index, wc))
				out_table.tuples.push_back(temp);
			else for (int i = 0; i < temp.size(); i++)
				switch (current_table.attribute[i].type)
				{
					case 0: delete (Datai*)temp[i]; break;
					case 1: delete (Dataf*)temp[i]; break;
					case 2: delete (Datac*)temp[i]; break;
				}
			temp.clear();
		 }
	 }
	display(out_table, attri_proj);
}

void RecordManager::display(Table& out_table, vector<int>& attri_proj)
{
	int count=0;
	for(int i=0; i< out_table.attribute_num; i++)
		out_table.attribute[i].is_display = 0;
	for (int i = 0; i < attri_proj.size(); i++) {
		out_table.attribute[attri_proj[i]].is_display = 1;
		count++;
	}

	if(out_table.tuples.size())
	{
		cout << '+';
		for (int i = 0; i < count; i++) {
			for (int j = 0; j < 13; j++) cout << '-';
			cout << '+';
		}
		cout << endl;

		for (int i = 0; i < attri_proj.size(); i++) {
			cout << "| " << setiosflags(ios::left) << setw(12) << out_table.attribute[attri_proj[i]].name;
		}
		cout << "|" << endl;
		
		cout << '+';
		for (int i = 0; i < count; i++) {
			for (int j = 0; j < 13; j++) cout << '-';
			cout << '+';
		}
		cout << endl;

		for (int j = 0; j < out_table.tuples.size(); j++) {
			for (int i = 0; i < out_table.attribute_num; i++) 
				if (out_table.attribute[i].is_display == true)
					switch (out_table.tuples[j][i]->type)  {
						case 0: cout << "| " << setiosflags(ios::left)<< setw(12) << ((Datai*)out_table.tuples[j][i])->x; break;
						case 1: cout << "| " << setiosflags(ios::left) << setw(12) << ((Dataf*)out_table.tuples[j][i])->x; break;
						case 2: cout << "| " << setiosflags(ios::left) << setw(12) << ((Datac*)out_table.tuples[j][i])->x; break;
						default: break;
					}	
			cout <<"| "<< endl;
		}

		cout << '+';
		for (int i = 0; i < count; i++) {
			for (int j = 0; j < 13; j++) cout << '-';
			cout << '+';
		}
		cout << endl;
	}
	else
		cout << "Empty set\n";

	for (int j = 0; j < out_table.tuples.size(); j++) {
		for (int i = 0; i < out_table.attribute_num; i++)
			switch (out_table.attribute[i].type)  {
				case 0: delete (Datai*)out_table.tuples[j][i]; break;
				case 1: delete (Dataf*)out_table.tuples[j][i]; break;
				case 2: delete (Datac*)out_table.tuples[j][i]; break;
			}
	}
}

bool RecordManager::tuplesatisfied(Tuple& temp, int(&index)[MAX_ATTRIBUTE_NUM], where& wc)
{
	for (int i = 0; i < wc.cnumber;i++) {
		if (wc.conditions[i].dataw == NULL)  continue;    //  后面没有where的条件描述

		// !! 要根据tuple的rownumber遍历一列的数据，但是rownumber在tuple结构里不知道怎么正确地遍历

		switch(current_table.attribute[index[i]].type) {   
			case 0:
			switch (wc.conditions[i].flag) {
				case eq: if (!(((Datai*)temp[index[i]])->x == ((Datai*)wc.conditions[i].dataw)->x)) return false;break;
				case leq: if (!(((Datai*)temp[index[i]])->x <= ((Datai*)wc.conditions[i].dataw)->x)) return false; break;
				case l: if (!(((Datai*)temp[index[i]])->x < ((Datai*)wc.conditions[i].dataw)->x)) return false; break;
				case geq: if (!(((Datai*)temp[index[i]])->x >= ((Datai*)wc.conditions[i].dataw)->x)) return false; break;
				case g: if (!(((Datai*)temp[index[i]])->x > ((Datai*)wc.conditions[i].dataw)->x)) return false; break;
				case neq: if (!(((Datai*)temp[index[i]])->x != ((Datai*)wc.conditions[i].dataw)->x)) return false; break;
				default: break;
			}
			break;
		
			case 1:
			switch (wc.conditions[i].flag) {
				case eq: if (!(abs(((Dataf*)temp[index[i]])->x - ((Dataf*)wc.conditions[i].dataw)->x) < MIN_THETA)) return false; break;
				case leq: if (!(((Dataf*)temp[index[i]])->x <= ((Dataf*)wc.conditions[i].dataw)->x)) return false; break;
				case l: if (!(((Dataf*)temp[index[i]])->x < ((Dataf*)wc.conditions[i].dataw)->x)) return false; break;
				case geq: if (!(((Dataf*)temp[index[i]])->x >= ((Dataf*)wc.conditions[i].dataw)->x)) return false; break;
				case g: if (!(((Dataf*)temp[index[i]])->x > ((Dataf*)wc.conditions[i].dataw)->x)) return false; break;
				case neq: if (!(((Dataf*)temp[index[i]])->x != ((Dataf*)wc.conditions[i].dataw)->x)) return false; break;
				default: break;
			}
			break;

			case 2:
			switch (wc.conditions[i].flag) {
			case eq: if (!(((Datac*)temp[index[i]])->x == ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			case leq: if (!(((Datac*)temp[index[i]])->x <= ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			case l: if (!(((Datac*)temp[index[i]])->x < ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			case geq: if (!(((Datac*)temp[index[i]])->x >= ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			case g: if (!(((Datac*)temp[index[i]])->x > ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			case neq: if (!(((Datac*)temp[index[i]])->x != ((Datac*)wc.conditions[i].dataw)->x)) return false; break;
			default: break;
			}
			break;
		}
	}
	return true;
}

void RecordManager::Delete(where& wc)
{
	 Tuple temp;
	 string stringRow;
	 string file = "Table_" + current_table.table_name + "_tuple";
	 const int recordNum = blockSize / (current_table.length + 1);
	 int ti;
	 float tj;
	 char tk[maxStringlength];
	 int index[MAX_ATTRIBUTE_NUM], j=0;
	 int inPos = -2, No;

	memset(index, -1, sizeof(index));
	while(1)
	{
		if(j == wc.cnumber)
			break;

		for(int i = 0; i < current_table.attribute_num; i++)
		{
			if(current_table.attribute[i].name == wc.conditions[j].entity)
			{
				index[j++] = i;
				break;
			}
		}
	}

	for (int i = 0; i < wc.cnumber; i++)
	{
		if (wc.conditions[i].flag == eq)
		{
			for (int j = 0; j < current_table.index.size(); j++)
			{
				if (current_table.index[j].attri_no == index[i])
				{
					int k = current_table.index[j].attri_no;
					IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[k].type);
					Data* temp2 =  wc.conditions[i].dataw;
					inPos = Index_m.Find(temp2);
					break;
				}
			}
			if (inPos != -2) {
				break;
			}
		}

	}// inPos != -2 used index, inPos == -2 index is not used
	if (inPos != -2) {
		if(inPos != -1)
		{
			for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
			{
				No = bf.getaBlock(file, blockOffset);
				char *pdata = blocks[No].vals + inPos;
				int c_pos = 1;
				for (int attr_index = 0; attr_index < current_table.attribute_num; attr_index++)
				{
					switch (current_table.attribute[attr_index].type)
					{
					case 0:
						memcpy(&ti, &(pdata[c_pos]), sizeof(int));
						c_pos += sizeof(int);
						temp.push_back(new Datai(ti));
						break;
					case 1:
						memcpy(&tj, &(pdata[c_pos]), sizeof(float));	
						c_pos += sizeof(float);
						temp.push_back(new Dataf(tj));
						break;
					case 2:
						int strLen = current_table.attribute[attr_index].type_size + 1;
						memcpy(tk, &(pdata[c_pos]), strLen);
						c_pos += strLen;
						temp.push_back(new Datac(tk));
						break;
					}
				}
				if (tuplesatisfied(temp, index, wc))
				{
					pdata[0] = Deleted;
					for (int i = 0; i < current_table.attribute_num; i++)
					{
						if (current_table.attribute[i].state == 2)
						{
							/* find the alias */
							for(j = 0; j < current_table.index.size(); j++)
								if(current_table.index[j].attri_no == i)
									break;

							IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[i].type);
							Index_m.Delete(temp[i]);
						}
					}
					for (int i = 0; i < temp.size(); i++)
						switch (current_table.attribute[i].type)
						{
							case 0: delete (Datai*)temp[i]; break;
							case 1: delete (Dataf*)temp[i]; break;
							case 2: delete (Datac*)temp[i]; break;
						}
					return;
				}
				for (int i = 0; i < temp.size(); i++)
					switch (current_table.attribute[i].type)
					{
						case 0: delete (Datai*)temp[i]; break;
						case 1: delete (Dataf*)temp[i]; break;
						case 2: delete (Datac*)temp[i]; break;
					}
				temp.clear();
			}
		}
	}

	for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
	{
		No = bf.getaBlock(file, blockOffset);
		for (register int offset = 0; offset < recordNum; offset++)
		{
			int position = offset * (current_table.length + 1);
			stringRow = blocks[No].getvals(position, position + (current_table.length + 1));
			if (stringRow[0] == Empty)
				continue;//该行是空的
			int c_pos = 1;//当前在数据流中指针的位置，0表示该位是否有效，因此数据从第一位开始
			for (int attr_index = 0; attr_index < current_table.attribute_num; attr_index++)
			{
				switch (current_table.attribute[attr_index].type)
				{
					case 0:
					memcpy(&ti, &(stringRow.c_str()[c_pos]), sizeof(int));
					c_pos += sizeof(int);
					temp.push_back(new Datai(ti));
					break;
					case 1:
					memcpy(&tj, &(stringRow.c_str()[c_pos]), sizeof(float));
					c_pos += sizeof(float);
					temp.push_back(new Dataf(tj));
					break;
					case 2:
					int strLen = current_table.attribute[attr_index].type_size + 1;
					memcpy(tk, &(stringRow.c_str()[c_pos]), strLen);
					c_pos += strLen;
					temp.push_back(new Datac(tk));
					break;
				}
			}

			if(tuplesatisfied(temp, index, wc))
			{
				blocks[No].vals[position] = Deleted;
				for (int i = 0; i < current_table.attribute_num; i++)
				{
					if (current_table.attribute[i].state == 2)
					{
						/* find the alias */
						for(j = 0; j < current_table.index.size(); j++)
							if(current_table.index[j].attri_no == i)
								break;

						IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[i].type);
						Index_m.Delete(temp[i]);
					}
				}
			}

			for (int i = 0; i < temp.size(); i++)
				switch (current_table.attribute[i].type)
				{
					case 0: delete (Datai*)temp[i]; break;
					case 1: delete (Dataf*)temp[i]; break;
					case 2: delete (Datac*)temp[i]; break;
				}
			temp.clear();
		 }
		 bf.write(No);
	 }
}

void RecordManager::DropTable(string table_name)
{
	table_name = "Table_" + table_name + "_tuple";
	if (remove(table_name.c_str()) != 0)
		cout << "Record drop failed\n";
	else
	{
		bf.invalid(table_name);
		cout << "Record drop succeeded\n";
	}
}

bool RecordManager::Search_without_Index(vector<int>& uni_no)
{
	 string file = "Table_" + current_table.table_name + "_tuple";
	 const int recordNum = blockSize / (current_table.length + 1);
	 int ti;
	 float tj;
	 char tk[maxStringlength];
	 int position;
	 int (&uni_offset)[10] = current_table.unique_attri_offset;

     for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
	 {
        int No = bf.getaBlock(file, blockOffset);
        for (register int offset = 0; offset < recordNum;offset++)
		{
			position = offset * (current_table.length + 1);
			if (blocks[No].vals[position] == Empty)
				continue;
			for(int u=0; uni_offset[u] != -1; u++)
			{
				switch (current_table.attribute[uni_no[u]].type)
				{
				case 0:
					memcpy(&ti, &(blocks[No].vals[position+uni_offset[u]]), sizeof(int));
					if(ti == stoi(raw_data[uni_no[u]].name))
						return false;
					break;
				case 1:
					memcpy(&tj, &(blocks[No].vals[position+uni_offset[u]]), sizeof(float));
					if(tj == stof(raw_data[uni_no[u]].name))
						return false;
					break;
				case 2:
					memcpy(tk, &(blocks[No].vals[position+uni_offset[u]]), current_table.attribute[uni_no[u]].type_size+1);
					if(string(tk) == raw_data[uni_no[u]].name)
						return false;
					break;
				}
			}
        }
    }
	return true;
}

bool RecordManager::Search_with_Index()
{
	int i, j;
	for (i = 0; i < current_table.attribute_num; i++)
	{
		if (current_table.attribute[i].state == 2)
		{
			/* find the alias */
			for(j = 0; j < current_table.index.size(); j++)
				if(current_table.index[j].attri_no == i)
					break;

			IndexManager Index_m(current_table.table_name + '_' + current_table.index[j].index_name, current_table.attribute[i].type);
			Data* temp;
			switch (current_table.attribute[i].type)
			{
				case 0: temp = new Datai(stoi(raw_data[i].name)); break;
				case 1: temp = new Dataf(stof(raw_data[i].name)); break;
				case 2: temp = new Datac(raw_data[i].name); break;
			}
			int addr = Index_m.Find(temp);
			delete temp;
			if (addr != -1)
				return false;
		}
	}
	return true;
}

void RecordManager::CreateIndex(string table_name, int attri_no, string index_name)
{
	int i;
	string file = "Table_" + current_table.table_name + "_tuple";
	const int recordNum = blockSize / (current_table.length + 1);
	int ti;
	float tj;
	char tk[maxStringlength];
	int position;
	int index_attri, size = 1;
	Data* temp;
	IndexManager Index_m(table_name + '_' + index_name, current_table.attribute[attri_no].type);
	Index_m.CreateIndex();

	for(i=0; i < current_table.attribute_num; i++)
    {
        if(i == attri_no)
		{
            index_attri = size;
			break;
		}
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

     for (register int blockOffset = 0; blockOffset < current_table.blockNum; blockOffset++)
	 {
        int No = bf.getaBlock(file, blockOffset);
        for (register int offset = 0; offset < recordNum;offset++)
		{
			position = offset * (current_table.length + 1);
			if (blocks[No].vals[position] == Empty)
				continue;
			switch (current_table.attribute[i].type)
			{
			case 0:
				memcpy(&ti, &(blocks[No].vals[position+index_attri]), sizeof(int));
				temp = new Datai(ti);
				Index_m.Insert(temp, position);
				break;
			case 1:
				memcpy(&tj, &(blocks[No].vals[position+index_attri]), sizeof(float));
				temp = new Dataf(tj);
				Index_m.Insert(temp, position);
				break;
			case 2:
				memcpy(tk, &(blocks[No].vals[position+index_attri]), current_table.attribute[i].type_size+1);
				temp = new Datac(tk);
				Index_m.Insert(temp, position);
				break;
			}
			delete temp;
        }
    }
}