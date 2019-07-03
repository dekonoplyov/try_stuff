#include <array>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

/*
Output of
> objdump -M intel -D chunk.o 

chunk.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:   48 c7 c0 01 00 00 00    mov    rax,0x1
   7:   48 c7 c7 01 00 00 00    mov    rdi,0x1
   e:   48 8d 35 0a 00 00 00    lea    rsi,[rip+0xa]        # 0x1f
  15:   48 c7 c2 11 00 00 00    mov    rdx,0x11
  1c:   0f 05                   syscall 
  1e:   c3                      ret    
  1f:   48                      rex.W
  20:   65 6c                   gs ins BYTE PTR es:[rdi],dx
  22:   6c                      ins    BYTE PTR es:[rdi],dx
  23:   6f                      outs   dx,DWORD PTR ds:[rsi]
  24:   2c 20                   sub    al,0x20
  26:   59                      pop    rcx
  27:   6f                      outs   dx,DWORD PTR ds:[rsi]
  28:   75 72                   jne    0x9c
  2a:   20 4e 61                and    BYTE PTR [rsi+0x61],cl
  2d:   6d                      ins    DWORD PTR es:[rdi],dx
  2e:   65 0a 00                or     al,BYTE PTR gs:[rax]

 */

std::array<uint8_t, 4> getEncodedMessageSize(size_t messageSize)
{
    return {
        static_cast<uint8_t>(messageSize & 0xff),
        static_cast<uint8_t>((messageSize & 0xff00) >> 8),
        static_cast<uint8_t>((messageSize & 0xff0000) >> 16),
        static_cast<uint8_t>((messageSize & 0xff000000) >> 24)};
}

std::vector<uint8_t> createMachineCode(const std::string& message)
{
    const auto encodedMessageSize = getEncodedMessageSize(message.size());
    std::vector<uint8_t> machineCode{
#ifdef __linux__
        0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, // Store the "write" system call number 0x01 for Linux
#elif __APPLE__
        0x48, 0xc7, 0xc0, 0x04, 0x00, 0x00, 0x02, // Store the "write" system call number 0x02000004 for macOS
#endif
        0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00, // Store stdin file descriptor 0x01
        0x48, 0x8d, 0x35, 0x0a, 0x00, 0x00, 0x00, // Store the location of the string to write (3 instructions from the current instruction pointer)
        // Store the length of the string
        0x48, 0xc7, 0xc2, encodedMessageSize[0], encodedMessageSize[1], encodedMessageSize[2], encodedMessageSize[3],
        0x0f, 0x05, // Execute the system call
        0xc3 // return instruction
    };

    for (auto c : message) {
        machineCode.emplace_back(c);
    }

    return machineCode;
}

void printMachineCode(const std::vector<uint8_t>& machineCode)
{
    size_t konto = 0;
    std::cout << "Machine code generated:\n\n";
    std::cout << std::hex;
    for (auto b : machineCode) {
        std::cout << static_cast<int>(b) << ' ';
        ++konto;
        if (konto % 7 == 0) {
            std::cout << std::endl;
        }
    }

    std::cout << std::dec << "\n\n";
}

size_t estimateMemorySize(size_t machineCodeSize)
{
    // Get the machine page size
    size_t pageSizeMultiple = sysconf(_SC_PAGE_SIZE);

    auto factor = machineCodeSize / pageSizeMultiple;
    auto requiredSize = factor * pageSizeMultiple;
    if (requiredSize >= machineCodeSize) {
        return requiredSize;
    } else {
        return requiredSize + pageSizeMultiple;
    }
}

void callMachineCode(const std::vector<uint8_t>& machineCode)
{
    // Get the required memory size for mmap
    const auto requiredMemorySize = estimateMemorySize(machineCode.size());
    const auto mmapProt = PROT_READ | PROT_WRITE | PROT_EXEC;
    const auto mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
    uint8_t* mem = static_cast<uint8_t*>(mmap(NULL, requiredMemorySize, mmapProt, mmapFlags, -1, 0));
    if (mem == MAP_FAILED) {
        std::cerr << "Can't allocate memory\n";
        std::exit(1);
    }
    // Copy machine code to executable memory
    for (size_t i = 0; i < machineCode.size(); ++i) {
        mem[i] = machineCode[i];
    }

    void (*func)();
    // Cast the address of our generated code to a function pointer and call the function
    func = (void (*)())mem;
    func();

    // Release the mmaped memory
    munmap(mem, requiredMemorySize);
}

int main(int argc, char const* argv[])
{
    std::cout << "What's your name?\n";
    std::string name;
    std::getline(std::cin, name);
    std::string greetings = "Hello, " + name + "\n";

    const auto machineCode = createMachineCode(greetings);
    printMachineCode(machineCode);

    callMachineCode(machineCode);

    return 0;
}
