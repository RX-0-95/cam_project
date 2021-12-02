#include "task_pretask.h"


namespace pretask
{
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

void pretask_setup(){
  setup_uart();
}
    
} // namespace pretask


