import { fetchTc } from './api.js';

const deg = '\u00B0';
const nbsp = '\u00A0';
const delta = '\u0394';

const DEFAULT_DISPLAY_IDS = {
    tc: 'temperature-display-tc',
    bt: 'temperature-display-bt',
    et: 'temperature-display-et',
    deltaBt: 'temperature-display-delta-bt',
    deltaEt: 'temperature-display-delta-et'
};

/**
 * Update a specific temperature display element.
 * @param {string} id
 * @param {string} prefix
 * @param {number} temperature
 */
function updateTemperatureDisplay(id, prefix, temperature) {
    const display = document.getElementById(id);
    if (!display) return;

    const formattedTemp = typeof temperature === 'number' ? temperature.toFixed(1) : '----.-';
    display.innerHTML = `<span class="temp-nowrap">${prefix}${formattedTemp}${nbsp}${deg}C</span>`;
}

export class TemperatureDisplayController {
    constructor(roastDataManager, displayIds = DEFAULT_DISPLAY_IDS, intervalMs = 1000) {
        this.roastDataManager = roastDataManager;
        this.displayIds = displayIds;
        this.intervalMs = intervalMs;
        this.timer = null;
        this.isUpdatingTc = false;
    }

    start() {
        if (this.timer !== null) return;

        this.update();
        this.timer = setInterval(() => this.update(), this.intervalMs);
    }

    stop() {
        if (this.timer === null) return;

        clearInterval(this.timer);
        this.timer = null;
    }

    update() {
        const bt = this.roastDataManager.getLatestBT();
        const et = this.roastDataManager.getLatestET();
        const deltaBt = this.roastDataManager.getLatestDeltaBT();
        const deltaEt = this.roastDataManager.getLatestDeltaET();

        updateTemperatureDisplay(this.displayIds.bt, 'BT: ', bt.temperature);
        updateTemperatureDisplay(this.displayIds.et, 'ET: ', et.temperature);
        updateTemperatureDisplay(this.displayIds.deltaBt, `${delta}BT: `, deltaBt.deltaTemperature);
        updateTemperatureDisplay(this.displayIds.deltaEt, `${delta}ET: `, deltaEt.deltaTemperature);
        this.updateTc();
    }

    async updateTc() {
        if (this.isUpdatingTc || !document.getElementById(this.displayIds.tc)) return;

        this.isUpdatingTc = true;
        try {
            const data = await fetchTc();
            updateTemperatureDisplay(this.displayIds.tc, 'TC: ', data.tc);
        } catch (error) {
            updateTemperatureDisplay(this.displayIds.tc, 'TC: ', undefined);
            console.error('TC update failed:', error);
        } finally {
            this.isUpdatingTc = false;
        }
    }
}
