/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include <stdio.h>
#include <string.h>
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "rc.h"
#include "usart.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t thread1_Handle;
const osThreadAttr_t thread1_attr = {
  .name = "thread1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t thread2_Handle;
const osThreadAttr_t thread2_attr = {
  .name = "thread2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t thread3_Handle;
const osThreadAttr_t thread3_attr = {
  .name = "thread3",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


// global variables
uint8_t body_control = 1;
uint8_t rc_state = STATE_STOP;
uint16_t speed_f		   = 500;
uint16_t speed_b		   = 500;
int16_t turn_left_speed_l  = -500;
int16_t turn_left_speed_r  = 500;
int16_t turn_right_speed_l = 500;
int16_t turn_right_speed_r = -500;

uint8_t bt_rx_buffer[50] = {0,};
uint8_t rx_buffer[50] = {0,};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void thread1(void *argument);
void thread2(void *argument);
void thread3(void *argument);

PUTCHAR_PROTOTYPE{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART6) {
		// Bluetooth
		HAL_UART_Receive_IT(&huart6, (uint8_t *)bt_rx_buffer, 1);
		HAL_UART_Transmit(&huart2, (uint8_t *)bt_rx_buffer, 1, 1000);
	}
	else if (huart->Instance == USART2) {
		// USB
		HAL_UART_Receive_IT(&huart2, (uint8_t *)rx_buffer, 1);
		HAL_UART_Transmit(&huart2, (uint8_t *)rx_buffer, 1, 1000);
	}
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

	// Initialize the motors
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

	// Initialize the bluetooth and USB DMA
  	HAL_UART_Receive_DMA(&huart6, (uint8_t *)bt_rx_buffer, 1);
	HAL_UART_Receive_DMA(&huart2, (uint8_t *)rx_buffer, 1);

	// Initialize the bluetooth and USB Interrupt
	HAL_UART_Receive_IT(&huart6, (uint8_t *)bt_rx_buffer, 1);
	HAL_UART_Receive_IT(&huart2, (uint8_t *)rx_buffer, 1);

	printf("RTOS Begin\n");
	
	wheel_stop();

	ssd1306_Init();
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(15, 30);
	ssd1306_WriteString("RC CAR PROJECT", Font_7x10, White);
	ssd1306_SetCursor(25, 50);
	ssd1306_WriteString("67 JO 8228", Font_7x10, White);
	ssd1306_UpdateScreen();

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
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  thread1_Handle = osThreadNew(thread1, NULL, &thread1_attr);
  thread2_Handle = osThreadNew(thread2, NULL, &thread2_attr);
  thread3_Handle = osThreadNew(thread3, NULL, &thread3_attr);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  	for(;;){
		// Heartbeat
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		osDelay(500);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// body control
void thread1(void *argument){
	static uint8_t last_buffer = 0;
	for(;;){

		if (body_control == 1) {
			switch (bt_rx_buffer[0]) {
				case 'w' :
					rc_state = go_forward(speed_f);
					break;
				case 'a' :
					rc_state = turn_left(turn_left_speed_l, turn_left_speed_r);
					break;
				case 's' :
					rc_state = go_backward(speed_b);
					break;
				case 'd' :
					rc_state = turn_right(turn_right_speed_l, turn_right_speed_r);
					break;
				case '0' : // 정지
					rc_state = wheel_stop();
					break;
				default: wheel_stop();
					break;
			}
		}
		else if( last_buffer != bt_rx_buffer[0] ){
			switch (bt_rx_buffer[0]) {
				case 'i' : // 전진속도 증가
					speed_f += 10;
					speed_f = (speed_f > RC_MAX_SPEED) ? RC_MAX_SPEED : speed_f; // 최대 속도 제한
					rc_state = STATE_INC_FOWARD_SPEED;
					break;
					
				case 'j' : // 턴 속도 감소
					turn_left_speed_l += 10;
					turn_left_speed_r -= 10;

					turn_right_speed_l -= 10;
					turn_right_speed_r += 10;

					turn_left_speed_l = (turn_left_speed_l > RC_MAX_SPEED) ? RC_MAX_SPEED : turn_left_speed_l; // 최대 속도 제한
					turn_right_speed_r = (turn_right_speed_r > RC_MAX_SPEED) ? RC_MAX_SPEED : turn_right_speed_r; // 최대 속도 제한

					turn_left_speed_r = (turn_left_speed_r < -1000) ? -1000 : turn_left_speed_r; // 최소 속도 제한
					turn_right_speed_l = (turn_right_speed_l < -1000) ? -1000 : turn_right_speed_l; // 최소 속도 제한

					rc_state = STATE_DEC_TURN_SPEED;
					break;

				case 'k' : // 전진속도 감소
					speed_f -= 10;
					speed_f = (speed_f < RC_MIN_SPEED) ? RC_MIN_SPEED : speed_f; // 최소 속도 제한
					rc_state = STATE_DEC_FOWARD_SPEED;
					break;

				case 'l' : // 턴 속도 증가
					turn_left_speed_l -= 10;
					turn_left_speed_r += 10;

					turn_right_speed_l += 10;
					turn_right_speed_r -= 10;

					turn_right_speed_l = (turn_left_speed_l > RC_MAX_SPEED) ? RC_MAX_SPEED : turn_left_speed_l; // 최대 속도 제한
					turn_left_speed_r = (turn_right_speed_r > RC_MAX_SPEED) ? RC_MAX_SPEED : turn_right_speed_r; // 최대 속도 제한

					turn_right_speed_r = (turn_left_speed_r < -1000) ? -1000 : turn_left_speed_r; // 최소 속도 제한
					turn_left_speed_l = (turn_right_speed_l < -1000) ? -1000 : turn_right_speed_l; // 최소 속도 제한

					rc_state = STATE_INC_TURN_SPEED;
					break;

					case '1' : // start button : set max speed
						speed_f		  	    =  RC_MAX_SPEED;
						speed_b		        =  RC_MAX_SPEED;
						turn_left_speed_l   = -RC_MAX_SPEED;
						turn_left_speed_r   =  RC_MAX_SPEED;
						turn_right_speed_l  =  RC_MAX_SPEED;
						turn_right_speed_r  = -RC_MAX_SPEED;
						break;

					case '2' : // start button : set max speed
						speed_f		  	    =  500;
						speed_b		        =  500;
						turn_left_speed_l   = -500;
						turn_left_speed_r   =  500;
						turn_right_speed_l  =  500;
						turn_right_speed_r  = -500;
						break;

				default: wheel_stop();
					break;
			}
		}
		last_buffer = bt_rx_buffer[0];
		osDelay(1);
	}
}

void thread2(void *argument){
	uint8_t txt_buf[11] = {0,};
	for(;;){
		ssd1306_SetCursor(0, 0);
		switch (rc_state) {
			case STATE_STOP:
				ssd1306_WriteString("   STOP    ", Font_11x18, White);
				break;
			case STATE_FORWARD:
				ssd1306_WriteString("  FORWARD  ", Font_11x18, White);
				break;
			case STATE_BACKWARD:
				ssd1306_WriteString(" BACKWARD  ", Font_11x18, White);
				break;
			case STATE_TURN_LEFT:
				ssd1306_WriteString(" TURN LEFT ", Font_11x18, White);
				break;
			case STATE_TURN_RIGHT:
				ssd1306_WriteString("TURN RIGHT ", Font_11x18, White);
				break;

			case STATE_INC_FOWARD_SPEED: 
				__itoa(speed_f, txt_buf, 10);
				ssd1306_WriteString(strcat(txt_buf," /1000 +"), Font_11x18, White);
				break;
			case STATE_DEC_FOWARD_SPEED:
				__itoa(speed_f, txt_buf, 10);
				ssd1306_WriteString(strcat(txt_buf," /1000 -"), Font_11x18, White);
				break;
			case STATE_INC_TURN_SPEED:
				__itoa(turn_left_speed_r, txt_buf, 10);
				ssd1306_WriteString("T SPD: ", Font_11x18, White);
				ssd1306_WriteString(txt_buf, Font_11x18, White);
				break;
			case STATE_DEC_TURN_SPEED:
				__itoa(turn_left_speed_r, txt_buf, 10);
				ssd1306_WriteString("T SPD: ", Font_11x18, White);
				ssd1306_WriteString(txt_buf, Font_11x18, White);
				break;

			default:
				ssd1306_WriteString("   STOP    ", Font_11x18, White);
				break;
		}
		ssd1306_UpdateScreen();		
		osDelay(20);
	}
}

void thread3(void *argument){
	for(;;){
		if ( bt_rx_buffer[0] == 'w' || bt_rx_buffer[0] == 'a' || bt_rx_buffer[0] == 's' || bt_rx_buffer[0] == 'd' || bt_rx_buffer[0] == '0' ) {
			body_control = 1;
		}
		else {
			body_control = 0;
		}
		osDelay(1);
	}
}
/* USER CODE END Application */

