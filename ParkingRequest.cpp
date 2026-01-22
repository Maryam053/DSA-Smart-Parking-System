#include "ParkingRequest.h"

ParkingRequest::ParkingRequest() {
    requestID = -1;
    vehicleID = "";
    requestedZone = -1;
    requestTime = 0;
    state = REQUESTED;
}

ParkingRequest::ParkingRequest(int requestID, string vehicleID, int requestedZone, int requestTime) {
    this->requestID = requestID;
    this->vehicleID = vehicleID;
    this->requestedZone = requestedZone;
    this->requestTime = requestTime;
    this->state = REQUESTED;
}

int ParkingRequest::getRequestID() const {
    return requestID;
}

string ParkingRequest::getVehicleID() const {
    return vehicleID;
}

int ParkingRequest::getRequestedZone() const {
    return requestedZone;
}

int ParkingRequest::getRequestTime() const {
    return requestTime;
}

RequestState ParkingRequest::getState() const {
    return state;
}

bool ParkingRequest::allocate() {
    if (state == REQUESTED) {
        state = ALLOCATED;
        return true;
    }
    return false;
}

bool ParkingRequest::occupy() {
    if (state == ALLOCATED) {
        state = OCCUPIED;
        return true;
    }
    return false;
}

bool ParkingRequest::release() {
    if (state == OCCUPIED) {
        state = RELEASED;
        return true;
    }
    return false;
}

bool ParkingRequest::cancel() {
    if (state == REQUESTED || state == ALLOCATED) {
        state = CANCELLED;
        return true;
    }
    return false;
}