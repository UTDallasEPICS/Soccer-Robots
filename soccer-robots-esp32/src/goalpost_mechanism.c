#include "goalpost_mechanism.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "wifi_connectivity.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define IR_SENSOR_PIN       GPIO_NUM_5   // IR receiver wire - GPIO5

//DRV8833 input pins for one motor
#define MOTOR_IN1_PIN       GPIO_NUM_18  // DRV8833 IN1 (forward)
#define MOTOR_IN2_PIN       GPIO_NUM_16  // DRV8833 IN2 (reverse)

//active-LOW
#define GOALPOST_LED_GPIO   GPIO_NUM_15

#define DETECT_DELAY_MS     500          // wait time after ball detection
#define SPIN_TIME_MS        2000         // how long to spin the motor

//server config
// change to your Raspberry Pi / game server IP
#define GOAL_SERVER_IP   "???"

#define GOAL_SERVER_PORT 30000

static const char *TAG = "IR_MOTOR";

//led functions
static void goalpost_led_init(void)
{
    gpio_reset_pin(GOALPOST_LED_GPIO);
    gpio_set_direction(GOALPOST_LED_GPIO, GPIO_MODE_OUTPUT);
    // active-LOW LED: 1 = OFF, 0 = ON
    gpio_set_level(GOALPOST_LED_GPIO, 1);   // start OFF
}

static void goalpost_led_set(int on)
{
    // on == 1 -> LED ON (drive low), on == 0 -> LED OFF (drive high)
    gpio_set_level(GOALPOST_LED_GPIO, on ? 0 : 1);
}

static void goalpost_led_pulse(int times)
{
    for (int i = 0; i < times; ++i) {
        goalpost_led_set(1); // ON
        vTaskDelay(pdMS_TO_TICKS(200));
        goalpost_led_set(0); // OFF
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

//motor helper functions
static void motor_init(void)
{
    // Configure motor pins as plain GPIO outputs
    gpio_reset_pin(MOTOR_IN1_PIN);
    gpio_reset_pin(MOTOR_IN2_PIN);
    gpio_set_direction(MOTOR_IN1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN2_PIN, GPIO_MODE_OUTPUT);

    // Start with motor stopped (both low => coast)
    gpio_set_level(MOTOR_IN1_PIN, 0);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

static void motor_spin_forward(void)
{
    // Spin the motor forward: IN1 = 1, IN2 = 0
    gpio_set_level(MOTOR_IN1_PIN, 1);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

static void motor_stop(void)
{
    gpio_set_level(MOTOR_IN1_PIN, 0);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

//ir sensor functions
static void ir_sensor_init(void)
{
    // Configure IR sensor as input with internal pull-up
    // Adafruit break-beam receiver is open-collector and needs a pull-up.
    gpio_config_t io_conf = { 
        .pin_bit_mask = (1ULL << IR_SENSOR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,       // enable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // no pulldown
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

static int ir_beam_broken(void)
{
    // no ball -> beam intact -> receiver output HIGH (1)
    // ball passes -> beam broken -> receiver output LOW (0)
    return (gpio_get_level(IR_SENSOR_PIN) == 0);
}

//network functions
static int connect_to_goal_server(void)
{
    struct sockaddr_in dest_addr;
    int sock = -1;

    dest_addr.sin_addr.s_addr = inet_addr(GOAL_SERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(GOAL_SERVER_PORT);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    ESP_LOGI(TAG, "Connecting to %s:%d", GOAL_SERVER_IP, GOAL_SERVER_PORT);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        close(sock);
        return -1;
    }

    ESP_LOGI(TAG, "Successfully connected to goal server");
    return sock;
}

static void send_goal_to_server(int sock)
{
    if (sock < 0) {
        return;
    }

    // Follow the convention of messages ending with '|'
    const char *msg = "GOAL|\n";
    int len = strlen(msg);

    int sent = send(sock, msg, len, 0);
    if (sent < 0) {
        ESP_LOGE(TAG, "Error sending GOAL message: errno %d", errno);
    } else {
        ESP_LOGI(TAG, "Sent GOAL message to server");
    }
}

//goalpost task
void goalpost_mechanism_task(void *pvParameters)
{
    // Hardware init
    ir_sensor_init();
    motor_init();
    goalpost_led_init();

    ESP_LOGI(TAG, "Goalpost mechanism starting...");

    // At this point Wi-Fi should already be up (wifi_setup_init was called in app_main)
    // Now connect to the game server:
    int sock = -1;
    while (sock < 0) {
        sock = connect_to_goal_server();
        if (sock < 0) {
            ESP_LOGW(TAG, "Retrying server connection in 2 seconds...");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

    ESP_LOGI(TAG, "Waiting for ball...");

    while (true) {
        if (ir_beam_broken()) {
            // Beam broken → "ball detected"
            ESP_LOGI(TAG, "Ball detected! Beam broken.");
            goalpost_led_pulse(3);  // 3 quick blinks

            vTaskDelay(pdMS_TO_TICKS(DETECT_DELAY_MS));

            // Spin the motor
            goalpost_led_set(1);    // LED ON while spinning
            motor_spin_forward();

            // Notify server about the goal
            send_goal_to_server(sock);

            vTaskDelay(pdMS_TO_TICKS(SPIN_TIME_MS));

            // Stop motor and LED
            motor_stop();
            goalpost_led_set(0);

            // Wait until beam is restored before re-arming
            while (ir_beam_broken()) {
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            ESP_LOGI(TAG, "Beam restored, re-armed.");
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // small poll delay
    }

    // Not really reached, but for completeness:
    if (sock >= 0) {
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}
