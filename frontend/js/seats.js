document.addEventListener('DOMContentLoaded', async () => {
    const seatGrid = document.getElementById('seat-grid');
    const selectionDetails = document.getElementById('selection-details');
    const paxCountDisplay = document.getElementById('pax-count');
    const selectedSeatsList = document.getElementById('selected-seats-list');
    const confirmBtn = document.getElementById('confirm-seats-btn');

    const selection = JSON.parse(sessionStorage.getItem('current_selection'));
    if (!selection) {
        window.location.href = 'index.html';
        return;
    }

    const paxCount = parseInt(selection.passengers);
    paxCountDisplay.textContent = paxCount;

    let selectionPhase = 'outbound';
    let outboundSeats = [];
    let inboundSeats = [];
    let currentSelectedSeats = [];

    async function loadSeatMap(flightInfo, phase) {
        currentSelectedSeats = [];
        updateUI();

        selectionDetails.innerHTML = `
            <span style="color: var(--primary-red); font-weight: 700;">${phase.toUpperCase()} LEG:</span> 
            ${flightInfo.flightId} | ${flightInfo.source} to ${flightInfo.destination} | ${selection.cabinClass.toUpperCase()}
        `;

        const response = await API.get(`/flight/${flightInfo.flightId}`);
        if (!response.success) {
            alert('Error loading flight details');
            return;
        }

        const schedule = response.flight.schedules.find(s => s.schedule_id === flightInfo.scheduleId);
        if (!schedule) {
            alert('Error loading schedule details');
            return;
        }

        renderGrid(schedule.seats[selection.cabinClass], phase);
    }

    function renderGrid(seatData, phase) {
        const bookedSeats = seatData.booked;
        const totalPossibleSeats = seatData.total;
        const rows = Math.ceil(totalPossibleSeats / 6);
        const seatLetters = ['A', 'B', 'C', 'D', 'E', 'F'];

        seatGrid.innerHTML = '';

        for (let r = 1; r <= rows; r++) {
            for (let c = 0; c < 7; c++) {
                if (c === 3) {
                    const aisle = document.createElement('div');
                    aisle.className = 'seat aisle';
                    aisle.textContent = r;
                    seatGrid.appendChild(aisle);
                    continue;
                }

                const letterIndex = c < 3 ? c : c - 1;
                const seatId = `${r}${seatLetters[letterIndex]}`;
                const seat = document.createElement('div');
                
                const seatGlobalIndex = (r - 1) * 6 + letterIndex;
                if (seatGlobalIndex >= totalPossibleSeats) {
                    seat.className = 'seat aisle';
                } else {
                    const isBooked = bookedSeats.includes(seatId);
                    seat.className = `seat ${selection.cabinClass} ${isBooked ? 'booked' : ''}`;
                    seat.textContent = seatId;
                    seat.dataset.id = seatId;

                    if (!isBooked) {
                        seat.addEventListener('click', () => {
                            if (seat.classList.contains('selected')) {
                                seat.classList.remove('selected');
                                currentSelectedSeats = currentSelectedSeats.filter(id => id !== seatId);
                            } else {
                                if (currentSelectedSeats.length < paxCount) {
                                    seat.classList.add('selected');
                                    currentSelectedSeats.push(seatId);
                                } else {
                                    alert(`You can only select ${paxCount} seat(s).`);
                                }
                            }
                            updateUI();
                        });
                    }
                }
                seatGrid.appendChild(seat);
            }
        }
    }

    function updateUI() {
        selectedSeatsList.textContent = currentSelectedSeats.length > 0 ? currentSelectedSeats.join(', ') : 'None';
        confirmBtn.disabled = currentSelectedSeats.length !== paxCount;
        confirmBtn.textContent = (selectionPhase === 'outbound' && selection.inbound) ? 'Next: Return Seats' : 'Confirm Seats';
    }

    confirmBtn.addEventListener('click', () => {
        if (selectionPhase === 'outbound') {
            outboundSeats = [...currentSelectedSeats];
            if (selection.inbound) {
                selectionPhase = 'inbound';
                window.scrollTo({ top: 0, behavior: 'smooth' });
                loadSeatMap(selection.inbound, 'inbound');
            } else {
                completeSelection();
            }
        } else {
            inboundSeats = [...currentSelectedSeats];
            completeSelection();
        }
    });

    function completeSelection() {
        const updatedSelection = {
            ...selection,
            outbound: { ...selection.outbound, selectedSeats: outboundSeats },
            inbound: selection.inbound ? { ...selection.inbound, selectedSeats: inboundSeats } : null,
            // Fallback for single legacy selection
            selectedSeats: outboundSeats
        };
        sessionStorage.setItem('current_selection', JSON.stringify(updatedSelection));
        window.location.href = 'booking.html';
    }

    // Initial load
    loadSeatMap(selection.outbound, 'outbound');
});
