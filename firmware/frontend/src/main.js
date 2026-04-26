import './styles.css';
import { PlotController } from './plot.js';
import { roastDataManager } from './roastDataManager.js';
import { initializeTriggers } from './trigger.js';
import { TemperatureDisplayController } from './display.js';

const APP_CONFIG = {
  plotElementId: 'plot_temperature',
  displayIntervalMs: 1000
};

function createApp() {
  const plotController = new PlotController(APP_CONFIG.plotElementId, roastDataManager);
  const displayController = new TemperatureDisplayController(
    roastDataManager,
    undefined,
    APP_CONFIG.displayIntervalMs
  );
  const disposeTriggers = initializeTriggers(plotController);

  displayController.start();

  return {
    plotController,
    displayController,
    dispose() {
      disposeTriggers();
      displayController.stop();
      plotController.stopPlotting();
    }
  };
}

createApp();
