document.addEventListener('DOMContentLoaded', () => {
    const summaryContainer = document.getElementById('selection-summary');
    const paxContainer = document.getElementById('passenger-forms-container');
    const bookingForm = document.getElementById('booking-form');
    const totalPriceDisplay = document.getElementById('total-price-display');
    
    const selection = JSON.parse(sessionStorage.getItem('current_selection'));
    const user = API.getUser();

    if (!selection) {
        window.location.href = 'index.html';
        return;
    }

    if (user) {
        document.getElementById('contact-email').value = user.email || '';
    }

    const paxCount = parseInt(selection.passengers);
    let totalPrice = 0;

    // Render Selection Summary
    function renderLeg(leg, title) {
        if (!leg) return '';
        totalPrice += leg.price * paxCount;
        return `
            <div class="card" style="padding: 1.5rem; border-left: 5px solid var(--primary-red);">
                <p style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; font-weight: 700;">${title}</p>
                <h3 style="margin: 0.5rem 0;">${leg.source} ➔ ${leg.destination}</h3>
                <p style="font-size: 0.9rem;">${leg.date} | ${leg.departure}</p>
                <p style="font-size: 0.85rem; color: var(--text-secondary); margin-top: 0.5rem;">Seats: ${leg.selectedSeats.join(', ')}</p>
            </div>
        `;
    }

    summaryContainer.innerHTML = renderLeg(selection.outbound, 'Outbound Leg');
    if (selection.inbound) {
        summaryContainer.innerHTML += renderLeg(selection.inbound, 'Return Leg');
    }
    totalPriceDisplay.textContent = `₹${totalPrice.toLocaleString()}`;

    // Generate passenger forms
    for (let i = 1; i <= paxCount; i++) {
        const card = document.createElement('div');
        card.className = 'card animate';
        card.style.animationDelay = `${i * 0.1}s`;
        card.innerHTML = `
            <h3 style="margin-bottom: 1.5rem; color: var(--primary-red);">Passenger ${i} ${i === 1 ? '(Primary)' : ''}</h3>
            <div class="search-form" style="grid-template-columns: 2fr 1fr 1fr;">
                <div class="form-group">
                    <label>Full Name</label>
                    <input type="text" class="pax-name" placeholder="As per Passport / ID" required>
                </div>
                <div class="form-group">
                    <label>Age</label>
                    <input type="number" class="pax-age" min="0" max="110" required>
                </div>
                <div class="form-group">
                    <label>Gender</label>
                    <select class="pax-gender" required>
                        <option value="Male">Male</option>
                        <option value="Female">Female</option>
                        <option value="Other">Other</option>
                    </select>
                </div>
            </div>
        `;
        paxContainer.appendChild(card);
    }

    bookingForm.addEventListener('submit', async (e) => {
        e.preventDefault();

        if (!API.getUser()) {
            alert('Please login to complete your booking.');
            sessionStorage.setItem('redirect_after_login', 'booking.html');
            window.location.href = 'login.html';
            return;
        }

        const names = document.querySelectorAll('.pax-name');
        const ages = document.querySelectorAll('.pax-age');
        const genders = document.querySelectorAll('.pax-gender');

        const passengers = [];
        for (let i = 0; i < paxCount; i++) {
            passengers.push({
                name: names[i].value,
                age: parseInt(ages[i].value),
                gender: genders[i].value
            });
        }

        async function createBooking(leg) {
            const bookingData = {
                user_id: API.getUser().user_id,
                flight_id: leg.flightId,
                schedule_id: leg.scheduleId,
                class: selection.cabinClass,
                travel_date: leg.date,
                seats: leg.selectedSeats,
                passengers: passengers
            };
            return await API.post('/book', bookingData);
        }

        const outboundRes = await createBooking(selection.outbound);
        if (!outboundRes.success) {
            alert('Outbound booking failed: ' + outboundRes.message);
            return;
        }

        let inboundRes = null;
        if (selection.inbound) {
            inboundRes = await createBooking(selection.inbound);
            if (!inboundRes.success) {
                alert('Outbound confirmed, but Return booking failed: ' + inboundRes.message);
                // In a real app we'd handle cancellation/rollback here
            }
        }

        const finalResult = {
            outbound: outboundRes.booking,
            inbound: inboundRes ? inboundRes.booking : null
        };

        sessionStorage.setItem('last_booking_final', JSON.stringify(finalResult));
        window.location.href = 'confirmation.html';
    });
});
