#ifndef PARKINGSLOT_H
#define PARKINGSLOT_H

class ParkingSlot {
private:
    int slotID;
    int zoneID;
    bool isAvailable;

public:
    ParkingSlot();
    ParkingSlot(int slotID, int zoneID);

    int getSlotID() const;
    int getZoneID() const;
    bool getAvailability() const;

    void occupySlot();
    void freeSlot();
};

#endif
