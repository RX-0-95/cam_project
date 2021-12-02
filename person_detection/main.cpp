
#include "main_function.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stdio.h"

//tasks

#include "task_pretask.h"
#include "task_person_detection.h"
#include "task_capture_transfer.h"



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
    pretask::pretask_setup();

    
    TaskHandle_t g_pd = NULL;
    uint32_t pd_status = xTaskCreate(person_detection_task,
                                "Person Detection",
                                1024*5,
                                NULL,
                                2,
                                &g_pd);
    
    if (pd_status != pdPASS){
        printf("Can not create person detection task\n");
    }else{
        printf("Create person detection task\n");

    }
    

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);
    TaskHandle_t gLEDtask = NULL;
    uint32_t status = xTaskCreate(GreenLEDTask,
                            "Green LED",
                            512,
                            NULL,
                            3,
                            &gLEDtask);
    if (status != pdPASS){
        printf("Can not create green LED task\n");
    }else{
        printf("Create green LED task\n");
    }
    
    /*
    TaskHandle_t g_ct = NULL;
    uint32_t ct_status = xTaskCreate(capture_transfer_task,
                                    "Capture Transfer",
                                    5*1024,
                                    NULL,
                                    4,
                                    &g_ct);
    
    if (ct_status != pdPASS){
        printf("Can not create capture transfer task\n");
    }else{
        printf("Create capture transfer task\n");
    }
    */
    


    vTaskStartScheduler();
    /*
    setup();
    for (;;){
        loop();
    }
    */
   //should never go here
   for(;;){
    
   }
   
    
    

    //pretask_setup(NULL);


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
   return 95;
}