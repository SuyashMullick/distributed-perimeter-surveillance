document.addEventListener('DOMContentLoaded', () => {
    const statusIndicator = document.getElementById('connection-status');
    const metricsContainer = document.getElementById('metrics-container');
    const nodesBody = document.querySelector('#nodes-table tbody');
    const alertsBody = document.querySelector('#alerts-table tbody');

    async function fetchState() {
        try {
            const [statusRes, alertsRes] = await Promise.all([
                fetch('/api/status'),
                fetch('/api/alerts')
            ]);

            if (statusRes.ok && alertsRes.ok) {
                statusIndicator.textContent = 'Live';
                statusIndicator.className = 'status-indicator connected';

                const state = await statusRes.json();
                const alerts = await alertsRes.json();

                renderMetrics(state.metrics || {});
                renderNodes(state.nodes || {});
                renderAlerts(alerts || []);
            } else {
                throw new Error('Server returned error');
            }
        } catch (err) {
            statusIndicator.textContent = 'Disconnected';
            statusIndicator.className = 'status-indicator disconnected';
        }
    }

    function renderMetrics(metrics) {
        metricsContainer.innerHTML = '';
        for (const [key, value] of Object.entries(metrics)) {
            metricsContainer.innerHTML += `
                <div class="metric-card">
                    <div class="label">${key}</div>
                    <div class="value">${value}</div>
                </div>
            `;
        }
    }

    function renderNodes(nodes) {
        nodesBody.innerHTML = '';
        const sortedNodeKeys = Object.keys(nodes).sort();
        for (const id of sortedNodeKeys) {
            const n = nodes[id];
            const cls = n.health === 'OK' ? 'health-ok' : (n.health === 'FAILED' ? 'health-failed' : 'health-degraded');
            nodesBody.innerHTML += `
                <tr>
                    <td>${id}</td>
                    <td class="${cls}">${n.health}</td>
                    <td>${n.uptime_s.toFixed(1)}</td>
                    <td>${n.last_seen_age_s.toFixed(2)}</td>
                    <td>${n.last_sequence_number}</td>
                </tr>
            `;
        }
    }

    function renderAlerts(alerts) {
        alertsBody.innerHTML = '';
        const recent = alerts.slice().reverse(); // Show newest first
        for (const a of recent) {
            const cls = 'class-' + a.classification.toLowerCase();
            alertsBody.innerHTML += `
                <tr>
                    <td>${a.timestamp_utc}</td>
                    <td>${a.alert_id.substring(0,8)}...</td>
                    <td>${a.source_node_id}</td>
                    <td class="${cls}">${a.classification}</td>
                    <td>${a.processing_latency_ms.toFixed(1)}</td>
                </tr>
            `;
        }
    }

    // Refresh at 1 Hz
    setInterval(fetchState, 1000);
    fetchState();
});
