//#include "main_function.h"

#include "detect_config.h"
#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "person_detection_model_data.h"

#include "motion_detection.h"
#include "tiny_cv.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

//#include "arducam.h"

#include "pico/stdlib.h"
#include "hardware/irq.h"

#include "tensorflow/lite/micro/micro_time.h"
#include <climits>

#define TF_LITE_MICRO_EXECUTION_TIME_BEGIN      \
  int32_t start_ticks;                          \
  int32_t duration_ticks;                       \
  int32_t duration_ms;

#define TF_LITE_MICRO_EXECUTION_TIME(reporter, func)                    \
  if (tflite::ticks_per_second() == 0) {                                \
    TF_LITE_REPORT_ERROR(reporter,                                      \
                         "no timer implementation found");              \
  }                                                                     \
  start_ticks = tflite::GetCurrentTimeTicks();                          \
  func;                                                                 \
  duration_ticks = tflite::GetCurrentTimeTicks() - start_ticks;         \
  if (duration_ticks > INT_MAX / 1000) {                                \
    duration_ms = duration_ticks / (tflite::ticks_per_second() / 1000); \
  } else {                                                              \
    duration_ms = (duration_ticks * 1000) / tflite::ticks_per_second(); \
  }                                                                     \
  TF_LITE_REPORT_ERROR(reporter, "%s took %d ticks (%d ms)", #func,     \
                                    duration_ticks, duration_ms);

#define TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(reporter)            \
  if (tflite::ticks_per_second() == 0) {                                \
    TF_LITE_REPORT_ERROR(reporter,                                      \
                         "no timer implementation found");              \
  }                                                                     \
  start_ticks = tflite::GetCurrentTimeTicks();

#define TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(reporter, desc)        \
  duration_ticks = tflite::GetCurrentTimeTicks() - start_ticks;         \
  if (duration_ticks > INT_MAX / 1000) {                                \
    duration_ms = duration_ticks / (tflite::ticks_per_second() / 1000); \
  } else {                                                              \
    duration_ms = (duration_ticks * 1000) / tflite::ticks_per_second(); \
  }                                                                     \
  TF_LITE_REPORT_ERROR(reporter, "%s took %d ticks (%d ms)", desc,      \
                                    duration_ticks, duration_ms);

namespace
{
    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr; 

    constexpr int kTensorArenaSize = 136*1024;
    static uint8_t tensor_arena[kTensorArenaSize];

    MotionDetector * motion_detector = nullptr;

    //TinyCv related
    static TinyImage* tg_img = nullptr;
    static uint32_t pos_pixel_count = 0;
    static const uint8_t bitmap_threshold = 32;


} // namespace

#if defined (SEND_IMAGE_AFTER_CAPTURE)||defined (SEND_IMAGE_AFTER_INFERENCE)
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


void setup(){
    setup_uart();
    // setup logging
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // map the model into a usable data structure
    model = tflite::GetModel(g_person_detect_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION){
        TF_LITE_REPORT_ERROR(error_reporter,
                            "Model provider is schema version %d not equal"
                            "to spport version %d.",
                            model->version(),TFLITE_SCHEMA_VERSION);
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
    static tflite::MicroInterpreter static_interpreter(
        model,micro_op_resolver,tensor_arena,kTensorArenaSize,error_reporter);
    interpreter = &static_interpreter;

    // allocate memory from the tensor_arena for the model's tensor

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    // Get information about the memory area to use for the model's input.
    input = interpreter->input(0);

    TF_LITE_REPORT_ERROR(error_reporter,"Setup Clear!!!");

    // Detect motion
    //Allocate space for motion detector 
    static int8_t prev_frame_data[kNumRows*kNumCols];
    static int8_t frame_diff_data[kNumRows*kNumCols];
    static MotionDetector motion_detecto(input->data.int8,kNumRows,kNumCols,
                          prev_frame_data,frame_diff_data,BITMAP_THRESHOLD,
                          DETECT_PERCENT_THRESHOLD);
    motion_detector = &motion_detecto;

    // TinyCv related
    static TinyImage tg_im(kNumRows,kNumCols);    
    tg_img = &tg_im;
}

void loop(){
    TF_LITE_MICRO_EXECUTION_TIME_BEGIN
    TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_START(error_reporter);

    // get image from provider
    if (kTfLiteOk != GetImage(error_reporter,kNumCols,kNumRows,kNumChannels,
        input->data.int8)){
            TF_LITE_REPORT_ERROR(error_reporter,"Image capture failed");
        }
    TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter,"GetImage")


    // motion detection
    
    bool has_motion = motion_detector->detect_motion();
    /*
    pos_pixel_count = bit_map_transfer((uint8_t*)diff_buf,kNumRows,kNumCols,
                                      tg_img,bitmap_threshold);
    */

    //TF_LITE_REPORT_ERROR(error_reporter,"===>pos_count %d \n",pos_pixel_count);
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
    
    /*
    if (pos_pixel_count >= (uint32_t)kNumRows*kNumCols*0.05){
      if (has_motion) TF_LITE_REPORT_ERROR(error_reporter,"===>Detect Motion!!!\n");

    }
    */
    uint8_t header[2] = {0x55, 0xAA};
    uart_write_blocking(IMAGE_UART_ID, header, 2);
    uart_write_blocking(IMAGE_UART_ID,(uint8_t*)motion_detector->frame_diff_data,kMaxImageSize);

    //Run model on the input
    /*
    if (kTfLiteOk != interpreter->Invoke()) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }
    TF_LITE_MICRO_EXECUTION_TIME_SNIPPET_END(error_reporter, "Invoke")

    TfLiteTensor* output = interpreter->output(0);

    // Process the inference results.
    int8_t person_score = output->data.uint8[kPersonIndex];
    int8_t no_person_score = output->data.uint8[kNotAPersonIndex];
    RespondToDetection(error_reporter, person_score, no_person_score);
    */
}
