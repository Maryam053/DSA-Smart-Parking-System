#include "RollbackManager.h"

RollbackManager::RollbackManager() {
    top = nullptr;
    size = 0;
    maxSize = 1000;
}

RollbackManager::RollbackManager(int maxSize) {
    top = nullptr;
    size = 0;
    this->maxSize = maxSize;
}

RollbackManager::~RollbackManager() {
    clear();
}

void RollbackManager::pushOperation(const AllocationOperation& operation) {
    if (size >= maxSize) {
        if (top != nullptr) {
            if (top->next == nullptr) {
                delete top;
                top = nullptr;
                size = 0;
            } else {
                StackNode* current = top;
                StackNode* prev = nullptr;
                while (current->next != nullptr) {
                    prev = current;
                    current = current->next;
                }
                delete current;
                if (prev != nullptr) {
                    prev->next = nullptr;
                }
                size--;
            }
        }
    }
    
    StackNode* newNode = new StackNode(operation);
    newNode->next = top;
    top = newNode;
    size++;
}

bool RollbackManager::popOperation(AllocationOperation& operation) {
    if (isEmpty()) {
        return false;
    }
    
    operation = top->operation;
    StackNode* temp = top;
    top = top->next;
    delete temp;
    size--;
    return true;
}

bool RollbackManager::peekOperation(AllocationOperation& operation) const {
    if (isEmpty()) {
        return false;
    }
    
    operation = top->operation;
    return true;
}

int RollbackManager::getSize() const {
    return size;
}

bool RollbackManager::isEmpty() const {
    return top == nullptr;
}

void RollbackManager::clear() {
    while (!isEmpty()) {
        StackNode* temp = top;
        top = top->next;
        delete temp;
    }
    size = 0;
}