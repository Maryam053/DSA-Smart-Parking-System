#include "ParkingSystem.h"
#include <iostream>

ParkingSystem::ParkingSystem(int zoneCount) {
    this->zoneCount = zoneCount;
    zones = new Zone[zoneCount];
    engine = nullptr;
    rollbackManager = new RollbackManager(1000);
    
    activeRequestCapacity = 10;
    activeRequests = new ParkingRequest[activeRequestCapacity];
    activeRequestCount = 0;
    
    historyHead = nullptr;
    historyCount = 0;
    nextRequestID = 1;
}

ParkingSystem::~ParkingSystem() {
    delete[] zones;
    delete engine;
    delete rollbackManager;
    delete[] activeRequests;
    
    while (historyHead != nullptr) {
        HistoryNode* temp = historyHead;
        historyHead = historyHead->next;
        delete temp;
    }
}

void ParkingSystem::setupZone(int zoneID, int areaCount) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == -1 || zones[i].getZoneID() == zoneID) {
            zones[i] = Zone(zoneID, areaCount);
            
            if (engine == nullptr) {
                engine = new AllocationEngine(zones, zoneCount);
            }
            return;
        }
    }
}

void ParkingSystem::setupParkingArea(int zoneID, int areaIndex, int areaID, int slotCapacity) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == zoneID) {
            zones[i].initializeArea(areaIndex, areaID, slotCapacity);
            return;
        }
    }
}

void ParkingSystem::addZoneAdjacency(int zoneID1, int zoneID2) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == zoneID1) {
            zones[i].addAdjacentZone(zoneID2);
        }
        if (zones[i].getZoneID() == zoneID2) {
            zones[i].addAdjacentZone(zoneID1);
        }
    }
}

int ParkingSystem::createParkingRequest(string vehicleID, int requestedZone, int requestTime) {
    if (activeRequestCount >= activeRequestCapacity) {
        expandActiveRequests();
    }
    
    int requestID = nextRequestID++;
    activeRequests[activeRequestCount++] = ParkingRequest(requestID, vehicleID, requestedZone, requestTime);
    
    return requestID;
}

bool ParkingSystem::allocateParking(int requestID) {
    ParkingRequest* request = findActiveRequest(requestID);
    if (request == nullptr) {
        return false;
    }
    
    if (request->getState() != REQUESTED) {
        return false;
    }
    
    AllocationResult result = engine->allocateSlot(*request);
    
    if (result.success) {
        request->allocate();
        
        AllocationOperation op(requestID, request->getVehicleID(), 
                              result.allocatedSlotID, result.allocatedZoneID,
                              request->getRequestTime(), REQUESTED, ALLOCATED);
        rollbackManager->pushOperation(op);
        
        addToHistory(*request, result.allocatedSlotID, result.allocatedZoneID, result.isCrossZone);
        
        return true;
    }
    
    return false;
}

bool ParkingSystem::occupyParking(int requestID) {
    ParkingRequest* request = findActiveRequest(requestID);
    if (request == nullptr) {
        return false;
    }
    
    if (request->occupy()) {
        HistoryNode* histNode = findInHistory(requestID);
        if (histNode != nullptr) {
            histNode->request = *request;
        }
        return true;
    }
    
    return false;
}

bool ParkingSystem::releaseParking(int requestID, int releaseTime) {
    ParkingRequest* request = findActiveRequest(requestID);
    if (request == nullptr) {
        return false;
    }
    
    HistoryNode* histNode = findInHistory(requestID);
    if (histNode == nullptr) {
        return false;
    }
    
    if (request->release()) {
        engine->freeSlot(histNode->allocatedSlotID, histNode->allocatedZoneID);
        
        histNode->request = *request;
        histNode->releaseTime = releaseTime;
        
        removeActiveRequest(requestID);
        
        return true;
    }
    
    return false;
}

bool ParkingSystem::cancelRequest(int requestID) {
    ParkingRequest* request = findActiveRequest(requestID);
    if (request == nullptr) {
        return false;
    }
    
    RequestState oldState = request->getState();
    
    if (request->cancel()) {
        if (oldState == ALLOCATED) {
            HistoryNode* histNode = findInHistory(requestID);
            if (histNode != nullptr) {
                engine->freeSlot(histNode->allocatedSlotID, histNode->allocatedZoneID);
                histNode->request = *request;
            }
        } else if (oldState == REQUESTED) {
            addToHistory(*request, -1, -1, false);
        }
        
        removeActiveRequest(requestID);
        return true;
    }
    
    return false;
}

bool ParkingSystem::rollbackLastAllocation() {
    AllocationOperation op;
    if (!rollbackManager->popOperation(op)) {
        return false;
    }
    
    ParkingRequest* request = findActiveRequest(op.requestID);
    
    engine->freeSlot(op.allocatedSlotID, op.allocatedZoneID);
    
    if (request != nullptr) {
        request->cancel();
    }
    
    return true;
}

bool ParkingSystem::rollbackLastKAllocations(int k) {
    int rolledBack = 0;
    for (int i = 0; i < k; i++) {
        if (rollbackLastAllocation()) {
            rolledBack++;
        } else {
            break;
        }
    }
    return rolledBack > 0;
}

ParkingAnalytics ParkingSystem::getAnalytics() const {
    ParkingAnalytics analytics;
    
    int totalDuration = 0;
    int completedCount = 0;
    int crossZoneCount = 0;
    
    HistoryNode* current = historyHead;
    while (current != nullptr) {
        analytics.totalRequests++;
        
        if (current->request.getState() == RELEASED && current->releaseTime != -1) {
            completedCount++;
            int duration = current->releaseTime - current->request.getRequestTime();
            totalDuration += duration;
        }
        
        if (current->request.getState() == CANCELLED) {
            analytics.cancelledRequests++;
        }
        
        if (current->isCrossZone) {
            crossZoneCount++;
        }
        
        current = current->next;
    }
    
    analytics.completedRequests = completedCount;
    analytics.crossZoneAllocations = crossZoneCount;
    
    if (completedCount > 0) {
        analytics.averageParkingDuration = (double)totalDuration / completedCount;
    }
    
    int totalSlots = 0;
    int occupiedSlots = 0;
    for (int i = 0; i < zoneCount; i++) {
        totalSlots += zones[i].getTotalCapacity();
        occupiedSlots += (zones[i].getTotalCapacity() - zones[i].getTotalAvailableSlots());
    }
    
    if (totalSlots > 0) {
        analytics.zoneUtilizationRate = (double)occupiedSlots / totalSlots * 100.0;
    }
    
    return analytics;
}

void ParkingSystem::printZoneUtilization() const {
    std::cout << "\n=== Zone Utilization ===" << std::endl;
    for (int i = 0; i < zoneCount; i++) {
        int total = zones[i].getTotalCapacity();
        int available = zones[i].getTotalAvailableSlots();
        int occupied = total - available;
        double utilization = (total > 0) ? ((double)occupied / total * 100.0) : 0.0;
        
        std::cout << "Zone " << zones[i].getZoneID() << ": "
                  << occupied << "/" << total << " occupied ("
                  << utilization << "%)" << std::endl;
    }
}

int ParkingSystem::getPeakUsageZone() const {
    int peakZoneID = -1;
    int maxOccupied = -1;
    
    for (int i = 0; i < zoneCount; i++) {
        int occupied = zones[i].getTotalCapacity() - zones[i].getTotalAvailableSlots();
        if (occupied > maxOccupied) {
            maxOccupied = occupied;
            peakZoneID = zones[i].getZoneID();
        }
    }
    
    return peakZoneID;
}

void ParkingSystem::expandActiveRequests() {
    activeRequestCapacity *= 2;
    ParkingRequest* newArray = new ParkingRequest[activeRequestCapacity];
    for (int i = 0; i < activeRequestCount; i++) {
        newArray[i] = activeRequests[i];
    }
    delete[] activeRequests;
    activeRequests = newArray;
}

ParkingRequest* ParkingSystem::findActiveRequest(int requestID) {
    for (int i = 0; i < activeRequestCount; i++) {
        if (activeRequests[i].getRequestID() == requestID) {
            return &activeRequests[i];
        }
    }
    return nullptr;
}

void ParkingSystem::removeActiveRequest(int requestID) {
    for (int i = 0; i < activeRequestCount; i++) {
        if (activeRequests[i].getRequestID() == requestID) {
            for (int j = i; j < activeRequestCount - 1; j++) {
                activeRequests[j] = activeRequests[j + 1];
            }
            activeRequestCount--;
            return;
        }
    }
}

void ParkingSystem::addToHistory(const ParkingRequest& request, int slotID, int zoneID, bool crossZone) {
    HistoryNode* newNode = new HistoryNode(request, slotID, zoneID, crossZone);
    newNode->next = historyHead;
    historyHead = newNode;
    historyCount++;
}

HistoryNode* ParkingSystem::findInHistory(int requestID) {
    HistoryNode* current = historyHead;
    while (current != nullptr) {
        if (current->request.getRequestID() == requestID) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

Zone* ParkingSystem::getZones() {
    return zones;
}

int ParkingSystem::getZoneCount() const {
    return zoneCount;
}

ParkingRequest* ParkingSystem::getActiveRequest(int requestID) {
    return findActiveRequest(requestID);
}