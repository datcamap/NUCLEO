/*
 * it.c
 *
 *  Created on: May 12, 2025
 *      Author: datad
 */

#include "main_app.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htiem6;
extern TIM_HandleTypeDef htiem2;
extern CAN_HandleTypeDef hcan1;

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void TIM6_DAC_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htiem6); // Interrupt processing API (it checks what type of interrupt that occurred)
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}

void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htiem2);
}

void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_SCE_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}

void EXTI15_10_IRQHandler (void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
