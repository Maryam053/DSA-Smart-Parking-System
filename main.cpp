#include <iostream>
#include "ParkingSystem.h"

using namespace std;

// Global system pointer
ParkingSystem* parkingSystem = nullptr;

void printTestHeader(const string& testName) {
    cout << "\n==========================================\n";
    cout << "TEST: " << testName << endl;
    cout << "==========================================\n";
}

void printTestResult(bool passed) {
    if (passed) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }
}

bool test1_BasicAllocation() {
    printTestHeader("Basic Slot Allocation in Same Zone");
    ParkingSystem system(3);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest(1001, 1, 100);
    bool allocated = system.allocateParking(reqID);
    printTestResult(allocated);
    return allocated;
}

bool test2_CrossZoneAllocation() {
    printTestHeader("Cross-Zone Allocation");
    ParkingSystem system(3);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 1);
    system.setupZone(2, 1);
    system.setupParkingArea(2, 0, 201, 5);
    system.addZoneAdjacency(1, 2);
    int req1 = system.createParkingRequest(1001, 1, 100);
    system.allocateParking(req1);
    int req2 = system.createParkingRequest(1002, 1, 101);
    bool crossZoneAllocated = system.allocateParking(req2);
    ParkingAnalytics analytics = system.getAnalytics();
    bool hasCrossZone = (analytics.crossZoneAllocations > 0);
    printTestResult(crossZoneAllocated && hasCrossZone);
    return crossZoneAllocated && hasCrossZone;
}

bool test3_StateTransitions() {
    printTestHeader("Request State Transitions");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest(1001, 1, 100);
    bool step1 = system.allocateParking(reqID);
    ParkingRequest* req = system.getActiveRequest(reqID);
    bool isAllocated = (req != nullptr && req->getState() == ALLOCATED);
    bool step2 = system.occupyParking(reqID);
    bool isOccupied = (req != nullptr && req->getState() == OCCUPIED);
    bool step3 = system.releaseParking(reqID, 200);
    bool passed = step1 && isAllocated && step2 && isOccupied && step3;
    printTestResult(passed);
    return passed;
}

bool test4_InvalidStateTransition() {
    printTestHeader("Invalid State Transition Prevention");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest(1001, 1, 100);
    bool shouldFail = system.occupyParking(reqID);
    printTestResult(!shouldFail);
    return !shouldFail;
}

bool test5_CancelRequested() {
    printTestHeader("Cancellation from REQUESTED State");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest(1001, 1, 100);
    bool cancelled = system.cancelRequest(reqID);
    ParkingAnalytics analytics = system.getAnalytics();
    bool counted = (analytics.cancelledRequests == 1);
    printTestResult(cancelled && counted);
    return cancelled && counted;
}

bool test6_CancelAllocated() {
    printTestHeader("Cancellation from ALLOCATED State (Slot Restoration)");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 2);
    int req1 = system.createParkingRequest(1001, 1, 100);
    system.allocateParking(req1);
    int availableBefore = system.getZones()[0].getTotalAvailableSlots();
    system.cancelRequest(req1);
    int availableAfter = system.getZones()[0].getTotalAvailableSlots();
    bool slotRestored = (availableAfter > availableBefore);
    printTestResult(slotRestored);
    return slotRestored;
}

bool test7_RollbackSingle() {
    printTestHeader("Rollback Last Allocation");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int availableBefore = system.getZones()[0].getTotalAvailableSlots();
    int reqID = system.createParkingRequest(1001, 1, 100);
    system.allocateParking(reqID);
    int availableAfterAlloc = system.getZones()[0].getTotalAvailableSlots();
    bool rolled = system.rollbackLastAllocation();
    int availableAfterRollback = system.getZones()[0].getTotalAvailableSlots();
    bool success = rolled && (availableAfterRollback > availableAfterAlloc);
    printTestResult(success);
    return success;
}

bool test8_RollbackMultiple() {
    printTestHeader("Rollback Last K Allocations");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 10);
    int availableBefore = system.getZones()[0].getTotalAvailableSlots();
    for (int i = 0; i < 3; i++) {
        int reqID = system.createParkingRequest(1001 + i, 1, 100 + i);
        system.allocateParking(reqID);
    }
    int availableAfterAlloc = system.getZones()[0].getTotalAvailableSlots();
    bool rolled = system.rollbackLastKAllocations(2);
    int availableAfterRollback = system.getZones()[0].getTotalAvailableSlots();
    bool success = rolled && (availableAfterRollback == availableAfterAlloc + 2);
    printTestResult(success);
    return success;
}

bool test9_AverageDuration() {
    printTestHeader("Analytics - Average Parking Duration");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 10);
    int req1 = system.createParkingRequest(1001, 1, 100);
    system.allocateParking(req1);
    system.occupyParking(req1);
    system.releaseParking(req1, 200);
    int req2 = system.createParkingRequest(1002, 1, 150);
    system.allocateParking(req2);
    system.occupyParking(req2);
    system.releaseParking(req2, 350);
    ParkingAnalytics analytics = system.getAnalytics();
    bool correct = (analytics.averageParkingDuration == 150.0);
    cout << "Average Duration: " << analytics.averageParkingDuration << endl;
    printTestResult(correct);
    return correct;
}

bool test10_AnalyticsAfterRollback() {
    printTestHeader("Analytics Correctness After Rollback");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 10);
    int req1 = system.createParkingRequest(1001, 1, 100);
    int req2 = system.createParkingRequest(1002, 1, 101);
    int req3 = system.createParkingRequest(1003, 1, 102);
    system.allocateParking(req1);
    system.allocateParking(req2);
    system.allocateParking(req3);
    ParkingAnalytics before = system.getAnalytics();
    system.rollbackLastKAllocations(2);
    ParkingAnalytics after = system.getAnalytics();
    bool historyPreserved = (after.totalRequests == before.totalRequests);
    cout << "Requests before: " << before.totalRequests << ", after: " << after.totalRequests << endl;
    printTestResult(historyPreserved);
    return historyPreserved;
}

bool test11_ZoneUtilization() {
    printTestHeader("Zone Utilization Rate");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 10);
    for (int i = 0; i < 5; i++) {
        int reqID = system.createParkingRequest(1001 + i, 1, 100 + i);
        system.allocateParking(reqID);
    }
    ParkingAnalytics analytics = system.getAnalytics();
    bool correct = (analytics.zoneUtilizationRate == 50.0);
    cout << "Utilization Rate: " << analytics.zoneUtilizationRate << "%" << endl;
    printTestResult(correct);
    return correct;
}

bool test12_PeakUsageZone() {
    printTestHeader("Peak Usage Zone Identification");
    ParkingSystem system(3);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 10);
    system.setupZone(2, 1);
    system.setupParkingArea(2, 0, 201, 10);
    for (int i = 0; i < 7; i++) {
        int reqID = system.createParkingRequest(2001 + i, 2, 200 + i);
        system.allocateParking(reqID);
    }
    for (int i = 0; i < 3; i++) {
        int reqID = system.createParkingRequest(1001 + i, 1, 100 + i);
        system.allocateParking(reqID);
    }
    int peakZone = system.getPeakUsageZone();
    bool correct = (peakZone == 2);
    cout << "Peak Usage Zone: " << peakZone << endl;
    system.printZoneUtilization();
    printTestResult(correct);
    return correct;
}

void runAllTests() {
    cout << "\n======================================================\n";
    cout << "  Smart Parking Allocation & Zone Management System  \n";
    cout << "              Comprehensive Test Suite               \n";
    cout << "======================================================\n";
    
    int passed = 0;
    int total = 12;
    
    if (test1_BasicAllocation()) passed++;
    if (test2_CrossZoneAllocation()) passed++;
    if (test3_StateTransitions()) passed++;
    if (test4_InvalidStateTransition()) passed++;
    if (test5_CancelRequested()) passed++;
    if (test6_CancelAllocated()) passed++;
    if (test7_RollbackSingle()) passed++;
    if (test8_RollbackMultiple()) passed++;
    if (test9_AverageDuration()) passed++;
    if (test10_AnalyticsAfterRollback()) passed++;
    if (test11_ZoneUtilization()) passed++;
    if (test12_PeakUsageZone()) passed++;
    
    cout << "\n==========================================\n";
    cout << "FINAL RESULTS: " << passed << "/" << total << " tests passed\n";
    cout << "==========================================\n\n";
}

void initializeSystem() {
    cout << "\n=== Initializing Parking System ===\n";
    cout << "Setting up 3 zones with parking areas...\n";
    
    parkingSystem = new ParkingSystem(3);
    
    // Zone 1: 2 areas
    parkingSystem->setupZone(1, 2);
    parkingSystem->setupParkingArea(1, 0, 101, 5);
    parkingSystem->setupParkingArea(1, 1, 102, 5);
    
    // Zone 2: 1 area
    parkingSystem->setupZone(2, 1);
    parkingSystem->setupParkingArea(2, 0, 201, 8);
    
    // Zone 3: 1 area
    parkingSystem->setupZone(3, 1);
    parkingSystem->setupParkingArea(3, 0, 301, 6);
    
    // Add adjacencies
    parkingSystem->addZoneAdjacency(1, 2);
    parkingSystem->addZoneAdjacency(2, 3);
    
    cout << "System initialized successfully!\n";
    cout << "Zone 1: 10 slots (2 areas)\n";
    cout << "Zone 2: 8 slots (1 area)\n";
    cout << "Zone 3: 6 slots (1 area)\n";
}

void displayMenu() {
    cout << "\n========================================\n";
    cout << "    SMART PARKING MANAGEMENT SYSTEM    \n";
    cout << "========================================\n";
    cout << "1.  Create Parking Request\n";
    cout << "2.  Allocate Parking\n";
    cout << "3.  Occupy Parking Slot\n";
    cout << "4.  Release Parking Slot\n";
    cout << "5.  Cancel Request\n";
    cout << "6.  Rollback Last Allocation\n";
    cout << "7.  Rollback Last K Allocations\n";
    cout << "8.  View Zone Utilization\n";
    cout << "9.  View Analytics\n";
    cout << "10. View Peak Usage Zone\n";
    cout << "11. Run Automated Tests\n";
    cout << "0.  Exit\n";
    cout << "========================================\n";
    cout << "Enter your choice: ";
}

void handleCreateRequest() {
    int vehicleID, zoneID, requestTime;
    cout << "\n--- Create Parking Request ---\n";
    cout << "Enter Vehicle ID: ";
    cin >> vehicleID;
    cout << "Enter Requested Zone (1-3): ";
    cin >> zoneID;
    cout << "Enter Request Time: ";
    cin >> requestTime;
    
    int requestID = parkingSystem->createParkingRequest(vehicleID, zoneID, requestTime);
    cout << "Request created successfully! Request ID: " << requestID << "\n";
}

void handleAllocateParking() {
    int requestID;
    cout << "\n--- Allocate Parking ---\n";
    cout << "Enter Request ID: ";
    cin >> requestID;
    
    bool success = parkingSystem->allocateParking(requestID);
    if (success) {
        cout << "Parking allocated successfully!\n";
    } else {
        cout << "Failed to allocate parking. Check request ID or availability.\n";
    }
}

void handleOccupyParking() {
    int requestID;
    cout << "\n--- Occupy Parking Slot ---\n";
    cout << "Enter Request ID: ";
    cin >> requestID;
    
    bool success = parkingSystem->occupyParking(requestID);
    if (success) {
        cout << "Parking slot occupied successfully!\n";
    } else {
        cout << "Failed to occupy. Ensure parking is allocated first.\n";
    }
}

void handleReleaseParking() {
    int requestID, releaseTime;
    cout << "\n--- Release Parking Slot ---\n";
    cout << "Enter Request ID: ";
    cin >> requestID;
    cout << "Enter Release Time: ";
    cin >> releaseTime;
    
    bool success = parkingSystem->releaseParking(requestID, releaseTime);
    if (success) {
        cout << "Parking slot released successfully!\n";
    } else {
        cout << "Failed to release. Check request ID.\n";
    }
}

void handleCancelRequest() {
    int requestID;
    cout << "\n--- Cancel Request ---\n";
    cout << "Enter Request ID: ";
    cin >> requestID;
    
    bool success = parkingSystem->cancelRequest(requestID);
    if (success) {
        cout << "Request cancelled successfully!\n";
    } else {
        cout << "Failed to cancel. Check request ID.\n";
    }
}

void handleRollbackSingle() {
    cout << "\n--- Rollback Last Allocation ---\n";
    bool success = parkingSystem->rollbackLastAllocation();
    if (success) {
        cout << "Last allocation rolled back successfully!\n";
    } else {
        cout << "No allocations to rollback.\n";
    }
}

void handleRollbackK() {
    int k;
    cout << "\n--- Rollback Last K Allocations ---\n";
    cout << "Enter number of allocations to rollback: ";
    cin >> k;
    
    bool success = parkingSystem->rollbackLastKAllocations(k);
    if (success) {
        cout << "Rolled back " << k << " allocations successfully!\n";
    } else {
        cout << "Failed to rollback. Not enough allocations.\n";
    }
}

void handleViewUtilization() {
    parkingSystem->printZoneUtilization();
}

void handleViewAnalytics() {
    cout << "\n=== Parking Analytics ===\n";
    ParkingAnalytics analytics = parkingSystem->getAnalytics();
    cout << "Total Requests: " << analytics.totalRequests << "\n";
    cout << "Completed Requests: " << analytics.completedRequests << "\n";
    cout << "Cancelled Requests: " << analytics.cancelledRequests << "\n";
    cout << "Average Parking Duration: " << analytics.averageParkingDuration << "\n";
    cout << "Overall Utilization: " << analytics.zoneUtilizationRate << "%\n";
    cout << "Cross-Zone Allocations: " << analytics.crossZoneAllocations << "\n";
}

void handleViewPeakZone() {
    int peakZone = parkingSystem->getPeakUsageZone();
    cout << "\n=== Peak Usage Zone ===\n";
    cout << "Zone with highest usage: Zone " << peakZone << "\n";
    parkingSystem->printZoneUtilization();
}

int main() {
    int choice;
    bool systemInitialized = false;
    
    cout << "\n======================================================\n";
    cout << "  Smart Parking Allocation & Zone Management System  \n";
    cout << "======================================================\n";
    
    while (true) {
        displayMenu();
        cin >> choice;
        
        if (choice == 0) {
            cout << "\nExiting system. Thank you!\n";
            if (parkingSystem != nullptr) {
                delete parkingSystem;
            }
            break;
        }
        
        if (choice == 11) {
            runAllTests();
            continue;
        }
        
        if (!systemInitialized && choice != 11) {
            initializeSystem();
            systemInitialized = true;
        }
        
        switch (choice) {
            case 1:
                handleCreateRequest();
                break;
            case 2:
                handleAllocateParking();
                break;
            case 3:
                handleOccupyParking();
                break;
            case 4:
                handleReleaseParking();
                break;
            case 5:
                handleCancelRequest();
                break;
            case 6:
                handleRollbackSingle();
                break;
            case 7:
                handleRollbackK();
                break;
            case 8:
                handleViewUtilization();
                break;
            case 9:
                handleViewAnalytics();
                break;
            case 10:
                handleViewPeakZone();
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    }
    
    return 0;
}