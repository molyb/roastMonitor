import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { initializeTriggers } from '../trigger.js';

describe('initializeTriggers', () => {
    let errorSpy;
    let warnSpy;

    beforeEach(() => {
        document.body.innerHTML = `
            <button class="control-button" value="start"></button>
            <button class="control-button" value="stop"></button>
            <button class="control-button" value="reset"></button>
            <button class="control-button" value="export"></button>
            <button class="control-button" value="mystery"></button>
        `;
        errorSpy = vi.spyOn(console, 'error').mockImplementation(() => {});
        warnSpy = vi.spyOn(console, 'warn').mockImplementation(() => {});
    });

    afterEach(() => {
        errorSpy.mockRestore();
        warnSpy.mockRestore();
        document.body.innerHTML = '';
    });

    it('routes control button clicks to the matching plot controller actions', () => {
        const plotController = {
            startPlotting: vi.fn(),
            stopPlotting: vi.fn(),
            resetPlot: vi.fn(),
            downloadPlotAlog: vi.fn()
        };

        initializeTriggers(plotController);

        document.querySelector('[value="start"]').click();
        document.querySelector('[value="stop"]').click();
        document.querySelector('[value="reset"]').click();
        document.querySelector('[value="export"]').click();

        expect(plotController.startPlotting).toHaveBeenCalledTimes(1);
        expect(plotController.stopPlotting).toHaveBeenCalledTimes(1);
        expect(plotController.resetPlot).toHaveBeenCalledTimes(1);
        expect(plotController.downloadPlotAlog).toHaveBeenCalledTimes(1);
    });

    it('warns for unknown button actions and reports missing controllers', () => {
        const dispose = initializeTriggers(null);

        document.querySelector('[value="start"]').click();
        expect(errorSpy).toHaveBeenCalledWith('PlotController is not provided.');

        dispose();

        const plotController = {
            startPlotting: vi.fn(),
            stopPlotting: vi.fn(),
            resetPlot: vi.fn()
        };

        initializeTriggers(plotController);
        document.querySelector('[value="mystery"]').click();

        expect(warnSpy).toHaveBeenCalledWith('Unknown control action: mystery');
    });

    it('detaches listeners when disposed', () => {
        const plotController = {
            startPlotting: vi.fn(),
            stopPlotting: vi.fn(),
            resetPlot: vi.fn(),
            downloadPlotAlog: vi.fn()
        };

        const dispose = initializeTriggers(plotController);
        dispose();

        document.querySelector('[value="start"]').click();
        expect(plotController.startPlotting).not.toHaveBeenCalled();
    });
});
