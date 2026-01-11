#ifndef ALLOCATIONENGINE_H
#define ALLOCATIONENGINE_H
// Allocation logic updated for cross-zone support(.h)

#include "Zone.h"
#include "ParkingRequest.h"

struct AllocationResult {
    bool success;
    int allocatedSlotID;
    int allocatedZoneID;
    bool isCrossZone;
    
    AllocationResult() {
        success = false;
        allocatedSlotID = -1;
        allocatedZoneID = -1;
        isCrossZone = false;
    }
};

class AllocationEngine {
private:
    Zone* zones;
    int zoneCount;

public:
    AllocationEngine();
    AllocationEngine(Zone* zones, int zoneCount);
    
    AllocationResult allocateSlot(ParkingRequest& request);
    bool freeSlot(int slotID, int zoneID);
    
    ParkingSlot* findSlotInZone(int zoneID);
    ParkingSlot* findSlotInAdjacentZones(int requestedZoneID);
    ParkingSlot* findSlotByID(int slotID, int zoneID);
    
    Zone* getZone(int zoneID);
};

#endif