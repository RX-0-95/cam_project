#include "task_pretask.h"
#include "detect_config.h"
#include "model_settings.h"
#include "person_detection_model_data.h"

#include "image_provider.h"
#include "motion_detection.h"
#include "detection_responder.h"
#include "tiny_cv.h"

namespace pretask
{
#if defined (SEND_IMAGE_AFTER_CAPTURE)||defined (SEND_IMAGE_AFTER_INFERENCE)||defined (SEND_JPEG_IMAGE)
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
#else
void setup_uart() {}
#endif// defined (SEND_IMAGE_AFTER_CAPTURE)||defined (SEND_IMAGE_AFTER_INFERENCE)

void setup_model(){
    pretask::model = tflite::GetModel(g_person_detect_model_data);
    if (pretask::model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(pretask::error_reporter,
                            "Model provider is schema version %d not equal"
                            "to spport version %d.",
                            pretask::model->version(),TFLITE_SCHEMA_VERSION);
        return;
    }
    // pull in only the operation implementations we need
    
    static tflite::MicroMutableOpResolver<5> micro_op_resolver;
    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddDepthwiseConv2D();
    micro_op_resolver.AddReshape();
    micro_op_resolver.AddSoftmax();
    
    //Build interrpter to run the model with
    
    static tflite::MicroInterpreter static_interpreter(model,micro_op_resolver,tensor_arena,kTensorArenaSize,error_reporter);
    pretask::interpreter = &static_interpreter;
    
    // allocate memory from the tensor_arena for the model's tensor
    
    TfLiteStatus allocate_status = pretask::interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(pretask::error_reporter, "AllocateTensors() failed");
        return;
    }
    pretask::input = pretask::interpreter->input(0);
    TF_LITE_REPORT_ERROR(pretask::error_reporter,"Person detection setup clear!!!");       
    
}

void pretask_setup(){
  setup_uart();
  setup_model();
}
    
} // namespace pretask


