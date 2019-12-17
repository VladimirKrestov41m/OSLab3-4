#include<stdio.h>
#include<Windows.h>
#include<stdlib.h>
#include<string>

struct Position {
	short int pageAddress;
	short int symbolAddress;
};

struct TLB {
	short int PageNumber;
	short int FrameNumber;
} *TLB_Table;

unsigned char* PhysicalMemory;
short int* PageTable;

int InitFiles();
void InitMemory();
void FreeResources();




short int JumpOverCountByte(FILE*, int);

short int GetByte(FILE*);
short int CheckPageinphysicalMemory(unsigned short int, unsigned short int, short int);
unsigned char GetSymbolFromPage(unsigned char*, short int);
void resetPointer(short int);
bool GetPageFromFile(FILE*, unsigned short int, short int, unsigned short int);

short int currentItemOfTLB = 0, currentItemOfPageTable = 0, /*maxCurrentOfTLB = 0,*/ maxCurrentOfPageTable = 0;
int currentItemOfphysicalMemory = 0, maxCurrentphysicalMemory = 0;

FILE* sourceFile, * addressesFile, * resultFile;
unsigned short int _pageSize = 256;
unsigned short int _physicalMemorySize = 128;
unsigned short int _pageTableSize = 256;
unsigned short int _pagesCount = 256;
unsigned short int TLB_Size = 16;

short int myError = 0;
Position pos;
short int delta = 0;

int main() {
	if (InitFiles() == 1)
	{
		printf("File exception");
		return 1;
	}

	InitMemory();
	printf("Initialization successful.\n");







	/*maxCurrentOfTLB = TLB_Size;
	maxCurrentOfPageTable = _pageTableSize;
	maxCurrentphysicalMemory = _physicalMemorySize;*/

	myError = pos.symbolAddress = GetByte(addressesFile);
	myError = pos.pageAddress = GetByte(addressesFile);

	while (myError != -1) {
		
		delta = CheckPageinphysicalMemory(TLB_Size, _pageTableSize, pos.pageAddress);
		if (delta != -1) {
			fputc(GetSymbolFromPage(&PhysicalMemory[delta], pos.symbolAddress), resultFile);
		}
		else {
			if (GetPageFromFile(sourceFile, TLB_Size, pos.pageAddress, _pageSize)) {
				fputc(GetSymbolFromPage(&PhysicalMemory[PageTable[pos.pageAddress]], pos.symbolAddress), resultFile);
			}
			else {
				myError = -2;
				break;
			}
		}

		if (JumpOverCountByte(addressesFile, 2) == -1)
			break;

		myError = pos.symbolAddress = GetByte(addressesFile);
		myError = pos.pageAddress = GetByte(addressesFile);
	}








	if (myError == -1)
		printf("Memory addressing exception!\n");
	if (myError == -2)
		printf("Memory reading exception!\n");
	
	system("pause");
	return 0;
}

int InitFiles()
{
	sourceFile = fopen("sourcePage.txt", "r");

	if (sourceFile == NULL)
	{
		return 1;
	}

	addressesFile = fopen("sourceAddresses.dat", "rb");

	if (addressesFile == NULL)
	{
		fclose(sourceFile);
		return 1;
	}

	resultFile = fopen("resultText.txt", "w+");

	if (resultFile == NULL)
	{
		fclose(sourceFile);
		fclose(addressesFile);
		return 1;
	}
}

void InitMemory()
{
	PhysicalMemory = (unsigned char*)malloc((_pageSize * _physicalMemorySize * sizeof(unsigned char)) + 1);
	memset(PhysicalMemory, '\0', (_pageSize * _physicalMemorySize * sizeof(unsigned char)) + 1);

	PageTable = (short int*)malloc(_pageTableSize * sizeof(short int));
	for (int i = 0; i < _pageTableSize; i++) {
		PageTable[i] = -1;
	}

	TLB_Table = (TLB*)malloc(TLB_Size * sizeof(TLB));
	for (int i = 0; i < TLB_Size; i++) {
		TLB_Table->PageNumber = -1;
		TLB_Table->FrameNumber = -1;
	}
}

void FreeResources()
{
	fclose(resultFile);
	fclose(sourceFile);
	fclose(addressesFile);

	free(PhysicalMemory);
	free(PageTable);
	free(TLB_Table);
}

short int GetByte(FILE* f) {
	if (!feof(f))
		return getc(f);
	else
		return -1;
}

short int JumpOverCountByte(FILE* f, int count) {
	for (int i = 0; i < count; i++) {
		if (GetByte(f) == -1)
			return -1;
	}
	return 0;
}

short int CheckPageinphysicalMemory(unsigned short int TLB_Size, unsigned short int sizeOfPageTable, short int number) {

	for (int i = 0; i < TLB_Size; i++)
		if (TLB_Table[i].PageNumber == number)
			return TLB_Table[i].FrameNumber;
	if (PageTable[number] != -1)
		return PageTable[number];
	return -1;
}

unsigned char GetSymbolFromPage(unsigned char* page, short int number) {
	return page[number];
}

void resetPointer(short int position) {
	for (int i = 0; i < _pageTableSize; i++)
		if (PageTable[i] == position) {
			PageTable[i] = -1;
			break;
		}
	for (int i = 0; i < TLB_Size; i++)
		if (TLB_Table[i].FrameNumber == position) {
			TLB_Table[i].PageNumber = -1;
			TLB_Table[i].FrameNumber = -1;
			break;
		}
}

short int findAFreeTLB() {
	for (int i = 0; i < TLB_Size; i++)
		if (TLB_Table[i].PageNumber == -1)
			return i;
		else - 1;
}
//Ќе затираютс€ ссылки удаленных данных из пам€ти
bool GetPageFromFile(FILE* f, unsigned short int TLB_Size, short int numberOfPage, unsigned short int sizeOfPage) {
	fpos_t filePos = numberOfPage * sizeOfPage;
	unsigned short int e = 0;
	short int position = 0;
	short int goToFreeTLB = -1;
	rewind(f);
	fsetpos(f, &filePos);
	if (currentItemOfphysicalMemory > _physicalMemorySize)
		currentItemOfphysicalMemory = 0;
	if (currentItemOfTLB > TLB_Size)
		currentItemOfTLB = 0;
	position = currentItemOfphysicalMemory * sizeOfPage;
	//if((goToFreeTLB = findAFreeTLB()) == -1)
	resetPointer(position);
	e = fread(&PhysicalMemory[position], sizeof(unsigned char), sizeOfPage, f);
	PageTable[numberOfPage] = position;
	TLB_Table[currentItemOfTLB].PageNumber = numberOfPage;
	TLB_Table[currentItemOfTLB].FrameNumber = position;
	currentItemOfTLB++;
	currentItemOfphysicalMemory++;
	if (e < sizeOfPage)
		return false;
	return true;
}