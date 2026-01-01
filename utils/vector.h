#pragma once
#include "kheap.h"

// The "T" stands for "Type" (int, char, void*, etc.)
template <typename T>
class Vector {
private:
    T* buffer;          // The raw array on the Heap
    size_t length;      // How many items we have
    size_t capacity;    // How many items we can fit before resizing

public:
    // 1. Constructor
    Vector() {
        length = 0;
        capacity = 2;   // Start small
        buffer = (T*)KHeap::allocate(sizeof(T) * capacity); // Manual allocation
    }

    // 2. Destructor (Cleanup)
    ~Vector() {
        if (buffer != nullptr) {
            // Note: In a full OS, we would call destructors for each item here.
            // For now, we just free the memory to keep it simple.
            KHeap::free(buffer);
        }
    }

    // 3. Add an item to the end
    void push_back(T element) {
        // If full, resize!
        if (length >= capacity) {
            size_t new_capacity = capacity * 2;
            
            // Allocate new bigger block
            T* new_buffer = (T*)KHeap::allocate(sizeof(T) * new_capacity);

            // Copy old items manually
            for (size_t i = 0; i < length; i++) {
                new_buffer[i] = buffer[i];
            }

            // Delete old block
            KHeap::free(buffer);

            // Switch to new block
            buffer = new_buffer;
            capacity = new_capacity;
        }

        // Add the new item
        buffer[length] = element;
        length++;
    }

    // 4. Remove the last item
    void pop_back() {
        if (length > 0) {
            length--;
        }
    }

    // 5. Access item at index (read/write)
    // Allows usage like: myVec[0] = 5;
    T& operator[](size_t index) {
        return buffer[index];
    }

    // 6. Get size
    size_t size() const {
        return length;
    }
};