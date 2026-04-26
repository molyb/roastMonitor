import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { fetchLog } from '../api.js';
import { RoastDataManager, roastDataManager } from '../roastDataManager.js';

vi.mock('../api.js', () => ({
    fetchLog: vi.fn()
}));

describe('RoastDataManager', () => {
    beforeEach(() => {
        clearInterval(roastDataManager.pollTimer);
        localStorage.clear();
        vi.clearAllMocks();
    });

    afterEach(() => {
        clearInterval(roastDataManager.pollTimer);
    });

    it('loads persisted roast data from localStorage', () => {
        localStorage.setItem('roastData', JSON.stringify({
            beanTemperature: [{ timestamp: 1000, temperature: 180.5 }],
            environmentTemperature: [{ timestamp: 1200, temperature: 190.5 }],
            isRoastSessionActive: true
        }));

        const manager = new RoastDataManager({ autoStartPolling: false });

        expect(manager.getLatestBT()).toEqual({ timestamp: 1000, temperature: 180.5 });
        expect(manager.getLatestET()).toEqual({ timestamp: 1200, temperature: 190.5 });
        expect(manager.isRoastSessionActive).toBe(true);
        expect(manager.lastFetchDate).toBe(1200);
    });

    it('calculates delta temperatures from the recent slope window', () => {
        const manager = new RoastDataManager({ autoStartPolling: false });
        manager.setSmoothingAlpha(1);

        manager.pushBt(0, 100);
        manager.pushBt(1000, 101);
        manager.pushBt(2000, 102);

        expect(manager.getDeltaBT()).toEqual([
            { timestamp: 0, deltaTemperature: 0 },
            { timestamp: 1000, deltaTemperature: 60 },
            { timestamp: 2000, deltaTemperature: 60 }
        ]);
        expect(manager.getLatestDeltaBT()).toEqual({ timestamp: 2000, deltaTemperature: 60 });
    });

    it('prunes old samples and aligned delta cache entries outside the window', () => {
        const manager = new RoastDataManager({ autoStartPolling: false });
        manager.setSmoothingAlpha(1);

        manager.pushBt(0, 100);
        manager.pushBt(60000, 110);
        manager.pushBt(121000, 120);
        manager.getDeltaBT();

        manager.lastFetchDate = 121000;
        manager.pruneOldData();

        expect(manager.beanTemperature).toEqual([
            { timestamp: 60000, temperature: 110 },
            { timestamp: 121000, temperature: 120 }
        ]);
        expect(manager.deltaBeanTemperature).toEqual([
            { timestamp: 60000, deltaTemperature: 0 },
            { timestamp: 121000, deltaTemperature: 0 }
        ]);
    });

    it('polls new log records and prunes old data when no roast session is active', async () => {
        fetchLog.mockResolvedValue({
            log: [
                { date: 1000, BT: 180, ET: 190 },
                { date: 122000, BT: 181, ET: 191 }
            ]
        });

        const manager = new RoastDataManager({ autoStartPolling: false });
        await manager.pollLog();

        expect(fetchLog).toHaveBeenCalledWith(0);
        expect(manager.beanTemperature).toEqual([{ timestamp: 122000, temperature: 181 }]);
        expect(manager.environmentTemperature).toEqual([{ timestamp: 122000, temperature: 191 }]);
        expect(manager.lastFetchDate).toBe(122000);
    });

    it('resets in-memory and persisted roast data', () => {
        const manager = new RoastDataManager({ autoStartPolling: false });
        manager.setRecording(true);
        manager.push(1000, 180, 190);

        expect(localStorage.getItem('roastData')).not.toBeNull();

        manager.reset();

        expect(manager.beanTemperature).toEqual([]);
        expect(manager.environmentTemperature).toEqual([]);
        expect(manager.deltaBeanTemperature).toEqual([]);
        expect(manager.deltaEnvironmentTemperature).toEqual([]);
        expect(manager.isRecording).toBe(false);
        expect(manager.isRoastSessionActive).toBe(false);
        expect(manager.lastFetchDate).toBe(0);
        expect(localStorage.getItem('roastData')).toBeNull();
    });
});
