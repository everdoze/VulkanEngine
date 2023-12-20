#include "freelist.hpp"
#include "core/logger/logger.hpp"

namespace Engine {

    FreelistNode::FreelistNode(u32 offset, u32 size) {
        this->prev = nullptr;
        this->next = nullptr;
        this->offset = offset;
        this->size = size;
        this->is_free = true;
    };

    FreelistNode::FreelistNode(u32 offset, u32 size, b8 is_free) {
        this->prev = nullptr;
        this->next = nullptr;
        this->offset = offset;
        this->size = size;
        this->is_free = is_free;
    };

    void FreelistNode::FreeBlock() {
        is_free = true;
        b8 delete_this = false;
        if (this->prev && this->prev->IsFree()) {
            if (this->prev->prev) {
                this->offset = this->offset - this->prev->size;
                this->size += this->prev->size;
                this->prev->prev->next = this;
                FreelistNode* buffer = this->prev->prev;
                delete this->prev;
                this->prev = buffer;
            } else {
                this->prev->size += this->size;
                if (this->next) {
                    if (this->next->is_free) {
                        this->prev->size += this->next->size;
                        if (this->next->next) {
                            this->prev->next = this->next->next;
                            this->next->prev = this->prev;
                        } else {
                            this->prev->next = nullptr;
                        }
                        delete this->next;
                    } else {
                        this->next->prev = this->prev;
                        this->prev->next = this->next;
                    }
                    
                }
                return delete this;
            }
        }
        if (this->next && this->next->IsFree()) {
            this->size += this->next->size;
            FreelistNode* buffer = this->next->next;
            delete this->next;
            this->next = buffer;
        }
    };

    b8 FreelistNode::InsertBefore(FreelistNode* node) {
        if (node) {
            if (this->Previous()) {
                this->prev->next = node;
                node->prev = this->prev;
                node->next = this;
                this->prev = node;
                return false;
            } else {
                node->prev = this->prev;
                node->next = this;
                if (this->prev) {
                    this->prev->next = node;
                    this->prev = node;
                } else {
                    this->prev = node;
                }
                
                return true;
            }
        }
        return false;
    };
    
    FreelistNode::~FreelistNode() {
        this->prev = nullptr;
        this->next = nullptr;
        this->size = 0;
        this->offset = 0;
    };

    Freelist::Freelist(u32 total_size) {
        this->total_size = total_size;
        first_node = new FreelistNode(0, total_size);
    };

    Freelist::~Freelist() {
        FreelistNode* current = first_node;
        FreelistNode* next = current->Next();
        while (current) {
            delete current;
            current = next;
            if (current) {
                next = current->Next();
            }
        }
    };


    FreelistNode* Freelist::AllocateBlock(u32 size) {
        FreelistNode* current = first_node;
        while (current) {
           if (current->IsFree()) {
                if (current->GetSize() > size) {
                    FreelistNode* occupied = new FreelistNode(current->GetMemoryOffset(), size, false);
                    current->offset += size;
                    current->size -= size;
                    if (current->InsertBefore(occupied)) { // True means that inserted is a first item in the list
                        first_node = occupied;
                    }
                    return occupied;
                }
           }
           current = current->Next();
        }

        u32 free_space = this->FreeSpace();
        WARN("Freelist::AllocateBlock - not enougth memory to fit the request ( requested: %uB avaliable: %lluB ).", size, free_space);
        return nullptr;
    };

    
    void Freelist::Clear() {
        FreelistNode* next = first_node->Next();
        FreelistNode* buffer = next;
        while (next) {
            next = next->Next();
            delete buffer;
            buffer = next;
        }
        first_node->SetNext(nullptr);
        first_node->SetSize(this->total_size);
        first_node->SetFree(true);
    };

    u32 Freelist::FreeSpace() {
        u32 free_space = total_size;
        FreelistNode* node = first_node;
        while (node) {
            if (!node->IsFree()) {
                free_space -= node->GetSize();
            }
            node = node->Next();
        }
        return free_space;
    };

}