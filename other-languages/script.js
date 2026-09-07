document.addEventListener('DOMContentLoaded', () => {
    const modeSelect = document.getElementById('mode');
    const typeSelect = document.getElementById('type');
    const presetGroup = document.getElementById('presetGroup');
    const customGroup = document.getElementById('customGroup');
    const proxyField = document.getElementById('proxyField');
    const directoryField = document.getElementById('directoryField');
    const form = document.getElementById('configForm');
    const output = document.getElementById('output');

    // Switch between Presets and Custom Config mode
    modeSelect.addEventListener('change', (e) => {
        const isPreset = e.target.value === 'preset';
        presetGroup.classList.toggle('hidden', !isPreset);
        customGroup.classList.toggle('hidden', isPreset);
    });

    // Switch between Proxy Port and Standalone Directory mode
    typeSelect.addEventListener('change', (e) => {
        const isProxy = e.target.value === '1';
        proxyField.classList.toggle('hidden', !isProxy);
        directoryField.classList.toggle('hidden', isProxy);
    });

    // Handle form submit and communicate with Node.js backend
    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        output.classList.remove('hidden', 'error', 'success');
        output.textContent = "Deploying NGINX configuration and issuing SSL certificate...";

        const payload = {
            mode: modeSelect.value,
            preset: document.getElementById('preset').value,
            configName: document.getElementById('configName').value,
            type: typeSelect.value,
            proxy: document.getElementById('proxy').value,
            directory: document.getElementById('directory').value,
            domain: document.getElementById('domain').value
        };

        try {
            const response = await fetch('/api/deploy', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });

            const data = await response.json();
            output.textContent = data.message;
            output.classList.add(data.success ? 'success' : 'error');
        } catch (err) {
            output.textContent = "Error: Unable to connect to the backend server.";
            output.classList.add('error');
        }
    });
});
