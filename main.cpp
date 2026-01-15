#include <iostream>
#include "ParkingSystem.h"

using namespace std;

void printTestHeader(const string& testName) {
    cout << "\n==========================================\n";
    cout << "TEST: " << testName << endl;
    cout << "==========================================\n";
}

void printTestResult(bool passed) {
    if (passed) {
        cout << "✓ PASSED" << endl;
    } else {
        cout << "✗ FAILED" << endl;
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
    
    cout << "Requests before: " << before.totalRequests 
         << ", after: " << after.totalRequests << endl;
    
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

int main() {
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════╗\n";
    cout << "║  Smart Parking Allocation & Zone Management System  ║\n";
    cout << "║              Comprehensive Test Suite               ║\n";
    cout << "╚══════════════════════════════════════════════════════╝\n";
    
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
    
    cout << "\n╔══════════════════════════════════════════════════════╗\n";
    cout << "║              DEMONSTRATION SCENARIO                  ║\n";
    cout << "╚══════════════════════════════════════════════════════╝\n";
    
    ParkingSystem demo(3);
    
    cout << "\nSetting up city with 3 zones..." << endl;
    demo.setupZone(1, 2);
    demo.setupParkingArea(1, 0, 101, 3);
    demo.setupParkingArea(1, 1, 102, 3);
    
    demo.setupZone(2, 1);
    demo.setupParkingArea(2, 0, 201, 4);
    
    demo.setupZone(3, 1);
    demo.setupParkingArea(3, 0, 301, 5);
    
    demo.addZoneAdjacency(1, 2);
    demo.addZoneAdjacency(2, 3);
    
    cout << "\nProcessing parking requests..." << endl;
    
    int r1 = demo.createParkingRequest(5001, 1, 1000);
    demo.allocateParking(r1);
    demo.occupyParking(r1);
    cout << "✓ Vehicle 5001 parked in Zone 1" << endl;
    
    int r2 = demo.createParkingRequest(5002, 1, 1001);
    demo.allocateParking(r2);
    demo.occupyParking(r2);
    cout << "✓ Vehicle 5002 parked in Zone 1" << endl;
    
    int r3 = demo.createParkingRequest(5003, 2, 1002);
    demo.allocateParking(r3);
    cout << "✓ Vehicle 5003 allocated in Zone 2" << endl;
    
    demo.releaseParking(r1, 1500);
    cout << "✓ Vehicle 5001 released (Duration: 500)" << endl;
    
    demo.printZoneUtilization();
    
    ParkingAnalytics finalStats = demo.getAnalytics();
    cout << "\n=== Final Analytics ===" << endl;
    cout << "Total Requests: " << finalStats.totalRequests << endl;
    cout << "Completed: " << finalStats.completedRequests << endl;
    cout << "Cancelled: " << finalStats.cancelledRequests << endl;
    cout << "Average Duration: " << finalStats.averageParkingDuration << endl;
    cout << "Overall Utilization: " << finalStats.zoneUtilizationRate << "%" << endl;
    cout << "Cross-Zone Allocations: " << finalStats.crossZoneAllocations << endl;
    
    return 0;
}