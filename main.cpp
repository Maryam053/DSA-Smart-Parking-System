#include <iostream>
#include <windows.h>
#include <string>
#include "ParkingSystem.h"

using namespace std;

ParkingSystem* parkingSystem = nullptr;

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void clearScreen() {
    system("cls");
}

void pauseScreen() {
    cout << "\n  Press Enter to continue...";
    cin.ignore(10000, '\n');
    cin.get();
}

void printBanner() {
    setColor(11);
    cout << "\n";
    cout << "  ========================================================\n";
    cout << "  ||                                                    ||\n";
    cout << "  ||    SMART PARKING MANAGEMENT SYSTEM                ||\n";
    cout << "  ||    City-Wide Parking Allocation & Analytics       ||\n";
    cout << "  ||                                                    ||\n";
    cout << "  ========================================================\n";
    setColor(7);
}

void printSuccess(const string& message) {
    setColor(10);
    cout << "\n  [SUCCESS] " << message << "\n";
    setColor(7);
}

void printError(const string& message) {
    setColor(12);
    cout << "\n  [ERROR] " << message << "\n";
    setColor(7);
}

void printInfo(const string& message) {
    setColor(14);
    cout << "\n  [INFO] " << message << "\n";
    setColor(7);
}

void printTestHeader(const string& testName) {
    setColor(13);
    cout << "\n==========================================\n";
    cout << "TEST: " << testName << endl;
    cout << "==========================================\n";
    setColor(7);
}

void printTestResult(bool passed) {
    if (passed) {
        setColor(10);
        cout << "[PASSED]" << endl;
    } else {
        setColor(12);
        cout << "[FAILED]" << endl;
    }
    setColor(7);
}

// Test functions remain same as before...
bool test1_BasicAllocation() {
    printTestHeader("Basic Slot Allocation in Same Zone");
    ParkingSystem system(3);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest("V1001", 1, 100);
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
    int req1 = system.createParkingRequest("V1001", 1, 100);
    system.allocateParking(req1);
    int req2 = system.createParkingRequest("V1002", 1, 101);
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
    int reqID = system.createParkingRequest("V1001", 1, 100);
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
    int reqID = system.createParkingRequest("V1001", 1, 100);
    bool shouldFail = system.occupyParking(reqID);
    printTestResult(!shouldFail);
    return !shouldFail;
}

bool test5_CancelRequested() {
    printTestHeader("Cancellation from REQUESTED State");
    ParkingSystem system(2);
    system.setupZone(1, 1);
    system.setupParkingArea(1, 0, 101, 5);
    int reqID = system.createParkingRequest("V1001", 1, 100);
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
    int req1 = system.createParkingRequest("V1001", 1, 100);
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
    int reqID = system.createParkingRequest("V1001", 1, 100);
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
        string vehID = "V" + to_string(1001 + i);
        int reqID = system.createParkingRequest(vehID, 1, 100 + i);
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
    int req1 = system.createParkingRequest("V1001", 1, 100);
    system.allocateParking(req1);
    system.occupyParking(req1);
    system.releaseParking(req1, 200);
    int req2 = system.createParkingRequest("V1002", 1, 150);
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
    int req1 = system.createParkingRequest("V1001", 1, 100);
    int req2 = system.createParkingRequest("V1002", 1, 101);
    int req3 = system.createParkingRequest("V1003", 1, 102);
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
        string vehID = "V" + to_string(1001 + i);
        int reqID = system.createParkingRequest(vehID, 1, 100 + i);
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
        string vehID = "V" + to_string(2001 + i);
        int reqID = system.createParkingRequest(vehID, 2, 200 + i);
        system.allocateParking(reqID);
    }
    for (int i = 0; i < 3; i++) {
        string vehID = "V" + to_string(1001 + i);
        int reqID = system.createParkingRequest(vehID, 1, 100 + i);
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
    clearScreen();
    setColor(11);
    cout << "\n======================================================\n";
    cout << "  Smart Parking Allocation & Zone Management System  \n";
    cout << "              Comprehensive Test Suite               \n";
    cout << "======================================================\n";
    setColor(7);
    
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
    if (passed == total) {
        setColor(10);
        cout << "ALL TESTS PASSED! " << passed << "/" << total << "\n";
    } else {
        setColor(14);
        cout << "RESULTS: " << passed << "/" << total << " tests passed\n";
    }
    setColor(7);
    cout << "==========================================\n\n";
    
    pauseScreen();
}

void initializeSystem() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [INITIALIZING SYSTEM]\n";
    setColor(7);
    cout << "  Setting up 3 zones with parking areas...\n\n";
    
    parkingSystem = new ParkingSystem(3);
    
    parkingSystem->setupZone(1, 2);
    parkingSystem->setupParkingArea(1, 0, 101, 5);
    parkingSystem->setupParkingArea(1, 1, 102, 5);
    cout << "  + Zone 1: 10 slots (2 areas) - READY\n";
    
    parkingSystem->setupZone(2, 1);
    parkingSystem->setupParkingArea(2, 0, 201, 8);
    cout << "  + Zone 2: 8 slots (1 area) - READY\n";
    
    parkingSystem->setupZone(3, 1);
    parkingSystem->setupParkingArea(3, 0, 301, 6);
    cout << "  + Zone 3: 6 slots (1 area) - READY\n";
    
    parkingSystem->addZoneAdjacency(1, 2);
    parkingSystem->addZoneAdjacency(2, 3);
    cout << "\n  Adjacencies configured: Zone 1 <-> Zone 2 <-> Zone 3\n";
    
    printSuccess("System initialized successfully!");
    pauseScreen();
}

void showUserGuide() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [USER GUIDE - HOW TO USE]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    cout << "\n  TYPICAL PARKING WORKFLOW:\n\n";
    cout << "  1. CREATE REQUEST      : Register a vehicle parking request\n";
    cout << "                           Example: Vehicle LEB-234 wants Zone 1\n\n";
    cout << "  2. ALLOCATE PARKING    : System assigns a parking slot\n";
    cout << "                           Note your Request ID!\n\n";
    cout << "  3. OCCUPY SLOT         : Vehicle arrives and parks\n\n";
    cout << "  4. RELEASE SLOT        : Vehicle leaves and frees the slot\n\n";
    cout << "  --------------------------------------------------------\n";
    cout << "\n  IMPORTANT NOTES:\n";
    cout << "  - Vehicle IDs can be alphanumeric (e.g., LEB-234, ABC-1234)\n";
    cout << "  - Remember your Request ID after creating a request\n";
    cout << "  - Follow the workflow: Create -> Allocate -> Occupy -> Release\n";
    cout << "  - You can cancel at REQUESTED or ALLOCATED stage\n";
    cout << "  - Use Rollback to undo recent allocations\n";
    cout << "  --------------------------------------------------------\n";
    pauseScreen();
}

void displayMenu() {
    clearScreen();
    printBanner();
    
    setColor(14);
    cout << "\n  [MAIN MENU]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    cout << "   1.  Create Parking Request\n";
    cout << "   2.  Allocate Parking\n";
    cout << "   3.  Occupy Parking Slot\n";
    cout << "   4.  Release Parking Slot\n";
    cout << "   5.  Cancel Request\n";
    cout << "   6.  Rollback Last Allocation\n";
    cout << "   7.  Rollback Last K Allocations\n";
    cout << "   8.  View Zone Utilization\n";
    cout << "   9.  View Analytics Dashboard\n";
    cout << "   10. View Peak Usage Zone\n";
    cout << "   11. Run Automated Tests\n";
    cout << "   12. View User Guide\n";
    cout << "   0.  Exit System\n";
    cout << "  --------------------------------------------------------\n";
    setColor(11);
    cout << "\n  Enter your choice: ";
    setColor(7);
}

void handleCreateRequest() {
    string vehicleID;
    int zoneID, requestTime;
    
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [CREATE PARKING REQUEST]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Vehicle IDs can include letters and numbers (e.g., LEB-234)");
    cout << "\n  Enter Vehicle ID: ";
    cin >> vehicleID;
    
    cout << "  Enter Requested Zone (1-3): ";
    if (!(cin >> zoneID) || zoneID < 1 || zoneID > 3) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid zone! Must be 1, 2, or 3.");
        pauseScreen();
        return;
    }
    
    cout << "  Enter Request Time: ";
    if (!(cin >> requestTime)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid time! Please enter a number.");
        pauseScreen();
        return;
    }
    
    int requestID = parkingSystem->createParkingRequest(vehicleID, zoneID, requestTime);
    printSuccess("Request created successfully!");
    setColor(11);
    cout << "  Vehicle ID: " << vehicleID << "\n";
    cout << "  Request ID: " << requestID << " (Remember this!)\n";
    setColor(7);
    
    pauseScreen();
}

void handleAllocateParking() {
    int requestID;
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [ALLOCATE PARKING]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("This assigns a parking slot to your request");
    
    cout << "\n  Enter Request ID: ";
    if (!(cin >> requestID)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid Request ID! Please enter a number.");
        pauseScreen();
        return;
    }
    
    bool success = parkingSystem->allocateParking(requestID);
    if (success) {
        printSuccess("Parking allocated successfully!");
    } else {
        printError("Failed to allocate. Possible reasons:");
        cout << "  - Invalid Request ID\n";
        cout << "  - Request already allocated\n";
        cout << "  - No slots available\n";
    }
    
    pauseScreen();
}

void handleOccupyParking() {
    int requestID;
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [OCCUPY PARKING SLOT]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Vehicle has arrived and is parking in the slot");
    
    cout << "\n  Enter Request ID: ";
    if (!(cin >> requestID)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid Request ID! Please enter a number.");
        pauseScreen();
        return;
    }
    
    bool success = parkingSystem->occupyParking(requestID);
    if (success) {
        printSuccess("Parking slot occupied successfully!");
    } else {
        printError("Failed to occupy. Possible reasons:");
        cout << "  - Invalid Request ID\n";
        cout << "  - Request not allocated yet (Allocate first!)\n";
        cout << "  - Request already occupied or released\n";
    }
    
    pauseScreen();
}

void handleReleaseParking() {
    int requestID, releaseTime;
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [RELEASE PARKING SLOT]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Vehicle is leaving and freeing the slot");
    
    cout << "\n  Enter Request ID: ";
    if (!(cin >> requestID)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid Request ID! Please enter a number.");
        pauseScreen();
        return;
    }
    
    cout << "  Enter Release Time: ";
    if (!(cin >> releaseTime)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid time! Please enter a number.");
        pauseScreen();
        return;
    }
    
    bool success = parkingSystem->releaseParking(requestID, releaseTime);
    if (success) {
        printSuccess("Parking slot released successfully!");
    } else {
        printError("Failed to release. Possible reasons:");
        cout << "  - Invalid Request ID\n";
        cout << "  - Request not occupied yet (Occupy first!)\n";
    }
    
    pauseScreen();
}

void handleCancelRequest() {
    int requestID;
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [CANCEL REQUEST]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Cancel a pending or allocated request");
    
    cout << "\n  Enter Request ID: ";
    if (!(cin >> requestID)) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid Request ID! Please enter a number.");
        pauseScreen();
        return;
    }
    
    bool success = parkingSystem->cancelRequest(requestID);
    if (success) {
        printSuccess("Request cancelled successfully!");
    } else {
        printError("Failed to cancel. Possible reasons:");
        cout << "  - Invalid Request ID\n";
        cout << "  - Request already occupied or released\n";
    }
    
    pauseScreen();
}

void handleRollbackSingle() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [ROLLBACK LAST ALLOCATION]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Undo the most recent parking allocation");
    
    bool success = parkingSystem->rollbackLastAllocation();
    if (success) {
        printSuccess("Last allocation rolled back successfully!");
    } else {
        printError("No allocations to rollback.");
    }
    
    pauseScreen();
}

void handleRollbackK() {
    int k;
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [ROLLBACK LAST K ALLOCATIONS]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    printInfo("Undo multiple recent allocations");
    
    cout << "\n  Enter number of allocations to rollback: ";
    if (!(cin >> k) || k < 1) {
        cin.clear();
        cin.ignore(10000, '\n');
        printError("Invalid number! Must be positive.");
        pauseScreen();
        return;
    }
    
    bool success = parkingSystem->rollbackLastKAllocations(k);
    if (success) {
        printSuccess("Allocations rolled back successfully!");
        setColor(11);
        cout << "  Rolled back: " << k << " allocations\n";
        setColor(7);
    } else {
        printError("Failed to rollback. Not enough allocations.");
    }
    
    pauseScreen();
}

void handleViewUtilization() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [ZONE UTILIZATION]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    parkingSystem->printZoneUtilization();
    
    pauseScreen();
}

void handleViewAnalytics() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [ANALYTICS DASHBOARD]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    
    ParkingAnalytics analytics = parkingSystem->getAnalytics();
    
    setColor(11);
    cout << "  Total Requests:           ";
    setColor(7);
    cout << analytics.totalRequests << "\n";
    
    setColor(11);
    cout << "  Completed Requests:       ";
    setColor(7);
    cout << analytics.completedRequests << "\n";
    
    setColor(11);
    cout << "  Cancelled Requests:       ";
    setColor(7);
    cout << analytics.cancelledRequests << "\n";
    
    setColor(11);
    cout << "  Avg Parking Duration:     ";
    setColor(7);
    cout << analytics.averageParkingDuration << " units\n";
    
    setColor(11);
    cout << "  Overall Utilization:      ";
    setColor(7);
    cout << analytics.zoneUtilizationRate << "%\n";
    
    setColor(11);
    cout << "  Cross-Zone Allocations:   ";
    setColor(7);
    cout << analytics.crossZoneAllocations << "\n";
    
    cout << "  --------------------------------------------------------\n";
    
    pauseScreen();
}

void handleViewPeakZone() {
    clearScreen();
    printBanner();
    setColor(14);
    cout << "\n  [PEAK USAGE ZONE]\n";
    setColor(7);
    cout << "  --------------------------------------------------------\n";
    
    int peakZone = parkingSystem->getPeakUsageZone();
    setColor(10);
    cout << "  Zone with highest usage: Zone " << peakZone << "\n\n";
    setColor(7);
    
    parkingSystem->printZoneUtilization();
    
    pauseScreen();
}

int main() {
    int choice;
    bool systemInitialized = false;
    
    while (true) {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            printError("Invalid input! Please enter a number.");
            pauseScreen();
            continue;
        }
        
        if (choice == 0) {
            clearScreen();
            printBanner();
            setColor(14);
            cout << "\n  Thank you for using Smart Parking System!\n";
            cout << "  Goodbye!\n\n";
            setColor(7);
            if (parkingSystem != nullptr) {
                delete parkingSystem;
            }
            break;
        }
        
        if (choice == 11) {
            runAllTests();
            continue;
        }
        
        if (choice == 12) {
            showUserGuide();
            continue;
        }
        
        if (!systemInitialized && choice >= 1 && choice <= 10) {
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
                printError("Invalid choice! Please select 0-12.");
                pauseScreen();
        }
    }
    
    return 0;
}