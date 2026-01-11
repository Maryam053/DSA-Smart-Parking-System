#include "AllocationEngine.h"

AllocationEngine::AllocationEngine() {
    zones = nullptr;
    zoneCount = 0;
}

AllocationEngine::AllocationEngine(Zone* zones, int zoneCount) {
    this->zones = zones;
    this->zoneCount = zoneCount;
}

AllocationResult AllocationEngine::allocateSlot(ParkingRequest& request) {
    AllocationResult result;
    
    ParkingSlot* slot = findSlotInZone(request.getRequestedZone());
    
    if (slot != nullptr) {
        slot->occupySlot();
        result.success = true;
        result.allocatedSlotID = slot->getSlotID();
        result.allocatedZoneID = slot->getZoneID();
        result.isCrossZone = false;
        return result;
    }
    
    slot = findSlotInAdjacentZones(request.getRequestedZone());
    
    if (slot != nullptr) {
        slot->occupySlot();
        result.success = true;
        result.allocatedSlotID = slot->getSlotID();
        result.allocatedZoneID = slot->getZoneID();
        result.isCrossZone = true;
        return result;
    }
    
    result.success = false;
    return result;
}

bool AllocationEngine::freeSlot(int slotID, int zoneID) {
    ParkingSlot* slot = findSlotByID(slotID, zoneID);
    if (slot != nullptr) {
        slot->freeSlot();
        return true;
    }
    return false;
}

ParkingSlot* AllocationEngine::findSlotInZone(int zoneID) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == zoneID) {
            return zones[i].findAvailableSlot();
        }
    }
    return nullptr;
}

ParkingSlot* AllocationEngine::findSlotInAdjacentZones(int requestedZoneID) {
    Zone* requestedZone = nullptr;
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == requestedZoneID) {
            requestedZone = &zones[i];
            break;
        }
    }
    
    if (requestedZone == nullptr) {
        return nullptr;
    }
    
    for (int i = 0; i < requestedZone->getAdjacentZoneCount(); i++) {
        int adjacentZoneID = requestedZone->getAdjacentZone(i);
        ParkingSlot* slot = findSlotInZone(adjacentZoneID);
        if (slot != nullptr) {
            return slot;
        }
    }
    
    return nullptr;
}

ParkingSlot* AllocationEngine::findSlotByID(int slotID, int zoneID) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == zoneID) {
            for (int j = 0; j < zones[i].getAreaCount(); j++) {
                ParkingArea* area = zones[i].getArea(j);
                if (area != nullptr) {
                    for (int k = 0; k < area->getCapacity(); k++) {
                        ParkingSlot* slot = area->getSlot(k);
                        if (slot != nullptr && slot->getSlotID() == slotID) {
                            return slot;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

Zone* AllocationEngine::getZone(int zoneID) {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i].getZoneID() == zoneID) {
            return &zones[i];
        }
    }
    return nullptr;
}