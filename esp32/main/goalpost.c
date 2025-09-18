//THIS IS THE CODE FLASHED ONTO THE GOAL POST ESP, NOT THE ROBOT

//For now, you will want to think about connecting it to the raspberry pi. Code should
//be availaable for it in main.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define RELAY_PIN 3  // GPIO Pin connected to the relay IN pin

#define BLINK_PIN 15

void app_main() {
    // Initialize the GPIO pin for relay control
    gpio_set_direction(RELAY_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_PIN, 1);
    ESP_LOGI("Relay", "GPIO initialized as output pin");

    while (1) {
        // Turn the relay ON (motor should start running)
        gpio_set_level(RELAY_PIN, 1);  // 1 = HIGH, relay on
        ESP_LOGI("Relay", "Relay ON: Motor running");
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait for 5 seconds

        // Turn the relay OFF (motor should stop)
        gpio_set_level(RELAY_PIN, 0);  // 0 = LOW, relay off
        ESP_LOGI("Relay", "Relay OFF: Motor stopped");
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait for 5 seconds
    }
}