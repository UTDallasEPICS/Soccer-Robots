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

#define MOTOR_IN1_PIN       GPIO_NUM_18  // DRV8833 IN1 (forward)
#define MOTOR_IN2_PIN       GPIO_NUM_16  // DRV8833 IN2 (reverse)

//active-LOW
#define GOALPOST_LED_GPIO   GPIO_NUM_15

#define DETECT_DELAY_MS     500          // wait time after ball detection
#define SPIN_TIME_MS        2000         // how long to spin the motor

#define GOAL_LISTEN_PORT 30000

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
    //configure motor pins as plain GPIO outputs
    gpio_reset_pin(MOTOR_IN1_PIN);
    gpio_reset_pin(MOTOR_IN2_PIN);
    gpio_set_direction(MOTOR_IN1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_IN2_PIN, GPIO_MODE_OUTPUT);

    //start with motor stopped (both low => coast)
    gpio_set_level(MOTOR_IN1_PIN, 0);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

static void motor_spin_forward(void)
{
    //spin the motor forward: IN1 = 1, IN2 = 0
    gpio_set_level(MOTOR_IN1_PIN, 1);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

static void motor_stop(void)
{
    gpio_set_level(MOTOR_IN1_PIN, 0);
    gpio_set_level(MOTOR_IN2_PIN, 0);
}

//IR sensor functions
static void ir_sensor_init(void)
{
    //configure IR sensor as input with internal pull-up
    //Adafruit break-beam receiver is open-collector and needs a pull-up.
    /*
    turns on the ESP32’s internal pull-up resistor because 
    the Adafruit break-beam sensor has an open-collector output 
    it basically pulls the line low when the beam is broken
    otherwise it floats and needs a pull-up
    */
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
static int wait_for_pi_connection(void)
{
    int listen_sock = -1;
    int client_sock = -1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    //create TCP socket
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return -1;
    }

    //allow quick reuse of the port if we reset
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //bind to all local interfaces on GOAL_LISTEN_PORT
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port   = htons(GOAL_LISTEN_PORT);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        return -1;
    }

    err = listen(listen_sock, 1);
    if (err < 0) {
        ESP_LOGE(TAG, "Error during listen: errno %d", errno);
        close(listen_sock);
        return -1;
    }

    ESP_LOGI(TAG, "Waiting for Raspberry Pi to connect on port %d...", GOAL_LISTEN_PORT);

    //block here until the client/Pi connects
    client_sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
    if (client_sock < 0) {
        ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
        close(listen_sock);
        return -1;
    }

    ESP_LOGI(TAG, "Raspberry Pi connected!");

    close(listen_sock);
    return client_sock;
}

static void send_goal_to_server(int sock)
{
    if (sock < 0) {
        return;
    }

    //Follow the convention of messages ending with '|'
    const char *msg = "GOAL|\n";
    int len = strlen(msg);

    int sent = send(sock, msg, len, 0);
    if (sent < 0) {
        ESP_LOGE(TAG, "Error sending GOAL message: errno %d", errno);
    } else {
        ESP_LOGI(TAG, "Sent GOAL message to server");
    }
}

/*
//goalpost task
void goalpost_mechanism_task(void *pvParameters)
{
    //hardware init
    ir_sensor_init();
    motor_init();
    goalpost_led_init();

    ESP_LOGI(TAG, "Goalpost mechanism starting...");

    
    //wait for the Raspberry Pi to connect to us
    int sock = -1;
    while (sock < 0) {
        sock = wait_for_pi_connection();
        if (sock < 0) {
            ESP_LOGW(TAG, "Failed to get Pi connection, retrying in 2 seconds...");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
    
    int sock = -1;

    ESP_LOGI(TAG, "Waiting for ball...");

    while (true) {
        if (ir_beam_broken()) {
            //beam broken → "ball detected"
            ESP_LOGI(TAG, "Ball detected! Beam broken.");
            goalpost_led_pulse(3);  // 3 quick blinks

            vTaskDelay(pdMS_TO_TICKS(DETECT_DELAY_MS));

            //spin the motor
            goalpost_led_set(1);    // LED ON while spinning
            motor_spin_forward();

            //notify server about the goal
            //send_goal_to_server(sock);

            vTaskDelay(pdMS_TO_TICKS(SPIN_TIME_MS));

            //stop motor and LED
            motor_stop();
            goalpost_led_set(0);

            //wait until beam is restored before re-arming
            while (ir_beam_broken()) {
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            ESP_LOGI(TAG, "Beam restored, re-armed.");
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // small poll delay
    }

    //cleanup on exit (never reached in this example)
    if (sock >= 0) {
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}
*/

//task for testing goalpost mechanism without wifi
void goalpost_mechanism_task(void *pvParameters)
{
    //hardware init
    ir_sensor_init();
    motor_init();
    goalpost_led_init();

    ESP_LOGI(TAG, "Goalpost HARDWARE TEST: no Wi-Fi sockets, just IR + motor");

    bool last_broken = false;

    while (true) {

        bool broken = ir_beam_broken();

        //log when the beam state changes
        if (broken != last_broken) {
            ESP_LOGI(TAG, "IR beam is now: %s", broken ? "BROKEN" : "OK");
            last_broken = broken;
        }

        //trigger ONCE per break (edge trigger)
        if (broken) {
            ESP_LOGI(TAG, "Ball detected! Spinning motor once.");

            goalpost_led_pulse(3);   // blink a bit
            goalpost_led_set(1);     // LED ON while spinning

            motor_spin_forward();    // turn motor ON
            vTaskDelay(pdMS_TO_TICKS(SPIN_TIME_MS));

            motor_stop();            // turn motor OFF
            goalpost_led_set(0);     // LED OFF

            ESP_LOGI(TAG, "Motor stopped, waiting for beam to restore...");

            // Wait until beam is no longer broken
            while (ir_beam_broken()) {
                vTaskDelay(pdMS_TO_TICKS(50));
            }

            ESP_LOGI(TAG, "Beam restored, re-armed.");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

