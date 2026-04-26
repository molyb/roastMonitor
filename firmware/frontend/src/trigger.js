/**
 * Initialize control button triggers
 * @param {PlotController} plotController
 */
export function initializeTriggers(plotController) {
    const actionHandlers = {
        start: () => plotController.startPlotting(),
        stop: () => plotController.stopPlotting(),
        reset: () => plotController.resetPlot(),
        export: () => {
            if (typeof plotController.downloadPlotAlog === 'function') {
                plotController.downloadPlotAlog();
                return;
            }

            console.error('No download exporter implemented on plotController');
        }
    };

    /**
     * Handle control button click events
     * @param {Event} event
     */
    function handleControlClick(event) {
        if (!plotController) {
            console.error('PlotController is not provided.');
            return;
        }

        const buttonValue = event.target.value;
        const action = actionHandlers[buttonValue];
        if (!action) {
            console.warn(`Unknown control action: ${buttonValue}`);
            return;
        }

        try {
            action();
        } catch (error) {
            console.error(`Error sending '${buttonValue}' command:`, error);
        }
    }

    const buttons = document.querySelectorAll('.control-button');
    buttons.forEach((button) => {
        button.addEventListener('click', handleControlClick);
    });

    return () => {
        buttons.forEach((button) => {
            button.removeEventListener('click', handleControlClick);
        });
    };
}
