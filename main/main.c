#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_err.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

#define ECHO_GPIO GPIO_NUM_12
#define TRIGGER_GPIO GPIO_NUM_13
//#define MAX_DISTANCE_CM 400 // Maximum of 4 meters

int32_t expCount = 0;
TimerHandle_t timer;

void vTimerCallback(TimerHandle_t pxTimer)
{
    const int32_t MaxExpCount = 100;
    expCount += 1;
    if(expCount == MaxExpCount)
    {
        xTimerStop(pxTimer, 0);
    }
}

float ultra(void)
{
    float dist;
    
    gpio_set_direction(ECHO_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(TRIGGER_GPIO, GPIO_MODE_OUTPUT);
    rtc_gpio_deinit(TRIGGER_GPIO);
    rtc_gpio_set_level(TRIGGER_GPIO, 0);
    vTaskDelay(1 / portTICK_RATE_MS);
    rtc_gpio_set_level(TRIGGER_GPIO, 1);
    vTaskDelay(5 / portTICK_RATE_MS);
    rtc_gpio_set_level(TRIGGER_GPIO, 0);

    while(rtc_gpio_get_level(ECHO_GPIO) == 0);
    xTimerReset(timer, 100);
    xTimerStart(timer, 100);
    while(rtc_gpio_get_level(ECHO_GPIO) == 1);
    xTimerStop(timer, 0);
    
    dist = xTimerGetPeriod(timer) / portTICK_PERIOD_MS;
    dist = dist * 0.01716;

    return dist;
}

void app_main()
{
    rtc_gpio_set_level(TRIGGER_GPIO, 0);
    printf("HC-SR04 interfacing with ESP-IDF..........\n");
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    xTimerCreate("Timer", (500 / portTICK_PERIOD_MS), pdTRUE, (void *)0, vTimerCallback);

    while(1)
    {
        printf("Distance: %.2f cm", ultra());
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
}

// void ultrasonic_test(void *pvParameters)
// {
//     float distance;

//     ultrasonic_sensor_t sensor = {
//         .trigger_pin = TRIGGER_GPIO,
//         .echo_pin = ECHO_GPIO
//     };

//     ultrasonic_init(&sensor);

//     while (true) {
//         esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
//         if (res == ESP_OK) {
//             printf("Distance: %0.04f m\n", distance);
//         } // Print error
//         else {
//             printf("Error %d: ", res);
//             switch (res) {
//                 case ESP_ERR_ULTRASONIC_PING:
//                     printf("Cannot ping (device is in invalid state)\n");
//                     break;
//                 case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
//                     printf("Ping timeout (no device found)\n");
//                     break;
//                 case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
//                     printf("Echo timeout (i.e. distance too big)\n");
//                     break;
//                 default:
//                     printf("%s\n", esp_err_to_name(res));
//             }
//         }

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

// void app_main()
// {
//     xTaskCreate(ultrasonic_test, "ultrasonic_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
// }
