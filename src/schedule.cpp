#include "schedule.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include "config.h"
#include "memory.h"
#include "states.h"

// Network variables
static AsyncWebServer server(WEB_SERVER_PORT);
static bool wifiConnected = false;
static unsigned long lastNtpTime = 0;
static unsigned long lastWifiTime = 0;

// Scheduler variables
static ScheduleTime openSched = {99, 99};
static ScheduleTime closeSched = {99, 99};
static int lastCheckedMinute = -1;
static unsigned long lastScheduleTime = 0;

// Forward declarations
static void setupWebServer();
static void checkReconnectWiFi();

// Initialize scheduler, Wi-Fi, RTC, and web server
bool setupScheduler() {
  Serial.print("Initializing Scheduler...");

  loadSchedule(openSched, closeSched);

  Serial.print("Done\n");
  Serial.printf("*Loaded Schedule: Open = %02d:%02d, Close = %02d:%02d\n", openSched.hour, openSched.minute,
    closeSched.hour, closeSched.minute);

  // Initialize Wi-Fi using WiFiManager libary
  Serial.printf("Initializing Wi-Fi (SSID: %s)...\n", WIFI_AP_NAME);
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  // Set autoconnect timeout (s)
  wm.setConnectTimeout(30);
  // Set captive portal timeout (s)
  wm.setConfigPortalTimeout(120);
  wifiConnected = wm.autoConnect(WIFI_AP_NAME, NULL);
  if (!wifiConnected) {
    return false;
  }

  // Initializes system time using NTP server
  Serial.print("Initializing Time Server...");
  configTzTime(TIME_ZONE, NTP_SERVER);
  Serial.print("Done\n");

  // Start web server over Wi-Fi
  Serial.print("Initializing Web Server...");
  setupWebServer();
  server.begin();
  Serial.print("Done\n");

  lastWifiTime = millis();
  return true;
}

// Check scheduled times for action triggers
void checkSchedule() {
  unsigned long currentTime = millis();

  // Periodically check/reconnect to Wi-Fi
  if (currentTime - lastWifiTime >= WIFI_CHECK_INTERVAL) {
    lastWifiTime = currentTime;
    checkReconnectWiFi();
  }

  // Check if interval time is met
  if (currentTime - lastScheduleTime <= SCHEDULE_CHECK_INTERVAL) {
    return;
  }
  lastScheduleTime = currentTime;

  struct tm timeinfo;
  // Check if time is available
  if (!getLocalTime(&timeinfo) && wifiConnected) {
    if (wifiConnected) {
      Serial.print("ERROR: Failed to Obtain Time\n");
      enterState(SystemState::ERROR);
    }
    return;
  }

  // Prevent multiple triggers in the same minute
  if (timeinfo.tm_min == lastCheckedMinute) {
    return;
  }
  lastCheckedMinute = timeinfo.tm_min;

  // Check open schedule
  if (openSched.hour == timeinfo.tm_hour && openSched.minute == timeinfo.tm_min) {
    Serial.printf("Scheduler: Open Trigger (%02d:%02d)\n", timeinfo.tm_hour, timeinfo.tm_min);
    triggerOpen();
  }

  // Check close schedule
   if (closeSched.hour == timeinfo.tm_hour && closeSched.minute == timeinfo.tm_min) {
    Serial.printf("Scheduler: Close Trigger (%02d:%02d)\n", timeinfo.tm_hour, timeinfo.tm_min);
    triggerClose();
  }
}

// Sync RTC with NTP server
void syncRTC() {
  unsigned long currentTime = millis();

  // Periodically check/reconnect to Wi-Fi
  if (currentTime - lastWifiTime >= WIFI_CHECK_INTERVAL) {
    lastWifiTime = currentTime;
    checkReconnectWiFi();
  }

  if (wifiConnected) {
    // Periodically check NTP time
    if (lastNtpTime == 0 || (currentTime - lastNtpTime > NTP_SYNC_INTERVAL)) {
      struct tm timeinfo;
      // Synchronize system time with NTP server
      if (!getLocalTime(&timeinfo)) {
        Serial.print("ERROR: Failed to Obtain Time\n");
        enterState(SystemState::ERROR);
        return;
      }
      lastNtpTime = millis();
    }
  }
}

// Check Wi-Fi status and reconnect if necessary
static void checkReconnectWiFi() {
  wl_status_t currentStatus = WiFi.status();

  if (currentStatus != WL_CONNECTED) {
    // Not connected
    if (wifiConnected) {
      Serial.print("Wi-Fi Disconnected. Attempting Reconnect...\n");
      wifiConnected = false;
      server.end();
    }
    // Attempt to reconnect
    WiFi.begin();
  } else {
    // Connected
    if (!wifiConnected) {
      Serial.printf("Wi-Fi Reconnected (IP: %s)\n", WiFi.localIP().toString().c_str());
      wifiConnected = true;
      // Force RTC sync
      lastNtpTime = 0;
      server.begin();
    }
  }
}

// HTML web server page
const char index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>AutoBlinds</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
<body>

  <h2>Manual Control</h2>
  <form action="/open" method="get" style="display: inline-block; margin-right: 5px;">
    <button type="submit">Open</button>
  </form>
  <form action="/close" method="get" style="display: inline-block;">
    <button type="submit">Close</button>
  </form>
  <br>
  <br>

  <h2>Schedule</h2>
  <form action="/setSchedule" method="get">
    <label for="openTime">Open Time:</label>
    <input type="time" id="openTime" name="openTime" value=""> <input type="checkbox" id="openSet" name="openSet" title="Clear Open Time"> Clear this time<br>

    <label for="closeTime">Close Time:</label>
    <input type="time" id="closeTime" name="closeTime" value=""> <input type="checkbox" id="closeSet" name="closeSet" title="Clear Close Time"> Clear this time<br>
    <br>
    <button type="submit">Save</button>
  </form>

</body>
</html>
)rawliteral";


// Initialize web server routes
static void setupWebServer() {

  // Root page ("/")
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  // Handle open trigger
  server.on("/open", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.print("Web Server: Open Trigger\n");
    triggerOpen();
    // Redirect back to root page
    request->send(200, "text/html", "text/plain");
  });

  // Handle close trigger
  server.on("/close", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.print("Web Server: Close Trigger\n");
    triggerClose();
    // Redirect back to root page
    request->send(200, "text/html", "text/plain");
  });

  // Handle schedule form submission
  server.on("/setSchedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.print("Web Server: Schedule Form Submission\n");
    ScheduleTime tempOpenSched = {99, 99};
    ScheduleTime tempCloseSched = {99, 99};

    bool openSet = request->hasParam("openSet");
    bool closeSet = request->hasParam("closeSet");
    int hour = -1, minute = -1;

    // Process open time parameter if set
    if (!openSet && request->hasParam("openTime")) {
      String openTimeStr = request->getParam("openTime")->value();
      // Convert time string to hour and minute
      if (sscanf(openTimeStr.c_str(), "%d:%d", &hour, &minute) == 2) {
        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
          tempOpenSched.hour = hour; tempOpenSched.minute = minute;
        }
      }
    }

    // Process close time parameter if set
    if (!closeSet && request->hasParam("closeTime")) {
      String closeTimeStr = request->getParam("closeTime")->value();
      // Convert time string to hour and minute
      if (sscanf(closeTimeStr.c_str(), "%d:%d", &hour, &minute) == 2) {
        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
          tempCloseSched.hour = hour; tempCloseSched.minute = minute;
        }
      }
    }

    // Attempt to save new schedule times
    if (saveSchedule(tempOpenSched, tempCloseSched)) {
      openSched = tempOpenSched;
      closeSched = tempCloseSched;
      Serial.printf("Saved Schedule: Open = %02d:%02d, Close = %02d:%02d\n", openSched.hour, openSched.minute,
        closeSched.hour, closeSched.minute);
      lastCheckedMinute = -1;
    } else {
      Serial.print("ERROR: Failed to Save Schedule\n");
      enterState(SystemState::ERROR);
    }
    request->send(200, "text/html", "text/plain");
  });

  // Handle not found
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Error 404: Not found");
  });
}
