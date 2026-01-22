// Simulated Backend Data
let parkingData = {
    zones: [
        { id: 1, name: "Zone 1", total: 10, occupied: 0 },
        { id: 2, name: "Zone 2", total: 8, occupied: 0 },
        { id: 3, name: "Zone 3", total: 6, occupied: 0 }
    ],
    requests: [],
    nextRequestID: 1,
    analytics: {
        total: 0,
        completed: 0,
        cancelled: 0,
        avgDuration: 0,
        crossZone: 0,
        totalRevenue: 0  // NEW: Total revenue tracking
    },
    pricing: {
        perUnitRate: 10,        // 10 PKR per time unit
        crossZonePenalty: 50    // 50 PKR extra for cross-zone
    }
};

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    updateAllStats();
    updateZoneDisplay();
    addLog('System initialized - All zones operational', 'success');
});

// Utility Functions
function getCurrentTime() {
    const now = new Date();
    return now.toLocaleTimeString('en-US', { hour12: false });
}

function addLog(message, type = 'info') {
    const logContainer = document.getElementById('activityLog');
    const logEntry = document.createElement('div');
    logEntry.className = `log-entry ${type}`;
    logEntry.innerHTML = `
        <span class="log-time">${getCurrentTime()}</span>
        <span class="log-message">${message}</span>
    `;
    logContainer.insertBefore(logEntry, logContainer.firstChild);
    
    // Keep only last 10 entries
    while (logContainer.children.length > 10) {
        logContainer.removeChild(logContainer.lastChild);
    }
}

function showResult(elementId, message, isSuccess) {
    const element = document.getElementById(elementId);
    element.textContent = message;
    element.className = `result-message ${isSuccess ? 'success' : 'error'}`;
    element.style.display = 'block';
    
    setTimeout(() => {
        element.style.display = 'none';
    }, 5000);
}

// NEW: Calculate Parking Cost
function calculateCost(duration, isCrossZone) {
    const baseCost = duration * parkingData.pricing.perUnitRate;
    const crossZoneFee = isCrossZone ? parkingData.pricing.crossZonePenalty : 0;
    return baseCost + crossZoneFee;
}

// Update Statistics
function updateAllStats() {
    const totalSlots = parkingData.zones.reduce((sum, zone) => sum + zone.total, 0);
    const totalOccupied = parkingData.zones.reduce((sum, zone) => sum + zone.occupied, 0);
    const totalAvailable = totalSlots - totalOccupied;
    const utilization = totalSlots > 0 ? ((totalOccupied / totalSlots) * 100).toFixed(0) : 0;
    
    document.getElementById('totalSlots').textContent = totalSlots;
    document.getElementById('availableSlots').textContent = totalAvailable;
    document.getElementById('occupiedSlots').textContent = totalOccupied;
    document.getElementById('utilization').textContent = utilization + '%';
    
    // Analytics
    document.getElementById('totalRequests').textContent = parkingData.analytics.total;
    document.getElementById('completedRequests').textContent = parkingData.analytics.completed;
    document.getElementById('cancelledRequests').textContent = parkingData.analytics.cancelled;
    document.getElementById('avgDuration').textContent = parkingData.analytics.avgDuration.toFixed(0);
    document.getElementById('crossZone').textContent = parkingData.analytics.crossZone;
    
    // NEW: Update revenue
    document.getElementById('totalRevenue').textContent = parkingData.analytics.totalRevenue.toFixed(0);
    
    const peakZone = getPeakZone();
    document.getElementById('peakZone').textContent = peakZone > 0 ? `Zone ${peakZone}` : '-';
}

function getPeakZone() {
    let maxOccupied = -1;
    let peakZone = -1;
    
    parkingData.zones.forEach(zone => {
        if (zone.occupied > maxOccupied) {
            maxOccupied = zone.occupied;
            peakZone = zone.id;
        }
    });
    
    return peakZone;
}

function updateZoneDisplay() {
    parkingData.zones.forEach(zone => {
        const zoneCard = document.querySelector(`[data-zone="${zone.id}"]`);
        if (!zoneCard) return;
        
        const available = zone.total - zone.occupied;
        const statusSpan = zoneCard.querySelector('.zone-status');
        statusSpan.textContent = `${available}/${zone.total}`;
        
        // Update status color
        const percentage = (zone.occupied / zone.total) * 100;
        if (percentage >= 80) {
            statusSpan.className = 'zone-status full';
        } else if (percentage >= 50) {
            statusSpan.className = 'zone-status partial';
        } else {
            statusSpan.className = 'zone-status available';
        }
        
        // Update slot visuals
        const slots = zoneCard.querySelectorAll('.slot');
        slots.forEach((slot, index) => {
            if (index < zone.occupied) {
                slot.className = 'slot occupied';
            } else {
                slot.className = 'slot available';
            }
        });
    });
}

// Main Functions
function createRequest() {
    const vehicleID = document.getElementById('vehicleID').value.trim();
    const zone = parseInt(document.getElementById('zone').value);
    const requestTime = parseInt(document.getElementById('requestTime').value);
    
    if (!vehicleID) {
        showResult('requestResult', 'Please enter Vehicle ID', false);
        return;
    }
    
    if (!zone || zone < 1 || zone > 3) {
        showResult('requestResult', 'Please enter valid Zone (1-3)', false);
        return;
    }
    
    if (!requestTime || requestTime < 0) {
        showResult('requestResult', 'Please enter valid Request Time', false);
        return;
    }
    
    const requestID = parkingData.nextRequestID++;
    const request = {
        id: requestID,
        vehicleID: vehicleID,
        zone: zone,
        requestTime: requestTime,
        state: 'REQUESTED',
        allocatedZone: null,
        allocatedSlot: null,
        isCrossZone: false,  // NEW: Track if cross-zone
        cost: 0              // NEW: Track parking cost
    };
    
    parkingData.requests.push(request);
    parkingData.analytics.total++;
    
    showResult('requestResult', `✅ Request created! ID: ${requestID}`, true);
    addLog(`Request #${requestID} created for ${vehicleID} (Zone ${zone})`, 'success');
    
    updateAllStats();
    
    // Clear inputs
    document.getElementById('vehicleID').value = '';
    document.getElementById('zone').value = '';
    document.getElementById('requestTime').value = '';
}

function allocateParking() {
    const requestID = parseInt(document.getElementById('allocateID').value);
    
    if (!requestID) {
        showResult('allocateResult', 'Please enter Request ID', false);
        return;
    }
    
    const request = parkingData.requests.find(r => r.id === requestID);
    
    if (!request) {
        showResult('allocateResult', 'Request not found', false);
        return;
    }
    
    if (request.state !== 'REQUESTED') {
        showResult('allocateResult', 'Request already processed', false);
        return;
    }
    
    // Find available zone
    const requestedZone = parkingData.zones.find(z => z.id === request.zone);
    
    if (requestedZone && requestedZone.occupied < requestedZone.total) {
        // Allocate in requested zone
        request.state = 'ALLOCATED';
        request.allocatedZone = requestedZone.id;
        request.allocatedSlot = requestedZone.occupied + 1;
        request.isCrossZone = false;  // NEW: Same zone
        
        showResult('allocateResult', `✅ Allocated in Zone ${requestedZone.id}, Slot ${request.allocatedSlot}`, true);
        addLog(`Request #${requestID} allocated in Zone ${requestedZone.id}`, 'success');
    } else {
        // Try adjacent zones (cross-zone allocation)
        const availableZone = parkingData.zones.find(z => z.occupied < z.total);
        
        if (availableZone) {
            request.state = 'ALLOCATED';
            request.allocatedZone = availableZone.id;
            request.allocatedSlot = availableZone.occupied + 1;
            request.isCrossZone = true;  // NEW: Cross zone
            parkingData.analytics.crossZone++;
            
            showResult('allocateResult', `✅ Cross-zone allocation: Zone ${availableZone.id}, Slot ${request.allocatedSlot} (+₨50 penalty)`, true);
            addLog(`Request #${requestID} cross-allocated to Zone ${availableZone.id} (₨50 penalty)`, 'info');
        } else {
            showResult('allocateResult', '❌ No slots available in any zone', false);
            addLog(`Request #${requestID} failed - No slots available`, 'error');
            return;
        }
    }
    
    updateAllStats();
    document.getElementById('allocateID').value = '';
}

function occupySlot() {
    const requestID = parseInt(document.getElementById('occupyID').value);
    
    if (!requestID) {
        showResult('occupyResult', 'Please enter Request ID', false);
        return;
    }
    
    const request = parkingData.requests.find(r => r.id === requestID);
    
    if (!request) {
        showResult('occupyResult', 'Request not found', false);
        return;
    }
    
    if (request.state !== 'ALLOCATED') {
        showResult('occupyResult', 'Request must be allocated first', false);
        return;
    }
    
    request.state = 'OCCUPIED';
    request.occupyTime = Date.now();
    
    // Update zone occupancy
    const zone = parkingData.zones.find(z => z.id === request.allocatedZone);
    if (zone) {
        zone.occupied++;
    }
    
    showResult('occupyResult', `✅ Slot occupied in Zone ${request.allocatedZone}`, true);
    addLog(`Request #${requestID} occupied (${request.vehicleID})`, 'success');
    
    updateAllStats();
    updateZoneDisplay();
    document.getElementById('occupyID').value = '';
}

// Cancel Request Function
function cancelRequest() {
    const requestID = parseInt(document.getElementById('cancelID').value);
    
    if (!requestID) {
        showResult('cancelResult', 'Please enter Request ID', false);
        return;
    }
    
    const requestIndex = parkingData.requests.findIndex(r => r.id === requestID);
    
    if (requestIndex === -1) {
        showResult('cancelResult', 'Request not found', false);
        return;
    }
    
    const request = parkingData.requests[requestIndex];
    
    // Check if request can be cancelled - Only REQUESTED or ALLOCATED
    if (request.state === 'OCCUPIED') {
        showResult('cancelResult', '❌ Cannot cancel - vehicle is already parked!', false);
        return;
    }
    
    if (request.state === 'RELEASED') {
        showResult('cancelResult', '❌ Cannot cancel a released request', false);
        return;
    }
    
    if (request.state === 'CANCELLED') {
        showResult('cancelResult', '❌ Request already cancelled', false);
        return;
    }
    
    const previousState = request.state;
    
    // Update request state
    request.state = 'CANCELLED';
    
    // Update analytics
    parkingData.analytics.cancelled++;
    
    showResult('cancelResult', `✅ Request #${requestID} cancelled successfully`, true);
    addLog(`Request #${requestID} cancelled (was ${previousState}) - ${request.vehicleID}`, 'error');
    
    updateAllStats();
    updateZoneDisplay();
    document.getElementById('cancelID').value = '';
}

// NEW: Release Slot with Pricing Calculation
function releaseSlot() {
    const requestID = parseInt(document.getElementById('releaseID').value);
    const releaseTime = parseInt(document.getElementById('releaseTime').value);
    
    if (!requestID) {
        showResult('releaseResult', 'Please enter Request ID', false);
        return;
    }
    
    if (!releaseTime) {
        showResult('releaseResult', 'Please enter Release Time', false);
        return;
    }
    
    const request = parkingData.requests.find(r => r.id === requestID);
    
    if (!request) {
        showResult('releaseResult', 'Request not found', false);
        return;
    }
    
    if (request.state !== 'OCCUPIED') {
        showResult('releaseResult', 'Slot must be occupied first', false);
        return;
    }
    
    request.state = 'RELEASED';
    request.releaseTime = releaseTime;
    
    // Calculate duration and cost
    const duration = releaseTime - request.requestTime;
    const cost = calculateCost(duration, request.isCrossZone);
    request.cost = cost;
    
    // Update zone occupancy
    const zone = parkingData.zones.find(z => z.id === request.allocatedZone);
    if (zone && zone.occupied > 0) {
        zone.occupied--;
    }
    
    // Update analytics
    parkingData.analytics.completed++;
    parkingData.analytics.totalRevenue += cost;  // NEW: Add to revenue
    
    // Recalculate average duration
    const completedRequests = parkingData.requests.filter(r => r.state === 'RELEASED');
    if (completedRequests.length > 0) {
        const totalDuration = completedRequests.reduce((sum, r) => {
            return sum + (r.releaseTime - r.requestTime);
        }, 0);
        parkingData.analytics.avgDuration = totalDuration / completedRequests.length;
    }
    
    // NEW: Show cost breakdown
    const costBreakdown = request.isCrossZone 
        ? `Duration: ${duration} units | Base: ₨${duration * 10} + Cross-zone: ₨50`
        : `Duration: ${duration} units`;
    
    showResult('releaseResult', `✅ Slot released! ${costBreakdown} | Total: ₨${cost}`, true);
    addLog(`Request #${requestID} released from Zone ${request.allocatedZone} | Cost: ₨${cost}`, 'success');
    
    updateAllStats();
    updateZoneDisplay();
    document.getElementById('releaseID').value = '';
    document.getElementById('releaseTime').value = '';
}

// Add some demo animations
setInterval(() => {
    const statsCards = document.querySelectorAll('.stat-card');
    statsCards.forEach((card, index) => {
        setTimeout(() => {
            card.style.animation = 'none';
            setTimeout(() => {
                card.style.animation = '';
            }, 10);
        }, index * 100);
    });
}, 10000);

// Easter egg - click on logo
document.querySelector('.logo-icon').addEventListener('click', () => {
    const logo = document.querySelector('.logo-icon');
    logo.style.animation = 'none';
    setTimeout(() => {
        logo.style.animation = 'float 3s ease-in-out infinite, spin 1s ease-in-out';
    }, 10);
    
    addLog('🎉 Welcome to Smart Parking System!', 'success');
});

// Add spin animation
const style = document.createElement('style');
style.textContent = `
    @keyframes spin {
        from { transform: rotate(0deg) translateY(0); }
        to { transform: rotate(360deg) translateY(-10px); }
    }
`;
document.head.appendChild(style);