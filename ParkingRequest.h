#ifndef PARKINGREQUEST_H
#define PARKINGREQUEST_H

#include <string>
using namespace std;

enum RequestState {
    REQUESTED,
    ALLOCATED,
    OCCUPIED,
    RELEASED,
    CANCELLED
};

class ParkingRequest {
private:
    int requestID;
    string vehicleID;
    int requestedZone;
    int requestTime;
    RequestState state;

public:
    ParkingRequest();
    ParkingRequest(int requestID, string vehicleID, int requestedZone, int requestTime);
    
    int getRequestID() const;
    string getVehicleID() const;
    int getRequestedZone() const;
    int getRequestTime() const;
    RequestState getState() const;
    
    bool allocate();
    bool occupy();
    bool release();
    bool cancel();
};

#endif