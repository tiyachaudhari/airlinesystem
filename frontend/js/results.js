document.addEventListener('DOMContentLoaded', async () => {
    const resultsContainer = document.getElementById('results-container');
    const routeTitle = document.getElementById('route-title');
    const searchDetails = document.getElementById('search-details');
    const progressContainer = document.getElementById('selection-progress');
    const progressText = document.getElementById('progress-text');
    const selectedOutboundInfo = document.getElementById('selected-outbound');

    const lastSearch = JSON.parse(sessionStorage.getItem('last_search'));

    if (!lastSearch) {
        window.location.href = 'index.html';
        return;
    }

    let selectionPhase = 'outbound';
    let outboundFlightData = null;

    async function loadFlights(from, to, date, currentPhase) {
        resultsContainer.innerHTML = `
            <div style="text-align: center; padding: 5rem;">
                <div style="width: 40px; height: 40px; border: 4px solid var(--light-gray); border-top-color: var(--primary-red); border-radius: 50%; margin: 0 auto 1rem; animation: spin 1s linear infinite;"></div>
                <p>Searching for ${currentPhase} flights...</p>
            </div>
        `;
        
        if (lastSearch.searchType === 'round-trip') {
            progressContainer.style.display = 'block';
            if (currentPhase === 'outbound') {
                progressText.textContent = 'Step 1: Select Outbound Flight';
                selectedOutboundInfo.textContent = '';
            } else {
                progressText.textContent = 'Step 2: Select Return Flight';
                const ob = outboundFlightData;
                selectedOutboundInfo.textContent = `Selected: ${ob.flightId} (${ob.source} to ${ob.destination})`;
            }
        }

        const response = await API.post('/search', {
            source: from,
            destination: to,
            class: lastSearch.cabinClass
        });

        if (response.success && response.flights && response.flights.length > 0) {
            routeTitle.textContent = `${from} to ${to}`;
            searchDetails.textContent = `${date} | ${lastSearch.passengers} Passenger(s) | ${lastSearch.cabinClass.charAt(0).toUpperCase() + lastSearch.cabinClass.slice(1)}`;
            renderResults(response.flights, currentPhase, date);
        } else {
            resultsContainer.innerHTML = `
                <div style="text-align: center; padding: 5rem;" class="animate">
                    <h2 style="color: var(--text-secondary);">No Flights Found</h2>
                    <p style="margin-top: 1rem;">Sorry, we couldn't find any flights for this route.</p>
                    <button class="btn-primary" style="margin-top: 2rem;" onclick="window.location.href='index.html'">Try Another Search</button>
                </div>
            `;
        }
    }

    function renderResults(flights, currentPhase, date) {
        resultsContainer.innerHTML = '';
        flights.forEach(flight => {
            flight.schedules.forEach(schedule => {
                const price = schedule.dynamic_price[lastSearch.cabinClass];
                const card = document.createElement('div');
                card.className = 'flight-card animate';
                card.innerHTML = `
                    <div class="flight-main">
                        <div class="flight-info">
                            <div class="airline-info">
                                <div class="airline-logo"></div>
                                <div>
                                    <span style="font-weight: 700;">MatAir</span>
                                    <div style="font-size: 0.75rem; color: var(--text-secondary);">${flight.flight_id} | ${schedule.aircraft}</div>
                                </div>
                            </div>
                            <div class="flight-time-line">
                                <div class="time-box">
                                    <span class="time">${schedule.departure_time}</span>
                                    <span class="airport">${flight.source}</span>
                                </div>
                                <div class="duration-line">
                                    <span class="duration-text">${Math.floor(schedule.duration_mins/60)}h ${schedule.duration_mins%60}m</span>
                                    <span class="plane-icon">✈</span>
                                </div>
                                <div class="time-box">
                                    <span class="time">${schedule.arrival_time}</span>
                                    <span class="airport">${flight.destination}</span>
                                </div>
                            </div>
                        </div>
                        <div style="display: flex; gap: 10px; align-items: center;">
                            <span style="font-size: 0.85rem; font-weight: 600; color: ${schedule.status === 'On Time' ? 'var(--success)' : 'var(--warning)'};">
                                ● ${schedule.status}
                            </span>
                            <span style="font-size: 0.85rem; color: var(--text-secondary);"> | ${schedule.available_seats[lastSearch.cabinClass]} seats left</span>
                        </div>
                    </div>
                    <div class="flight-price-box">
                        <span class="price-label">Price per adult</span>
                        <span class="price-value">₹${price.toLocaleString()}</span>
                        <button class="view-btn">Select Flight</button>
                    </div>
                `;

                card.querySelector('.view-btn').addEventListener('click', () => {
                    const selectionInfo = {
                        flightId: flight.flight_id,
                        scheduleId: schedule.schedule_id,
                        source: flight.source,
                        destination: flight.destination,
                        price: price,
                        date: date,
                        departure: schedule.departure_time,
                        arrival: schedule.arrival_time
                    };

                    if (lastSearch.searchType === 'round-trip' && currentPhase === 'outbound') {
                        outboundFlightData = selectionInfo;
                        selectionPhase = 'inbound';
                        window.scrollTo({ top: 0, behavior: 'smooth' });
                        loadFlights(lastSearch.destination, lastSearch.source, lastSearch.returnDate, 'inbound');
                    } else {
                        // Final selection
                        const selection = {
                            ...lastSearch,
                            outbound: outboundFlightData || selectionInfo,
                            inbound: currentPhase === 'inbound' ? selectionInfo : null,
                            // Legacy fields for existing pages
                            flightId: (outboundFlightData || selectionInfo).flightId,
                            scheduleId: (outboundFlightData || selectionInfo).scheduleId
                        };
                        sessionStorage.setItem('current_selection', JSON.stringify(selection));
                        window.location.href = 'seats.html';
                    }
                });

                resultsContainer.appendChild(card);
            });
        });
    }

    // Initial load
    loadFlights(lastSearch.source, lastSearch.destination, lastSearch.date, 'outbound');
});
