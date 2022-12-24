#include "Bplus.h"
using namespace std;

void BpTree::insert(Data* key, int offset, string table_name)
{
	//===================================================================================
	//init
	int pos = 0;
	Bpnode curnode;//get root node info
	curnode.init(col_type,maxchildren);
	curnode.getNode(rootpos,table_name,col_type,maxchildren);
	int nextnode = rootpos;
	//======================================================================================
	//find correct node
	while (curnode.state!=Leaf&&numofnode!=1)
	{
		int isbreak = 0;
		for (int i = 0; i < curnode.nowChildren-1; i++)
		{
			isbreak = 0;
			if (col_type == 0)
			{
				if (((Datai*)key)->x < ((int*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}				
			}
			else if(col_type==1)
			{
				if (((Dataf*)key)->x < ((float*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else
			{
				string compare(((char*)curnode.data+i*256));
				if (((Datac*)key)->x < compare)
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
		}
		if(isbreak==0)
			nextnode = curnode.pointer[curnode.nowChildren-1];
		curnode.getNode(nextnode, table_name, col_type, maxchildren);
	}
	
	//found correct node
	int i;
	int curAddr = curnode.nodeAddr;
	int& maxchildrenAnd = maxchildren;
	int& numofnodeAnd = numofnode;
	int& rootposAnd = rootpos;
	if (curnode.nowChildren < maxchildren-1)
	{
		i = curnode.insert(key, offset, col_type);	
		curnode.saveNode(curnode.nodeAddr,col_type,maxchildren);//save
		curnode.freeNode(col_type);
	}
	else
	{
		curnode.freeNode(col_type);
		insertBack(curAddr, key, col_type, offset, maxchildrenAnd, numofnodeAnd, rootposAnd,table_name);
	}
}


void BpTree::create(int col_type_in, string table_name)
{
	string filename = "Table_" + table_name + "_bptree";
	int num = bf.getaBlock(filename,0);
	char *myblock = blocks[num].vals;
	maxchildren = (blockSize - HeaderOffset - 4 - 16) / (index_length[col_type_in] + 4);
	// maxchildren = 3; //debug
	numofnode = 1;
	rootpos = 0;
	col_type = col_type_in;
	*(int *)myblock = col_type;
	*(int *)(myblock + 4) = maxchildren;
	*(int *)(myblock + 8) = numofnode;
	*(int *)(myblock + 12) = rootpos;
	char save[256];
	strncpy(save, table_name.c_str(), 255);
	memcpy(myblock + 20, save, 256);
	bf.write(num);
	//========header========================================================================
	//==========root=====================================================================
	filename = "Table_" + table_name + "_index";
	num = bf.getaBlock(filename,0);
	myblock = blocks[num].vals;
	Bpnode root;
	root.state = Root;
	root.nowChildren = 0;
	root.blocknum = num;
	if (col_type == 0)
		root.data = (void*)(new int[maxchildren]);
	else if(col_type == 1)
		root.data = (void*)(new float[maxchildren]);
	else
		root.data = (void*)(new char[maxchildren*256]);
	root.pointer = new int[maxchildren+1];
	root.pointer[maxchildren] = -1;
	root.fatherAddr = -1;
	root.nodeAddr = 0;
	root.saveNode(0,col_type,maxchildren);
	root.freeNode(col_type);
	//================================================================================
}

void Bpnode::init(int col_type,int maxchildren)
{
	state = 2;   // root = 0 , internal = 1, leaf = 2
	nodeAddr = -1;
	nowChildren = 0;
	fatherAddr = -1;
	if (col_type == 0)
		data = new int[maxchildren];
	else if (col_type == 1)
		data = new float[maxchildren];
	else
		data = new char[maxchildren*256];
	pointer = new int[maxchildren + 1];
	pointer[maxchildren] = -1;
}

int Bpnode::insert(Data* key, int offset, int col_type)//leaf node insert
{
	int j;
	int isbreak = 0;
	if (nowChildren == 0)
	{
		string temp;
		if (col_type == 0)
			((int*)(data))[0] = ((Datai*)key)->x;
		else if(col_type==1)
			((float*)(data))[0] = ((Dataf*)key)->x;
		else
		{
			temp = ((Datac*)key)->x;
			strncpy(((char*)data+0*256), temp.c_str(), 256);
		}			
		pointer[0]=offset;
		nowChildren++;
		return 0;
	}
	for (j = nowChildren - 1; j >= 0; j--)
	{
		if (col_type == 0)
		{
			if (compare(key, data, col_type,j) == -1)
			{
				((int*)data)[j + 1] = ((int*)data)[j];
				pointer[j + 1] = pointer[j];
			}
			else
			{
				((int*)data)[j + 1] = ((Datai*)key)->x;
				pointer[j + 1] = offset;
				isbreak = 1;
				break;
			}
		}
		else if (col_type == 1)
		{
			if (compare(key,data,col_type,j)==-1)
			{
				((float*)data)[j + 1] = ((float*)data)[j];
				pointer[j + 1] = pointer[j];
			}
			else
			{
				((float*)data)[j + 1] = ((Dataf*)key)->x;
				pointer[j + 1] = offset;
				isbreak = 1;
				break;
			}
		}
		else
		{
			if(compare(key, data, col_type,j) == -1)
			{
				strncpy(((char*)data+(j+1)*256), ((char*)data+j*256), 255);
				pointer[j + 1] = pointer[j];
			}
			else
			{
				string temp = ((Datac*)key)->x;
				strncpy(((char*)data+ (j + 1) * 256), temp.c_str(), 255);
				pointer[j + 1] = offset;
				isbreak = 1;
				break;
			}
		}
		
	}
	if (isbreak == 0)
	{
		if (col_type == 0)
			((int*)data)[0] = ((Datai*)key)->x;
		else if (col_type == 1)
			((float*)data)[0] = ((Dataf*)key)->x;
		else
		{
			string temp = ((Datac*)key)->x;
			strncpy(((char*)data+0*256), temp.c_str(), 256);
		}
			
		pointer[0] = offset;
	}
	nowChildren++;
	return j;
}

int Bpnode::insert(void* key, int offset, int col_type)// internal insert
{
	int j;
	int isbreak = 0;
	if (nowChildren == 0)
	{
		pointer[0] = offset;	
		nowChildren++;
		return 0;
	}
	else if (nowChildren == 1)
	{
		
		if (col_type == 0)
			((int*)(data))[0] = *(int*)key;
		else if (col_type == 1)
			((float*)(data))[0] = *(float*)key;
		else
		{
			string temp = *(string*)key;
			strncpy(((char*)data+0*256),temp.c_str(),256);
		}
			
		pointer[1] = offset;
		
		nowChildren++;
		return 0;
	}
	for (j = nowChildren - 2; j >= 0; j--)
	{
		isbreak = 0;
		if (col_type == 0)
		{
			if (compare(key, &((int*)data)[j], col_type) == -1)
			{
				((int*)data)[j + 1] = ((int*)data)[j];
				pointer[j + 2] = pointer[j + 1];
			}
			else
			{
				((int*)data)[j + 1] = *(int*)key;
				pointer[j + 2] = offset;
				isbreak = 1;
				break;
			}
		}
		else if (col_type == 1)
		{
			if (compare(key, &((float*)data)[j], col_type) == -1)
			{
				((float*)data)[j + 1] = ((float*)data)[j];
				pointer[j + 2] = pointer[j + 1];
			}
			else
			{
				((float*)data)[j + 1] = *(float*)key;
				pointer[j + 2] = offset;
				isbreak = 1;
				break;
			}
		}
		else
		{
			string temp(((char*)data+j*256));
			if (*(string*)key < temp)
			{
				strncpy(((char*)data+(j+1)*256), ((char*)data+j*256), 256);
				pointer[j + 2] = pointer[j + 1];
			}
			else
			{
				strncpy(((char*)data + (j + 1) * 256), (*(string*)key).c_str(), 256);
				pointer[j + 2] = offset;
				isbreak = 1;
				break;
			}
		}

	}
	if (isbreak == 0)
	{
		if (col_type == 0)
			((int*)data)[0] = *(int*)key;
		else if (col_type == 1)
			((float*)data)[0] = *(float*)key;
		else
		{
			strncpy(((char*)data+0*256), (*(string*)key).c_str(), 256);
		}
			
		pointer[1] = offset;
	}
	nowChildren++;
	return j;

}


int BpTree::find(Data* key)
{
	string indexname = "Table_" + table_name + "_index";
	//======================================================================================
	//init
	Bpnode curnode;
	curnode.init(col_type,maxchildren);
	curnode.getNode(rootpos, table_name, col_type, maxchildren);
	int nextnode = curnode.nodeAddr;
	//======================================================================================
	while (curnode.state != Leaf && numofnode != 1)
	{
		int isbreak = 0;
		for (int i = 0; i < curnode.nowChildren -1; i++)
		{
			isbreak = 0;
			if (col_type == 0)
			{
				if (((Datai*)key)->x < ((int*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else if (col_type == 1)
			{
				if (((Dataf*)key)->x < ((float*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else
			{
				string temp(((char*)curnode.data+i*256));
				if (((Datac*)key)->x < temp)
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
		}
		if (isbreak==0)
			nextnode = curnode.pointer[curnode.nowChildren - 1];
		curnode.getNode(nextnode, table_name, col_type, maxchildren);
		isbreak = 0;
	}
	for (int i = 0; i < curnode.nowChildren; i++)
	{
		if (compare(key, curnode.data, col_type, i) == 0)
		{
			int save = curnode.pointer[i];
			curnode.freeNode(col_type);
			return save;
		}			
		else if (compare(key, curnode.data, col_type, i) < 0)
			break;
	}
	curnode.freeNode(col_type);
	return -1;
}

vector<int> BpTree::RangeFind(Data* key1, Data* key2)
{
	vector<int> result;
	//===============================================================================================
	string indexname = "Table_" + table_name + "_index";
	//====================================================================================
	//======================================================================================
	//init
	Bpnode curnode;
	curnode.init(col_type,maxchildren);
	curnode.getNode(rootpos, table_name, col_type, maxchildren);
	int nextnode = curnode.nodeAddr;
	//======================================================================================
	while (curnode.state != Leaf && numofnode != 1)
	{
		int isbreak = 0;
		for (int i = 0; i < curnode.nowChildren - 1; i++)
		{
			isbreak = 0;
			if (col_type == 0)
			{
				if (((Datai*)key1)->x < ((int*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else if (col_type == 1)
			{
				if (((Dataf*)key1)->x < ((float*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else
			{
				string temp(((char*)curnode.data+i*256));
				if (((Datac*)key1)->x < temp)
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
		}
		if (isbreak == 0)
			nextnode = curnode.pointer[curnode.nowChildren - 1];
		curnode.getNode(nextnode, table_name, col_type, maxchildren);
		isbreak = 0;
	}
	if (curnode.nowChildren == 0)
	{
		curnode.freeNode(col_type);
		return result;
	}
	while(1)
	{
		int i=0;
		if (compare(key2, curnode.data, col_type, i) <= 0)
			result.push_back(curnode.pointer[i]);
		else
			break;
		if (i == curnode.nowChildren - 1)
		{
			curnode.getNode(curnode.pointer[maxchildren], table_name, col_type, maxchildren);
			i = 0;
		}
		i++;
	}
	return result;
}


void BpTree::deletenode(Data* key)
{
	//====================================================================================
	//===========================================================================================
	//init
	int pos = 0;
	Bpnode curnode;//get root node info
	curnode.init(col_type,maxchildren);
	curnode.getNode(rootpos, table_name, col_type, maxchildren);
	int nextnode = rootpos;
	//======================================================================================
	//find correct node
	while (curnode.state != Leaf && numofnode != 1)
	{
		int isbreak = 0;
		for (int i = 0; i < curnode.nowChildren - 1; i++)
		{
			isbreak = 0;
			if (col_type == 0)
			{
				if (((Datai*)key)->x < ((int*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else if (col_type == 1)
			{
				if (((Dataf*)key)->x < ((float*)(curnode.data))[i])
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
			else
			{
				string temp(((char*)curnode.data+i*256));
				if (((Datac*)key)->x < temp)
				{
					nextnode = curnode.pointer[i];
					isbreak = 1;
					break;
				}
			}
		}
		if (isbreak == 0)
			nextnode = curnode.pointer[curnode.nowChildren - 1];
		curnode.getNode(nextnode, table_name, col_type, maxchildren);
		isbreak = 0;
	}
	//found correct node
	if (curnode.nowChildren > 1)
	{
		curnode.deleteRecord(key, col_type);
		curnode.saveNode(curnode.nodeAddr, col_type, maxchildren);
		curnode.freeNode(col_type);
	}
	else
	{
		int save = curnode.nodeAddr;
		curnode.freeNode(col_type);
		deleteBack(save,key,col_type,maxchildren, numofnode, rootpos,table_name);
	}
}

void Bpnode::deleteRecord(Data* key, int col_type)//delete leaf
{
	int i;
	for (i = 0; i < nowChildren; i++)
	{
		if (compare(key, data, col_type, i) == 0)
		{
			pointer[i] = -1;
			if (col_type == 0)
				((int*)data)[i] = 0;
			else if (col_type == 1)
				((float*)data)[i] = 0;
			else
				strncpy(((char*)data+i*256), "#",1);
			break;
		}
			
	}
	for (i; i < nowChildren-1; i++)
	{
		pointer[i] = pointer[i + 1];
		if (col_type == 0)
			((int*)data)[i] = ((int*)data)[i + 1];
		else if (col_type == 1)
			((float*)data)[i] = ((float*)data)[i + 1];
		else
			strncpy(((char*)data+i*256), ((char*)data+(i+1)*256), 256);
	}
	if (col_type == 0)
		((int*)data)[i] = 0;
	else if (col_type == 1)
		((float*)data)[i] = 0;
	else
		strncpy(((char*)data+i*256), "#", 1);
	nowChildren--;
	return;
}

void Bpnode::deleteRecord(void* key, int col_type)//delete internal
{
	int i=0;
	for (i = 0; i < nowChildren - 1; i++)
	{
		if (col_type == 0)
		{
			if (compare(key, &((int*)data)[0], col_type) < 0)
			{
				i--;
				break;
			}
			else if (compare(key, &((int*)data)[i], col_type) == 0)
			{
				pointer[i + 1] = -1;
				((int*)data)[i] = 0;
				break;
			}
		}
		else if (col_type == 1)
		{
			if (compare(key, &((float*)data)[0], col_type) < 0)
			{
				i--;
				break;
			}
			else if (compare(key, &((float*)data)[i], col_type) == 0)
			{
				pointer[i + 1] = -1;
				((float*)data)[i] = 0;
				break;
			}
		}
		else
		{
			string temp(((char*)data+0*256));
			string temp1(((char*)data+i*256));
			if (*(string*)key < temp)
			{
				i--;
				break;
			}
			else if (*(string*)key == temp1)
			{
				pointer[i + 1] = -1;
				strncpy(((char*)data+i*256), "#", 1);
				break;
			}
		}

	}
	for (i; i < nowChildren - 1; i++)
	{
		if (i < 0)
		{
			pointer[i + 1] = pointer[i + 2];
			continue;
		}
		pointer[i + 1] = pointer[i + 2];
		if (col_type == 0)
			((int*)data)[i] = ((int*)data)[i + 1];
		else if (col_type == 1)
			((float*)data)[i] = ((float*)data)[i + 1];
		else
			strncpy(((char*)data+i*256), ((char*)data+(i+1)*256), 256);
	}
	if (col_type == 0)
		((int*)data)[i] = 0;
	else if (col_type == 1)
		((float*)data)[i] = 0;
	else
		strncpy(((char*)data+i*256), "#", 1);
	nowChildren--;
	return;
}

void Bpnode::updateInternalKey(Data* originKey, Data*updateKey, int col_type)
{
	int i;
	for (i = 0; i < nowChildren; i++)
	{
		if (compare(originKey, data, col_type, i) == 0)
		{
			if (col_type == 0)
				((int*)data)[i] = ((Datai*)updateKey)->x;
			else if (col_type == 1)
				((float*)data)[i] = ((Dataf*)updateKey)->x;
			else
				strncpy(((char*)data+i*256), (((Datac*)updateKey)->x).c_str(), 256);
		}
	}
	return;
}

void Bpnode::mergeLeaf(Bpnode& node, int col_type,int maxchildren,string table_name)
{															     
	int temp = nowChildren;
	Bpnode father;
	father.init(col_type,maxchildren);
	father.getNode(fatherAddr, table_name, col_type,maxchildren);
	void* deleteKey = NULL;
	if (col_type == 0)
	{
		deleteKey = new int;
		*(int*)deleteKey = ((int*)(node.data))[0];
	}
	else if (col_type == 1)
	{
		deleteKey = new float;
		*(float*)deleteKey = ((float*)(node.data))[0];
	}
	else
	{
		deleteKey = new string;
		string temp(((char*)(node.data)+0*256));
		*(string*)deleteKey = temp;
	}
	father.deleteRecord(deleteKey, col_type);
	father.saveNode(father.nodeAddr,col_type,maxchildren);
	father.freeNode(col_type);
	//===================================================================================
	for (int i =0; i < node.nowChildren; i++)
	{
		if (col_type == 0)
		{
			((int*)data)[i + temp] = ((int*)node.data)[i];
			((int*)node.data)[i] = 0;
			pointer[i + temp] = node.pointer[i];
			node.pointer[i] = -1;
		}
		else if (col_type == 1)
		{
			((float*)data)[i + temp] = ((float*)node.data)[i];
			((float*)node.data)[i] = 0;
			pointer[i + temp] = node.pointer[i];
			node.pointer[i] = -1;
		}
		else
		{
			((string*)data)[i + temp] = ((string*)node.data)[i];
			strncpy(((char*)(node.data)+(i + temp)*256), ((char*)(node.data)+i*256), 256);
			strncpy(((char*)(node.data)+i*256),"#", 1);
			pointer[i + temp] = node.pointer[i];
			node.pointer[i] = -1;
		}
	}
	pointer[maxchildren] = node.pointer[maxchildren];
	nowChildren += node.nowChildren;
	node.nowChildren = 0;
	delete deleteKey;
	return;
}

void Bpnode::mergeInternal(Bpnode& node, int col_type, string table_name,int maxchildren)
{
	Bpnode firstSon;
	firstSon.init(col_type, maxchildren);
	firstSon.getNode(node.pointer[0], table_name, col_type, maxchildren);
	firstSon.fatherAddr = nodeAddr;
	firstSon.saveNode(firstSon.nodeAddr,col_type,maxchildren);
	firstSon.freeNode(col_type);
	void* firstdata;
	Bpnode son;
	son.init(col_type, maxchildren);
	son.getNode(node.pointer[0], table_name, col_type, maxchildren);
	while (son.state != Leaf)
		son.getNode(son.pointer[0], table_name, col_type, maxchildren);
//======================================================================================================================
	//insert first son
	if (col_type == 0)
		((int*)data)[nowChildren - 1] = ((int*)son.data)[0];
	else if (col_type == 1)
		((float*)data)[nowChildren - 1] = ((float*)son.data)[0];
	else
		strncpy(((char*)data+(nowChildren - 1)*256), ((char*)son.data + 0*256),256);
	pointer[nowChildren] = node.pointer[0];
	
//	nowChildren++;
//======================================================================================================================
	Bpnode renewSon;
	renewSon.init(col_type, maxchildren);
	for (int i = 1; i < node.nowChildren; i++)
	{
		if (col_type == 0)
		{
			((int*)data)[i + nowChildren -1] = ((int*)node.data)[i-1];
			pointer[i + nowChildren] = node.pointer[i];
			renewSon.getNode(pointer[i + nowChildren], table_name, col_type, maxchildren);
			renewSon.fatherAddr = nodeAddr;
			renewSon.saveNode(renewSon.nodeAddr,col_type,maxchildren);
		}
		else if (col_type == 1)
		{
			((float*)data)[i + nowChildren - 1] = ((float*)node.data)[i - 1];
			pointer[i + nowChildren] = node.pointer[i];
			renewSon.getNode(pointer[i + nowChildren], table_name, col_type, maxchildren);
			renewSon.fatherAddr = nodeAddr;
			renewSon.saveNode(renewSon.nodeAddr, col_type, maxchildren);
		}
		else
		{
			strncpy(((char*)data+(i + nowChildren - 1)*256), ((char*)son.data+(i-1)*256), 256);
			pointer[i + nowChildren] = node.pointer[i];
			renewSon.getNode(pointer[i + nowChildren], table_name, col_type, maxchildren);
			renewSon.fatherAddr = nodeAddr;
			renewSon.saveNode(renewSon.nodeAddr, col_type, maxchildren);
		}
	}
	renewSon.freeNode(col_type);
	//======================================================================================================================
	Bpnode father;
	father.init(col_type, maxchildren);
	father.getNode(fatherAddr, table_name, col_type, maxchildren);
	father.deleteRecord(son.data,col_type);
	father.saveNode(father.nodeAddr,col_type,maxchildren);
	father.freeNode(col_type);
	nowChildren += node.nowChildren;
	return;
}


void Bpnode::cleannode(int numofnode, int& rootpos,int col_type,int maxchildren,string table_name)
{
	int deleteAddr = nodeAddr;
	Bpnode lastnode;
	lastnode.init(col_type, maxchildren);
	lastnode.getNode(numofnode - 1,table_name,col_type,maxchildren);
	if (nodeAddr == numofnode - 1)
	{
		lastnode.freeNode(col_type);
		return;
	}	
	lastnode.nodeAddr = deleteAddr;
	lastnode.saveNode(lastnode.nodeAddr,col_type,maxchildren);
	lastnode.getNode(deleteAddr,table_name,col_type,maxchildren);
	if (lastnode.state == Root)// if last node is root
	{
		rootpos = deleteAddr;
		Bpnode temp;
		temp.init(col_type, maxchildren);
		for (int i = 0; i < lastnode.nowChildren; i++)// renew father node pos for every son of root
		{
			temp.getNode(lastnode.pointer[i], table_name, col_type, maxchildren);
			temp.fatherAddr = rootpos;
			temp.saveNode(temp.nodeAddr,col_type,maxchildren);
		}
		temp.freeNode(col_type);
	}
	else if (lastnode.state == Internal)
	{
		Bpnode temp;
		temp.init(col_type, maxchildren);
		for (int i = 0; i < lastnode.nowChildren; i++)//renew son node
		{
			
			temp.getNode(lastnode.pointer[i], table_name, col_type, maxchildren);
			temp.fatherAddr = deleteAddr;
			temp.saveNode(temp.nodeAddr, col_type, maxchildren);
		}
		temp.getNode(lastnode.fatherAddr, table_name, col_type, maxchildren);
		Bpnode son;
		son.init(col_type, maxchildren);
		son.getNode(lastnode.pointer[0], table_name, col_type, maxchildren);
		while(son.state!=Leaf)
			son.getNode(son.pointer[0], table_name, col_type, maxchildren);
		for (int i = 0; i < temp.nowChildren; i++) //renew father node
		{
			if (col_type == 0)
			{
				if (((int*)son.data)[0] < ((int*)temp.data)[0])
				{
					temp.pointer[0] = deleteAddr;
					break;
				}				
				else if (((int*)son.data)[0] == ((int*)temp.data)[i])
				{
					temp.pointer[i + 1] = deleteAddr;
					break;
				}		
			}
			else if (col_type == 1)
			{
				if (((float*)son.data)[0] < ((float*)temp.data)[0])
				{
					temp.pointer[0] = deleteAddr;
					break;
				}				
				else if (((float*)son.data)[0] == ((float*)temp.data)[i])
				{
					temp.pointer[i + 1] = deleteAddr;
					break;
				}
			}
			else
			{
				string temp1(((char*)son.data + 0*256));
				string temp2(((char*)temp.data + 0*256));
				string temp3(((char*)temp.data + i*256));
				if (temp1 < temp2)
				{
					temp.pointer[0] = deleteAddr;
					break;
				}				
				else if (temp1 == temp3)
				{
					temp.pointer[i + 1] = deleteAddr;
					break;
				}			
			}
		}
		temp.saveNode(temp.nodeAddr,col_type,maxchildren);
		temp.freeNode(col_type);
		son.freeNode(col_type);
	}
	else
	{
		Bpnode temp;
		temp.init(col_type, maxchildren);
		temp.getNode(lastnode.fatherAddr, table_name, col_type, maxchildren);
		int i = 0;
		for (i = 0; i < temp.nowChildren; i++) //update father node
		{
			if (col_type == 0)
			{
				if (((int*)lastnode.data)[0] < ((int*)temp.data)[0])
				{
					temp.pointer[0] = deleteAddr;
					break;
				}
				else if (((int*)lastnode.data)[0] == ((int*)temp.data)[i])
				{
					temp.pointer[i+1] = deleteAddr;
					break;
				}				
			}
			else if (col_type == 1)
			{
				if (((float*)lastnode.data)[0] < ((float*)temp.data)[0])
				{
					temp.pointer[0] = deleteAddr;
					break;
				}
				else if (((float*)lastnode.data)[0] == ((float*)temp.data)[i])
				{
					temp.pointer[i+1] = deleteAddr;
					break;
				}			
			}
			else
			{
				string temp1(((char*)lastnode.data + 0*256));
				string temp2(((char*)temp.data+0*256));
				string temp3(((char*)temp.data+i*256));
				if (temp1 < temp2)
				{
					temp.pointer[0] = deleteAddr;
					break;
				}
				else if (temp1 == temp3)
				{
					temp.pointer[i+1] = deleteAddr;
					break;
				}
					
				
			}

		}
		int save = temp.nodeAddr;
		temp.saveNode(temp.nodeAddr, col_type, maxchildren);
		int isbreak = 0;
		temp.getNode(temp.nodeAddr,table_name,col_type,maxchildren);
		while (temp.state != Root && (compare(lastnode.data, temp.data, col_type) == -1))
			temp.getNode(temp.fatherAddr, table_name, col_type, maxchildren);

		if (compare(lastnode.data, temp.data, col_type) != -1)//renew prevoius leaf node
		{
			int j = 0;
			for (j = 0; j < temp.nowChildren; j++)
			{
				if (compare(lastnode.data, temp.data, col_type, j) == 0)
					break;
			}
			temp.getNode(temp.pointer[j], table_name, col_type, maxchildren);
			while(temp.state!=Leaf)
				temp.getNode(temp.pointer[temp.nowChildren - 1], table_name, col_type, maxchildren);
			temp.pointer[maxchildren] = deleteAddr;
			temp.saveNode(temp.nodeAddr, col_type, maxchildren);
		}
		temp.freeNode(col_type);
	}
	lastnode.freeNode(col_type);
}

int Bpnode::rightBorrowOK(int col_type, string table_name, int maxchildren)
{
	Bpnode rightSibling;
	rightSibling.init(col_type, maxchildren);
	rightSibling.getNode(fatherAddr, table_name, col_type, maxchildren);
	int rightSiblingPos = 0;
	for (rightSiblingPos = 0; rightSiblingPos < rightSibling.nowChildren; rightSiblingPos++) //find right sibling pos
	{
		if (compare(data, rightSibling.data, col_type, rightSiblingPos) < 0)
			break;
		else if (compare(data, rightSibling.data, col_type, rightSiblingPos) == 0)
		{
			rightSiblingPos++;
			break;
		}			
	}
	if (rightSiblingPos == rightSibling.nowChildren - 1)
		return 0;
	rightSibling.getNode(rightSibling.pointer[rightSiblingPos + 1], table_name, col_type, maxchildren);
	if (rightSibling.state == Leaf)
	{
		if (rightSibling.nowChildren > maxchildren / 2)
		{
			rightSibling.freeNode(col_type);
			return rightSiblingPos + 1;
		}		
		else
		{
			rightSibling.freeNode(col_type);
			return 0;
		}
	}
	else
	{
		if (rightSibling.nowChildren > maxchildren/2 + 1)
		{
			rightSibling.freeNode(col_type);
			return rightSiblingPos + 1;
		}
		else
		{
			rightSibling.freeNode(col_type);
			return 0;
		}
	}
	
}

int Bpnode::leftBorrowOK(int col_type, string table_name, int maxchildren)
{
	Bpnode leftSibling;
	leftSibling.init(col_type, maxchildren);
	leftSibling.getNode(fatherAddr, table_name, col_type, maxchildren);
	int leftSiblingPos = 0;
	for (leftSiblingPos = 0; leftSiblingPos < leftSibling.nowChildren; leftSiblingPos++) //find right sibling pos
	{
		if (compare(data, leftSibling.data, col_type, leftSiblingPos) < 0)
			return 0;
		else if (compare(data, leftSibling.data, col_type, leftSiblingPos) == 0)
		{
			leftSiblingPos++;
			break;
		}			
	}
	leftSibling.getNode(leftSibling.pointer[leftSiblingPos - 1], table_name, col_type, maxchildren);
	if (leftSibling.state == Leaf)
	{
		if (leftSibling.nowChildren > maxchildren / 2)
		{
			leftSibling.freeNode(col_type);
			return leftSiblingPos + 1;
		}		
		else
		{
			leftSibling.freeNode(col_type);
			return 0;
		}
	}
	else
	{
		if (leftSibling.nowChildren > maxchildren / 2 + 1)
		{
			leftSibling.freeNode(col_type);
			return leftSiblingPos + 1;
		}
		else
		{
			leftSibling.freeNode(col_type);
			return 0;
		}
	}
}

void Bpnode::leftBorrowInternal(int col_type, string table_name, int maxchildren)
{
	int leftSiblingPos = this->leftBorrowOK(col_type, table_name, maxchildren);
	Bpnode father;
	father.init(col_type, maxchildren);
	father.getNode(fatherAddr, table_name, col_type, maxchildren);
	Bpnode leftSibling;
	leftSibling.init(col_type, maxchildren);
	leftSibling.getNode(father.pointer[leftSiblingPos], table_name, col_type, maxchildren);
	//=========================================================================================================
	Bpnode son;
	son.init(col_type, maxchildren);
	son.getNode(leftSibling.pointer[nowChildren - 1], table_name, col_type, maxchildren);
	son.fatherAddr = nodeAddr;
	son.saveNode(son.nodeAddr,col_type,maxchildren);
	Data* originKey = NULL;
	Data* updateKey = NULL;
	son.getNode(pointer[0], table_name, col_type, maxchildren);
	while (son.state != Leaf)
		son.getNode(son.pointer[0], table_name, col_type, maxchildren);
	int save = leftSibling.pointer[leftSibling .nowChildren - 1];
	if (col_type == 0)
	{		
		updateKey = new Datai(((int*)leftSibling.data)[leftSibling.nowChildren-2]);
		originKey = new Datai(((int*)son.data)[0]);
	}
	else if (col_type == 1)
	{
		updateKey = new Dataf(((float*)leftSibling.data)[leftSibling.nowChildren - 2]);
		originKey = new Dataf(((float*)son.data)[0]);
	}
	else
	{
		string temp1(((char*)leftSibling.data+(leftSibling.nowChildren - 2)*256));
		string temp2(((char*)son.data+0*256));
		updateKey = new Datac(temp1);
		originKey = new Datac(temp2);
	}
	//================================================================================================================
	leftSibling.deleteRecord(updateKey,col_type);
	while (compare(son.data, father.data, col_type, 0) < 0 && father.state != Root)
		father.getNode(father.fatherAddr, table_name, col_type, maxchildren);
	father.updateInternalKey(originKey, updateKey, col_type);
	//================================================================================================================
	for (int i = nowChildren - 1; i > 0; i++)
	{
		if (col_type == 0)
			((int*)data)[i] = ((int*)data)[i - 1];
		else if (col_type == 1)
			((float*)data)[i] = ((float*)data)[i - 1];
		else
			strncpy(((char*)data+i*256), ((char*)data+(i-1)*256), 256);
		pointer[i + 1] = pointer[i];
	}
	pointer[1] = pointer[0];
	son.getNode(pointer[1], table_name, col_type, maxchildren);
	while (son.state!=Leaf)
		son.getNode(son.pointer[0], table_name, col_type, maxchildren);
	if (col_type == 0)
		((int*)data)[0] = ((int*)son.data)[0];
	else if (col_type == 1)
		((float*)data)[0] = ((float*)son.data)[0];
	else
		strncpy(((char*)data+0*256), ((char*)son.data+0*256), 256);
	pointer[0] = save;
	//================================================================================================================
	son.saveNode(son.nodeAddr,col_type,maxchildren);
	leftSibling.saveNode(leftSibling.nodeAddr, col_type, maxchildren);
	father.saveNode(father.nodeAddr, col_type, maxchildren);
	son.freeNode(col_type);
	leftSibling.freeNode(col_type);
	father.freeNode(col_type);
	delete originKey;
	delete updateKey;
	return;
}

void Bpnode::leftBorrowLeaf(int col_type, string table_name, int maxchildren)
{
	int leftSiblingPos = this->leftBorrowOK(col_type, table_name, maxchildren);
	Bpnode father;
	father.init(col_type, maxchildren);
	father.getNode(fatherAddr, table_name, col_type, maxchildren);
	Bpnode leftSibling;
	leftSibling.init(col_type, maxchildren);
	leftSibling.getNode(father.pointer[leftSiblingPos], table_name, col_type, maxchildren);
	//=========================================================================================================
	Data* originKey = NULL;
	Data* updateKey = NULL;
	int Addr = leftSibling.pointer[leftSibling.nowChildren - 1];
	if (col_type == 0)
	{
		updateKey = new Datai(((int*)leftSibling.data)[leftSibling.nowChildren - 1]);
		originKey = new Datai(((int*)data)[0]);
	}
	else if (col_type == 1)
	{
		updateKey = new Dataf(((float*)leftSibling.data)[leftSibling.nowChildren - 1]);
		originKey = new Dataf(((float*)data)[0]);
	}
	else
	{
		string temp1(((char*)leftSibling.data+(leftSibling.nowChildren - 1)*256));
		string temp2(((char*)data+0*256));
		updateKey = new Datac(temp1);
		originKey = new Datac(temp2);
	}
	//================================================================================================================
	leftSibling.deleteRecord(updateKey, col_type);
	father.updateInternalKey(originKey, updateKey, col_type);
	this->insert(updateKey, Addr, col_type);
	//================================================================================================================
	//================================================================================================================
	leftSibling.saveNode(leftSibling.nodeAddr, col_type, maxchildren);
	father.saveNode(father.nodeAddr, col_type, maxchildren);
	leftSibling.freeNode(col_type);
	father.freeNode(col_type);

	delete originKey;
	delete updateKey;
	return;
}


void Bpnode::rightBorrowInternal(int col_type, string table_name, int maxchildren)
{
	int rightSiblingPos = this->rightBorrowOK(col_type, table_name, maxchildren);
	Bpnode father;
	father.init(col_type, maxchildren);
	father.getNode(fatherAddr, table_name, col_type, maxchildren);
	Bpnode rightSibling;
	rightSibling.init(col_type, maxchildren);
	rightSibling.getNode(father.pointer[rightSiblingPos], table_name, col_type, maxchildren);
	//=========================================================================================================
	pointer[nowChildren] = rightSibling.pointer[0];
	Bpnode son;
	son.init(col_type, maxchildren);
	son.getNode(rightSibling.pointer[0], table_name, col_type, maxchildren);
	while(son.state!=Leaf)
		son.getNode(son.pointer[0], table_name, col_type, maxchildren);
	Data* originKey = NULL;
	Data* updateKey = NULL;
	Data* sonKey = NULL;
	if (col_type == 0)
	{
		((int*)data)[nowChildren - 1] = ((int*)son.data)[0];
		nowChildren++;
		originKey = new Datai(((int*)son.data)[0]);
		updateKey = new Datai(((int*)rightSibling.data)[0]);
		sonKey = new Datai(((int*)son.data)[0]);
	}
	else if (col_type == 1)
	{
		((float*)data)[nowChildren - 1] = ((float*)son.data)[0];
		nowChildren++;
		originKey = new Dataf(((float*)son.data)[0]);
		updateKey = new Dataf(((float*)rightSibling.data)[0]);
		sonKey = new Dataf(((float*)son.data)[0]);
	}
	else
	{
		strncpy(((char*)data+(nowChildren - 1)*256), ((char*)son.data+0*256), 256);
		nowChildren++;
		string temp1(((char*)son.data+0*256));
		string temp2(((char*)rightSibling.data+0*256));
		string temp3(((char*)son.data+0*256));
		originKey = new Datac(temp1);
		updateKey = new Datac(temp2);
		sonKey = new Datac(temp3);
	}
	//================================================================================================================
	son.getNode(rightSibling.pointer[0], table_name, col_type, maxchildren);
	son.fatherAddr = nodeAddr;
	rightSibling.deleteRecord(updateKey, col_type);
	//================================================================================================================
	while (compare(sonKey, father.data, col_type, 0) < 0 && father.state!=Root)
		father.getNode(father.fatherAddr, table_name, col_type, maxchildren);
	father.updateInternalKey(sonKey, updateKey, col_type);
	//================================================================================================================
	son.saveNode(son.nodeAddr, col_type, maxchildren);
	rightSibling.saveNode(rightSibling.nodeAddr, col_type, maxchildren);
	father.saveNode(father.nodeAddr, col_type, maxchildren);
	son.freeNode(col_type);
	rightSibling.freeNode(col_type);
	father.freeNode(col_type);
	delete originKey;
	delete updateKey;
	delete sonKey;
	return;
}

void Bpnode::rightBorrowLeaf(int col_type, string table_name, int maxchildren)
{
	int rightSiblingPos = this->rightBorrowOK(col_type, table_name, maxchildren);
	Bpnode father;
	father.init(col_type, maxchildren);
	father.getNode(fatherAddr, table_name, col_type, maxchildren);
	Bpnode rightSibling;
	rightSibling.init(col_type, maxchildren);
	rightSibling.getNode(father.pointer[rightSiblingPos], table_name, col_type, maxchildren);
	//=========================================================================================================
	pointer[nowChildren] = rightSibling.pointer[0];
	Data* originKey = NULL;
	Data* updateKey = NULL;
	if (col_type == 0)
	{
		originKey = new Datai(((int*)rightSibling.data)[0]);
		updateKey = new Datai(((int*)rightSibling.data)[1]);
		((int*)data)[nowChildren] = ((int*)rightSibling.data)[0];
	}
	else if (col_type == 1)
	{
		originKey = new Dataf(((float*)rightSibling.data)[0]);
		updateKey = new Dataf(((float*)rightSibling.data)[1]);
		((float*)data)[nowChildren] = ((float*)rightSibling.data)[0];
	}
	else
	{
		string temp1(((char*)rightSibling.data+0*256));
		string temp2(((char*)rightSibling.data+1*256));
		originKey = new Datac(temp1);
		updateKey = new Datac(temp2);
		strncpy(((char*)data+ nowChildren*256), ((char*)rightSibling.data+0*256),256);
	}
	//================================================================================================================
	rightSibling.deleteRecord(updateKey, col_type);
	//================================================================================================================
	while (compare(originKey, father.data, col_type, 0) < 0 && father.state != Root)
		father.getNode(father.fatherAddr, table_name, col_type, maxchildren);
	father.updateInternalKey(originKey, updateKey, col_type);
	nowChildren++;
	//================================================================================================================
	rightSibling.saveNode(rightSibling.nodeAddr, col_type, maxchildren);
	father.saveNode(father.nodeAddr, col_type, maxchildren);
	rightSibling.freeNode(col_type);
	father.freeNode(col_type);
	delete originKey;
	delete updateKey;
	return;
}

int Bpnode::rightMergeOK(int col_type, string table_name, int maxchildren)
{
	Bpnode rightSibling;
	rightSibling.init(col_type, maxchildren);
	rightSibling.getNode(fatherAddr, table_name, col_type, maxchildren);
	int rightSiblingPos = 0;
	for (rightSiblingPos = 0; rightSiblingPos < rightSibling.nowChildren - 1; rightSiblingPos++) //find right sibling pos
	{
		if (compare(data, rightSibling.data, col_type, rightSiblingPos) < 0)
		{
			rightSibling.freeNode(col_type);
			return rightSiblingPos + 1;
		}			
		else if (compare(data, rightSibling.data, col_type, rightSiblingPos) == 0)
		{
			rightSiblingPos++;
			rightSibling.freeNode(col_type);
			return rightSiblingPos + 1;
		}
	}
	rightSibling.freeNode(col_type);
	return -1;
}
int Bpnode::leftMergeOK(int col_type, string table_name, int maxchildren)
{
	Bpnode leftSibling;
	leftSibling.init(col_type, maxchildren);
	leftSibling.getNode(fatherAddr, table_name, col_type, maxchildren);
	int leftSiblingPos = 0;
	for (leftSiblingPos = 0; leftSiblingPos < leftSibling.nowChildren; leftSiblingPos++) //find right sibling pos
	{
		if (compare(data, leftSibling.data, col_type, leftSiblingPos) < 0)
		{
			leftSibling.freeNode(col_type);
			return -1;
		}		
		else if (compare(data, leftSibling.data, col_type, leftSiblingPos) == 0)
		{
			leftSiblingPos++;
			leftSibling.freeNode(col_type);
			return leftSiblingPos - 1;
		}
	}
	leftSibling.freeNode(col_type);
	return -1;
}


int deleteBack(int curAddr, Data* key, int col_type, int& maxchildren, int& numofnode, int& rootpos, string table_name)
{
	int status = 0;
	int isbreak = 0;
	Bpnode curnode;
	curnode.init(col_type,maxchildren);
	curnode.getNode(curAddr, table_name, col_type, maxchildren);
	while (status!=-1&& curnode.state!=Root)
	{
		if (status == 1 && curnode.nowChildren > maxchildren/2)
		{
			status = -1;
			isbreak = 1;
			continue;
		}
		if (curnode.nowChildren > maxchildren / 2)//usual delete
		{
			curnode.deleteRecord(key, col_type);
			status = -1;
			isbreak = 1;
			continue;
		}
		else
		{			
			if (curnode.rightBorrowOK(col_type, table_name, maxchildren))
			{
				if(curnode.state==Internal)
					curnode.rightBorrowInternal(col_type, table_name, maxchildren);
				else
					curnode.rightBorrowLeaf(col_type, table_name, maxchildren);
				if (status == 0)
					curnode.deleteRecord(key, col_type);
				status = -1;
				isbreak = 1;
				continue;
			}				
			else if (curnode.leftBorrowOK(col_type, table_name, maxchildren))
			{
				if(curnode.state==Internal)
					curnode.leftBorrowInternal(col_type, table_name, maxchildren);
				else
					curnode.leftBorrowLeaf(col_type, table_name, maxchildren);
				if (status == 0)
					curnode.deleteRecord(key, col_type);
				status = -1;
				isbreak = 1;
				continue;
			}
			if(status==0)
				curnode.deleteRecord(key, col_type);
			if (curnode.rightMergeOK(col_type, table_name, maxchildren)>=0)
			{
				Bpnode father;
				father.init(col_type, maxchildren);
				father.getNode(curnode.fatherAddr, table_name, col_type, maxchildren);
				int rightSiblingPos = curnode.rightMergeOK(col_type, table_name, maxchildren);
				rightSiblingPos = father.pointer[rightSiblingPos];
				father.freeNode(col_type);
				Bpnode RightSibling;
				RightSibling.init(col_type,maxchildren);
				RightSibling.getNode(rightSiblingPos,table_name,col_type,maxchildren);
				if (curnode.state == Leaf)
					curnode.mergeLeaf(RightSibling, col_type, maxchildren,table_name);
				else
					curnode.mergeInternal(RightSibling, col_type, table_name,maxchildren);
				curnode.saveNode(curnode.nodeAddr,col_type,maxchildren);
				int deleteAddr = RightSibling.nodeAddr;
				RightSibling.cleannode(numofnode,rootpos,col_type,maxchildren, table_name);
				numofnode--;
				if (curnode.fatherAddr == numofnode)
					curnode.getNode(deleteAddr,table_name,col_type,maxchildren);
				else
					curnode.getNode(curnode.fatherAddr, table_name, col_type, maxchildren);
				status = 1;
				RightSibling.freeNode(col_type);
			}
			else if (curnode.leftMergeOK(col_type, table_name, maxchildren) >= 0)
			{
				Bpnode father;
				father.init(col_type, maxchildren);
				father.getNode(curnode.fatherAddr, table_name, col_type, maxchildren);
				int leftSiblingPos = curnode.rightMergeOK(col_type, table_name, maxchildren);
				leftSiblingPos = father.pointer[leftSiblingPos];
				father.freeNode(col_type);
				Bpnode leftSibling;
				leftSibling.init(col_type, maxchildren);
				leftSibling.getNode(leftSiblingPos, table_name, col_type, maxchildren);
				if (curnode.state == Leaf)
					curnode.mergeLeaf(leftSibling, col_type, maxchildren, table_name);
				else
					leftSibling.mergeInternal(curnode, col_type, table_name, maxchildren);
				curnode.saveNode(curnode.nodeAddr, col_type, maxchildren);
				int deleteAddr = leftSibling.nodeAddr;
				leftSibling.cleannode(numofnode, rootpos, col_type, maxchildren, table_name);
				numofnode--;
				if (curnode.fatherAddr == numofnode)
					curnode.getNode(deleteAddr,table_name,col_type,maxchildren);
				else
					curnode.getNode(curnode.fatherAddr, table_name, col_type, maxchildren);
				status = 1;
				leftSibling.freeNode(col_type);
			}
			else
			{
				cout << "Index Delete ERROR";
				status = -1;
			}
		}
	}
	curnode.saveNode(curnode.nodeAddr, col_type, maxchildren);
	if (isbreak == 0)
	{
		Bpnode root;
		root.init(col_type, maxchildren);
		root.getNode(rootpos, table_name, col_type, maxchildren);
		if (root.nowChildren == 1)
		{
			rootpos = root.pointer[0];
			Bpnode newroot;
			newroot.init(col_type, maxchildren);
			newroot.getNode(rootpos, table_name, col_type, maxchildren);
			newroot.state = Root;
			newroot.saveNode(newroot.nodeAddr, col_type, maxchildren);
			root.cleannode(numofnode, rootpos, col_type, maxchildren, table_name);
			numofnode--;
		}
		root.freeNode(col_type);
	}
	curnode.freeNode(col_type);
	return 0;
}

int insertBack(int curAddr, Data* key, int col_type, int offset, int& maxchildren, int& numofnode, int& rootpos,string table_name)
{
	int status = 0;
	int isbreak = 0;
	int returnAddr = -1;
	void* returnKey = NULL;
	if (col_type == 0)
		returnKey = new int;
	else if (col_type == 1)
		returnKey = new float;
	else
		returnKey = new string;
	Bpnode curnode;
	curnode.init(col_type,maxchildren);
	curnode.getNode(curAddr,table_name,col_type,maxchildren);
	//======================================================================================================
	curnode.insert(key,offset,col_type); //leaf insert
	while (status != -1 && curnode.state != Root)
	{
		if (status == 1 && curnode.nowChildren <= maxchildren )
		{
			status = -1;
			isbreak = 1;
			continue;
		}
		if (curnode.nowChildren < maxchildren )//usual insert (leaf)
		{
			status = -1;
			isbreak = 1;
			continue;
		}
		else
		{
			Bpnode splitnode;
			splitnode.init(col_type, maxchildren);
			splitnode.getNode(numofnode, table_name,col_type,maxchildren);
			if (curnode.state == Leaf)
			{
				splitnode.state = Leaf;
				splitnode.nodeAddr = numofnode;
				numofnode++;
				splitnode.fatherAddr = curnode.fatherAddr;
				splitnode.nowChildren = (maxchildren + 1) / 2;
				curnode.nowChildren = curnode.nowChildren - (maxchildren + 1) / 2;
				splitnode.pointer[maxchildren] = -1;
				//=====================================================================================
				curnode.splitLeafnode(splitnode,col_type,maxchildren);
				returnAddr = splitnode.nodeAddr;
				if (col_type == 0)
					*(int*)returnKey = ((int*)splitnode.data)[0];
				else if (col_type == 1)
					*(float*)returnKey = ((float*)splitnode.data)[0];
				else
				{
					string temp1(((char*)splitnode.data+0*256));
					*(string*)returnKey = temp1;
				}
					
				int save = curnode.fatherAddr;
				curnode.saveNode(curnode.nodeAddr,col_type,maxchildren);
				curnode.getNode(save, table_name, col_type, maxchildren);
				curnode.insert(returnKey,returnAddr,col_type);//internal insert
				status = 1;
			}
			else
			{
				splitnode.state = Internal;
				splitnode.nodeAddr = numofnode;
				numofnode++;
				splitnode.fatherAddr = curnode.fatherAddr;
				splitnode.nowChildren = (maxchildren + 1) / 2;
				curnode.nowChildren = curnode.nowChildren - (maxchildren + 1) / 2;
				//=====================================================================================
				curnode.splitInternalnode(splitnode, col_type, maxchildren,table_name);
				returnAddr = splitnode.nodeAddr;
				Bpnode findReturnKey;
				findReturnKey.init(col_type,maxchildren);
				findReturnKey.getNode(splitnode.pointer[0],table_name,col_type,maxchildren);
				while (findReturnKey.state != Leaf)
					findReturnKey.getNode(findReturnKey.pointer[0],table_name,col_type,maxchildren);			
				if (col_type == 0)
					*(int*)returnKey = ((int*)findReturnKey.data)[0];
				else if (col_type == 1)
					*(float*)returnKey = ((float*)findReturnKey.data)[0];
				else
				{
					string temp1(((char*)findReturnKey.data));
					*(string*)returnKey = temp1;
				}
					
				int save = curnode.fatherAddr;
				curnode.saveNode(curnode.nodeAddr,col_type,maxchildren);
				curnode.getNode(save, table_name, col_type, maxchildren);
				curnode.insert(returnKey, returnAddr, col_type);//internal insert
				status = 1;
			}
			splitnode.saveNode(splitnode.nodeAddr,col_type,maxchildren);
			splitnode.freeNode(col_type);
		}
	}
	if (isbreak == 0 && ((status != 0 && curnode.nowChildren > maxchildren)|| (status == 0 && curnode.nowChildren >= maxchildren))) // root node is full
	{
		Bpnode newroot;
		newroot.init(col_type,maxchildren);
		newroot.getNode(numofnode,table_name,col_type,maxchildren);	
		newroot.state = Root;
		newroot.nodeAddr = numofnode;
		numofnode++;	
		rootpos = newroot.nodeAddr;
		newroot.fatherAddr = -1;
		newroot.nowChildren = 1;
		newroot.pointer[0] = curnode.nodeAddr;
		curnode.fatherAddr = newroot.nodeAddr;
		if (numofnode != 2)
			curnode.state = Internal;
		else
			curnode.state = Leaf;
		//======================================================================================
		Bpnode splitnode;
		splitnode.init(col_type,maxchildren);
		splitnode.getNode(numofnode,table_name,col_type,maxchildren);
		if (curnode.state == Internal)
		{		
			splitnode.state = Internal;
			splitnode.nodeAddr = numofnode;
			numofnode++;
			splitnode.fatherAddr = curnode.fatherAddr;
			splitnode.nowChildren = (maxchildren + 1) / 2;
			curnode.nowChildren = curnode.nowChildren - (maxchildren + 1) / 2;
			//=====================================================================================
			curnode.splitInternalnode(splitnode, col_type, maxchildren, table_name);
			Bpnode findReturnKey;
			findReturnKey.init(col_type,maxchildren);
			findReturnKey.getNode(splitnode.pointer[0],table_name,col_type,maxchildren);
			while (findReturnKey.state != Leaf)
				findReturnKey.getNode(findReturnKey.pointer[0], table_name, col_type, maxchildren);
			if (col_type == 0)
				((int*)newroot.data)[0] = ((int*)findReturnKey.data)[0];
			else if (col_type == 1)
				((float*)newroot.data)[0] = ((float*)findReturnKey.data)[0];
			else
				strncpy(((char*)newroot.data), ((char*)findReturnKey.data),256);
			newroot.pointer[1] = splitnode.nodeAddr;	
			newroot.nowChildren++;
			findReturnKey.freeNode(col_type);
		}
		else //Leaf
		{
			splitnode.state = Leaf;
			splitnode.nodeAddr = numofnode;
			numofnode++;
			splitnode.fatherAddr = curnode.fatherAddr;
			splitnode.nowChildren = (maxchildren + 1) / 2;
			curnode.nowChildren = curnode.nowChildren - (maxchildren + 1) / 2;
			splitnode.pointer[maxchildren] = -1;
			//=====================================================================================
			curnode.splitLeafnode(splitnode, col_type, maxchildren);
			if (col_type == 0)
				((int*)newroot.data)[0] = ((int*)splitnode.data)[0];
			else if (col_type == 1)
				((float*)newroot.data)[0] = ((float*)splitnode.data)[0];
			else
				strncpy(((char*)newroot.data), ((char*)splitnode.data), 256);
			newroot.pointer[1] = splitnode.nodeAddr;
			newroot.nowChildren++;
		}
		splitnode.saveNode(splitnode.nodeAddr,col_type,maxchildren);
		splitnode.freeNode(col_type);
		newroot.saveNode(newroot.nodeAddr, col_type, maxchildren);
		newroot.freeNode(col_type);
	}
	curnode.saveNode(curnode.nodeAddr,col_type,maxchildren);
	curnode.freeNode(col_type);
	delete returnKey;
	return 0;
}

void Bpnode::splitLeafnode(Bpnode& splitnode, int col_type,int maxchildren)
{
	int temp = maxchildren/2;
	for (int i = 0 ; i < splitnode.nowChildren; i++)//leaf node
	{
		(splitnode.pointer)[i] = pointer[i + temp];
		pointer[i + temp] = -1;
		if (col_type == 0)
		{
			((int*)(splitnode.data))[i] = ((int*)data)[i + temp];
			((int*)data)[i + temp] = 0;
		}
		else if (col_type == 1)
		{
			((float*)(splitnode.data))[i] = ((float*)data)[i + temp];
			((float*)data)[i+temp] = 0;
		}
		else
		{
			strncpy(((char*)(splitnode.data)+i*256), ((char*)data+(i + temp)*256),256);
			strncpy(((char*)data+(i + temp)*256), "#", 1);
		}
	}
	splitnode.pointer[maxchildren] = pointer[maxchildren];
	return;
}

void Bpnode::splitInternalnode(Bpnode& splitnode, int col_type, int maxchildren , string table_name)
{
	int temp = maxchildren / 2;
	Bpnode son;
	son.init(col_type,maxchildren);
	son.getNode(pointer[temp+1],table_name,col_type,maxchildren);
	splitnode.pointer[0] = pointer[temp + 1];
	if (col_type == 0)
		((int*)data)[temp] = 0;
	else if (col_type == 1)
		((float*)data)[temp] = 0;
	else
		strncpy(((char*)data+temp*256), "#", 1);
	temp++;
	son.fatherAddr = splitnode.nodeAddr;
	son.saveNode(son.nodeAddr,col_type,maxchildren);
	for (int i = 1; i < splitnode.nowChildren; i++)//leaf node
	{
		son.getNode(pointer[temp + 1], table_name, col_type, maxchildren);
		splitnode.pointer[i] = pointer[i + temp];
		pointer[i+temp] = -1;
		son.fatherAddr = splitnode.nodeAddr;
		son.saveNode(son.nodeAddr, col_type, maxchildren);
		if (col_type == 0)
		{
			((int*)(splitnode.data))[i-1] = ((int*)data)[i + temp -1];
			((int*)data)[i + temp - 1] = 0;
		}
		else if (col_type == 1)
		{
			((float*)(splitnode.data))[i-1] = ((float*)data)[i + temp - 1];
			((float*)data)[i + temp - 1] = 0;
		}
		else
		{
			strncpy(((char*)(splitnode.data)+(i-1)*256), ((char*)data+(i + temp - 1)*256),256);
			strncpy(((char*)data+(i + temp - 1)*256),"#", 1);
		}
	}
	son.freeNode(col_type);
	return;
}


void Bpnode::getNode(int nodeAddr_in, string table_name,int col_type, int maxchildren )
{
	blocknum = bf.getaBlock("Table_" + table_name + "_index", nodeAddr_in);
	char* myblock = blocks[blocknum].vals;
	//========node info========================================================================
	state = *(int*)(myblock + 0);
	nodeAddr = *(int*)(myblock + 4);
	nowChildren = *(int*)(myblock + 8);
	fatherAddr = *(int*)(myblock + 12);
	//==========root=====================================================================
	memcpy(data, myblock + HeaderOffset , PointerOffset);
	memcpy(pointer, myblock + HeaderOffset + PointerOffset, (maxchildren + 1) * sizeof(int));
}

void Bpnode::saveNode(int nodeAddr_in, int col_type, int maxchildren)
{
	char* myblock = blocks[blocknum].vals;
	//========node info========================================================================
	*(int*)(myblock + 0) = state;
	*(int*)(myblock + 4) = nodeAddr_in;
	*(int*)(myblock + 8) = nowChildren;
	*(int*)(myblock + 12) = fatherAddr;
	//==========root=====================================================================
	memcpy(myblock + HeaderOffset , data, PointerOffset);
	memcpy(myblock + HeaderOffset + PointerOffset, pointer, (maxchildren + 1) * sizeof(int));
	bf.write(blocknum);
}

void Bpnode::freeNode(int col_type)
{
	if(data!=NULL)
		delete[] data;
	if(pointer!=NULL)
		delete[] pointer;
}

BpTree::BpTree(string tablename_in, int col_type_in)
{
	table_name = tablename_in;
	int No;
	No = bf.getaBlock("Table_" + tablename_in + "_bptree", 0);
	char * myblock = blocks[No].vals;

	col_type = *(int *)myblock;
	maxchildren = *(int *)(myblock + 4);
	numofnode = *(int *)(myblock + 8);
	rootpos = *(int *)(myblock + 12);
}

void BpTree::Bpsave()
{
	int No;
	No = bf.getaBlock("Table_" + table_name + "_bptree", 0);
	char * myblock = blocks[No].vals;

	*(int *)myblock = col_type;
	*(int *)(myblock + 4) = maxchildren;
	*(int *)(myblock + 8) = numofnode;
	*(int *)(myblock + 12) = rootpos;
	char save[256];
	strncpy(save, table_name.c_str(), 255);
	memcpy(myblock + 20, save, 256);
	bf.write(No);
}