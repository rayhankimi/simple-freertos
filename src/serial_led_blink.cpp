#include <Arduino.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#if CONFIG_FREE_RTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 22;
static const int led_pin = 2;
static volatile int led_delay = 500;  

SemaphoreHandle_t xMutex;

void toggle_led(void *parameter) {
  for (;;) {
    int delay_value;

    // Get delay value
    if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
      delay_value = led_delay;
      xSemaphoreGive(xMutex);
    }

    digitalWrite(led_pin, HIGH);
    for (int i = 0; i < delay_value / 10; i++) {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      if (xSemaphoreTake(xMutex, 0)) {  // Nonblocking check
        delay_value = led_delay;
        xSemaphoreGive(xMutex);
      }
    }

    digitalWrite(led_pin, LOW);
    for (int i = 0; i < delay_value / 10; i++) {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      if (xSemaphoreTake(xMutex, 0)) {  // Nonblocking check
        delay_value = led_delay;
        xSemaphoreGive(xMutex);
      }
    }
  }
}

void read_serial(void *parameter) {
  char buf[buf_len];
  memset(buf, 0, buf_len);

  for (;;) {
    if (Serial.available() > 0) {
      String str = Serial.readStringUntil('\n');
      str.toCharArray(buf, buf_len);
      int new_delay = atoi(buf);

      new_delay = (new_delay < 100) ? 100 : new_delay;
      new_delay = (new_delay > 200000) ? 200000 : new_delay;

      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        led_delay = new_delay;
        xSemaphoreGive(xMutex);
      }

      Serial.print("New delay: ");
      Serial.println(led_delay);
      memset(buf, 0, buf_len);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Enter new delay in ms");

  xMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    toggle_led,
    "Toggle LED Blink",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );
  xTaskCreatePinnedToCore(
    read_serial,
    "Read Serial",
    1024,
    NULL,
    1,
    NULL,
    app_cpu
  );

  vTaskDelete(NULL);
}

void loop() {}
