#include "ParkingArea.h"

ParkingArea::ParkingArea() {
    areaID = -1;
    zoneID = -1;
    capacity = 0;
    occupiedCount = 0;
    slots = nullptr;
}

ParkingArea::ParkingArea(int areaID, int zoneID, int capacity) {
    this->areaID = areaID;
    this->zoneID = zoneID;
    this->capacity = capacity;
    this->occupiedCount = 0;
    
    slots = new ParkingSlot[capacity];
    
    for (int i = 0; i < capacity; i++) {
        slots[i] = ParkingSlot(areaID * 1000 + i, zoneID);
    }
}

ParkingArea::~ParkingArea() {
    delete[] slots;
}

ParkingArea::ParkingArea(const ParkingArea& other) {
    areaID = other.areaID;
    zoneID = other.zoneID;
    capacity = other.capacity;
    occupiedCount = other.occupiedCount;
    
    if (other.slots != nullptr) {
        slots = new ParkingSlot[capacity];
        for (int i = 0; i < capacity; i++) {
            slots[i] = other.slots[i];
        }
    } else {
        slots = nullptr;
    }
}

ParkingArea& ParkingArea::operator=(const ParkingArea& other) {
    if (this != &other) {
        delete[] slots;
        
        areaID = other.areaID;
        zoneID = other.zoneID;
        capacity = other.capacity;
        occupiedCount = other.occupiedCount;
        
        if (other.slots != nullptr) {
            slots = new ParkingSlot[capacity];
            for (int i = 0; i < capacity; i++) {
                slots[i] = other.slots[i];
            }
        } else {
            slots = nullptr;
        }
    }
    return *this;
}

int ParkingArea::getAreaID() const {
    return areaID;
}

int ParkingArea::getZoneID() const {
    return zoneID;
}

int ParkingArea::getCapacity() const {
    return capacity;
}

int ParkingArea::getOccupiedCount() const {
    return occupiedCount;
}

int ParkingArea::getAvailableCount() const {
    int available = 0;
    for (int i = 0; i < capacity; i++) {
        if (slots[i].getAvailability()) {
            available++;
        }
    }
    return available;
}

ParkingSlot* ParkingArea::findAvailableSlot() {
    for (int i = 0; i < capacity; i++) {
        if (slots[i].getAvailability()) {
            return &slots[i];
        }
    }
    return nullptr;
}

ParkingSlot* ParkingArea::getSlot(int index) {
    if (index >= 0 && index < capacity) {
        return &slots[index];
    }
    return nullptr;
}

ParkingSlot* ParkingArea::getSlots() {
    return slots;
}