#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>

#define WM_PASSWORD "md123456"
#define STATUS_PRINT_INTERVAL (5000) // 5 seconds
#define SLEEP_TIMEOUT (20000) // 20 seconds
#define MSG_SEND_RETRY_INTERVAL (10 * 1000000) // 10 seconds
#define WIFI_CONFIG_PORTAL_TIMEOUT (3 * 60) // 3 minutes
#define MAX_WIFI_CONNECTION_RETRIES (3)
#define WIFI_RECONNECT_SLEEP_TIMEOUT_BEFORE_RESET (1 * 1000000) // 1 second

WiFiClientSecure https;
WiFiManagerParameter electricity_group("group", "Electricity Group", "1", 2);

int8_t last_state = -1;
RTC_DATA_ATTR int wifi_connection_retries = 0;

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  case ESP_SLEEP_WAKEUP_GPIO:
    Serial.println("Wakeup caused by GPIO");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void enter_deep_sleep_wake_on_pin(uint8_t electricity_state)
{
  esp_deep_sleep_enable_gpio_wakeup(1 << GPIO_NUM_3, electricity_state ? ESP_GPIO_WAKEUP_GPIO_LOW : ESP_GPIO_WAKEUP_GPIO_HIGH);
  esp_deep_sleep_start();
}

void enter_deep_sleep_wake_on_timer(uint64_t time_us)
{
  esp_deep_sleep(time_us);
}

void setup()
{
  pinMode(GPIO_NUM_3, INPUT);
  Serial.begin(115200);

  print_wakeup_reason();

  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  bool res;
  char ssid[32];
  sprintf(ssid, "MD_%06X", WIFI_getChipId());
  wm.addParameter(&electricity_group);
  wm.setConfigPortalTimeout(180);
  res = wm.autoConnect(ssid, WM_PASSWORD);
  if (!res)
  {
    Serial.println("Failed to connect to WiFi and hit timeout.");
    Serial.flush();
    if (wifi_connection_retries >= MAX_WIFI_CONNECTION_RETRIES)
    {
      Serial.println("Max retries reached. Going to sleep until reset or pin change. Zzzz...");
      Serial.flush();
      wifi_connection_retries = 0;
      enter_deep_sleep_wake_on_pin(digitalRead(GPIO_NUM_3));
    }
    wifi_connection_retries++;
    esp_deep_sleep(WIFI_RECONNECT_SLEEP_TIMEOUT_BEFORE_RESET);
  }
  Serial.println("Connected to WiFi");
  https.setInsecure(); // NOT RECOMMENDED
}

char buffer[1024];

bool send_status_update()
{
  bool result = false;
  size_t sent = 0;
  buffer[0] = 0;
  sprintf(buffer, "{\"group\": \"%s\", \"state\": \"%s\"}", electricity_group.getValue(), digitalRead(GPIO_NUM_3) == 1 ? "ON" : "OFF");
  https.stop();
  String response_code_line;
  if (https.connect(API_ENDPOINT_HOST, 443))
  {
    sent += https.print(String("POST ") + API_ENDPOINT_PATH + " HTTP/1.1\r\n");
    sent += https.print(String("Host: ") + API_ENDPOINT_HOST + "\r\n");
    sent += https.print(F("Content-Type: application/json\r\n"));
    sent += https.println(F("User-Agent: ESP"));
    sent += https.printf("Content-Length: %d\r\n", strlen(buffer));
    sent += https.print(F("Connection: close\r\n"));
    sent += https.print(F("\r\n"));
    sent += https.print(buffer);
    https.flush();
    Serial.println("Status update sent");
    while (https.connected())
    {
      if (!response_code_line.length())
      {
        response_code_line = https.readStringUntil('\n');
        Serial.println(response_code_line);
      }
      https.readStringUntil('\n');
    }
    auto code = response_code_line.substring(9, 12).toInt();
    if (code >= 200 && code < 300) {
      result = true;
    }
  }
  else
  {
    Serial.println("Failed to connect to API");
  }
  return result;
}

unsigned long last_status_update = 0;

void loop()
{
  uint8_t state = digitalRead(GPIO_NUM_3);
  if (millis() - last_status_update > STATUS_PRINT_INTERVAL)
  {
    last_status_update = millis();
    Serial.printf("State: %d\n", state);
    Serial.printf("Wifi ssid: %s\n", WiFi.SSID().c_str());
    Serial.printf("Wifi RSSI: %d\n", WiFi.RSSI());
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }
  if (state != last_state)
  {
    Serial.printf("State changed to: %d\n", state);
    last_state = state;
    if (!send_status_update()) {
      Serial.println("Failed to send status update");
      Serial.println("Going to sleep. Then retry. Zzzz...");
      Serial.flush();
      esp_deep_sleep(MSG_SEND_RETRY_INTERVAL);
    }
  }
  if (millis() > SLEEP_TIMEOUT)
  {
    Serial.println("Going to sleep. Zzzz...");
    Serial.flush();
    enter_deep_sleep_wake_on_pin(state);
  }
  delay(100);
}