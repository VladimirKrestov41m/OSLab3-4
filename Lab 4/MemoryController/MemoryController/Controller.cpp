#include<stdio.h>
#include<Windows.h>
#include<stdlib.h>
#include<string>

#define T 100

struct TLB
{
	short int PageNumber;
	short int FrameNumber;
} *TLB_Table;

struct Page
{
	int LastTime;
	bool R;
	short int Frame;
} *PageTable;

unsigned char* PhysicalMemory;
short int _pageAddress;
short int _symbolAddress;

int InitFiles();
void InitMemory();
void FreeResources();
void ResetR_and_Times();
short int GetFrameFromVirtualMemory(unsigned short int, unsigned short int, short int);
unsigned char GetSymbolFromPage(unsigned char*, short int);
void GetPageFromFile(FILE*, unsigned short int, short int, unsigned short int);
void RemoveFrame(short int);
short int GetSymbol(FILE*);
void SkipSymbols(FILE*, int);
short int _currentTLBPos = 0,
_currentPhysicalMemoryPos = 0;
FILE* sourceFile, * addressesFile, * resultFile;
unsigned short int _pageSize = 256;
unsigned short int _physicalMemorySize = 128;
unsigned short int _pageTableSize = 256;
unsigned short int _TLB_Size = 16;

int main()
{
	system("chcp 1251");
	short int frameNumber = 0;

	if (InitFiles() == 1)
	{
		printf("File exception!\n");
		return 1;
	}

	InitMemory();
	printf("Initialization successful.\n\n");

	_symbolAddress = GetSymbol(addressesFile);
	_pageAddress = GetSymbol(addressesFile);

	int timer = 0;

	while (_pageAddress != EOF)
	{
		if (timer % T == 0)
		{
			ResetR_and_Times();
		}

		frameNumber = GetFrameFromVirtualMemory(_TLB_Size, _pageTableSize, _pageAddress);

		if (frameNumber == -1)
		{
			GetPageFromFile(sourceFile, _TLB_Size, _pageAddress, _pageSize);
			frameNumber = PageTable[_pageAddress].Frame;
		}

		fputc(GetSymbolFromPage(&PhysicalMemory[frameNumber], _symbolAddress), resultFile);

		SkipSymbols(addressesFile, 2);

		_symbolAddress = GetSymbol(addressesFile);
		_pageAddress = GetSymbol(addressesFile);

		timer++;
	}

	printf("\n\nProcess ended.\n");

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

	PageTable = (Page*)malloc(_pageTableSize * sizeof(Page));
	for (int i = 0; i < _pageTableSize; i++) {
		PageTable[i].Frame = -1;
		PageTable[i].LastTime = 0;
		PageTable[i].R = false;
	}

	TLB_Table = (TLB*)malloc(_TLB_Size * sizeof(TLB));
	for (int i = 0; i < _TLB_Size; i++) {
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

void ResetR_and_Times()
{
	//PageTable
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
	return PageTable[number].Frame;
}

unsigned char GetSymbolFromPage(unsigned char* page, short int number)
{
	unsigned char result = page[number];
	printf("%c", result);
	return result;
}

void GetPageFromFile(FILE* f, unsigned short int TLB_Size, short int pageNumber, unsigned short int pageSize)
{
	fpos_t filePos = pageNumber * pageSize;
	rewind(f);
	fsetpos(f, &filePos);

	short int position = 0;
	short int goToFreeTLB = -1;

	if (_currentPhysicalMemoryPos > _physicalMemorySize)
	{
		_currentPhysicalMemoryPos = 0;
	}

	if (_currentTLBPos > TLB_Size)
	{
		_currentTLBPos = 0;
	}

	position = _currentPhysicalMemoryPos * pageSize;
	RemoveFrame(position);
	fread(&PhysicalMemory[position], sizeof(unsigned char), pageSize, f);
	PageTable[pageNumber].Frame = position;
	TLB_Table[_currentTLBPos].PageNumber = pageNumber;
	TLB_Table[_currentTLBPos].FrameNumber = position;
	_currentTLBPos++;
	_currentPhysicalMemoryPos++;
}

void RemoveFrame(short int position)
{
	for (int i = 0; i < _pageTableSize; i++)
	{
		if (PageTable[i].Frame == position)
		{
			PageTable[i].Frame = -1;
			break;
		}
	}

	for (int i = 0; i < _TLB_Size; i++)
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

void SkipSymbols(FILE* f, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (GetSymbol(f) == EOF)
		{
			break;
		}
	}
}