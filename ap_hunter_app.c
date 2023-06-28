#include <furi_hal.h>

#define UART_CHANNEL FuriHalUartIdUSART1
#define BAUDRATE 115200
#define RX_BUF_SIZE 128

// Buffer to store received data
uint8_t rx_buffer[RX_BUF_SIZE];
size_t rx_buffer_len = 0;

// Worker thread
FuriThread* worker_thread;

// Worker thread event flags
typedef enum {
    WorkerEvtRxDone = (1 << 0),
} WorkerEvtFlags;

// UART receive callback
void uart_rx_callback(UartIrqEvent event, uint8_t data, void* context) {
    (void)context;
    if(event == UartIrqEventRXNE) {
        // Store received data in buffer
        rx_buffer[rx_buffer_len++] = data;

        // Signal worker thread
        furi_thread_flags_set(furi_thread_get_id(worker_thread), WorkerEvtRxDone);
    }
}

// Worker thread function
int32_t worker_thread_func(void* context) {
    (void)context;
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, FuriWaitForever);

        if(events & WorkerEvtRxDone) {
            // Process received data
            // ...

            // Clear buffer
            rx_buffer_len = 0;
        }
    }

    return 0;
}

// Initialize UART
void uart_init() {
    furi_hal_uart_init(UART_CHANNEL, BAUDRATE);
    furi_hal_uart_set_irq_cb(UART_CHANNEL, uart_rx_callback, NULL);
}

// Transmit data
void uart_transmit(uint8_t* data, size_t len) {
    furi_hal_uart_tx(UART_CHANNEL, data, len);
}

// Main function
int ap_hunter_app(void) {
    // Initialize UART
    uart_init();

    // Create worker thread
    worker_thread = furi_thread_alloc();
    furi_thread_set_name(worker_thread, "WorkerThread");
    furi_thread_set_stack_size(worker_thread, 1024);
    furi_thread_set_callback(worker_thread, worker_thread_func);
    furi_thread_start(worker_thread);

    // Transmit data
    uint8_t tx_data[] = "Hello, world!";
    uart_transmit(tx_data, sizeof(tx_data));

    // Main loop
    while(1) {
        // Main thread can perform other tasks or sleep to conserve power
    }

    return 0;
}