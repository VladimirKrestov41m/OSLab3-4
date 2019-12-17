#include<stdio.h>
#include<Windows.h>
#include<stdlib.h>
#include<string>

using namespace std;

short int pageAddress;
short int symbolAddress;

struct TLB
{
	short int PageNumber;
	short int FrameNumber;
} *TLB_Table;

unsigned char* PhysicalMemory;
short int* PageTable;

int InitFiles();
void InitMemory();
void FreeResources();
short int GetFrameFromVirtualMemory(unsigned short int, unsigned short int, short int);
unsigned char GetSymbolFromPage(unsigned char*, short int);
void GetPageFromFile(FILE*, unsigned short int, short int, unsigned short int);
void RemoveFrame(short int position);
short int GetSymbol(FILE*);
short int SkipSymbols(FILE*, int);

short int currentItemOfTLB = 0, currentItemOfPageTable = 0;
int currentItemOfphysicalMemory = 0;

FILE* sourceFile, * addressesFile, * resultFile;
unsigned short int _pageSize = 256;
unsigned short int _physicalMemorySize = 128;
unsigned short int _pageTableSize = 256;
unsigned short int _pagesCount = 256;
unsigned short int TLB_Size = 16;

short int myError = 0;

int main()
{
	short int frameNumber = 0;

	if (InitFiles() == 1)
	{
		printf("File exception");
		return 1;
	}

	InitMemory();
	printf("Initialization successful.\n");





	symbolAddress = GetSymbol(addressesFile);
	pageAddress = GetSymbol(addressesFile);

	while (pageAddress != EOF)
	{
		frameNumber = GetFrameFromVirtualMemory(TLB_Size, _pageTableSize, pageAddress);

		if (frameNumber == -1)
		{
			GetPageFromFile(sourceFile, TLB_Size, pageAddress, _pageSize);
			frameNumber = PageTable[pageAddress];
		}

		fputc(GetSymbolFromPage(&PhysicalMemory[frameNumber], symbolAddress), resultFile);

		if (SkipSymbols(addressesFile, 2) == -1)
		{
			break;
		}

		symbolAddress = GetSymbol(addressesFile);
		pageAddress = GetSymbol(addressesFile);
	}

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

short int GetFrameFromVirtualMemory(unsigned short int TLB_Size, unsigned short int sizeOfPageTable, short int number)
{
	// Does TLB_Table contain page number?
	for (int i = 0; i < TLB_Size; i++)
	{
		if (TLB_Table[i].PageNumber == number)
		{
			return TLB_Table[i].FrameNumber;
		}
	}

	// Does PageTable contain page number?
	return PageTable[number];
}

unsigned char GetSymbolFromPage(unsigned char* page, short int number)
{
	return page[number];
}

void GetPageFromFile(FILE* f, unsigned short int TLB_Size, short int pageNumber, unsigned short int pageSize)
{
	fpos_t filePos = pageNumber * pageSize;
	rewind(f);
	fsetpos(f, &filePos);

	short int position = 0;
	short int goToFreeTLB = -1;

	if (currentItemOfphysicalMemory > _physicalMemorySize)
	{
		currentItemOfphysicalMemory = 0;
	}

	if (currentItemOfTLB > TLB_Size)
	{
		currentItemOfTLB = 0;
	}

	position = currentItemOfphysicalMemory * pageSize;
	RemoveFrame(position);
	fread(&PhysicalMemory[position], sizeof(unsigned char), pageSize, f);
	PageTable[pageNumber] = position;
	TLB_Table[currentItemOfTLB].PageNumber = pageNumber;
	TLB_Table[currentItemOfTLB].FrameNumber = position;
	currentItemOfTLB++;
	currentItemOfphysicalMemory++;
}

void RemoveFrame(short int position)
{
	for (int i = 0; i < _pageTableSize; i++)
	{
		if (PageTable[i] == position)
		{
			PageTable[i] = -1;
			break;
		}
	}

	for (int i = 0; i < TLB_Size; i++)
	{
		if (TLB_Table[i].FrameNumber == position)
		{
			TLB_Table[i].PageNumber = -1;
			TLB_Table[i].FrameNumber = -1;
			break;
		}
	}
}

short int GetSymbol(FILE* f)
{
	if (!feof(f))
	{
		return getc(f);
	}
	else
	{
		return EOF;
	}
}

short int SkipSymbols(FILE* f, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (GetSymbol(f) == EOF)
		{
			return EOF;
		}
	}

	return 0;
}