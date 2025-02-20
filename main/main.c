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
#include <stdio.h>
#include <string.h>

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
void move(int);

typedef struct sockaddr SA;
/* static const int ip_protocol = 0; */

static uint8_t s_led_state = 0;
static int direction_test = 0;

static int lowerReverseBound = 0;
static int upperReverseBound = 0;
static int lowerForwardBound = 0;

void taskClient(void *pvParameters){
	const char* TAG;
	TAG = "CLIENT";
	while(1){
		// Create IPv4, TCP socket file descriptor
		// SOCK_STREAM = TCP Socket
		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd < 0){
			ESP_LOGE(TAG, "Unable to create socket: error %d", errno);
			close(sockfd);
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}
		
		ESP_LOGI(TAG, "Socket successfully created");

		// Setup host
		const char* hostname = "192.168.1.121";
		struct hostent *host = gethostbyname(hostname);
		if (host == NULL){
			ESP_LOGE(TAG, "cannot resolve host by hostname %s", hostname);
			close(sockfd);
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}

		ESP_LOGI(TAG, "Host setup successful\nHost Name: %s", host->h_name);

		// Setup socket parameters for host
		// set up socket address struct with server info
		struct sockaddr_in address;
		memset(&address, '\0', sizeof(address));
		address.sin_family = AF_INET;		// IPv4
		memcpy((char *)&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
		address.sin_port = htons(30000);	// Server port, big endian
		/* address.sin_addr.s_addr = *(in_addr_t*)host->h_addr;	// server ip */

		// Connect the socket address parameters with the socket
		// Block until connection is established or an error occurs
		// If successful, open the client file descriptor for reading and writing
		int err = connect(sockfd, (SA*)&address, sizeof(address));
		if (err != 0) {
			ESP_LOGE(TAG, "socket unable to connect: err %d", errno);
			close(sockfd);
			vTaskDelay(pdMS_TO_TICKS(10));
			continue;
		}

		ESP_LOGI(TAG, "Successfully connected");

		char* msg;
		msg = "Hello from ESP32";
		send(sockfd, msg, strlen(msg), 0);	// Send a TCP string

		// Receive messages from server
		char buffer[1024];
		while (1){
			recv(sockfd, buffer, 1024, 0);
			ESP_LOGI("CLIENT", "Response: %s", buffer);
			vTaskDelay(pdMS_TO_TICKS(1));
		}

		// Shutdown and Close Socket
		if (sockfd != -1){
			ESP_LOGE(TAG, "Shutting down socket and restarting");
			shutdown(sockfd, 0);
			close(sockfd);
		}
	}
	/* vTaskDelete(NULL); */
}

typedef struct {
	bool forward, left, right, back;
} Movement;

Movement* getMovementStruct(char *buffer, int length)
{
	Movement *theStruct = malloc(sizeof(Movement)); // Allocate memory
    *theStruct = (Movement){false, false, false, false}; // Proper struct initialization
	for(int i = 0; i < length; i++)
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
	return theStruct;
}

static void doMovement(Movement *movementDirections)
{
	ESP_LOGI("DEBUG", "Direction is %d, forward is %d, left is %d, right is %d, back is %d, ", direction_test, movementDirections->forward, movementDirections->left, movementDirections->right, movementDirections->back);
	if(movementDirections->forward)
	{
		if(direction_test < 100)
			direction_test++;
		if(direction_test == 1 || direction_test == upperReverseBound + 1)
			direction_test = lowerForwardBound;		
	}
	if(movementDirections->back)
	{
		if(direction_test > 0)
			direction_test--;
		if(direction_test == lowerReverseBound - 1)
			direction_test = 0;
		if(direction_test == lowerForwardBound - 2)
			direction_test = upperReverseBound;
	}
	if(movementDirections->left)
	{

	}
	if(movementDirections->right)
	{

	}
	else
	{
		ESP_LOGI("ERROR", "For some reason, none of the user inputs are being pressed.");
	}
	move(direction_test);
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
int getRawDutyFromPercent(float duty){	
	duty = duty / 100;	
	return (pow(2, LEDC_DUTY_RES) * duty);
}

float getPercentFromRawDuty(float duty)
{
	return (duty*100)/(pow(2, LEDC_DUTY_RES));
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

	// Channel Configuration
	// ledc_channel_config_t channel_conf2 = {
	// 	.speed_mode = LEDC_MODE,
	// 	.channel = LEDC_CHANNEL2,
	// 	.timer_sel = LEDC_TIMER,
	// 	.intr_type = LEDC_INTR_DISABLE,
	// 	.gpio_num = LEDC_OUTPUT_IO2,
	// 	.duty = 0,
	// 	.hpoint = 0
	// };
	// ESP_ERROR_CHECK(ledc_channel_config(&channel_conf2));
}

void move(int direction){
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL1, getRawDutyFromPercent(direction)));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL1));

	ESP_LOGI("DUTY CHECK", "Duty is: %lu", ledc_get_duty(LEDC_MODE, LEDC_CHANNEL1));

	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL2, getRawDutyFromPercent(direction)));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL2));

	// ESP_LOGI("DUTY CHECK", "Duty is: %lu", ledc_get_duty(LEDC_MODE, LEDC_CHANNEL2));

	// Pulse of : 800 - 1100 uS (Reverse)
	// Pulse of : 1900 - 2200 uS (Forward)

	// Frequency of 454 Hz = Period of 2202 uS
	// 800 uS = 36
	// 1100 uS = 50
	//
	// 1900 uS = 86
	// 2200 uS = 100
}

static void setup(){
	
	/* xTaskCreate( */
	/* 		taskMove, */
	/* 		"taskMove", */
	/* 		4096,			// Stack Size */
	/* 		NULL,			// Parameters */
	/* 		1,				// Priority */
	/* 		NULL); */

	/* xTaskCreate( */
	/* 		taskBlinkLED, */
	/* 		"taskBlinkLED", */
	/* 		2048,			// Stack Size */
	/* 		NULL,			// Parameters */
	/* 		1,				// Priority */
	/* 		NULL); */
}

void app_main() {
	//THESE ARE THE RAW DUTIES
	lowerReverseBound = convertPulseWidthToPercentDuty(800, true);
	upperReverseBound = convertPulseWidthToPercentDuty(1100, false);
	lowerForwardBound = convertPulseWidthToPercentDuty(1900, true);
	ESP_LOGI("BOUNDS", "Lower bound to reverse motor: %d, upper bound to reverse motor: %d, loewr bound to push motor forward: %d", lowerReverseBound, upperReverseBound, lowerForwardBound);
	
	ledc_setup();
	
	gpio_reset_pin(BLINK_GPIO);
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	s_led_state = 1;
	gpio_set_level(BLINK_GPIO, s_led_state);

	vTaskDelay(pdMS_TO_TICKS(500));
	
	if (wifi_setup_init()){
		/* xTaskCreate( */
		/* 		taskClient, */
		/* 		"taskClient", */
		/* 		8192,			// Stack Size */
		/* 		NULL,			// Parameters */
		/* 		1,				// Priority */
		/* 		NULL); */
		xTaskCreate(
			taskServer,
			"taskServer",
			4096,
			(void*)AF_INET,
			5,
			NULL
		);
	}
}
