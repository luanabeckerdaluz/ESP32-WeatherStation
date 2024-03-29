#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
/*LOG*/
#include "esp_err.h"
#include "esp_log.h"
/*ADC*/
#include "driver/adc.h"
#include "esp_adc_cal.h"
/*SPIFFS*/
#include "esp_spiffs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h>
/*WIFI*/
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_attr.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"
#include <sys/time.h>
#include <time.h>
/*PROJECT*/
#include "ESP-WeatherStation.h"

static const char *TAG = "ESP32-WeatherStation";

void init_app() {
	ESP_LOGI(TAG, "Setting all parameters");

	// Set status pin as output
	gpio_pad_select_gpio(STATUS_LED);
	gpio_set_direction(STATUS_LED, GPIO_MODE_OUTPUT);

	init_adc();

	init_spiffs();

    init_time();

    // ALL SET!
	for (int i=0; i<NUM_BLINKS_INIT; i++){
			gpio_set_level(STATUS_LED, 1); 
			vTaskDelay(DELAY_BLINK_ON / portTICK_PERIOD_MS);
			gpio_set_level(STATUS_LED, 0);
			vTaskDelay(DELAY_BLINK_OFF / portTICK_PERIOD_MS);
	}
	ESP_LOGI(TAG, "All Set");
}

void app_main() {
	// Init components
	init_app();

	int contSensorReads=0;
	float temperatures[TIMESTOUPLOADVALUE];
	FILE* f;

	// Open file to write
	ESP_LOGI(TAG, "Opening file");
	f = fopen("/spiffs/hello.txt", "w");

	int j;
	for(j=0; j<30; j++){
		ESP_LOGI(TAG, "-------------%d/%d-------------", (contSensorReads+1), TIMESTOUPLOADVALUE);

		// Read ADC
		uint32_t ADCread = ADCreading();
		ESP_LOGI(TAG, "ADC read: %d", ADCread);
		
		uint32_t voltage = ADCtoVoltage(ADCread);
		ESP_LOGI(TAG, "Voltage: %dmV", voltage);

		// Converts ADC value to Voltage (mV)
		float temperature = milivoltsTOcelciusLM35(voltage);
		temperatures[contSensorReads] = temperature;
		ESP_LOGI(TAG, "Temperature: %.1f ºC", temperature);

		// Update control variable
		contSensorReads++;

		// Statistic Treatment
		if(contSensorReads == TIMESTOUPLOADVALUE){
			ESP_LOGI(TAG, "-------------------------------");

			// Reset control variable
			contSensorReads = 0;

			// Get datetime on string
			char* datetime = get_datetime_ntp();
			ESP_LOGI(TAG, "Got datetime: %s", datetime);
			
			// Get vector median
			float tempCelcius = getArrayMedian(temperatures, TIMESTOUPLOADVALUE);
			ESP_LOGI(TAG, "Got array median: %.1f", tempCelcius);

			// Write on File
			if (f == NULL) {
		        ESP_LOGE(TAG, "Failed to open file for writing");
		        return;
		    }
		    fprintf(f, "%s: %.1f\n", datetime, tempCelcius);
		    ESP_LOGI(TAG, "Written: %s: %.1f", datetime, tempCelcius);
		}
		vTaskDelay(pdMS_TO_TICKS(DELAY_EACH_ADC_READ));
	}
	fclose(f);
	ESP_LOGI(TAG, "File written Closed");


	/*
	// ---------------------------------------------//
	// ---------------------------------------------//
	// ---------------------------------------------//
	*/
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/spiffs/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    while(fgets(line, sizeof(line), f)){
        // strip newline
        char* pos = strchr(line, '\n');
        if (pos) {
            *pos = '\0';
        }
        ESP_LOGI(TAG, "Read from file: '%s'", line);
    
        //do something with buf here;
        //The input of fgets will be NULL as soon 
        //as its input fp has been fully read, then exit the loop  
    }
    fclose(f);
	/*
	// ---------------------------------------------//
	// ---------------------------------------------//
	// ---------------------------------------------//
	*/


    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}

