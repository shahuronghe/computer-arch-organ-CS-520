#include "project02.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>
using namespace std;

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef unsigned char BYTE;

// Get the size of a file
long getFileSize(FILE *file)
{
	long lCurPos, lEndPos;
	lCurPos = ftell(file);
	fseek(file, 0, 2);
	lEndPos = ftell(file);
	fseek(file, lCurPos, 0);
	return lEndPos;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Invalid Arguments. " << endl;
    }
    Simple_Pipe simple_pipe;
    for (int i = 0; i < REG_COUNT; i++)
    {
        simple_pipe.registers[i] = 0;
    }

    const char *filePath = "trace1";	
	BYTE *fileBuf;
	FILE *file = NULL;
	if ((file = fopen(filePath, "rb")) == NULL)
		cout << "Could not open specified file" << endl;
	else
		cout << "File opened successfully" << endl;

	long fileSize = getFileSize(file);
	fileBuf = new BYTE[fileSize];
	fread(fileBuf, fileSize, 1, file);
	for (int i = 0; i < fileSize; i+=4){
        char buffer [1];
		sprintf(buffer, "%u", fileBuf[i]);
        printf("0x%x ", (int)fileBuf[i]);
        cout<<buffer<<" "; 

		sprintf(buffer, "%u", fileBuf[i + 1]);
        printf("0x%x ", (int)fileBuf[i + 1]);
        cout<<buffer << " "; 

        sprintf(buffer, "%u", fileBuf[i + 2]);
        printf("0x%x ", (int)fileBuf[i + 2]);
        cout<<buffer << " "; 

        sprintf(buffer, "%u", fileBuf[i + 3]);
        printf("0x%x ", (int)fileBuf[i + 3]);
        cout<<buffer<<endl; 
    }

	delete[]fileBuf;
    fclose(file); 





    


    simple_pipe.print_regs();
    std::cout << "Total execution cycles: " << execution_time << std::endl;
    std::cout << "\nIPC: " << (request_done / (double)execution_time) << std::endl
              << std::endl;

    return 0;
}

void Simple_Pipe::print_regs()
{
    printf("\nRegisters: \n");
    std::cout << "----------------------------------------" << std::endl;
    for (int i = 0; i < REG_COUNT; i += 2)
    {
        std::string regl("R");
        regl.append(std::to_string(i));
        regl.append(": ");

        std::string regr("R");
        regr.append(std::to_string(i + 1));
        regr.append(": ");
        if (i < 15)
        {
            std::cout << "  " << std::setiosflags(std::ios::left)
                      << std::setw(5) << regl << std::setw(10) << registers[i] << " |   "
                      << std::setw(5) << regr << std::setw(10) << registers[i + 1] << std::endl;
            std::cout << "----------------------------------------" << std::endl;
        }
        else
        {
            std::cout << "  " << std::setiosflags(std::ios::left)
                      << std::setw(5) << regl << std::setw(10) << registers[i] << " |   " << std::endl;
            std::cout << "----------------------------------------" << std::endl;
        }
    }
    printf("\n");
}
