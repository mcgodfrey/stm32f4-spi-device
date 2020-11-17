/* comms.c */
#include "comms.h"
#include <string.h>
#include "main.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define BUFFER_LEN 32


typedef enum {
  COMMS_STATUS_ERROR = 0x00,  
  COMMS_STATUS_OK = 0x01,
} CommsStatus_t;

// holds a reference to the comms task, so that the interrupt can wake it when an SPI transfer is complete
TaskHandle_t comms_task_handle;

uint8_t stored_message[BUFFER_LEN] = "initial string";
uint8_t comms_tx_buffer[BUFFER_LEN];
uint8_t comms_rx_buffer[BUFFER_LEN];
CommsStatus_t comms_status;

/* main comms thread task */
void comms_handler_task(void *pvParameters){
  // save a reference to this thread, so that the interrupt handler can wake it up
  comms_task_handle = xTaskGetCurrentTaskHandle();
  comms_status = COMMS_STATUS_OK;

  while(1){
    // The tx buffer contains the status, and the length of the stored message
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);

    comms_tx_buffer[0] = comms_status;
    comms_tx_buffer[1] = strlen(stored_message);
    // start an SPI DMA transfer of 2 bytes
    HAL_SPI_TransmitReceive_DMA(&hspi1, comms_tx_buffer, comms_rx_buffer, 2);
    // Sleep thread until the spi xfer is complete
    ulTaskNotifyTake(1, portMAX_DELAY);
	HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);


	
	uint8_t controller_write = comms_rx_buffer[1];
	if(controller_write){
	  // controller write to this device
	  uint8_t nbytes = comms_rx_buffer[1];  // Note: should check that this is less than BUFFER_LEN
	  memset(comms_rx_buffer, 0, BUFFER_LEN);
	  memset(comms_tx_buffer, 0, BUFFER_LEN);
	  HAL_SPI_TransmitReceive_DMA(&hspi1, comms_tx_buffer, comms_rx_buffer, nbytes);
      ulTaskNotifyTake(1, portMAX_DELAY);
	  //for(int i=0; i<nbytes; i++){
	//	  stored_message[i] = comms_
	  strncpy(stored_message, comms_rx_buffer, nbytes);
	}else{
	  // controller read from this device
	  memset(comms_tx_buffer, 0, BUFFER_LEN);
	  memset(comms_rx_buffer, 0, BUFFER_LEN);
	  strcpy(comms_tx_buffer, stored_message);
	  HAL_SPI_TransmitReceive_DMA(&hspi1, comms_tx_buffer, comms_rx_buffer, strlen(stored_message));
      ulTaskNotifyTake(1, portMAX_DELAY);
	}
  }
}

/* Callback when the DMA transfer is complete. 
   It just calls NotifyGive to wake the comms thread when it returns */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
  BaseType_t higherPriorityTaskWoken = 0;
  vTaskNotifyGiveFromISR(comms_task_handle, &higherPriorityTaskWoken);
  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
