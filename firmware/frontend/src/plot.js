import Chart from 'chart.js/auto';

const STORAGE_KEYS = {
  startTime: 'startTime',
  isRunning: 'isRunning'
};

const PLOT_CONFIG = {
  refreshIntervalMs: 1000,
  defaultXAxisMaxMin: 10,
  maxRoastDurationMin: 60,
  xAxisExpansionMarginMin: 0.5,
  xAxisExpansionStepMin: 1
};

const DATASET_INDEX = {
  bt: 0,
  et: 1,
  deltaBt: 2,
  deltaEt: 3
};

const DATASET_DEFINITIONS = [
  {
    label: 'BT',
    borderColor: '#636EFA',
    backgroundColor: '#636EFA',
    yAxisID: 'y'
  },
  {
    label: 'ET',
    borderColor: '#ef713b',
    backgroundColor: '#ef713b',
    yAxisID: 'y'
  },
  {
    label: 'ΔBT',
    borderColor: '#97a1fe',
    backgroundColor: '#97a1fe',
    yAxisID: 'y1',
    borderDash: [5, 5]
  },
  {
    label: 'ΔET',
    borderColor: '#ff9f8e',
    backgroundColor: '#ff9f8e',
    yAxisID: 'y1',
    borderDash: [5, 5]
  }
];

function createDataset(definition) {
  return {
    ...definition,
    data: [],
    tension: 0.1,
    pointRadius: 0,
    normalized: true,
    parsing: false
  };
}

function createChartOptions(initialXAxisMax) {
  return {
    responsive: true,
    maintainAspectRatio: false,
    interaction: {
      mode: 'index',
      intersect: false
    },
    animation: false,
    scales: {
      x: {
        type: 'linear',
        position: 'bottom',
        title: {
          display: true,
          text: 'Time [min]',
          color: '#333',
          font: {
            size: 16,
            weight: 'bold'
          }
        },
        min: 0,
        max: initialXAxisMax,
        ticks: {
          stepSize: 1,
          autoSkip: true,
          precision: 0,
          color: '#333',
          font: {
            size: 14
          }
        }
      },
      y: {
        type: 'linear',
        display: true,
        position: 'left',
        title: {
          display: true,
          text: 'BT / ET',
          color: '#333',
          font: {
            size: 16,
            weight: 'bold'
          }
        },
        min: 0,
        max: 250,
        ticks: {
          color: '#333',
          font: {
            size: 14
          }
        }
      },
      y1: {
        type: 'linear',
        display: true,
        position: 'right',
        title: {
          display: true,
          text: 'ΔBT / ΔET',
          color: '#333',
          font: {
            size: 16,
            weight: 'bold'
          }
        },
        min: 0,
        max: 25,
        grid: {
          drawOnChartArea: false
        },
        ticks: {
          color: '#333',
          font: {
            size: 14
          }
        }
      }
    },
    plugins: {
      legend: {
        position: 'top',
        align: 'start'
      }
    }
  };
}

function createChart(canvasId, initialXAxisMax) {
  const canvas = document.getElementById(canvasId);
  const context = canvas?.getContext('2d');

  if (!context) {
    throw new Error(`Plot canvas not found: ${canvasId}`);
  }

  return new Chart(context, {
    type: 'line',
    data: {
      labels: [],
      datasets: DATASET_DEFINITIONS.map(createDataset)
    },
    options: createChartOptions(initialXAxisMax)
  });
}

function toMinutesFromStart(timestamp, startTime) {
  return (timestamp - startTime) / 60000;
}

function buildSeriesPoints(series, startTime) {
  return series.map((item) => ({
    x: toMinutesFromStart(item.timestamp, startTime),
    y: item.temperature
  }));
}

function buildDeltaPoints(deltaSeries, sourcePoints) {
  return deltaSeries
    .slice(0, sourcePoints.length)
    .map((item, index) => ({
      x: sourcePoints[index].x,
      y: item.deltaTemperature
    }));
}

function getLatestXMinutes(points) {
  return points.length > 0 ? points.at(-1).x : 0;
}

function syncStartStopButtons(isRunning) {
  const buttons = document.querySelectorAll('.control-button');

  buttons.forEach((button) => {
    if (button.value === 'start') {
      button.disabled = isRunning;
    }

    if (button.value === 'stop') {
      button.disabled = !isRunning;
    }
  });
}

function saveNumber(key, value) {
  localStorage.setItem(key, String(value));
}

function loadNumber(key, fallbackValue = 0) {
  const savedValue = localStorage.getItem(key);
  if (savedValue === null) return fallbackValue;

  const parsedValue = parseInt(savedValue, 10);
  return Number.isNaN(parsedValue) ? fallbackValue : parsedValue;
}

function saveBoolean(key, value) {
  localStorage.setItem(key, value ? '1' : '0');
}

function loadBoolean(key, fallbackValue = false) {
  const savedValue = localStorage.getItem(key);
  if (savedValue === null) return fallbackValue;
  return savedValue === '1';
}

function removeStorageItem(key) {
  localStorage.removeItem(key);
}

function pad2(value) {
  return value.toString().padStart(2, '0');
}

function formatExportFilename(baseDate) {
  return `roastprofile_${baseDate.getFullYear()}${pad2(baseDate.getMonth() + 1)}${pad2(baseDate.getDate())}_${pad2(baseDate.getHours())}${pad2(baseDate.getMinutes())}.alog`;
}

function findClosestTemperature(targetTimestamp, series) {
  let closestRecord = null;
  let smallestDiff = Infinity;

  for (const record of series) {
    const diff = Math.abs(record.timestamp - targetTimestamp);
    if (diff < smallestDiff) {
      smallestDiff = diff;
      closestRecord = record;
    }
  }

  return closestRecord ? closestRecord.temperature : null;
}

function buildAlogExport(btSeries, etSeries, startTime) {
  const timex = [];
  const temp1 = [];
  const temp2 = [];

  for (const btRecord of btSeries) {
    timex.push(Number(toMinutesFromStart(btRecord.timestamp, startTime).toPrecision(10)));

    const etTemperature = findClosestTemperature(btRecord.timestamp, etSeries);
    temp1.push(etTemperature !== null ? Number(etTemperature.toFixed(1)) : -1.0);
    temp2.push(Number(btRecord.temperature.toFixed(2)));
  }

  const baseDate = new Date(startTime);

  return {
    filename: formatExportFilename(baseDate),
    content: {
      mode: 'C',
      roastisodate: `${baseDate.getFullYear()}-${pad2(baseDate.getMonth() + 1)}-${pad2(baseDate.getDate())}`,
      roasttime: `${pad2(baseDate.getHours())}:${pad2(baseDate.getMinutes())}:${pad2(baseDate.getSeconds())}`,
      timex,
      temp1,
      temp2
    }
  };
}

function downloadJsonFile(filename, content) {
  const blob = new Blob([JSON.stringify(content, null, 4)], { type: 'application/octet-stream' });
  const url = URL.createObjectURL(blob);
  const link = document.createElement('a');

  link.href = url;
  link.setAttribute('download', filename);
  link.click();

  setTimeout(() => URL.revokeObjectURL(url), 100);
}

export class PlotController {
  constructor(plotElementId, roastDataManager) {
    this.plotElementId = plotElementId;
    this.roastDataManager = roastDataManager;
    this.intervalMs = PLOT_CONFIG.refreshIntervalMs;
    this.defaultXAxisMaxMin = PLOT_CONFIG.defaultXAxisMaxMin;

    this.chart = createChart(this.plotElementId, this.defaultXAxisMaxMin);
    this.timer = null;
    this.startTime = loadNumber(STORAGE_KEYS.startTime, 0);
    this.isRunning = loadBoolean(STORAGE_KEYS.isRunning, false);

    this.restorePlot();

    if (this.isRunning) {
      this.startPlotting();
    } else {
      this.updateControlButtons();
    }
  }

  startPlotting() {
    if (this.timer !== null) return;

    if (this.startTime === 0) {
      this.startTime = Date.now();
      saveNumber(STORAGE_KEYS.startTime, this.startTime);
    }

    this.roastDataManager.setRecording(true);
    this.timer = setInterval(() => this.refreshPlot(), this.intervalMs);
    this.isRunning = true;
    saveBoolean(STORAGE_KEYS.isRunning, this.isRunning);
    this.updateControlButtons();
    this.refreshPlot();
  }

  stopPlotting() {
    if (this.timer !== null) {
      clearInterval(this.timer);
      this.timer = null;
    }

    this.isRunning = false;
    this.roastDataManager.setRecording(false);
    saveBoolean(STORAGE_KEYS.isRunning, this.isRunning);
    this.updateControlButtons();
  }

  resetPlot() {
    this.stopPlotting();
    this.roastDataManager.reset();
    this.startTime = 0;
    removeStorageItem(STORAGE_KEYS.startTime);

    this.clearChartData();
    this.resetXAxisRange();
    this.chart.update();
  }

  updateControlButtons() {
    syncStartStopButtons(this.isRunning);
  }

  restorePlot() {
    if (this.roastDataManager.beanTemperature.length === 0 || this.startTime <= 0) {
      this.updateControlButtons();
      return;
    }

    this.refreshPlot();
  }

  refreshPlot() {
    if (this.startTime <= 0) return;

    const plotData = this.buildPlotData();
    this.applyPlotData(plotData);
    this.expandXAxisIfNeeded(plotData.latestMinutes);
    this.chart.update('none');

    if (plotData.latestMinutes >= PLOT_CONFIG.maxRoastDurationMin) {
      this.stopPlotting();
    }
  }

  buildPlotData() {
    const btPoints = buildSeriesPoints(this.roastDataManager.beanTemperature, this.startTime);
    const etPoints = buildSeriesPoints(this.roastDataManager.environmentTemperature, this.startTime);
    const deltaBtPoints = buildDeltaPoints(this.roastDataManager.getDeltaBT(), btPoints);
    const deltaEtPoints = buildDeltaPoints(this.roastDataManager.getDeltaET(), etPoints);

    return {
      btPoints,
      etPoints,
      deltaBtPoints,
      deltaEtPoints,
      latestMinutes: getLatestXMinutes(btPoints)
    };
  }

  applyPlotData(plotData) {
    this.chart.data.datasets[DATASET_INDEX.bt].data = plotData.btPoints;
    this.chart.data.datasets[DATASET_INDEX.et].data = plotData.etPoints;
    this.chart.data.datasets[DATASET_INDEX.deltaBt].data = plotData.deltaBtPoints;
    this.chart.data.datasets[DATASET_INDEX.deltaEt].data = plotData.deltaEtPoints;
  }

  expandXAxisIfNeeded(latestMinutes) {
    const currentMax = this.chart.options.scales.x.max;
    if (latestMinutes < currentMax - PLOT_CONFIG.xAxisExpansionMarginMin) {
      return;
    }

    this.chart.options.scales.x.max = latestMinutes + PLOT_CONFIG.xAxisExpansionStepMin;
  }

  clearChartData() {
    this.chart.data.labels = [];
    this.chart.data.datasets.forEach((dataset) => {
      dataset.data = [];
    });
  }

  resetXAxisRange() {
    this.chart.options.scales.x.max = this.defaultXAxisMaxMin;
  }

  downloadPlotAlog() {
    const btSeries = this.roastDataManager.beanTemperature || [];
    const etSeries = this.roastDataManager.environmentTemperature || [];
    const exportStartTime = this.startTime > 0 ? this.startTime : Date.now();
    const alogFile = buildAlogExport(btSeries, etSeries, exportStartTime);

    downloadJsonFile(alogFile.filename, alogFile.content);
  }
}
