#ifndef ZONE_H
#define ZONE_H

#include "ParkingArea.h"

class Zone {
private:
    int zoneID;
    ParkingArea* areas;
    int areaCount;
    
    int* adjacentZones;
    int adjacentCount;
    int adjacentCapacity;

public:
    Zone();
    Zone(int zoneID, int areaCount);
    ~Zone();
    
    Zone(const Zone& other);
    Zone& operator=(const Zone& other);
    
    int getZoneID() const;
    int getAreaCount() const;
    
    void initializeArea(int areaIndex, int areaID, int slotCapacity);
    ParkingArea* getArea(int index);
    ParkingSlot* findAvailableSlot();
    
    int getTotalAvailableSlots() const;
    int getTotalCapacity() const;
    
    void addAdjacentZone(int zoneID);
    bool isAdjacentTo(int zoneID) const;
    int getAdjacentZoneCount() const;
    int getAdjacentZone(int index) const;
};

#endif