# Smart Parking Allocation & Zone Management System - Design Document

## Table of Contents
1. [System Overview](#system-overview)
2. [Data Structures](#data-structures)
3. [Zone and Slot Representation](#zone-and-slot-representation)
4. [Allocation Strategy](#allocation-strategy)
5. [Request Lifecycle State Machine](#request-lifecycle-state-machine)
6. [Cancellation and Rollback Design](#cancellation-and-rollback-design)
7. [Pricing System](#pricing-system)
8. [Analytics and History Tracking](#analytics-and-history-tracking)
9. [Web Interface Architecture](#web-interface-architecture)
10. [Time and Space Complexity Analysis](#time-and-space-complexity-analysis)

---

## System Overview

The Smart Parking System is a city-wide parking management solution that simulates real-world parking allocation across multiple zones. The system manages parking requests, allocates slots based on availability and zone constraints, enforces strict request lifecycles, supports rollback for cancellations, provides pricing calculations, and offers comprehensive usage analytics.

### Key Features
- **Multi-zone parking management** with 3 zones
- **Dynamic slot allocation** with same-zone preference
- **Cross-zone allocation** with penalty mechanism
- **State machine-based request lifecycle** (REQUESTED → ALLOCATED → OCCUPIED → RELEASED)
- **Cancellation support** for REQUESTED and ALLOCATED states only
- **Rollback mechanism** for last k allocations
- **Pricing system** with per-unit rates and cross-zone penalties
- **Real-time analytics** including revenue tracking
- **Interactive web UI** with live updates and visual feedback

---

## Data Structures

### 1. **Zone Structure**
```cpp
class Zone {
private:
    int zoneID;
    ParkingArea* areas;
    int areaCount;
    int* adjacentZones;
    int adjacentCount;
    int adjacentCapacity;
```

**Purpose:** Represents a parking zone with multiple parking areas.

**Key Components:**
- `zoneID`: Unique identifier for the zone
- `areas`: Array of parking areas within the zone
- `adjacentZones`: Dynamic array tracking connected zones for cross-zone allocation
- Custom adjacency management (no STL containers)

**Operations:**
- `addAdjacentZone()`: O(1) amortized - adds adjacent zone with dynamic resizing
- `getTotalAvailableSlots()`: O(n) - counts available slots across all areas
- `getTotalCapacity()`: O(n) - sums total capacity

---

### 2. **Parking Area Structure**
```cpp
class ParkingArea {
private:
    int areaID;
    ParkingSlot* slots;
    int slotCount;
```

**Purpose:** Container for multiple parking slots within a zone.

**Key Components:**
- `areaID`: Unique identifier for the area
- `slots`: Array of parking slots
- `slotCount`: Total number of slots in this area

**Operations:**
- `findAvailableSlot()`: O(n) - linear search for first available slot
- `releaseSlot()`: O(n) - finds and frees a specific slot

---

### 3. **Parking Slot Structure**
```cpp
class ParkingSlot {
private:
    int slotID;
    int zoneID;
    bool isAvailable;
```

**Purpose:** Individual parking slot with availability tracking.

**Key Components:**
- `slotID`: Unique slot identifier
- `zoneID`: Parent zone ID
- `isAvailable`: Boolean flag for availability

**Operations:**
- `occupy()`: O(1) - marks slot as occupied
- `release()`: O(1) - marks slot as available

---

### 4. **Vehicle Structure**
```cpp
class Vehicle {
private:
    string vehicleID;
    int preferredZone;
```

**Purpose:** Represents a vehicle requesting parking.

**Key Components:**
- `vehicleID`: Unique vehicle identifier (e.g., "LEB-234")
- `preferredZone`: Desired parking zone

---

### 5. **Parking Request Structure**
```cpp
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
    RequestState currentState;
```

**Purpose:** Tracks a parking request through its lifecycle.

**Key Components:**
- `requestID`: Unique request identifier
- `vehicleID`: Associated vehicle
- `requestedZone`: Preferred zone
- `requestTime`: Timestamp when request was created
- `currentState`: Current state in lifecycle

**State Transitions:**
- Valid: REQUESTED → ALLOCATED → OCCUPIED → RELEASED
- Valid: REQUESTED → CANCELLED
- Valid: ALLOCATED → CANCELLED
- Invalid: OCCUPIED → CANCELLED (must release first)
- Invalid: RELEASED → CANCELLED

---

### 6. **History Node (Linked List)**
```cpp
struct HistoryNode {
    ParkingRequest request;
    int allocatedSlotID;
    int allocatedZoneID;
    int releaseTime;
    bool isCrossZone;
    HistoryNode* next;
};
```

**Purpose:** Maintains complete parking history as a singly-linked list.

**Key Components:**
- Stores full request details
- Tracks allocation information
- Records cross-zone status for pricing
- Next pointer for linked list traversal

**Why Linked List?**
- Dynamic growth without reallocation
- O(1) insertion at head
- Efficient for analytics traversal
- History can grow indefinitely

---

### 7. **Rollback Stack**
```cpp
struct AllocationOperation {
    int requestID;
    string vehicleID;
    int allocatedSlotID;
    int allocatedZoneID;
    int requestTime;
    RequestState previousState;
    RequestState newState;
};

struct StackNode {
    AllocationOperation operation;
    StackNode* next;
};

class RollbackManager {
private:
    StackNode* top;
    int size;
    int maxSize;
```

**Purpose:** Stack-based rollback system for undoing allocations.

**Key Components:**
- Stack implemented via linked list
- Stores allocation operations
- Maximum capacity to prevent overflow
- LIFO (Last In First Out) ordering

**Operations:**
- `pushOperation()`: O(1) - adds operation to stack
- `popOperation()`: O(1) - removes and returns last operation
- `rollbackLastKAllocations()`: O(k) - undoes last k operations

---

### 8. **Allocation Engine**
```cpp
struct AllocationResult {
    bool success;
    int allocatedSlotID;
    int allocatedZoneID;
    bool isCrossZone;
};

class AllocationEngine {
private:
    Zone* zones;
    int zoneCount;
```

**Purpose:** Handles slot allocation logic with same-zone preference.

**Allocation Algorithm:**
1. Check requested zone for available slots
2. If found: Allocate in same zone (no penalty)
3. If not found: Check adjacent zones
4. Allocate in first available adjacent zone (cross-zone penalty)
5. If no slots available: Allocation fails

**Operations:**
- `allocateSlot()`: O(n×m) - searches zones and areas
- `freeSlot()`: O(n×m) - releases specific slot

---

### 9. **Parking System (Main Controller)**
```cpp
class ParkingSystem {
private:
    Zone* zones;
    int zoneCount;
    AllocationEngine* engine;
    RollbackManager* rollbackManager;
    ParkingRequest* activeRequests;
    int activeRequestCount;
    HistoryNode* historyHead;
    int nextRequestID;
```

**Purpose:** Central controller integrating all components.

**Key Responsibilities:**
- Manages all zones
- Coordinates allocation engine
- Maintains active requests array
- Tracks history via linked list
- Generates unique request IDs
- Calculates analytics

---

## Zone and Slot Representation

### Zone Configuration
The system simulates a city divided into 3 zones:

```
Zone 1: 10 slots (2 areas × 5 slots each)
Zone 2: 8 slots (2 areas × 4 slots each)
Zone 3: 6 slots (1 area × 6 slots)
Total: 24 parking slots
```

### Zone Adjacency Graph
```
Zone 1 ←→ Zone 2
Zone 2 ←→ Zone 3
Zone 1 ←→ Zone 3
```

All zones are connected (complete graph) for maximum flexibility in cross-zone allocation.

### Slot Identification
Each slot is uniquely identified by:
- **Zone ID** (1-3)
- **Area ID** (101, 102, 201, 202, 301)
- **Slot ID** (within area)

Example: Zone 1, Area 101, Slot 3

---

## Allocation Strategy

### Same-Zone Preference Algorithm
```
Algorithm: allocateSlot(request)
Input: ParkingRequest with preferred zone
Output: AllocationResult with slot details

1. Get preferred zone from request
2. Check if zone has available slots:
   - If YES:
     a. Find first available slot in zone
     b. Mark slot as occupied
     c. Return success with slot details
     d. Set isCrossZone = false
   
3. If NO slots in preferred zone:
   - Check adjacent zones:
     a. Iterate through adjacent zones
     b. Find first zone with available slot
     c. Allocate in that zone
     d. Set isCrossZone = true
     e. Apply cross-zone penalty
   
4. If no slots anywhere:
   - Return failure
```

### Cross-Zone Allocation
When preferred zone is full:
- System searches adjacent zones
- First available slot is allocated
- **Cross-zone flag** is set to true
- **₨50 penalty** is applied to final cost
- Cross-zone count is incremented for analytics

---

## Request Lifecycle State Machine

### State Diagram
```
       ┌─────────────┐
       │  REQUESTED  │ ◄─── Initial state
       └──────┬──────┘
              │
         allocate()
              │
              ▼
       ┌─────────────┐
       │  ALLOCATED  │
       └──────┬──────┘
              │
          occupy()
              │
              ▼
       ┌─────────────┐
       │  OCCUPIED   │
       └──────┬──────┘
              │
         release()
              │
              ▼
       ┌─────────────┐
       │  RELEASED   │ ◄─── Terminal state
       └─────────────┘

     Cancel allowed:
     REQUESTED ──cancel()──► CANCELLED
     ALLOCATED ──cancel()──► CANCELLED
     
     Cancel NOT allowed:
     OCCUPIED  ──X
     RELEASED  ──X
```

### State Transition Rules

| Current State | Valid Transitions | Invalid Transitions |
|---------------|-------------------|---------------------|
| REQUESTED | ALLOCATED, CANCELLED | OCCUPIED, RELEASED |
| ALLOCATED | OCCUPIED, CANCELLED | REQUESTED, RELEASED |
| OCCUPIED | RELEASED | REQUESTED, ALLOCATED, CANCELLED |
| RELEASED | None (terminal) | All |
| CANCELLED | None (terminal) | All |

### State Validation Logic
```cpp
bool transitionIsValid(RequestState current, RequestState next) {
    if (current == REQUESTED && (next == ALLOCATED || next == CANCELLED))
        return true;
    if (current == ALLOCATED && (next == OCCUPIED || next == CANCELLED))
        return true;
    if (current == OCCUPIED && next == RELEASED)
        return true;
    return false;
}
```

---

## Cancellation and Rollback Design

### Cancellation Rules

**Allowed States for Cancellation:**
- ✅ **REQUESTED**: Can cancel (no resources allocated)
- ✅ **ALLOCATED**: Can cancel (slot marked but not occupied, must free slot)
- ❌ **OCCUPIED**: Cannot cancel (vehicle already parked, must release first)
- ❌ **RELEASED**: Cannot cancel (already completed)

### Cancellation Process
```
1. Validate request exists
2. Check current state:
   - If OCCUPIED: Return error "Cannot cancel - vehicle is already parked!"
   - If RELEASED: Return error "Cannot cancel a released request"
   - If CANCELLED: Return error "Request already cancelled"
   
3. If ALLOCATED:
   - Free the allocated slot
   - Restore slot availability
   
4. Update state to CANCELLED
5. Increment cancelled count in analytics
6. Remove from active requests
7. Add to history with cancelled status
```

### Rollback Mechanism

**Stack-Based Rollback System:**
```cpp
Stack Operations:
- Push: Store allocation operation when slot allocated
- Pop: Retrieve and undo last allocation
- Peek: View last operation without removing

Data Stored per Operation:
- Request ID
- Vehicle ID
- Allocated Slot ID
- Allocated Zone ID
- Previous State
- New State
- Timestamp
```

**Rollback Process for Last K Allocations:**
```
Algorithm: rollbackLastKAllocations(k)

For i = 1 to k:
  1. Pop operation from stack
  2. Find associated request
  3. Free allocated slot in zone
  4. Revert request state to previous state
  5. Update analytics (decrement counts)
  6. Log rollback action

Return: Number of operations rolled back
```

**Edge Cases:**
- If k > available operations: Rollback all available
- If stack empty: Return 0 (no rollbacks performed)
- Rollback does not delete history entries (maintains full audit trail)

---

## Pricing System

### Pricing Model

**Base Rate:**
- ₨10 per time unit
- Applied to parking duration

**Cross-Zone Penalty:**
- ₨50 flat fee
- Applied only for cross-zone allocations
- One-time charge regardless of duration

### Cost Calculation Formula
```javascript
duration = releaseTime - requestTime

baseCost = duration × perUnitRate
         = duration × 10

crossZoneFee = isCrossZone ? 50 : 0

totalCost = baseCost + crossZoneFee
```

### Pricing Examples

**Example 1: Same-Zone Parking**
```
Request Time: 100
Release Time: 200
Duration: 100 units
Cross-Zone: No

Calculation:
Base Cost = 100 × ₨10 = ₨1000
Cross-Zone Fee = ₨0
Total Cost = ₨1000
```

**Example 2: Cross-Zone Parking**
```
Request Time: 150
Release Time: 300
Duration: 150 units
Cross-Zone: Yes

Calculation:
Base Cost = 150 × ₨10 = ₨1500
Cross-Zone Fee = ₨50
Total Cost = ₨1550
```

### Revenue Tracking
- Total revenue accumulated across all completed requests
- Updated on each slot release
- Displayed in analytics dashboard
- Persists throughout session

---

## Analytics and History Tracking

### Analytics Metrics

**Real-Time Statistics:**
1. **Total Requests**: Count of all requests created
2. **Completed Requests**: Requests that reached RELEASED state
3. **Cancelled Requests**: Requests cancelled before occupation
4. **Average Parking Duration**: Mean time vehicles spent parked
5. **Zone Utilization Rate**: Percentage of slots occupied
6. **Cross-Zone Allocations**: Count of penalty-incurring allocations
7. **Peak Usage Zone**: Zone with highest occupancy
8. **Total Revenue**: Sum of all parking charges (₨)

### Calculation Methods

**Average Duration:**
```
avgDuration = Σ(releaseTime - requestTime) / completedCount

For all requests where state == RELEASED
```

**Zone Utilization:**
```
utilization = (totalOccupied / totalSlots) × 100%

where:
  totalOccupied = sum of occupied slots across all zones
  totalSlots = sum of all slots in system (24)
```

**Peak Zone:**
```
peakZone = zone with max(occupiedSlots)

Iterate through all zones, find highest occupancy
```

**Total Revenue:**
```
totalRevenue = Σ(request.cost) for all RELEASED requests

Accumulated on each slot release
```

### History Management

**Data Structure:** Singly-linked list with head pointer

**Storage:**
```cpp
struct HistoryNode {
    ParkingRequest request;    // Full request details
    int allocatedSlotID;       // Where it was parked
    int allocatedZoneID;       // Which zone
    int releaseTime;           // When released (-1 if not released)
    bool isCrossZone;          // For pricing
    HistoryNode* next;         // Next history entry
};
```

**Insertion:** Always at head (O(1))
```cpp
void addToHistory(request, slotID, zoneID, isCrossZone) {
    HistoryNode* newNode = new HistoryNode(...);
    newNode->next = historyHead;
    historyHead = newNode;
    historyCount++;
}
```

**Traversal for Analytics:** Linear scan (O(n))
```cpp
HistoryNode* current = historyHead;
while (current != nullptr) {
    // Process analytics
    if (current->request.getState() == RELEASED) {
        // Count in completed
        // Add to duration sum
        // Add to revenue
    }
    current = current->next;
}
```

**Why Linked List?**
- Dynamic growth without reallocation
- Maintains insertion order (newest first)
- Efficient O(1) insertion
- Complete audit trail
- No size limit (memory permitting)

---

## Web Interface Architecture

### Technology Stack
- **HTML5**: Structure and content
- **CSS3**: Styling with animations
- **JavaScript (ES6)**: Logic and interactivity
- **No external frameworks**: Vanilla JavaScript only

### Component Structure

**1. Navigation Bar**
- Brand logo with float animation
- System status indicator (online/offline)
- Sticky positioning for accessibility

**2. Hero Section**
- Gradient title with animation
- System tagline
- Animated entrance effects

**3. Statistics Cards (4 cards)**
- Total Slots
- Available Slots
- Occupied Slots
- Utilization Percentage
- Real-time updates
- Hover effects with scaling

**4. Zone Overview Grid**
- Visual representation of 3 zones
- Color-coded status (available/partial/full)
- Slot count display
- Individual slot visualization
- Car emoji for occupied slots

**5. Control Panel (5 cards)**
- **Create Request**: Vehicle ID, Zone, Request Time
- **Allocate Parking**: Request ID input
- **Occupy Slot**: Request ID input
- **Cancel Request**: Request ID input (purple themed)
- **Release Slot**: Request ID, Release Time
- Each card has result message area

**6. Analytics Dashboard (7 metrics)**
- Total Requests
- Completed Requests
- Cancelled Requests
- Average Duration
- Cross-Zone Count
- Peak Zone
- **Total Revenue** (₨)

**7. Activity Log**
- Real-time event logging
- Color-coded entries (success/error/info)
- Timestamp for each entry
- Auto-scroll to latest
- Limited to 10 recent entries

### Data Flow

```
User Action (Click Button)
    ↓
JavaScript Function Called
    ↓
Validate Input Data
    ↓
Update parkingData Object
    ↓
Calculate Changes (pricing, analytics)
    ↓
Update DOM Elements
    ↓
Show Result Message
    ↓
Add Log Entry
    ↓
Refresh Statistics
    ↓
Update Zone Visualization
```

### State Management

**Central Data Object:**
```javascript
let parkingData = {
    zones: [...],           // Array of zone objects
    requests: [],           // Array of request objects
    nextRequestID: 1,       // Auto-increment counter
    analytics: {...},       // Metrics object
    pricing: {...}          // Pricing configuration
};
```

**State Updates:**
- All changes go through central `parkingData` object
- No direct DOM manipulation without state update
- State changes trigger UI refresh
- Unidirectional data flow

### Visual Design

**Color Scheme:**
- Primary: `#00d4ff` (Cyan)
- Secondary: `#ff6b9d` (Pink)
- Success: `#00ff88` (Green)
- Warning: `#ffd600` (Yellow)
- Danger: `#ff4757` (Red)
- Cancel: `#9b59b6` (Purple)

**Animations:**
- Float (logo): 3s infinite
- Pulse (status dot): 2s infinite
- Bounce (stat icons): 2s infinite
- Slide-in (log entries): 0.3s
- Gradient shift: 3s infinite

**Glassmorphism Effect:**
- Background: `rgba(255, 255, 255, 0.05)`
- Backdrop filter: `blur(10px)`
- Border: `rgba(255, 255, 255, 0.1)`
- Modern, sleek appearance

---

## Time and Space Complexity Analysis

### Core Operations

| Operation | Time Complexity | Space Complexity | Explanation |
|-----------|----------------|------------------|-------------|
| **Create Request** | O(1) | O(1) | Add to array with auto-increment ID |
| **Allocate Slot** | O(n×m) | O(1) | Search zones (n) and areas (m) |
| **Occupy Slot** | O(n) | O(1) | Find request in active array |
| **Release Slot** | O(n + m) | O(1) | Find request + update zone |
| **Cancel Request** | O(n) | O(1) | Find request, update state |
| **Rollback Single** | O(1) | O(1) | Pop from stack |
| **Rollback K Operations** | O(k×m) | O(1) | k pops, each frees slot (m areas) |
| **Add to History** | O(1) | O(1) | Prepend to linked list |
| **Calculate Analytics** | O(h) | O(1) | Traverse history (h entries) |
| **Get Zone Utilization** | O(z×a) | O(1) | Sum slots across zones (z) and areas (a) |
| **Find Peak Zone** | O(z) | O(1) | Iterate through zones |

### Data Structure Space Complexity

| Component | Space | Notes |
|-----------|-------|-------|
| **Zones Array** | O(z) | z = number of zones (3) |
| **Parking Areas** | O(z×a) | a = areas per zone |
| **Parking Slots** | O(s) | s = total slots (24) |
| **Active Requests** | O(r) | r = active requests |
| **History List** | O(h) | h = historical requests |
| **Rollback Stack** | O(k) | k = max stack size (1000) |
| **Adjacency Lists** | O(z²) | Worst case: complete graph |

**Total Space:** O(s + r + h + k) where s=24, h grows unbounded, k=1000

### Optimization Opportunities

**Current Implementation:**
- Linear search for available slots: O(n×m)
- Could use priority queue: O(log n)

**History Traversal:**
- Current: O(h) for analytics
- Could cache: O(1) with incremental updates

**Zone Lookup:**
- Current: O(z) linear search
- Could use hash map: O(1)

**Justified Choices:**
- Small dataset (3 zones, 24 slots) makes linear search acceptable
- Simplicity and clarity prioritized over micro-optimizations
- No STL containers as per project constraints
- Custom implementations demonstrate understanding of data structures

### Scalability Analysis

**Current System (3 zones, 24 slots):**
- All operations < 1ms
- Memory footprint < 10KB
- Suitable for simulation

**Scaled System (100 zones, 10,000 slots):**
- Allocation: O(100×50) = 5000 iterations worst-case
- Would benefit from indexed search
- History would grow significantly
- Consider database for persistence

---

## Testing Strategy

### Test Cases Implemented

1. **Basic Allocation**: Same-zone slot allocation
2. **Cross-Zone Allocation**: When preferred zone full
3. **State Transitions**: Valid lifecycle progression
4. **Invalid State Transition**: Preventing illegal moves
5. **Cancel REQUESTED**: Early cancellation
6. **Cancel ALLOCATED**: Slot restoration on cancel
7. **Cancel OCCUPIED**: Should fail with error
8. **Rollback Single**: Undo last allocation
9. **Rollback Multiple**: Undo last k allocations
10. **Average Duration**: Correct calculation
11. **Analytics After Rollback**: History preservation
12. **Zone Utilization**: Percentage calculation
13. **Peak Zone**: Highest occupancy detection
14. **Pricing Calculation**: Cost with/without penalty
15. **Revenue Tracking**: Cumulative sum

### Test Coverage
- ✅ All state transitions
- ✅ Boundary conditions (full zones, empty zones)
- ✅ Error handling (invalid IDs, invalid states)
- ✅ Rollback edge cases
- ✅ Analytics accuracy
- ✅ Pricing correctness

---

## Conclusion

The Smart Parking System demonstrates:
- **Strong data structure usage**: Arrays, linked lists, stacks
- **State machine implementation**: Enforced lifecycle
- **Algorithm design**: Allocation with preferences
- **Rollback mechanism**: Stack-based undo
- **Pricing logic**: Dynamic cost calculation
- **Analytics processing**: Real-time metrics
- **Web interface**: Interactive visualization
- **Modular design**: Clean separation of concerns

The system successfully simulates real-world parking management with emphasis on correct data structure usage, state validation, and user experience through an intuitive web interface.

---

## Future Enhancements

Potential improvements:
1. **Reservation System**: Book slots in advance
2. **Priority Parking**: VIP or disabled slots
3. **Multi-level Pricing**: Peak hours, weekend rates
4. **Payment Integration**: Real payment processing
5. **Mobile App**: Native iOS/Android applications
6. **Database Persistence**: Save state across sessions
7. **User Accounts**: Login and parking history
8. **Route Optimization**: Guide to nearest slot
9. **Notification System**: SMS/email alerts
10. **IoT Integration**: Physical sensors for slot detection

---

**Document Version:** 1.0  
**Last Updated:** January 2026  
**Authors:** Maryam053  
**Project:** DSA Semester Project - Smart Parking System
