#include <cassert>
#include <iostream>
#include <vector>

enum class ObjectType {
    INT,
    PAIR
};

struct Object {

    struct Pair {
        Object* head;
        Object* tail;
    };

    Object(ObjectType type)
        : next{nullptr}
        , marked{false}
        , type{type}
    {
    }

    Object* next;
    bool marked;
    ObjectType type;

    union {
        /* INT */
        int value;

        /* PAIR */
        Pair pair;
    };
};

struct VirtualMachine {
    VirtualMachine()
        : stack{std::vector<Object*>(STACK_MAX)}
        , stackSize{0}
        , firstObject{nullptr}
        , numObjects{0}
        , maxObjects{INITIAL_GC_THRESHOLD}
    {
    }

    void push(Object* o)
    {
        if (stackSize >= STACK_MAX) {
            throw std::runtime_error{"Stack overflow"};
        }
        stack[stackSize++] = o;
    }

    Object* pop()
    {
        if (stackSize <= 0) {
            throw std::runtime_error{"Stack underflow"};
        }
        return stack.at(--stackSize);
    }

    Object* newObject(ObjectType type)
    {
        if (numObjects >= maxObjects) {
            gc();
        }

        auto object = new Object(type);

        // insert to the list of allocated objects
        object->next = firstObject;
        firstObject = object;

        ++numObjects;
        return object;
    }

    void pushInt(int value)
    {
        auto object = newObject(ObjectType::INT);
        object->value = value;
        push(object);
    }

    Object* pushPair()
    {
        auto object = newObject(ObjectType::PAIR);
        object->pair.tail = pop();
        object->pair.head = pop();

        push(object);
        return object;
    }

    void markAll()
    {
        for (int i = 0; i < stackSize; ++i) {
            mark(stack.at(i));
        }
    }

    void mark(Object* object)
    {
        if (object->marked) {
            return;
        }

        object->marked = true;

        if (object->type == ObjectType::PAIR) {
            mark(object->pair.head);
            mark(object->pair.tail);
        }
    }

    void sweep()
    {
        Object** object = &firstObject;
        while (*object != nullptr) {
            if ((*object)->marked) {
                // This object was reached, so unmark it (for the next GC)
                // and move on to the next.
                (*object)->marked = false;
                object = &(*object)->next;
            } else {
                // This object wasn't reached, so remove it from the list
                // and free it.
                Object* unreached = *object;
                *object = unreached->next;
                delete unreached;
                --numObjects;
            }
        }
    }

    void gc()
    {
        int numObjects = this->numObjects;

        markAll();
        sweep();

        maxObjects = this->numObjects * 2;

        std::cout << "Collected: " << numObjects - this->numObjects
                  << ", remaining: " << this->numObjects << std::endl;
    }

    std::vector<Object*> stack;
    int stackSize;
    Object* firstObject;
    int numObjects;
    int maxObjects;

    static constexpr size_t STACK_MAX = 256;
    static constexpr size_t INITIAL_GC_THRESHOLD = 256;
};

void test1()
{
    std::cout << "Test 1: Objects on stack are preserved.\n";
    VirtualMachine vm;
    vm.pushInt(1);
    vm.pushInt(2);

    vm.gc();
    assert(vm.numObjects == 2);
}

void test2()
{
    std::cout << "Test 2: Unreached objects are collected.\n";
    VirtualMachine vm;
    vm.pushInt(1);
    vm.pushInt(2);
    vm.pop();
    vm.pop();

    vm.gc();
    assert(vm.numObjects == 0);
}

void test3()
{
    std::cout << "Test 3: Reach nested objects.\n";
    VirtualMachine vm;
    vm.pushInt(1);
    vm.pushInt(2);
    vm.pushPair();
    vm.pushInt(3);
    vm.pushInt(4);
    vm.pushPair();
    vm.pushPair();

    vm.gc();
    assert(vm.numObjects == 7);
}

void test4()
{
    std::cout << "Test 4: Handle cycles.\n";
    VirtualMachine vm;
    vm.pushInt(1);
    vm.pushInt(2);
    Object* a = vm.pushPair();
    vm.pushInt(3);
    vm.pushInt(4);
    Object* b = vm.pushPair();

    /* Set up a cycle, and also make 2 and 4 unreachable and collectible. */
    a->pair.tail = b;
    b->pair.tail = a;

    vm.gc();
    assert(vm.numObjects == 4);
}

int main(int argc, char const* argv[])
{
    test1();
    test2();
    test3();
    test4();
    return 0;
}
