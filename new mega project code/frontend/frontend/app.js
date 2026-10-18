document.addEventListener("DOMContentLoaded", () => {
    loadProfileData();
    startMetricPolling();
});

async function loadProfileData() {
    try {
        const res = await fetch('/api/profile');
        const data = await res.json();

        if (data.profile) {
            document.getElementById('user-fullname').innerText = data.profile.full_name;
            document.getElementById('user-title').innerText = data.profile.title;
            document.getElementById('user-bio').innerText = data.profile.bio;
        }

        const accountsContainer = document.getElementById('accounts-container');
        accountsContainer.innerHTML = '';

        if (data.accounts) {
            data.accounts.forEach(acc => {
                const card = document.createElement('div');
                card.className = 'account-card';
                card.innerHTML = `
                    <div class="acc-info">
                        <strong>${acc.platform_name}</strong>
                        <span>${acc.handle}</span>
                    </div>
                    <a href="${acc.url}" target="_blank" class="acc-link">Visit</a>
                `;
                accountsContainer.appendChild(card);
            });
        }
    } catch (err) {
        console.error("Error loading profile data:", err);
    }
}

async function executeBinary(type) {
    const terminal = document.getElementById('terminal-screen');
    terminal.innerText += `\n\n> Triggering /api/execute/${type}... Please wait...`;
    
    try {
        const res = await fetch(`/api/execute/${type}`);
        const data = await res.json();

        if (data.success) {
            terminal.innerText += `\n[SUCCESS - Executed in ${data.execution_time_seconds}s]\n` + data.output;
        } else {
            terminal.innerText += `\n[ERROR]: ${data.error}`;
        }
        terminal.scrollTop = terminal.scrollHeight;
    } catch (err) {
        terminal.innerText += `\n[FATAL ERROR]: Could not communicate with server API.`;
    }
}

function clearTerminal() {
    document.getElementById('terminal-screen').innerText = "> Terminal cleared. Waiting for command...";
}

function startMetricPolling() {
    setInterval(async () => {
        try {
            const res = await fetch('/api/system/metrics');
            const data = await res.json();

            document.getElementById('cpu-metric').innerText = `${data.cpu_usage_percent}%`;
            document.getElementById('ram-metric').innerText = `${data.ram_usage_mb} MB`;
            document.getElementById('uptime-metric').innerText = `${data.uptime_seconds}s`;
        } catch (e) {
            // Silently ignore polling errors
        }
    }, 2000);
}