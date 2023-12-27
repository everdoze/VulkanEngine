#pragma once

#include "defines.hpp"

namespace Engine {

    class FreelistNode {
        public:
            FreelistNode(u64 offset, u64 size);
            FreelistNode(u64 offset, u64 size, b8 is_free);
            FreelistNode(u64 offset, u64 size, b8 is_free, void* memory);
            ~FreelistNode();

            u64 GetMemoryOffset() { return offset; };
            u64 GetSize() { return size; };
            void* GetMemory() { return memory; };

            void FreeBlock();

            b8 IsFree() { return is_free; };

        protected:
            FreelistNode* Next() { return next; };
            FreelistNode* Previous() { return prev; };
            b8 InsertBefore(FreelistNode* node);
            void InsertAfter(FreelistNode* node);

            void SetSize(u64 size) { this->size = size; };
            void SetFree(b8 is_free) { this->is_free = is_free; };

            void SetNext(FreelistNode* next) { this->next = next; };
            void SetPrevious(FreelistNode* prev) { this->prev = prev; };

            void Initialize();

            FreelistNode* next;
            FreelistNode* prev;
            b8 is_free;
            u64 size;
            u64 offset;
            void* memory;
        
        friend class Freelist;
    };

    class Freelist {
        public:
            Freelist(u64 total_size);
            Freelist(u64 total_size, void* memory);
            ~Freelist();

            FreelistNode* AllocateBlock(u64 size);
            b8 FreeByOffset(u64 offset);
            void Clear();
            u64 FreeSpace();

            // FreelistNode* InsertByOffset(u64 offset, u64 size);

        protected:
            FreelistNode* first_node;

            u64 total_size;
    };

}