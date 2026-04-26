import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { TemperatureDisplayController } from '../display.js';

describe('TemperatureDisplayController', () => {
    beforeEach(() => {
        vi.useFakeTimers();
        document.body.innerHTML = `
            <div id="temperature-display-bt"></div>
            <div id="temperature-display-et"></div>
            <div id="temperature-display-delta-bt"></div>
            <div id="temperature-display-delta-et"></div>
        `;
    });

    afterEach(() => {
        vi.useRealTimers();
        document.body.innerHTML = '';
    });

    it('renders the latest temperature values immediately and on each interval', () => {
        const roastDataManager = {
            getLatestBT: vi.fn()
                .mockReturnValueOnce({ temperature: 180.12 })
                .mockReturnValueOnce({ temperature: 181.23 }),
            getLatestET: vi.fn()
                .mockReturnValueOnce({ temperature: 190.34 })
                .mockReturnValueOnce({ temperature: 191.45 }),
            getLatestDeltaBT: vi.fn()
                .mockReturnValueOnce({ deltaTemperature: 12.34 })
                .mockReturnValueOnce({ deltaTemperature: 13.45 }),
            getLatestDeltaET: vi.fn()
                .mockReturnValueOnce({ deltaTemperature: 9.87 })
                .mockReturnValueOnce({ deltaTemperature: 8.76 })
        };

        const controller = new TemperatureDisplayController(roastDataManager, undefined, 500);
        controller.start();

        expect(document.getElementById('temperature-display-bt').textContent).toContain('BT: 180.1');
        expect(document.getElementById('temperature-display-et').textContent).toContain('ET: 190.3');

        vi.advanceTimersByTime(500);

        expect(document.getElementById('temperature-display-bt').textContent).toContain('BT: 181.2');
        expect(document.getElementById('temperature-display-et').textContent).toContain('ET: 191.4');
    });

    it('stops interval updates and falls back to placeholders for invalid temperatures', () => {
        const roastDataManager = {
            getLatestBT: vi.fn()
                .mockReturnValueOnce({ temperature: undefined })
                .mockReturnValueOnce({ temperature: 200 }),
            getLatestET: vi.fn()
                .mockReturnValueOnce({ temperature: null })
                .mockReturnValueOnce({ temperature: 210 }),
            getLatestDeltaBT: vi.fn()
                .mockReturnValueOnce({ deltaTemperature: undefined })
                .mockReturnValueOnce({ deltaTemperature: 15 }),
            getLatestDeltaET: vi.fn()
                .mockReturnValueOnce({ deltaTemperature: null })
                .mockReturnValueOnce({ deltaTemperature: 16 })
        };

        const controller = new TemperatureDisplayController(roastDataManager, undefined, 500);
        controller.start();
        controller.stop();

        expect(document.getElementById('temperature-display-bt').textContent).toContain('----.-');
        expect(document.getElementById('temperature-display-et').textContent).toContain('----.-');

        vi.advanceTimersByTime(1000);

        expect(document.getElementById('temperature-display-bt').textContent).not.toContain('200.0');
        expect(document.getElementById('temperature-display-et').textContent).not.toContain('210.0');
    });
});
