document.addEventListener('DOMContentLoaded', async () => {
    const searchForm = document.getElementById('flight-search-form');
    const tabs = document.querySelectorAll('.search-tab');
    const returnDateContainer = document.getElementById('return-date-container');
    const returnDateInput = document.getElementById('return-date');
    const dateLabel = document.getElementById('date-label');
    const sourceSelect = document.getElementById('source');
    const destSelect = document.getElementById('destination');
    let searchType = 'one-way';

    // Populate cities dynamically from the database
    async function populateCities() {
        try {
            const data = await API.get('/flights');
            if (!data || !data.success) {
                console.error('API Error:', data?.message || 'Unknown error');
                renderOptions(sourceSelect, new Set(), 'Select Origin');
                renderOptions(destSelect, new Set(), 'Select Destination');
                return;
            }

            const sources = new Set();
            const destinations = new Set();

            data.flights.forEach(flight => {
                if (flight.source) sources.add(flight.source.trim());
                if (flight.destination) destinations.add(flight.destination.trim());
            });

            const renderOptions = (select, set, placeholder) => {
                select.innerHTML = `<option value="">${placeholder}</option>`;
                Array.from(set).sort().forEach(city => {
                    const opt = document.createElement('option');
                    opt.value = city;
                    opt.textContent = city;
                    select.appendChild(opt);
                });
            };

            renderOptions(sourceSelect, sources, 'Select Origin');
            renderOptions(destSelect, destinations, 'Select Destination');
        } catch (err) {
            console.error('Failed to load cities:', err);
            sourceSelect.innerHTML = '<option value="">Error loading cities</option>';
            destSelect.innerHTML = '<option value="">Error loading cities</option>';
        }
    }

    // Helper for rendering (moved out to be accessible in both try and catch)
    function renderOptions(select, set, placeholder) {
        select.innerHTML = `<option value="">${placeholder}</option>`;
        Array.from(set).sort().forEach(city => {
            const opt = document.createElement('option');
            opt.value = city;
            opt.textContent = city;
            select.appendChild(opt);
        });
    }

    // Load cities immediately
    populateCities();

    // Set default dates
    const dateInput = document.getElementById('date');
    const today = new Date().toISOString().split('T')[0];
    dateInput.value = today;
    dateInput.min = today;
    
    returnDateInput.min = today;

    // Tab switching logic
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            searchType = tab.dataset.type;

            if (searchType === 'round-trip') {
                returnDateContainer.style.display = 'block';
                returnDateInput.required = true;
                dateLabel.textContent = 'Depart Date';
            } else {
                returnDateContainer.style.display = 'none';
                returnDateInput.required = false;
                dateLabel.textContent = 'Departure Date';
            }
        });
    });

    // Handle search form submission
    searchForm.addEventListener('submit', async (e) => {
        e.preventDefault();

        const source = sourceSelect.value;
        const destination = destSelect.value;
        const date = dateInput.value;
        const returnDate = returnDateInput.value;
        const cabinClass = document.getElementById('class').value;
        const passengers = document.getElementById('passengers').value;

        if (source === destination) {
            alert('Source and destination cannot be the same!');
            return;
        }

        if (searchType === 'round-trip' && returnDate && returnDate < date) {
            alert('Return date cannot be before departure date!');
            return;
        }

        // Store search params in session to use on results page
        const searchParams = {
            source,
            destination,
            date,
            returnDate,
            cabinClass,
            passengers,
            searchType
        };
        sessionStorage.setItem('last_search', JSON.stringify(searchParams));

        // Redirect to results page
        window.location.href = 'results.html';
    });
});
