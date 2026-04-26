import { fetchLog } from './api.js';

const STORAGE_KEY = 'roastData';

const DATA_CONFIG = {
    pollIntervalMs: 1000,
    pruneWindowMs: 120000,
    deltaWindowMs: 20000,
    smoothingAlpha: 0.3,
    minSlopeSpanMs: 1000
};

const EMPTY_TEMPERATURE_RECORD = { timestamp: 0, temperature: 0 };
const EMPTY_DELTA_RECORD = { timestamp: 0, deltaTemperature: 0 };

function createTemperatureRecord(timestamp, temperature) {
    return { timestamp, temperature };
}

function createDeltaRecord(timestamp, deltaTemperature) {
    return { timestamp, deltaTemperature };
}

function getLastTimestamp(series) {
    return series.length > 0 ? series.at(-1).timestamp : 0;
}

function getLastItem(series, fallbackValue) {
    return series.length > 0 ? series.at(-1) : fallbackValue;
}

function resetArray(target) {
    target.length = 0;
}

function pruneSeries(series, deltaCache, threshold) {
    while (series.length > 1 && series[0].timestamp < threshold) {
        series.shift();

        if (deltaCache.length > 0) {
            deltaCache.shift();
        }
    }
}

function saveRoastData(data) {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(data));
}

function loadRoastData() {
    const saved = localStorage.getItem(STORAGE_KEY);
    return saved ? JSON.parse(saved) : null;
}

function clearRoastData() {
    localStorage.removeItem(STORAGE_KEY);
}

export class RoastDataManager {
    constructor({ autoStartPolling = true } = {}) {
        this.beanTemperature = [];
        this.environmentTemperature = [];
        this.deltaBeanTemperature = [];
        this.deltaEnvironmentTemperature = [];

        this.deltaTime4temperatureDiff = DATA_CONFIG.deltaWindowMs;
        this.smoothingAlpha = DATA_CONFIG.smoothingAlpha;
        this.isRecording = false;
        this.isRoastSessionActive = false;
        this.lastFetchDate = 0;
        this.pollTimer = null;

        this.loadFromLocalStorage();

        if (autoStartPolling) {
            this.startPolling();
        }
    }

    startPolling() {
        this.pollTimer = setInterval(() => this.pollLog(), DATA_CONFIG.pollIntervalMs);
    }

    setRecording(isRecording) {
        this.isRecording = isRecording;

        if (!isRecording) {
            return;
        }

        this.isRoastSessionActive = true;
        this.saveToLocalStorage();
    }

    async pollLog() {
        try {
            const data = await fetchLog(this.lastFetchDate);
            const records = Array.isArray(data?.log) ? data.log : [];

            records.forEach((record) => {
                this.push(record.date, record.BT, record.ET);
                this.lastFetchDate = Math.max(this.lastFetchDate, record.date);
            });

            if (!this.isRoastSessionActive) {
                this.pruneOldData();
            }
        } catch (error) {
            console.error('Polling error:', error);
        }
    }

    pruneOldData() {
        if (this.lastFetchDate === 0) return;

        const threshold = this.lastFetchDate - DATA_CONFIG.pruneWindowMs;
        pruneSeries(this.beanTemperature, this.deltaBeanTemperature, threshold);
        pruneSeries(this.environmentTemperature, this.deltaEnvironmentTemperature, threshold);
    }

    pushBt(timestamp, bt) {
        this.beanTemperature.push(createTemperatureRecord(timestamp, bt));
        this.persistIfRecording();
    }

    pushEt(timestamp, et) {
        this.environmentTemperature.push(createTemperatureRecord(timestamp, et));
        this.persistIfRecording();
    }

    push(timestamp, bt, et) {
        this.pushBt(timestamp, bt);
        this.pushEt(timestamp, et);
    }

    reset() {
        resetArray(this.beanTemperature);
        resetArray(this.environmentTemperature);
        resetArray(this.deltaBeanTemperature);
        resetArray(this.deltaEnvironmentTemperature);

        this.isRecording = false;
        this.isRoastSessionActive = false;
        this.lastFetchDate = 0;

        this.clearLocalStorage();
    }

    persistIfRecording() {
        if (!this.isRecording) return;
        this.saveToLocalStorage();
    }

    saveToLocalStorage() {
        try {
            saveRoastData({
                beanTemperature: this.beanTemperature,
                environmentTemperature: this.environmentTemperature,
                isRoastSessionActive: this.isRoastSessionActive
            });
        } catch (error) {
            console.error('Error saving to localStorage:', error);
        }
    }

    loadFromLocalStorage() {
        try {
            const savedData = loadRoastData();

            if (!savedData) {
                this.resetLoadedState();
                return;
            }

            this.beanTemperature = savedData.beanTemperature || [];
            this.environmentTemperature = savedData.environmentTemperature || [];
            this.isRoastSessionActive = savedData.isRoastSessionActive !== undefined
                ? savedData.isRoastSessionActive
                : this.beanTemperature.length > 0;

            this.lastFetchDate = Math.max(
                getLastTimestamp(this.beanTemperature),
                getLastTimestamp(this.environmentTemperature)
            );

            this.invalidateDeltaCaches();
        } catch (error) {
            console.error('Error loading from localStorage:', error);
            this.resetLoadedState();
        }
    }

    resetLoadedState() {
        this.beanTemperature = [];
        this.environmentTemperature = [];
        this.isRoastSessionActive = false;
        this.lastFetchDate = 0;
        this.invalidateDeltaCaches();
    }

    clearLocalStorage() {
        try {
            clearRoastData();
        } catch (error) {
            console.error('Error clearing localStorage:', error);
        }
    }

    invalidateDeltaCaches() {
        resetArray(this.deltaBeanTemperature);
        resetArray(this.deltaEnvironmentTemperature);
    }

    calculateSlope(data, currentIndex, windowMs) {
        const currentRecord = data[currentIndex];
        const threshold = currentRecord.timestamp - windowMs;

        let startIndex = currentIndex;
        while (startIndex > 0 && data[startIndex - 1].timestamp >= threshold) {
            startIndex--;
        }

        const count = currentIndex - startIndex + 1;
        if (count < 2) return 0;

        const first = data[startIndex];
        const last = data[currentIndex];
        if ((last.timestamp - first.timestamp) < DATA_CONFIG.minSlopeSpanMs) {
            return 0;
        }

        let sumTime = 0;
        let sumTemperature = 0;
        for (let index = startIndex; index <= currentIndex; index++) {
            sumTime += data[index].timestamp;
            sumTemperature += data[index].temperature;
        }

        const meanTime = sumTime / count;
        const meanTemperature = sumTemperature / count;

        let numerator = 0;
        let denominator = 0;
        for (let index = startIndex; index <= currentIndex; index++) {
            const deltaTime = data[index].timestamp - meanTime;
            denominator += deltaTime * deltaTime;
            numerator += deltaTime * (data[index].temperature - meanTemperature);
        }

        if (denominator === 0) return 0;

        const slopePerMs = numerator / denominator;
        return slopePerMs * 60000;
    }

    updateDeltaCache(data, cache) {
        if (cache.length > data.length) {
            resetArray(cache);
        }

        for (let index = cache.length; index < data.length; index++) {
            const rawDelta = this.calculateSlope(data, index, this.deltaTime4temperatureDiff);
            const previousDelta = cache.length > 0 ? cache.at(-1).deltaTemperature : rawDelta;
            const smoothedDelta = cache.length > 0
                ? this.smoothingAlpha * rawDelta + (1 - this.smoothingAlpha) * previousDelta
                : rawDelta;

            cache.push(createDeltaRecord(data[index].timestamp, smoothedDelta));
        }
    }

    getDeltaBT() {
        this.updateDeltaCache(this.beanTemperature, this.deltaBeanTemperature);
        return this.deltaBeanTemperature;
    }

    getDeltaET() {
        this.updateDeltaCache(this.environmentTemperature, this.deltaEnvironmentTemperature);
        return this.deltaEnvironmentTemperature;
    }

    getLatestBT() {
        return getLastItem(this.beanTemperature, EMPTY_TEMPERATURE_RECORD);
    }

    getLatestET() {
        return getLastItem(this.environmentTemperature, EMPTY_TEMPERATURE_RECORD);
    }

    getLatestDeltaBT() {
        if (this.beanTemperature.length === 0) {
            return EMPTY_DELTA_RECORD;
        }

        return getLastItem(this.getDeltaBT(), EMPTY_DELTA_RECORD);
    }

    getLatestDeltaET() {
        if (this.environmentTemperature.length === 0) {
            return EMPTY_DELTA_RECORD;
        }

        return getLastItem(this.getDeltaET(), EMPTY_DELTA_RECORD);
    }

    setSmoothingAlpha(alpha) {
        if (typeof alpha !== 'number' || alpha < 0 || alpha > 1) {
            return false;
        }

        this.smoothingAlpha = alpha;
        this.invalidateDeltaCaches();
        return true;
    }
}

export const roastDataManager = new RoastDataManager();
