
#include "main_function.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "stdio.h"

//tasks

#include "task_pretask.h"
#include "message_sender.h"
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
    static bool is_person = false;
    
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

    //pritorty queue setup
    static struct priority_level cam_priority{
        .p1 = 0x10,
        .p2 = 0x20,
        .p3 = 0x30,
        .p_len = 1
    }; 
    const uint8_t priority_meta_len = 2; 
    static uint8_t priporty_header[priority_meta_len] = {0xaa,0xff};    
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
    if (!person_detection_done){
        TF_LITE_MICRO_EXECUTION_TIME_BEGIN
        TF_LITE_REPORT_ERROR(error_reporter,"***>Start person detection\n");
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter)
        if(kTfLiteOk != interpreter->Invoke()){
                TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
        }
        person_detection_done = true;
        TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter, "Invoke")
        TfLiteTensor* output = interpreter->output(0);
        int8_t person_score = output->data.uint8[kPersonIndex];
        int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
        //update is_person score
        is_person = (person_score > 0)? true:false;
        RespondToDetection(error_reporter,person_score,no_person_score);
    }
 
}

void image_capture(void *args __attribute__((unused))){    

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
        // send priority queue
        //uart_write_blocking(IMAGE_UART_ID,&cam_priority.p3,cam_priority.p_len);
        uart_write_blocking(IMAGE_UART_ID,priporty_header,2);
        uart_write_blocking(IMAGE_UART_ID,&cam_priority.p2,cam_priority.p_len); 
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
    
    if (is_person){
        TF_LITE_REPORT_ERROR(error_reporter,"===>Detect Person!!!\n");
        uart_write_blocking(IMAGE_UART_ID,priporty_header,2);
        uart_write_blocking(IMAGE_UART_ID,&cam_priority.p3,cam_priority.p_len); 
        is_person = false;
    }

    capture_done = true;
    
}



int main(int argc, char* argv[]){
    stdio_init_all();
    setup_uart();
    setup_model(); 
    setup_capture();
    //pretask::pretask_setup();
    //setup();
    
    for(;;){
        image_capture(NULL);
        person_detection(NULL);
    }
   return 95;
}