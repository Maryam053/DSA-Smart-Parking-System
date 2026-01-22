#include "Zone.h"

Zone::Zone() {
    zoneID = -1;
    areaCount = 0;
    areas = nullptr;
    adjacentZones = nullptr;
    adjacentCount = 0;
    adjacentCapacity = 0;
}

Zone::Zone(int zoneID, int areaCount) {
    this->zoneID = zoneID;
    this->areaCount = areaCount;
    this->areas = new ParkingArea[areaCount];
    
    adjacentCapacity = 5;
    adjacentZones = new int[adjacentCapacity];
    adjacentCount = 0;
}

Zone::~Zone() {
    delete[] areas;
    delete[] adjacentZones;
}

Zone::Zone(const Zone& other) {
    zoneID = other.zoneID;
    areaCount = other.areaCount;
    adjacentCount = other.adjacentCount;
    adjacentCapacity = other.adjacentCapacity;
    
    if (other.areas != nullptr) {
        areas = new ParkingArea[areaCount];
        for (int i = 0; i < areaCount; i++) {
            areas[i] = other.areas[i];
        }
    } else {
        areas = nullptr;
    }
    
    if (other.adjacentZones != nullptr) {
        adjacentZones = new int[adjacentCapacity];
        for (int i = 0; i < adjacentCount; i++) {
            adjacentZones[i] = other.adjacentZones[i];
        }
    } else {
        adjacentZones = nullptr;
    }
}

Zone& Zone::operator=(const Zone& other) {
    if (this != &other) {
        delete[] areas;
        delete[] adjacentZones;
        
        zoneID = other.zoneID;
        areaCount = other.areaCount;
        adjacentCount = other.adjacentCount;
        adjacentCapacity = other.adjacentCapacity;
        
        if (other.areas != nullptr) {
            areas = new ParkingArea[areaCount];
            for (int i = 0; i < areaCount; i++) {
                areas[i] = other.areas[i];
            }
        } else {
            areas = nullptr;
        }
        
        if (other.adjacentZones != nullptr) {
            adjacentZones = new int[adjacentCapacity];
            for (int i = 0; i < adjacentCount; i++) {
                adjacentZones[i] = other.adjacentZones[i];
            }
        } else {
            adjacentZones = nullptr;
        }
    }
    return *this;
}

int Zone::getZoneID() const {
    return zoneID;
}

int Zone::getAreaCount() const {
    return areaCount;
}

void Zone::initializeArea(int areaIndex, int areaID, int slotCapacity) {
    if (areaIndex >= 0 && areaIndex < areaCount) {
        areas[areaIndex] = ParkingArea(areaID, zoneID, slotCapacity);
    }
}

ParkingArea* Zone::getArea(int index) {
    if (index >= 0 && index < areaCount) {
        return &areas[index];
    }
    return nullptr;
}

ParkingSlot* Zone::findAvailableSlot() {
    for (int i = 0; i < areaCount; i++) {
        ParkingSlot* slot = areas[i].findAvailableSlot();
        if (slot != nullptr) {
            return slot;
        }
    }
    return nullptr;
}

int Zone::getTotalAvailableSlots() const {
    int total = 0;
    for (int i = 0; i < areaCount; i++) {
        total += areas[i].getAvailableCount();
    }
    return total;
}

int Zone::getTotalCapacity() const {
    int total = 0;
    for (int i = 0; i < areaCount; i++) {
        total += areas[i].getCapacity();
    }
    return total;
}

void Zone::addAdjacentZone(int zoneID) {
    for (int i = 0; i < adjacentCount; i++) {
        if (adjacentZones[i] == zoneID) {
            return;
        }
    }
    
    if (adjacentCount >= adjacentCapacity) {
        adjacentCapacity *= 2;
        int* newArray = new int[adjacentCapacity];
        for (int i = 0; i < adjacentCount; i++) {
            newArray[i] = adjacentZones[i];
        }
        delete[] adjacentZones;
        adjacentZones = newArray;
    }
    
    adjacentZones[adjacentCount++] = zoneID;
}

bool Zone::isAdjacentTo(int zoneID) const {
    for (int i = 0; i < adjacentCount; i++) {
        if (adjacentZones[i] == zoneID) {
            return true;
        }
    }
    return false;
}

int Zone::getAdjacentZoneCount() const {
    return adjacentCount;
}

int Zone::getAdjacentZone(int index) const {
    if (index >= 0 && index < adjacentCount) {
        return adjacentZones[index];
    }
    return -1;
}