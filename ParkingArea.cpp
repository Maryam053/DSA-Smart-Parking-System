#ifndef PARKINGAREA_H
#define PARKINGAREA_H

#include "ParkingSlot.h"

class ParkingArea {
private:
    int areaID;
    int zoneID;
    ParkingSlot* slots;  // Dynamic array of slots
    int capacity;
    int occupiedCount;

public:
    ParkingArea();
    ParkingArea(int areaID, int zoneID, int capacity);
    ~ParkingArea();
    
    // Copy constructor and assignment operator (needed for proper memory management)
    ParkingArea(const ParkingArea& other);
    ParkingArea& operator=(const ParkingArea& other);
    
    int getAreaID() const;
    int getZoneID() const;
    int getCapacity() const;
    int getOccupiedCount() const;
    int getAvailableCount() const;
    
    // Find first available slot in this area
    ParkingSlot* findAvailableSlot();
    
    // Get slot by index
    ParkingSlot* getSlot(int index);
    
    // Get all slots (for iteration)
    ParkingSlot* getSlots();
};

#endif