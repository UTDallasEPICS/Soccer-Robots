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
#include "driver/timer.h"
#include <fcntl.h>
#include "freertos/semphr.h"

#define PORT					30000
#define KEEPALIVE_IDLE			CONFIG_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL		CONFIG_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT			CONFIG_KEEPALIVE_COUNT

#define BLINK_GPIO 15
#define BLINK_PERIOD 1000

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_TIMER				LEDC_TIMER_0
#define LEDC_FREQUENCY  350 // Frequency in Hertz. Set frequency at 4 kHz

#define LEDC_OUTPUT_IO1	16// Define the output GPIO
#define LEDC_OUTPUT_IO2	18 // Define the output GPIO

#define LEDC_CHANNEL1            LEDC_CHANNEL_0
#define LEDC_CHANNEL2            LEDC_CHANNEL_1

#define FADE_RESOLUTION			10


static timer_config_t config = {
    .alarm_en = TIMER_ALARM_DIS,        // don’t need alarm
    .auto_reload = false,               // Auto-reload timer. Don’t want this
	//currently, we are going to update the timer every half a millisecond, so we'll need to account for this.
    .divider = 40000,                     // Timer clock divider (80000 gives a 1 millisecond resolution. As divider = APB clock frequency / ticks per second. Frequency is 80 MHz, so 80Mhz/1000 means 80000 total )
    .counter_dir = TIMER_COUNT_UP,     // Count upwards
    .counter_en = false,         // Start the timer
    .intr_type = TIMER_INTR_LEVEL,     // Interrupt type
    .clk_src = TIMER_SRC_CLK_APB,      // Clock source (APB)
};


// Prototypes
static void setup();
void move();

typedef struct sockaddr SA;
/* static const int ip_protocol = 0; */

static uint8_t s_led_state = 0;

static bool charging;
static bool inGame;
static bool resetting;

static uint8_t lowerReverseBound = 16;
static uint8_t upperReverseBound = 49;
static uint8_t lowerForwardBound = 56;
static uint8_t upperForwardBound = 89;

TaskHandle_t doMovementHandle = NULL;
bool finishedMoving = false;
bool interruptMovement = false;

static float currentDirection[2] = {0, 0};
static int8_t currentTargets[2] = {0, 0};
static float startTargets[2] = {0, 0};

SemaphoreHandle_t waitForData;

typedef struct {
    int8_t fullForward[2];
    int8_t fullBack[2];
    int8_t forwardLeft[2];
    int8_t forwardRight[2];
	int8_t backLeft[2];
	int8_t backRight[2];
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
	{-35, -100},
	{-100, -35},
    {-50, 50},
    {50, -50},
    {0, 0}
};

//got a function from desmos, we are just plugging it in
float pwmFunction(uint8_t index, int16_t x)
{
	float a = fabs(currentTargets[index] - startTargets[index]);
	float c = fmin(startTargets[index], currentTargets[index]);
	float b;
	if(currentTargets[index] == startTargets[index])
	{
		b = 0;
	}
	else
	{
		b = (1/2.5) * (currentTargets[index] - startTargets[index]) / fabs(currentTargets[index] - startTargets[index]);
	}
	float returnVal = a / (1 + exp(-b*x/20)) + c;
	return returnVal;
}

typedef struct {
	bool forward, left, right, back;
} Movement;

static Movement *moveStruct;


void setMoveStruct(char *buffer, int length)
{
	//resetting the struct.
	moveStruct->forward = false;
	moveStruct->back = false;
	moveStruct->left = false;
	moveStruct->right = false;

	//now loop through buffer, if seeing the right keys, det it to true
	for(int i = 0; i < length; i++)
	{
		if(buffer[i] == 'u')
			moveStruct->forward = true;
		if(buffer[i] == 'd')
			moveStruct->back = true;
		if(buffer[i] == 'l')
			moveStruct->left  = true;
		if(buffer[i] == 'r')
			moveStruct->right = true;
	}

	//If pressing both, set them both to false.
	if(moveStruct->forward == true && moveStruct->back == true)
	{
		moveStruct->forward = false;
		moveStruct->back = false;
	}
	
	//If pressing both, set them both to false.
	if(moveStruct->left == true && moveStruct->right == true)
	{
		moveStruct->left = false;
		moveStruct->right = false;
	}
}

void beginMoving()
{
	startTargets[0] = currentDirection[0];
	startTargets[1] = currentDirection[1];
	timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
	timer_start(TIMER_GROUP_0, TIMER_0);
	//get final targets here
	if(moveStruct->forward)
	{
		//forwared left
		if(moveStruct->left)
		{
			currentTargets[0] = moveTargets.forwardLeft[0];
			currentTargets[1] = moveTargets.forwardLeft[1];
		}
		//forward and right
		else if(moveStruct->right)
		{
			currentTargets[0] = moveTargets.forwardRight[0];
			currentTargets[1] = moveTargets.forwardRight[1];
		}
		//full forward
		else
		{
			currentTargets[0] = moveTargets.fullForward[0];
			currentTargets[1] = moveTargets.fullForward[1];
		}
	}
	else if(moveStruct->back)
	{
		//back and pressing left
		if(moveStruct->left)
		{
			currentTargets[0] = moveTargets.backLeft[0];
			currentTargets[1] = moveTargets.backLeft[1];
		}
		//back and pressing right
		else if(moveStruct->right)
		{
			currentTargets[0] = moveTargets.backRight[0];
			currentTargets[1] = moveTargets.backRight[1];
		}
		//full force backward
		else
		{
			currentTargets[0] = moveTargets.fullBack[0];
			currentTargets[1] = moveTargets.fullBack[1];
		}
	}
	//only going left, as we know forward and back aren't presssed
	else if(moveStruct->left)
	{
		currentTargets[0] = moveTargets.fullLeft[0];
		currentTargets[1] = moveTargets.fullLeft[1];
	}
	//only going right, as we know forward and back aren't pressed
	else if(moveStruct->right)
	{
		currentTargets[0] = moveTargets.fullRight[0];
		currentTargets[1] = moveTargets.fullRight[1];
	}
	//otherwise nothing is pressed, want to go back to being stationary.
	else
	{
		currentTargets[0] = moveTargets.stop[0];
		currentTargets[1] = moveTargets.stop[1];
	}
}

void doMovement(void *pvParameters)
{
	//always running while connected
	while(true)
	{
		//assume when we start, we begin moving
		beginMoving();
		//this is the look to move the new target
		while(true)
		{
			uint64_t x = 0;
			timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &x);
			//we'll need to do x / 2 - 250 for two reasons. First, we divide by 2 because the timer increments every half a millisecond, so its value
			//is double what we need. Also, it only allows positive values, so we have to make it from 0-500 and then subtract by 250 to get -250 to 250.
			ESP_LOGI("DEBUG", "Direction Left is %f, Direction Right is %f, forward is %d, left is %d, right is %d, back is %d, x is %d.", 
				currentDirection[0], currentDirection[1], moveStruct->forward, moveStruct->left, moveStruct->right, moveStruct->back, (int16_t) (x/2 - 250));	
	
			//once we reach our limit, we break out of our movement loop and wait
			if(x >= 1000)
			{
				timer_pause(TIMER_GROUP_0, TIMER_0);
				timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 1000);
				break;
			}

			//otherwise, keep updating
			currentDirection[0] = pwmFunction(0, (int16_t) (x/2 - 250));
			currentDirection[1] = pwmFunction(1, (int16_t) (x/2 - 250));
			move();		
			if(interruptMovement)
			{
				break;
			}
		}
		if(interruptMovement)
		{
			continue;
		}
		//wait until new data is sent after we've reached our target
		finishedMoving = true;
		xSemaphoreTake(waitForData, portMAX_DELAY);
	}
}

//C has no send_all like python does, so we have to manually create it. Means all messages sent to the pi have to end with delimiter |
static void sendMessage(const int sock, const char* message)
{
	send(sock, message, strlen(message), 0);
	//delimiter to let reaspberry pi know this is the end of that message
	send(sock, "|", 1, 0);
}

//C has no recv_all like python does, so we basically have to manually create it. This means all messages sent to the esp have to end with a delimiter |
static int receiveMessage(const int sock, char* rx_buffer)
{
	uint8_t length = 0;
	while(true)
	{
		int8_t status = recv(sock, rx_buffer + length, 1, 0);
		ESP_LOGI("BIT", "Got char %c", *(rx_buffer + length));
		if(status == -1)
		{
			return -1;
		}
		if(status == 0)
		{
			return 0;
		}
		if(*(rx_buffer + length) == '|')
		{
			break;
		}
		length++;
	}
	return length;
}

static void on_receive(const int sock)
{
    int len;
    char rx_buffer[128];
	char* TAG = "SERVER_EVENT";

    do 
	{
        
		len = receiveMessage(sock, rx_buffer);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = '\0'; // Null-terminate whatever is received and treat it like a string

			//if received a readyCheck message, return the right result
			if(len >= 10 && strncmp(rx_buffer, "readyCheck", 10) == 0)
			{
				ESP_LOGI("MESSAGE", "Ready message received!");
				//if its any of these, are not read
				if(charging || resetting || inGame)
				{
					//send not ready
					sendMessage(sock, "not-ready");
				}
				else
				{
					//send ready
					sendMessage(sock, "ready");
					inGame = true;
					xTaskCreate(doMovement, "doMovement", 8192, NULL, 4, &doMovementHandle);
				}
				continue;
			}
			//if received game over message, break out
			if(len >= 5 && strncmp(rx_buffer, "reset", 5) == 0)
			{
				inGame = false;
				interruptMovement = false;
				//for now, we can probably just break our esp out of this on receive, that will also destroy the movement task. Though, that's only after we finish moving.
				break;
			}
			//if received an ignore message, ignore it
			if(len >= 6 && strncmp(rx_buffer, "ignore", 6) == 0)
			{
				ESP_LOGI("MESSAGE", "Ignore message received!");
				continue;
			}
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
			setMoveStruct(rx_buffer, len);
			//when data received and movement is blocking, activate semaphore to tell it to move now
			if(finishedMoving)
			{
				xSemaphoreGive(waitForData);
			}
			//if we were already moving to the previous target, set this to true to let program know we need to move differently now
			else
			{
				interruptMovement = true;
			}

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
		//create task to do movement

		// Respond to client
        on_receive(sock);

		// Close client socket
        shutdown(sock, 0);

		//before finishing the movement task, spin until it's done.
		while(finishedMoving != true)
		{
			ESP_LOGI("WAIT", "Waiting!");
		}
		//delete the movement task since socket disconnected
		vTaskDelete(doMovementHandle);
		doMovementHandle = NULL;

        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

//Raw Duty = (Percent/100) * (2^LEDC_DUTY_RES)

//Gets the raw duty value from the percentage from 0 to 100
float getRawDutyFromPercent(float duty){	
	duty /= 100;	
	return (pow(2, LEDC_DUTY_RES) * duty);
}

float getPercentFromRawDuty(float duty)
{
	return (duty*100)/(pow(2, LEDC_DUTY_RES));
}

float getRawDutyFromBaseDirection(float duty)
{
	//if duty is positive, remember valid values are from 86 - 100. If negative, from 36-50
	uint8_t range = upperReverseBound - lowerReverseBound;
	if(duty > 0.1)
	{
		//works as we first limit it to the range 0 to 33 by dividing by 100 to get it between 0 and 1, then multiplying by 33.
		duty /= 100.0;
		duty *= range;
		//Then, add 86 to put it in the range 56-89
		duty += lowerForwardBound;
	}
	else if(duty < -0.1)
	{
		//first limit it to the range -1 to 0  by dividing by (100 / range)
		duty /= 100.0;
		//then put it in the range -33 to 0 by multiplying by range
		duty *= range;
		//now, add 33 to put it in the range 0 and 33, and then add 16 to put it in the range 16 to 49
		duty += range + lowerReverseBound;
	}
	else
	{
		duty = 53;
	}
	return getRawDutyFromPercent(duty);
}

//converts pulse width (in ms) to the proper duty cycle RAW.
float convertPulseWidthToPercentDuty(int pulseWidth)
{
	//get it with pulse width over period. Convert from microseconds to seconds too.
	return pulseWidth/(pow(10, 6)/LEDC_FREQUENCY) * 100;
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

	uint8_t startPos = 50;
	//Move the left motor to start position
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL1, startPos / 100.0 * pow(2, LEDC_DUTY_RES)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL1));

	//Move the right motor
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL2,  startPos / 100.0 * pow(2, LEDC_DUTY_RES)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL2));		
}

void move(){
	//Move the left motor
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL1, getRawDutyFromBaseDirection(currentDirection[0])));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL1));

	//Move the right motor
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL2, getRawDutyFromBaseDirection(currentDirection[1])));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL2));

	//ACTUAL DUTIES ARE: 56-89 FOR FORWARD (INCLUSIVE)
	//AND THEN 16 TO 49 FOR REVERSE (INCLUSIVE), 16 IS FASTER THAN 49

}

void doBlink()
{
	gpio_reset_pin(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	s_led_state = 1;
	gpio_set_level(BLINK_GPIO, s_led_state);
}

void app_main() {
	moveStruct = malloc(sizeof(Movement));
	*moveStruct = (Movement) {false, false, false, false};
	charging = false;
	inGame = false;
	resetting = false;
	waitForData = xSemaphoreCreateBinary();
	timer_init(TIMER_GROUP_0, TIMER_0, &config);
	ledc_setup();
	
	doBlink();

	vTaskDelay(pdMS_TO_TICKS(1000));

	vTaskDelay(pdMS_TO_TICKS(3000));
	if (wifi_setup_init()){
		/* xTaskCreate( */
		/* 		taskClient, */
		/* 		"taskClient", */
		/* 		8192,			// Stack Size */
		/* 		NULL,			// Parameters */
		/* 		1,				// Priority */
		/* 		NULL //handle to delete task
		); */
		xTaskCreate(taskServer, "taskServer", 4096, (void*)AF_INET, 5, NULL);
	}

	vTaskDelay(pdMS_TO_TICKS(500));
}
