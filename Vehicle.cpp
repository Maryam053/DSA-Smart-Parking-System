#include "Vehicle.h"

Vehicle::Vehicle() {
    vehicleID = -1;
    preferredZone = -1;
}

Vehicle::Vehicle(int vehicleID, int preferredZone) {
    this->vehicleID = vehicleID;
    this->preferredZone = preferredZone;
}

int Vehicle::getVehicleID() const {
    return vehicleID;
}

int Vehicle::getPreferredZone() const {
    return preferredZone;
}
