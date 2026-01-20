#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
using namespace std;

class Vehicle {
private:
    string vehicleID;
    int preferredZone;

public:
    Vehicle();
    Vehicle(string vehicleID, int preferredZone);
    
    string getVehicleID() const;
    int getPreferredZone() const;
};

#endif