#include "wifi_connectivity.h"
#include "esp_log.h"
#include "esp_err.h"

#include "freertos/projdefs.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "hal/ledc_types.h"

#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "soc/clk_tree_defs.h"

/* #include "protocol_examples_common.h" // Wi-Fi connectivity */
#include <sys/socket.h>		// Sockets
#include <unistd.h>			// Close
#include <netdb.h>			// gethostbyname

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>

#define PORT					30000
#define KEEPALIVE_IDLE			CONFIG_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL		CONFIG_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT			CONFIG_KEEPALIVE_COUNT

#define BLINK_GPIO 15
#define BLINK_PERIOD 1000

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT // Set duty resolution to 13 bits
#define LEDC_TIMER				LEDC_TIMER_0
#define LEDC_FREQUENCY  454 // Frequency in Hertz. Set frequency at 4 kHz

#define LEDC_OUTPUT_IO1	16 // Define the output GPIO
#define LEDC_OUTPUT_IO2	18 // Define the output GPIO

#define LEDC_CHANNEL1            LEDC_CHANNEL_0
#define LEDC_CHANNEL2            LEDC_CHANNEL_1

#define FADE_RESOLUTION			10

// Prototypes
static void setup();
void move();

typedef struct sockaddr SA;
/* static const int ip_protocol = 0; */

static uint8_t s_led_state = 0;

//setting up things we need for natural movement
static int8_t xValForMotor = -50;

static int lowerReverseBound = 0;
static int upperReverseBound = 0;
static int lowerForwardBound = 0;

static float currentDirection[2] = {0, 0};
static int8_t currentTargets[2] = {0, 0};
static float startTargets[2] = {0, 0};

typedef struct {
    int8_t fullForward[2];
    int8_t fullBack[2];
    int8_t forwardLeft[2];
    int8_t forwardRight[2];
    int8_t fullLeft[2];
    int8_t fullRight[2];
    int8_t stop[2];
} MoveTargets;

// Initialize the struct after declaration
static MoveTargets moveTargets = {
    {85, 85},
    {-85, -85},
    {35, 100},
    {100, 35},
    {-50, 50},
    {50, -50},
    {0, 0}
};

float min(float x, float y)
{
	if(x <= y)
	{
		return x;
	}
	return y;
}

float pwmFunction(uint8_t index)
{
	float a = abs(currentTargets[index] - startTargets[index]);
	float c = min(startTargets[index], currentTargets[index]);
	float b = 2 * (currentTargets[index] - startTargets[index]) / abs(currentTargets[index] - startTargets[index]);
	return a / (1 + exp(-b*xValForMotor/20)) + c;
}

typedef struct {
	bool forward, left, right, back, inputChanged;
} Movement;

Movement* getMovementStruct(char *buffer, int length)
{
	Movement *theStruct = malloc(sizeof(Movement)); // Allocate memory
    *theStruct = (Movement){false, false, false, false}; // Proper struct initialization
	int i;
	//if its true
	if(buffer[0] == 't' && buffer[1] == 'r' && buffer[2] == 'u' && buffer[3] == 'e')
	{
		theStruct->inputChanged = true;
		i = 4;
	}
	else
	{
		i = 5;
		theStruct->inputChanged = false;
	}
	for(; i < length; i++)
	{
		if(buffer[i] == 'u')
			theStruct->forward = true;
		if(buffer[i] == 'd')
			theStruct->back = true;
		if(buffer[i] == 'l')
			theStruct->left  = true;
		if(buffer[i] == 'r')
			theStruct->right = true;
	}

	//If pressing both, set them both to false.
	if(theStruct->forward == true && theStruct->back == true)
	{
		theStruct->forward = false;
		theStruct->back = false;
	}
	
	//If pressing both, set them both to false.
	if(theStruct->left == true && theStruct->right == true)
	{
		theStruct->left = false;
		theStruct->right = false;
	}

	return theStruct;
}

static void doMovement(Movement *movementDirections)
{
	ESP_LOGI("DEBUG", "Direction Left is %f, Direction Right is %f, forward is %d, left is %d, right is %d, back is %d.", 
		currentDirection[0], currentDirection[1], movementDirections->forward, movementDirections->left, movementDirections->right, movementDirections->back);
	
	if(!movementDirections->inputChanged && xValForMotor >= 50)
	{
		//block
	}
	else if(!movementDirections->inputChanged)
	{
		currentDirection[0] = pwmFunction(0);
		currentDirection[1] = pwmFunction(1);
		//update x
		if(xValForMotor >= 50)
		{
			xValForMotor = 50;	
		}
	}
	else
	{
		startTargets[0] = currentDirection[0];
		startTargets[1] = currentDirection[1];
		//get final targets here
		xValForMotor = -50;
		currentDirection[0] = pwmFunction(0);
		currentDirection[1] = pwmFunction(1);
	}

	move();
}

static void on_receive(const int sock)
{
    int len;
    char rx_buffer[128];
	char* TAG = "SERVER_EVENT";

    do 
	{
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

			doMovement(getMovementStruct(rx_buffer, len));
			
            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            int to_write = len;
            while (to_write > 0) {
                int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    // Failed to retransmit, giving up
                    return;
                }
                to_write -= written;
            }
        }
    } while (len > 0);
}

void taskServer(void *pvParameters){	
	char* TAG = "SERVER";
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

	// IPv4 Socket Structure Configuration, using current IP and selected Port #
    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

	// Create Socket
    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

	// Bind socket to current IP address using newly created structure
    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

	// Start listening for connections
    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) 
	{
        ESP_LOGI(TAG, "Socket listening");
		
		// Accept a connection, collect client's IP
		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

		//Still having it block
		// int prevFlags = fcntl(sock, F_GETFL, 0);
		// fcntl(listen_sock, F_SETFL, prevFlags | O_NONBLOCK);

        // Convert ip address to string
		if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

		// Respond to client
        on_receive(sock);

		// Close client socket
        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

//Raw Duty = (Percent/100) * (2^LEDC_DUTY_RES)

//Gets the raw duty value from the percentage.
float getRawDutyFromPercent(float duty){	
	duty = duty / 100;	
	return (pow(2, LEDC_DUTY_RES) * duty);
}

float getPercentFromRawDuty(float duty)
{
	return (duty*100)/(pow(2, LEDC_DUTY_RES));
}

float getRawDutyFromBaseDirection(float duty)
{
	//if duty is positive, remember valid values are from 86 - 100. If negative, from 36-50
	uint8_t range = 14;
	if(duty > 0.1)
	{
		uint8_t lowerBound = 86;
		//works as we first limit it to the range 0 to 14 by dividing by (100/range).
		duty = duty / (100.0 / range);
		//Then, add 86 to put it in the range 86-100
		duty += lowerBound;
	}
	else if(duty < 0.1)
	{
		uint8_t lowerBound = 36;
		//first limit it to the range -14 to 0  by dividing by (100 / range)
		duty = duty / (100.0 / range);
		//Then, add 14 to it to put it in the range 0-14, then add 36 to put in range 36-50
		duty += lowerBound + range;
	}
	else
	{
		duty = 0;
	}
	return getRawDutyFromPercent(duty);
}

//converts pulse width (in ms) to the proper duty cycle RAW.
int convertPulseWidthToPercentDuty(int pulseWidth, bool lowerBound)
{
	//pulse width / 1000 = duty cycle * period. Cause pulse width initially in milliseconds.
	//so duty cycle = x, period = 1/freq, so pulseWidth/1000 = x/freq --> x = freq * pulseWidth/1000
	float rawDuty = pulseWidth / (1000.0) * LEDC_FREQUENCY;
	float trueDutyCycle = getPercentFromRawDuty(rawDuty);
	//Now, we need the percentage.

	if(lowerBound)
		return floor(trueDutyCycle);
	return ceil(trueDutyCycle);
}

static void ledc_setup(){
	// Timer Configuration
	gpio_reset_pin(LEDC_OUTPUT_IO1);
	gpio_set_direction(LEDC_OUTPUT_IO1, GPIO_MODE_OUTPUT);
	gpio_reset_pin(LEDC_OUTPUT_IO2);
	gpio_set_direction(LEDC_OUTPUT_IO2, GPIO_MODE_OUTPUT);
	
	ledc_timer_config_t timer_conf = {
		.speed_mode = LEDC_MODE,
		.duty_resolution = LEDC_DUTY_RES,
		.timer_num = LEDC_TIMER,
		.freq_hz = LEDC_FREQUENCY,
		.clk_cfg = LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));
	// Channel Configuration
	ledc_channel_config_t channel_conf1 = {
		.speed_mode = LEDC_MODE,
		.channel = LEDC_CHANNEL1,
		.timer_sel = LEDC_TIMER,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = LEDC_OUTPUT_IO1,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&channel_conf1));

	//Channel Configuration
	ledc_channel_config_t channel_conf2 = {
		.speed_mode = LEDC_MODE,
		.channel = LEDC_CHANNEL2,
		.timer_sel = LEDC_TIMER,
		.intr_type = LEDC_INTR_DISABLE,
		.gpio_num = LEDC_OUTPUT_IO2,
		.duty = 0,
		.hpoint = 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&channel_conf2));
}

void move(){
	//Move the left motor
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL1, getRawDutyFromBaseDirection(currentDirection[0])));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL1));
	ESP_LOGI("DUTY CHECK", "Left Duty is: %lu", ledc_get_duty(LEDC_MODE, LEDC_CHANNEL1));


	//Move the right motor
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL2, getRawDutyFromBaseDirection(currentDirection[1])));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL2));

	ESP_LOGI("DUTY CHECK", "Right Duty is: %lu", ledc_get_duty(LEDC_MODE, LEDC_CHANNEL2));

	// Pulse of : 800 - 1100 uS (Reverse)
	// Pulse of : 1900 - 2200 uS (Forward)

	// Frequency of 454 Hz = Period of 2202 uS
	// 800 uS = 36
	// 1100 uS = 50
	//
	// 1900 uS = 86
	// 2200 uS = 100
}

void doBlink()
{
	gpio_reset_pin(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	s_led_state = 1;
	gpio_set_level(BLINK_GPIO, s_led_state);
}

void app_main() {
	//THESE ARE THE RAW DUTIES
	lowerReverseBound = convertPulseWidthToPercentDuty(800, true);
	upperReverseBound = convertPulseWidthToPercentDuty(1100, false);
	lowerForwardBound = convertPulseWidthToPercentDuty(1900, true);
	ESP_LOGI("BOUNDS", "Lower bound to reverse motor: %d, upper bound to reverse motor: %d, lower bound to push motor forward: %d", lowerReverseBound, upperReverseBound, lowerForwardBound);
	
	ledc_setup();
	
	doBlink();

	// if (wifi_setup_init()){
	// 	/* xTaskCreate( */
	// 	/* 		taskClient, */
	// 	/* 		"taskClient", */
	// 	/* 		8192,			// Stack Size */
	// 	/* 		NULL,			// Parameters */
	// 	/* 		1,				// Priority */
	// 	/* 		NULL); */
	// 	xTaskCreate(
	// 		taskServer,
	// 		"taskServer",
	// 		4096,
	// 		(void*)AF_INET,
	// 		5,
	// 		NULL
	// 	);
	// }

	vTaskDelay(pdMS_TO_TICKS(500));
	move(90);

	
}
