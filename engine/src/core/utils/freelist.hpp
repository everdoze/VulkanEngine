#pragma once

#include "defines.hpp"

namespace Engine {

    class FreelistNode {
        public:
            FreelistNode(u32 offset, u32 size);
            FreelistNode(u32 offset, u32 size, b8 is_free);
            ~FreelistNode();

            u32 GetMemoryOffset() { return offset; };
            u32 GetSize() { return size; };

            void FreeBlock();

            b8 IsFree() { return is_free; };

        protected:
            FreelistNode* Next() { return next; };
            FreelistNode* Previous() { return prev; };
            b8 InsertBefore(FreelistNode* node);

            void SetSize(u32 size) { this->size = size; };
            void SetFree(b8 is_free) { this->is_free = is_free; };

            void SetNext(FreelistNode* next) { this->next = next; };
            void SetPrevious(FreelistNode* prev) { this->prev = prev; };

            FreelistNode* next;
            FreelistNode* prev;
            b8 is_free;
            u32 size;
            u32 offset;
        
        friend class Freelist;
    };

    class Freelist {
        public:
            Freelist(u32 total_size);
            ~Freelist();

            FreelistNode* AllocateBlock(u32 size);
            void Clear();
            u32 FreeSpace();

        protected:
            FreelistNode* first_node;

            u32 total_size;
    };

}