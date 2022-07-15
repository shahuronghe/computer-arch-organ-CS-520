#include "project02.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Invalid Arguments. " << endl;
        exit(1);
    }
    Simple_Pipe simple_pipe;
    for (int i = 0; i < REG_COUNT; i++)
    {
        simple_pipe.registers[i] = 0;
    }

    const char *filePath = argv[1];
    unsigned char *fileBuf;
    FILE *file = NULL;

    // opening the file
    if ((file = fopen(filePath, "rb")) == NULL)
    {
        cout << "Could not open specified file" << endl;
        exit(2);
    }

    long pos = ftell(file);
    fseek(file, 0, 2);
    long fileSize = ftell(file);
    fseek(file, pos, 0);
    fileBuf = new unsigned char[fileSize];

    // reading the file
    fread(fileBuf, fileSize, 1, file);

    // adding execution time for fetch and decode
    execution_time += 2;

    // parsing the binary file for execution and writeback to register file
    for (int i = 0; i < fileSize; i += 4)
    {
        int opcode = (int)fileBuf[i + 3];
        int dest = (int)fileBuf[i + 2];
        int left = (int)fileBuf[i + 1];
        int right = (int)fileBuf[i];
        switch (opcode)
        {
        case 0x00:
            // SET operation.
            simple_pipe.registers[dest] = left;
            execution_time++;
            request_done++;
            break;

        case 0x10:
            // ADD operation Type 1.
            simple_pipe.registers[dest] = simple_pipe.registers[left] + simple_pipe.registers[right];
            execution_time++;
            request_done++;
            break;

        case 0x11:
            // ADD operation Type 2.
            simple_pipe.registers[dest] = simple_pipe.registers[left] + right;
            execution_time++;
            request_done++;
            break;

        case 0x20:
            // SUB operation Type 1.
            simple_pipe.registers[dest] = simple_pipe.registers[left] - simple_pipe.registers[right];
            execution_time++;
            request_done++;
            break;

        case 0x21:
            // SUB operation Type 2.
            simple_pipe.registers[dest] = simple_pipe.registers[left] - right;
            execution_time++;
            request_done++;
            break;

        case 0x30:
            // MUL operation Type 1.
            simple_pipe.registers[dest] = simple_pipe.registers[left] * simple_pipe.registers[right];
            execution_time += 2;
            request_done++;
            break;

        case 0x31:
            // MUL operation Type 2.
            simple_pipe.registers[dest] = simple_pipe.registers[left] * right;
            execution_time += 2;
            request_done++;
            break;

        case 0x40:
            // DIV operation Type 1.
            simple_pipe.registers[dest] = simple_pipe.registers[left] / simple_pipe.registers[right];
            execution_time += 4;
            request_done++;
            break;

        case 0x41:
            // DIV operation Type 2.
            simple_pipe.registers[dest] = simple_pipe.registers[left] / right;
            execution_time += 4;
            request_done++;
            break;
        }
    }

    //clearing resources.
    delete[] fileBuf;
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
