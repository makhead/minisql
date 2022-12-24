#include "bufferManager.h"

buffermanager bf;
bufferBlock blocks[maxBlocks];
//block func
void bufferBlock::init()
{
	dirty = 0;
	valid = 0;
	fname = "NULL";
	offset = 0;
	memset(vals, Empty, blockSize);
	vals[blockSize + 1] = '\0'; //is end
}
string bufferBlock::getvals(int start, int end)
{
	string save = "";
	if (start >= 0 && start <= end && end <= blockSize)
	{
		int i;
		for (i = start; i < end; i++)
			save += vals[i];
		return save;
	}
	return string("#");
}
//manager func
buffermanager::buffermanager()
{
	for (int i = 0; i < maxBlocks; i++)
		blocks[i].init();
}
buffermanager::~buffermanager()
{
	for (int i = 0; i < maxBlocks; i++)
		writetoFile(i);
}
//status
int buffermanager::find_assign_buffer(string fname, int offset)
{
	int num;
	for (num = 0; num < maxBlocks; num++)
		if (blocks[num].fname == fname && blocks[num].offset == offset)
			return num;

	return -1;
}
void buffermanager::invalid(string fname)
{
	int i;
	for (i = 0; i < maxBlocks; i++) {
		if (blocks[i].fname == fname) {
			blocks[i].valid = 0;
			blocks[i].dirty = 0;
		}
	}
}
//file to block
void buffermanager::write(int num)
{
	blocks[num].dirty = 1;
	used(num);
}
void buffermanager::read(string fname, int offset, int num)
{
	blocks[num].valid = 1;
	blocks[num].dirty = 0;
	blocks[num].fname = fname;
	blocks[num].offset = offset;
	blocks[num].last_vis_time = clock();
    //open file
	FILE *f;
	if((f = fopen(fname.c_str(),"rb")) == NULL)//open fail
	{
		std::cout << "Open fail";
		return;
	}
	fseek(f, blockSize*offset, SEEK_SET);
	fread(blocks[num].vals, blockSize, 1, f);
	fclose(f);
}
void buffermanager::used(int num)
{
	blocks[num].last_vis_time = clock();
}
//insertion
bufferBlock buffermanager::find_Insertable_pos()
{
	string file = "Table_" + current_table.table_name + "_tuple";
	bufferBlock pos;
	if (current_table.blockNum == 0) { //new file and no block exist 
		pos.blockpos = inflate(file);
		pos.pos_in_block = 0;
		return pos;
	}
	int length = current_table.length + 1; //多余的一位放在开头，表示是否有效extra bit set in the start for checking validity
	int offset = current_table.blockNum - 1;//insert the newest ele into the last
	int No = getaBlock(file, offset);
	int rec = blockSize / length;
	for (int offset = 0; offset < rec; offset++) {
		int position = offset * length;
		char isEmpty = blocks[No].vals[position];//check first bit validity to judge if it has content
		if (isEmpty == Empty) {//find an empty space
			pos.blockpos = No;
			pos.pos_in_block = position;
			return pos;
		}
	}
	//this block is full，open another block
	pos.blockpos = inflate(file);
	pos.pos_in_block = 0;
	return pos;
}

unsigned int buffermanager::inflate(string fname)
{
	int num = find_Empty_Buffer(fname);
	blocks[num].init();
	blocks[num].valid = 1;
	blocks[num].dirty = 1;
	blocks[num].fname = fname;
	blocks[num].offset = current_table.blockNum++;
	blocks[num].last_vis_time = clock();
	changeblock();
	return num;
}
//getaBlock
unsigned int buffermanager::getaBlock(std::string fname, int offset)
{
	int num = find_assign_buffer(fname, offset);
	if(num == -1) //don't have this assign num
	{
		num = find_Empty_Buffer(fname);
		read(fname, offset, num);
	}
	return num;
}
//可以试着合并
unsigned int buffermanager::find_Empty_Buffer(std::string fname)
{
	int i;
	int fewest_vis_block = 0;
	for (i = 0; i < maxBlocks; i++) {
		if (!blocks[i].valid) {
			blocks[i].init();
			blocks[i].valid = 1;
			return i;
		}
		else if (blocks[fewest_vis_block].last_vis_time > blocks[i].last_vis_time && blocks[i].fname != fname) {
			fewest_vis_block = i;
		}
	}

	writetoFile(fewest_vis_block); //getaBlock
	blocks[fewest_vis_block].valid = 1;
	return fewest_vis_block;
}
void buffermanager::writetoFile(int num)
{
	if (!blocks[num].dirty) return;
	string filename = blocks[num].fname;
	FILE *fp;
	if ((fp = fopen(filename.c_str(), "r+b")) == NULL) {
		cout << "Open file error!" << endl;
		return;
	}
	fseek(fp, blockSize*blocks[num].offset, SEEK_SET);
	fwrite(blocks[num].vals, blockSize, 1, fp);
	blocks[num].init();
	fclose(fp);
}

void buffermanager::changeblock()
{
    string file = "Table_" + current_table.table_name + "_schema";
    int No = getaBlock(file, 0);
    char* begin = blocks[No].vals;
    int pos = 0;
    pos = pos + sizeof(int);
    pos = pos + sizeof(int);
    memcpy(&begin[pos], &current_table.blockNum, sizeof(int));
    write(No);
}