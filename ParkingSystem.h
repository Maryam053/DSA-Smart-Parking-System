#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

#include "Zone.h"
#include "Vehicle.h"
#include "ParkingRequest.h"
#include "AllocationEngine.h"
#include "RollbackManager.h"
#include <string>
using namespace std;

struct HistoryNode {
    ParkingRequest request;
    int allocatedSlotID;
    int allocatedZoneID;
    int releaseTime;
    bool isCrossZone;
    HistoryNode* next;
    
    HistoryNode(const ParkingRequest& req, int slotID, int zoneID, bool crossZone) {
        request = req;
        allocatedSlotID = slotID;
        allocatedZoneID = zoneID;
        releaseTime = -1;
        isCrossZone = crossZone;
        next = nullptr;
    }
};

struct ParkingAnalytics {
    double averageParkingDuration;
    double zoneUtilizationRate;
    int totalRequests;
    int completedRequests;
    int cancelledRequests;
    int crossZoneAllocations;
    
    ParkingAnalytics() {
        averageParkingDuration = 0.0;
        zoneUtilizationRate = 0.0;
        totalRequests = 0;
        completedRequests = 0;
        cancelledRequests = 0;
        crossZoneAllocations = 0;
    }
};

class ParkingSystem {
private:
    Zone* zones;
    int zoneCount;
    AllocationEngine* engine;
    RollbackManager* rollbackManager;
    
    ParkingRequest* activeRequests;
    int activeRequestCount;
    int activeRequestCapacity;
    
    HistoryNode* historyHead;
    int historyCount;
    
    int nextRequestID;
    
    void expandActiveRequests();
    ParkingRequest* findActiveRequest(int requestID);
    void removeActiveRequest(int requestID);
    void addToHistory(const ParkingRequest& request, int slotID, int zoneID, bool crossZone);
    HistoryNode* findInHistory(int requestID);

public:
    ParkingSystem(int zoneCount);
    ~ParkingSystem();
    
    void setupZone(int zoneID, int areaCount);
    void setupParkingArea(int zoneID, int areaIndex, int areaID, int slotCapacity);
    void addZoneAdjacency(int zoneID1, int zoneID2);
    
    int createParkingRequest(string vehicleID, int requestedZone, int requestTime);
    bool allocateParking(int requestID);
    bool occupyParking(int requestID);
    bool releaseParking(int requestID, int releaseTime);
    bool cancelRequest(int requestID);
    
    bool rollbackLastAllocation();
    bool rollbackLastKAllocations(int k);
    
    ParkingAnalytics getAnalytics() const;
    void printZoneUtilization() const;
    int getPeakUsageZone() const;
    
    Zone* getZones();
    int getZoneCount() const;
    ParkingRequest* getActiveRequest(int requestID);
};

#endif