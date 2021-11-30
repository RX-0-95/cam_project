
#include "main_function.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stdio.h"

#define GPIO_ON 1
#define GPIO_OFF 0


void GreenLEDTask(void *param){
    for(;;){
        gpio_put(PICO_DEFAULT_LED_PIN, GPIO_ON);
        vTaskDelay(1000);
        gpio_put(PICO_DEFAULT_LED_PIN, GPIO_OFF);
        vTaskDelay(1000);
    }
}

int main(int argc, char* argv[]){
    stdio_init_all();
    setup();
    for (;;){
        loop();
    }
    
  
    /*
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
    TaskHandle_t gLEDtask = NULL;
    uint32_t status = xTaskCreate(GreenLEDTask,
                            "Green LED",
                            1024,
                            NULL,
                            tskIDLE_PRIORITY,
                            &gLEDtask);
    vTaskStartScheduler();
    

    while (true){
        loop();
    }
    */
}