#ifndef PARKINGREQUEST_H
#define PARKINGREQUEST_H

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
    int vehicleID;
    int requestedZone;
    int requestTime;
    RequestState state;

public:
    ParkingRequest();
    ParkingRequest(int requestID, int vehicleID, int requestedZone, int requestTime);

    int getRequestID() const;
    int getVehicleID() const;
    int getRequestedZone() const;
    int getRequestTime() const;
    RequestState getState() const;

    bool allocate();
    bool occupy();
    bool release();
    bool cancel();
};

#endif
