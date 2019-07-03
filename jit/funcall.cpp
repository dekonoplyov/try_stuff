#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

struct MemoryPages {
    MemoryPages(size_t pagesRequested = 1)
    {
        pageSize = sysconf(_SC_PAGE_SIZE);
        pages = pagesRequested;
        const auto mmapProt = PROT_READ | PROT_WRITE | PROT_EXEC;
        const auto mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
        mem = static_cast<uint8_t*>(mmap(NULL, pages * pageSize, mmapProt, mmapFlags, -1, 0));
        if (mem == MAP_FAILED) {
            throw std::runtime_error{"Can't allocate memory\n"};
        }
    }

    ~MemoryPages()
    {
        munmap(mem, pages * pageSize);
    }

    void push(uint8_t data)
    {
        checkAvailableSpace(sizeof(data));
        mem[position] = data;
        ++position;
    }

    void push(void (*f)())
    {
        size_t fnAddress = reinterpret_cast<size_t>(f);
        checkAvailableSpace(sizeof(fnAddress));
        std::memcpy((mem + position), &fnAddress, sizeof(fnAddress));
        position += sizeof(fnAddress);
    }

    void push(const std::vector<uint8_t>& v)
    {
        checkAvailableSpace(v.size());
        std::memcpy((mem + position), v.data(), v.size());
        position += v.size();
    }

    // Check if it there is enough available space to push some data to the memory
    void checkAvailableSpace(size_t dataSize)
    {
        if (position + dataSize > pages * pageSize) {
            throw std::runtime_error("Not enough virtual memory allocated!");
        }
    }

    void showMemory()
    {
        std::cout << "\nMemory content: " << position << "/" << pages * pageSize << " bytes used\n";
        std::cout << std::hex;
        for (size_t i = 0; i < position; ++i) {
            std::cout << "0x" << (int)mem[i] << ' ';
            if (i % 16 == 0 && i > 0) {
                std::cout << '\n';
            }
        }
        std::cout << std::dec << "\n\n";
    }

    uint8_t* mem; // Pointer to the start of the executable memory
    size_t pageSize; // OS defined memory page size (typically 4096 bytes)
    size_t pages = 0; // no of memory pages requested from the OS
    size_t position = 0; // current position to the non used memory space
};

// Global vector that is modified by test()
std::vector<int> a{1, 2, 3};

// Function to be called from our generated machine code
void test()
{
    printf("Ohhh, boy ...\n");
    for (auto& e : a) {
        e -= 5;
    }
}

namespace AssemblyChunks {

const std::vector<uint8_t> functionPrologue{
    0x55, // push rbp
    0x48, 0x89, 0xe5, // mov rbp, rsp
};

const std::vector<uint8_t> functionEpilogue{
    0x5d, // pop rbp
    0xc3 // ret
};

} // namespace AssemblyChunks

int main(int argc, char const* argv[])
{
    MemoryPages mp;
    mp.push(AssemblyChunks::functionPrologue);

    // Push the call to the C++ function test (actually we push the address of the test function)
    mp.push(0x48);
    mp.push(0xb8);
    mp.push(test); // movabs rax, <function_address>

    mp.push(0xff);
    mp.push(0xd0); // call rax

    mp.push(AssemblyChunks::functionEpilogue);
    mp.showMemory();

    std::cout << "Global data initial values:\n";
    std::cout << a[0] << "\t" << a[1] << "\t" << a[2] << "\n";

    // Cast the address of our generated code to a function pointer and call the function
    void (*func)() = reinterpret_cast<void (*)()>(mp.mem);
    func();

    std::cout << "Global data after test() was called from the generated code:\n";
    std::cout << a[0] << "\t" << a[1] << "\t" << a[2] << "\n";

    return 0;
}
