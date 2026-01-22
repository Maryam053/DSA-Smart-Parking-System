#include "ParkingSlot.h"

ParkingSlot::ParkingSlot() {
    slotID = -1;
    zoneID = -1;
    isAvailable = true;
}

ParkingSlot::ParkingSlot(int slotID, int zoneID) {
    this->slotID = slotID;
    this->zoneID = zoneID;
    this->isAvailable = true;
}

int ParkingSlot::getSlotID() const {
    return slotID;
}

int ParkingSlot::getZoneID() const {
    return zoneID;
}

bool ParkingSlot::getAvailability() const {
    return isAvailable;
}

void ParkingSlot::occupySlot() {
    isAvailable = false;
}

void ParkingSlot::freeSlot() {
    isAvailable = true;
}
