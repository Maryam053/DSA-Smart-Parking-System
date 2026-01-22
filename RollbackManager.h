#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

#include "ParkingRequest.h"
#include <string>
using namespace std;

struct AllocationOperation {
    int requestID;
    string vehicleID;
    int allocatedSlotID;
    int allocatedZoneID;
    int requestTime;
    RequestState previousState;
    RequestState newState;
    
    AllocationOperation() {
        requestID = -1;
        vehicleID = "";
        allocatedSlotID = -1;
        allocatedZoneID = -1;
        requestTime = 0;
        previousState = REQUESTED;
        newState = REQUESTED;
    }
    
    AllocationOperation(int reqID, string vehID, int slotID, int zoneID, 
                       int time, RequestState prevState, RequestState newSt) {
        requestID = reqID;
        vehicleID = vehID;
        allocatedSlotID = slotID;
        allocatedZoneID = zoneID;
        requestTime = time;
        previousState = prevState;
        newState = newSt;
    }
};

struct StackNode {
    AllocationOperation operation;
    StackNode* next;
    
    StackNode(const AllocationOperation& op) : operation(op), next(nullptr) {}
};

class RollbackManager {
private:
    StackNode* top;
    int size;
    int maxSize;

public:
    RollbackManager();
    RollbackManager(int maxSize);
    ~RollbackManager();
    
    void pushOperation(const AllocationOperation& operation);
    bool popOperation(AllocationOperation& operation);
    bool peekOperation(AllocationOperation& operation) const;
    
    int getSize() const;
    bool isEmpty() const;
    void clear();
};

#endif