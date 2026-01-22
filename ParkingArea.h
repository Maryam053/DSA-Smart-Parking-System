#ifndef PARKINGAREA_H
#define PARKINGAREA_H

#include "ParkingSlot.h"

class ParkingArea {
private:
    int areaID;
    int zoneID;
    ParkingSlot* slots;
    int capacity;
    int occupiedCount;

public:
    ParkingArea();
    ParkingArea(int areaID, int zoneID, int capacity);
    ~ParkingArea();
    
    ParkingArea(const ParkingArea& other);
    ParkingArea& operator=(const ParkingArea& other);
    
    int getAreaID() const;
    int getZoneID() const;
    int getCapacity() const;
    int getOccupiedCount() const;
    int getAvailableCount() const;
    
    ParkingSlot* findAvailableSlot();
    ParkingSlot* getSlot(int index);
    ParkingSlot* getSlots();
};

#endif