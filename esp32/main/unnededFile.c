// void taskClient(void *pvParameters){
// 	const char* TAG;
// 	TAG = "CLIENT";
// 	while(1){
// 		// Create IPv4, TCP socket file descriptor
// 		// SOCK_STREAM = TCP Socket
// 		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
// 		if (sockfd < 0){
// 			ESP_LOGE(TAG, "Unable to create socket: error %d", errno);
// 			close(sockfd);
// 			vTaskDelay(pdMS_TO_TICKS(10));
// 			continue;
// 		}
		
// 		ESP_LOGI(TAG, "Socket successfully created");

// 		// Setup host
// 		const char* hostname = "192.168.1.121";
// 		struct hostent *host = gethostbyname(hostname);
// 		if (host == NULL){
// 			ESP_LOGE(TAG, "cannot resolve host by hostname %s", hostname);
// 			close(sockfd);
// 			vTaskDelay(pdMS_TO_TICKS(10));
// 			continue;
// 		}

// 		ESP_LOGI(TAG, "Host setup successful\nHost Name: %s", host->h_name);

// 		// Setup socket parameters for host
// 		// set up socket address struct with server info
// 		struct sockaddr_in address;
// 		memset(&address, '\0', sizeof(address));
// 		address.sin_family = AF_INET;		// IPv4
// 		memcpy((char *)&address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
// 		address.sin_port = htons(30000);	// Server port, big endian
// 		/* address.sin_addr.s_addr = *(in_addr_t*)host->h_addr;	// server ip */

// 		// Connect the socket address parameters with the socket
// 		// Block until connection is established or an error occurs
// 		// If successful, open the client file descriptor for reading and writing
// 		int err = connect(sockfd, (SA*)&address, sizeof(address));
// 		if (err != 0) {
// 			ESP_LOGE(TAG, "socket unable to connect: err %d", errno);
// 			close(sockfd);
// 			vTaskDelay(pdMS_TO_TICKS(10));
// 			continue;
// 		}

// 		ESP_LOGI(TAG, "Successfully connected");

// 		char* msg;
// 		msg = "Hello from ESP32";
// 		send(sockfd, msg, strlen(msg), 0);	// Send a TCP string

// 		// Receive messages from server
// 		char buffer[1024];
// 		while (1){
// 			recv(sockfd, buffer, 1024, 0);
// 			ESP_LOGI("CLIENT", "Response: %s", buffer);
// 			vTaskDelay(pdMS_TO_TICKS(1));
// 		}

// 		// Shutdown and Close Socket
// 		if (sockfd != -1){
// 			ESP_LOGE(TAG, "Shutting down socket and restarting");
// 			shutdown(sockfd, 0);
// 			close(sockfd);
// 		}
// 	}
// 	/* vTaskDelete(NULL); */
// }

// static void setup(){
	
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
// }