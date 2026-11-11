
const distanceValue = document.getElementById("distance");
const temperatureValue = document.getElementById("temperature");
const humidityValue = document.getElementById("humidity");
const gasValue = document.getElementById("gas");

const systemStatus = document.getElementById("system-status");
const systemStatusDot = document.getElementById("system-status-dot");
const statusDescription = document.getElementById("status-description");

const alertList = document.getElementById("alert-list");
const alertCount = document.getElementById("alert-count");

// Alert history elements
const alertHistoryList = document.getElementById("alert-history-list");
const historyCount = document.getElementById("history-count");

// Demo alert thresholds
const DISTANCE_WARNING_CM = 30;
const GAS_WARNING_LEVEL = 400;

// Alert history settings
const MAX_HISTORY_ENTRIES = 20;

// Tracks which alerts are currently active
const activeAlerts = new Set();

// Sensor history
const history = {
  labels: [],
  distance: [],
  temperature: [],
  humidity: [],
  gas: []
};

// Alert history data
const alertHistory = [];

// Chart configuration
const commonChartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  animation: false,

  interaction: {
    intersect: false,
    mode: "index"
  },

  plugins: {
    legend: {
      display: false
    }
  },

  scales: {
    x: {
      ticks: {
        color: "#91a0b8",
        maxTicksLimit: 6
      },

      grid: {
        color: "#29364d"
      }
    },

    y: {
      beginAtZero: true,

      ticks: {
        color: "#91a0b8"
      },

      grid: {
        color: "#29364d"
      }
    }
  }
};

// Create an individual sensor chart
function createSensorChart(canvasId, label, color, dataKey) {
  const canvas = document.getElementById(canvasId);

  return new Chart(canvas, {
    type: "line",

    data: {
      labels: history.labels,

      datasets: [
        {
          label: label,
          data: history[dataKey],
          borderColor: color,
          backgroundColor: color + "20",
          tension: 0.3,
          fill: true,
          pointRadius: 2,
          pointHoverRadius: 5,
          borderWidth: 2
        }
      ]
    },

    options: commonChartOptions
  });
}

// Create the four graphs
const distanceChart = createSensorChart(
  "distanceChart",
  "Distance (cm)",
  "#38bdf8",
  "distance"
);

const temperatureChart = createSensorChart(
  "temperatureChart",
  "Temperature (°C)",
  "#f59e0b",
  "temperature"
);

const humidityChart = createSensorChart(
  "humidityChart",
  "Humidity (%)",
  "#22c55e",
  "humidity"
);

const gasChart = createSensorChart(
  "gasChart",
  "Gas level",
  "#ef4444",
  "gas"
);

// Add an event to alert history
function addHistoryEntry(title, description, type) {
  const timestamp = new Date();

  const entry = {
    title,
    description,
    type,
    timestamp: timestamp.toLocaleString()
  };

  // Add newest event at the beginning
  alertHistory.unshift(entry);

  // Keep only the latest 20 events
  if (alertHistory.length > MAX_HISTORY_ENTRIES) {
    alertHistory.pop();
  }

  renderAlertHistory();
}

// Render alert history
function renderAlertHistory() {
  historyCount.textContent = `${alertHistory.length} EVENTS`;

  if (alertHistory.length === 0) {
    alertHistoryList.innerHTML = `
      <div class="history-empty">
        No events recorded yet
      </div>
    `;

    return;
  }

  alertHistoryList.innerHTML = alertHistory.map((entry) => {
    return `
      <div class="history-entry">
        <span class="history-entry-indicator ${entry.type}"></span>

        <div class="history-entry-content">
          <strong class="history-entry-title">
            ${entry.title}
          </strong>

          <span class="history-entry-description">
            ${entry.description}
          </span>
        </div>

        <span class="history-entry-time">
          ${entry.timestamp}
        </span>
      </div>
    `;
  }).join("");
}

// Update the hazard alert panel and record state changes
function updateHazardAlerts(distance, gas) {
  const alerts = [];

  // Obstacle alert
  if (distance < DISTANCE_WARNING_CM) {
    alerts.push({
      key: "obstacle",
      type: "warning",
      title: "Obstacle detected nearby",
      message: `Distance is ${distance} cm · Below ${DISTANCE_WARNING_CM} cm demo threshold`
    });
  }

  // Gas alert
  if (gas > GAS_WARNING_LEVEL) {
    alerts.push({
      key: "gas",
      type: "danger",
      title: "Elevated gas sensor reading",
      message: `MQ-2 analog value is ${gas} · Above ${GAS_WARNING_LEVEL} demo threshold`
    });
  }

  // Current active alert keys
  const currentAlertKeys = new Set(
    alerts.map((alert) => alert.key)
  );

  // Record newly activated alerts
  alerts.forEach((alert) => {
    if (!activeAlerts.has(alert.key)) {
      const indicatorClass =
        alert.type === "danger" ? "red" : "orange";

      addHistoryEntry(
        alert.title,
        alert.message,
        indicatorClass
      );
    }
  });

  // Record alerts that have cleared
  activeAlerts.forEach((previousKey) => {
    if (!currentAlertKeys.has(previousKey)) {
      if (previousKey === "obstacle") {
        addHistoryEntry(
          "Obstacle warning cleared",
          "Distance reading returned above the demo threshold",
          "green"
        );
      }

      if (previousKey === "gas") {
        addHistoryEntry(
          "Gas warning cleared",
          "MQ-2 reading returned below the demo threshold",
          "green"
        );
      }
    }
  });

  // Update active alert tracking
  activeAlerts.clear();

  currentAlertKeys.forEach((key) => {
    activeAlerts.add(key);
  });

  // Update active alert count
  alertCount.textContent = `${alerts.length} ACTIVE`;

  // Show normal state if there are no alerts
  if (alerts.length === 0) {
    alertList.innerHTML = `
      <div class="alert-item alert-normal">
        <span class="alert-indicator green"></span>

        <div>
          <strong>No active hazards detected</strong>
          <span>
            Current simulated readings are below the demo thresholds
          </span>
        </div>
      </div>
    `;

    return;
  }

  // Render active alerts
  alertList.innerHTML = alerts.map((alert) => {
    const indicatorClass =
      alert.type === "danger" ? "red" : "orange";

    const itemClass =
      alert.type === "danger" ? "alert-danger" : "alert-warning";

    return `
      <div class="alert-item ${itemClass}">
        <span class="alert-indicator ${indicatorClass}"></span>

        <div>
          <strong>${alert.title}</strong>
          <span>${alert.message}</span>
        </div>
      </div>
    `;
  }).join("");
}

// Update the main system status
function updateSystemStatus(distance, gas) {
  const obstacleDetected = distance < DISTANCE_WARNING_CM;
  const gasWarning = gas > GAS_WARNING_LEVEL;

  if (obstacleDetected || gasWarning) {
    systemStatus.textContent = "Hazard warning active";
    systemStatus.style.color = "#f59e0b";

    systemStatusDot.style.background = "#f59e0b";
    systemStatusDot.style.boxShadow = "0 0 10px #f59e0b";

    if (obstacleDetected && gasWarning) {
      statusDescription.textContent =
        "Obstacle and elevated gas sensor reading detected";
    } else if (obstacleDetected) {
      statusDescription.textContent =
        "The robot is detecting an object within 30 cm";
    } else {
      statusDescription.textContent =
        "The MQ-2 reading is above the demo threshold";
    }
  } else {
    systemStatus.textContent = "System operating normally";
    systemStatus.style.color = "#22c55e";

    systemStatusDot.style.background = "#22c55e";
    systemStatusDot.style.boxShadow = "0 0 10px #22c55e";

    statusDescription.textContent =
      "No current threshold warnings";
  }
}

// Generate simulated sensor readings
function generateSensorData() {
  const distance = Number((Math.random() * 150 + 10).toFixed(1));
  const temperature = Number((Math.random() * 10 + 25).toFixed(1));
  const humidity = Number((Math.random() * 25 + 45).toFixed(1));
  const gas = Math.floor(Math.random() * 500 + 100);

  // Update sensor cards
  distanceValue.textContent = distance;
  temperatureValue.textContent = temperature;
  humidityValue.textContent = humidity;
  gasValue.textContent = gas;

  // Update status and hazard alerts
  updateSystemStatus(distance, gas);
  updateHazardAlerts(distance, gas);

  // Add readings to sensor history
  const currentTime = new Date().toLocaleTimeString();

  history.labels.push(currentTime);
  history.distance.push(distance);
  history.temperature.push(temperature);
  history.humidity.push(humidity);
  history.gas.push(gas);

  // Keep only the latest 20 readings
  if (history.labels.length > 20) {
    history.labels.shift();
    history.distance.shift();
    history.temperature.shift();
    history.humidity.shift();
    history.gas.shift();
  }

  // Update graphs
  distanceChart.update();
  temperatureChart.update();
  humidityChart.update();
  gasChart.update();
}

// Start simulation
generateSensorData();

setInterval(generateSensorData, 2000);
