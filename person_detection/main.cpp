
#include "main_function.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stdio.h"

//tasks

#include "task_pretask.h"
//#include "task_person_detection.h"
//#include "task_capture_transfer.h"
//#include "task_capture.h"

#define GPIO_ON 1
#define GPIO_OFF 0

// prepare model related
namespace{
    static tflite::MicroErrorReporter task_error_reporter; 
    static tflite::ErrorReporter* error_reporter = &task_error_reporter;   
    static tflite::MicroInterpreter* interpreter = nullptr;
    static TfLiteTensor* input = nullptr; 
    static const tflite::Model * model = nullptr;
    static constexpr int kTensorArenaSize = 136*1024;
    static uint8_t tensor_arena[kTensorArenaSize];
    static bool person_detection_done = false; 
    
    //variable for image capture
    static const int captured_image_size = kNumCols*kNumRows*2;
    static uint8_t captured_image[captured_image_size]; //YUV 422 image 
    static bool capture_done = true;

    //Motion detection related
    static bool is_motion = true;
    MotionDetector * motion_detector = nullptr;
    static int8_t prev_frame_data[kNumRows*kNumCols];
    static int8_t frame_diff_data[kNumRows*kNumCols];
    static int8_t current_frame_data[kNumRows*kNumCols];
    //TinyCv related
   // static TinyImage* tg_img = nullptr;
    static uint32_t pos_pixel_count = 0;
    static const uint8_t bitmap_threshold = 32;
}

void setup_model(){
    model = tflite::GetModel(g_person_detect_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(error_reporter,
                           "Model provider is schema version %d not equal"
                           "to spport version %d.",
                        model->version(),TFLITE_SCHEMA_VERSION);
        return;
    }
    static tflite::MicroMutableOpResolver<5> micro_op_resolver;
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();
    static tflite::MicroInterpreter static_interpreter(
        model,micro_op_resolver,tensor_arena,kTensorArenaSize,error_reporter);
    interpreter = &static_interpreter;
    
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }
    input = interpreter->input(0);
    TF_LITE_REPORT_ERROR(error_reporter,"Person detection setup clear!!!");
}

void setup_capture(){
    /*
    static MotionDetector motion_detecto(input->data.int8,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
    motion_detector = &motion_detecto;
    */
    static MotionDetector motion_detecto(current_frame_data,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
    motion_detector = &motion_detecto;
   
}



//RX interrputer 
void on_uart_rx() {
    uint8_t cameraCommand = 0;
    //uart id is set in arducam
    while (uart_is_readable(IMAGE_UART_ID)){
        cameraCommand = uart_getc(IMAGE_UART_ID);
        //send it back?
        if (uart_is_writable(IMAGE_UART_ID)){
            uart_putc(IMAGE_UART_ID,cameraCommand);
        }
    }
}
void setup_uart(){
    // set up uart with the required speed.
    uint baud = uart_init(IMAGE_UART_ID,IMAGE_BAUD_RATE);
    // set the TX and RX pins by using the function selected on the GPIO
    // see datasheet for more information on function select
    gpio_set_function(IMAGE_UART_TX_PIN,GPIO_FUNC_UART);
    gpio_set_function(IMAGE_UART_RX_PIN,GPIO_FUNC_UART);
    // set data format
    uart_set_format(IMAGE_UART_ID,IMAGE_DATA_BITS,IMAGE_STOP_BITS,IMAGE_PARITY);
    // Turn off  FIFO's - do this char by char
    uart_set_fifo_enabled(IMAGE_UART_ID,false);
    // set RX interrupt
    // need to select the hanlder firsr
    // select correct interrupt for the UART we are using
    int UART_IRQ = IMAGE_UART_ID == uart0 ? UART0_IRQ:UART1_IRQ;

    // set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ,true);

    // enable the uart to send interrputs - RX only
    uart_set_irq_enables(IMAGE_UART_ID,true,false);
}


void setup(){
    stdio_init_all();
    setup_uart();
    setup_model(); 
    setup_capture();
}

//task for capture image and transfer


void person_detection(void *args __attribute__((unused))){
    person_detection_done = false;
    /*
    static MotionDetector motion_detecto(input->data.int8,kNumRows,kNumCols,
                        prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                        DETECT_PERCENT_THRESHOLD);
    motion_detector = &motion_detecto;
    */
    for(;;){
        /*
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter);
        // get image from provider
        if (kTfLiteOk != GetImage(error_reporter,kNumCols,kNumRows,kNumChannels,
            input->data.int8)){
                TF_LITE_REPORT_ERROR(error_reporter,"Image capture failed");
            }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter,"GetImage");
        */
        // motion detection
        //bool has_motion = motion_detector->detect_motion();
        /*
        if (has_motion){
            TF_LITE_REPORT_ERROR(error_reporter,"===>Detect Motion!!!\n");
            // Run model on the input
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter)
            if(kTfLiteOk != interpreter->Invoke()){
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
            }
            TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter, "Invoke")
            TfLiteTensor* output = interpreter->output(0);
            
            // process interfernce result
            int8_t person_score = output->data.uint8[kPersonIndex];
            int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
            RespondToDetection(error_reporter,person_score,no_person_score);
        }
        */
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_REPORT_ERROR(error_reporter,"***>Start person detection\n");
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter)
        if (!person_detection_done){
            if(kTfLiteOk != interpreter->Invoke()){
                    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
            }
            person_detection_done = true;
        }
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter, "Invoke")
        TfLiteTensor* output = interpreter->output(0);
        int8_t person_score = output->data.uint8[kPersonIndex];
        int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
        RespondToDetection(error_reporter,person_score,no_person_score);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void image_capture(void *args __attribute__((unused))){
    
    for (;;){
        capture_done = false;
        if (kTfLiteOk!=GetYUVImage(error_reporter,captured_image)){
            TF_LITE_REPORT_ERROR(error_reporter, "GetYUVImage() failed");
        }
        //load grey scale image (Y channnel of YUV422) to tensor
        uint16_t i,count;
        uint index = 0;
        int8_t* grey_image_data = input->data.int8;
        
        /*
        for (i=0;i<captured_image_size;i+=2){
            grey_image_data[index++] = captured_image[i]-128;
        }
        */
        
        for (i=0;i<captured_image_size;i+=2){
            current_frame_data[index++] = captured_image[i]-128;
        }
        
        
        //motion detection
        
        bool has_motion = motion_detector->detect_motion();
        if (has_motion){
            TF_LITE_REPORT_ERROR(error_reporter,"===>Detect Motion!!!\n");
            is_motion = true;
            //move the data to person detect scope
            if (person_detection_done){
                person_detection_done = false;
                memcpy(input->data.int8,current_frame_data,sizeof(int8_t)*kNumRows*kNumCols);
                TF_LITE_REPORT_ERROR(error_reporter,"===>Update person detect input!!!\n");
            }else{
                TF_LITE_REPORT_ERROR(error_reporter,"===>Person detect busy, not update!!!\n");
            }
        }else{
            is_motion = false;
        }
        capture_done = true;
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

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
    setup_uart();
    setup_model(); 
    setup_capture();
    //pretask::pretask_setup();
    //setup();
    
    /*
    for (;;){
        //person_detection(NULL);
        image_capture(NULL);
    }
    */
    
    TaskHandle_t g_pd = NULL;
  
    uint32_t pd_status = xTaskCreate(person_detection,
                                "Person Detection",
                                1024*4,
                                NULL,
                                2,
                                &g_pd);
    
    if (pd_status != pdPASS){
        printf("Can not create person detection task\n");
    }else{
        printf("Create person detection task\n");

    }
    /*
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
    */
        
    TaskHandle_t g_ct = NULL;
    uint32_t ct_status = xTaskCreate(image_capture,
                                    "Capture Transfer",
                                    5*1024,
                                    NULL,
                                    4,
                                    &g_ct);
    
    if (ct_status != pdPASS){
        printf("Can not create YUV capture transfer task\n");
    }else{
        printf("Create YUV capture transfer task\n");
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

   return 95;
}