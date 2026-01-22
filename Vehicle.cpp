#include "Vehicle.h"

Vehicle::Vehicle() {
    vehicleID = "";
    preferredZone = -1;
}

Vehicle::Vehicle(string vehicleID, int preferredZone) {
    this->vehicleID = vehicleID;
    this->preferredZone = preferredZone;
}

string Vehicle::getVehicleID() const {
    return vehicleID;
}

int Vehicle::getPreferredZone() const {
    return preferredZone;
}