/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "can.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint8_t uart1_tx_busy;
static char uart1_tx_line[96];

CAN_TxHeaderTypeDef can_tx_header;
CAN_RxHeaderTypeDef can_rx_header;
static uint8_t can_tx_data[8];
static uint8_t can_rx_data[8];
static uint32_t can_tx_mailbox;
volatile uint32_t can_tx_count;
volatile uint32_t can_rx_count;
/* USER CODE END Variables */
/* Definitions for LedTask */
osThreadId_t LedTaskHandle;
const osThreadAttr_t LedTask_attributes = {
  .name = "LedTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CommTask */
osThreadId_t CommTaskHandle;
const osThreadAttr_t CommTask_attributes = {
  .name = "CommTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartLedTask(void *argument);
void StartCommTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of LedTask */
  LedTaskHandle = osThreadNew(StartLedTask, NULL, &LedTask_attributes);

  /* creation of CommTask */
  CommTaskHandle = osThreadNew(StartCommTask, NULL, &CommTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartLedTask */
/**
  * @brief  Function implementing the LedTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartLedTask */
void StartLedTask(void *argument)
{
  /* USER CODE BEGIN StartLedTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    osDelay(500);
  }
  /* USER CODE END StartLedTask */
}

/* USER CODE BEGIN Header_StartCommTask */
/**
* @brief Function implementing the CommTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCommTask */
void StartCommTask(void *argument)
{
  /* USER CODE BEGIN StartCommTask */
  can_tx_header.StdId = 0x123U;
  can_tx_header.ExtId = 0U;
  can_tx_header.RTR = CAN_RTR_DATA;
  can_tx_header.IDE = CAN_ID_STD;
  can_tx_header.DLC = 8U;
  can_tx_header.TransmitGlobalTime = DISABLE;

  /* Infinite loop */
  for(;;)
  {
    can_tx_count++;
    can_tx_data[0] = (uint8_t)(can_tx_count >> 8);
    can_tx_data[1] = (uint8_t)(can_tx_count & 0xFFU);
    can_tx_data[2] = 0xAAU;
    can_tx_data[3] = 0x55U;
    can_tx_data[4] = 0x11U;
    can_tx_data[5] = 0x22U;
    can_tx_data[6] = 0x33U;
    can_tx_data[7] = 0x44U;
    if (HAL_CAN_AddTxMessage(&hcan, &can_tx_header, can_tx_data, &can_tx_mailbox) != HAL_OK)
    {
      can_tx_count--;
    }

    if (uart1_tx_busy == 0U)
    {
      int uart1_line_len;
      uart1_tx_busy = 1U;
      uart1_line_len = snprintf(uart1_tx_line, sizeof(uart1_tx_line),
                                "CAN TX #%lu RX #%lu d=[%02X %02X %02X %02X %02X %02X %02X %02X]\r\n",
                                (unsigned long)can_tx_count, (unsigned long)can_rx_count,
                                (unsigned int)can_rx_data[0], (unsigned int)can_rx_data[1],
                                (unsigned int)can_rx_data[2], (unsigned int)can_rx_data[3],
                                (unsigned int)can_rx_data[4], (unsigned int)can_rx_data[5],
                                (unsigned int)can_rx_data[6], (unsigned int)can_rx_data[7]);
      if (HAL_UART_Transmit_DMA(&huart1, (uint8_t *)uart1_tx_line,
                                (uint16_t)uart1_line_len) != HAL_OK)
      {
        uart1_tx_busy = 0U;
      }
    }

    osDelay(500);
  }
  /* USER CODE END StartCommTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
  {
    uart1_tx_busy = 0U;
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  if (hcan->Instance == CAN1)
  {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_header, can_rx_data) == HAL_OK)
    {
      can_rx_count++;
    }
  }
}
/* USER CODE END Application */

