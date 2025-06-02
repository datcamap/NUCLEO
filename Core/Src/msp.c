/*
 * ms.c
 *
 *  Created on: May 12, 2025
 *      Author: datad
 */

#include "main_app.h"

void HAL_MspInit(void)
{
	GPIO_InitTypeDef gpio_uart;

	// Enable clock for uart2 and gpioA
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Pin multiplexing
	gpio_uart.Pin = GPIO_PIN_2;
	gpio_uart.Mode = GPIO_MODE_AF_PP;
	gpio_uart.Pull = GPIO_PULLUP;
	gpio_uart.Alternate = GPIO_AF7_USART2;
	gpio_uart.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_uart);
	gpio_uart.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOA, &gpio_uart);

	// Enable interrupt
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 14, 0);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	// Enable timer 6 clock
	__HAL_RCC_TIM6_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();

	// Enable interrupt request number of timer 6
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 14, 0);
	// Enable interrupt request number of timer 2
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_SetPriority(TIM2_IRQn, 14, 0);
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
	// Enable timer 2 & GPIOA clock
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//Assign GPIOA pin 1 to TIM2 channel 1
	GPIO_InitTypeDef gpio_tim2IC;
	gpio_tim2IC.Pin = GPIO_PIN_0;
	gpio_tim2IC.Mode = GPIO_MODE_AF_PP;
	gpio_tim2IC.Pull = GPIO_PULLDOWN;
	gpio_tim2IC.Alternate = GPIO_AF1_TIM1;
	gpio_tim2IC.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio_tim2IC);

	// Enable interrupt request number of timer 2
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_SetPriority(TIM2_IRQn, 14, 0);
}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{
	// Enable timer 2 & GPIOA clock
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//Assign PA0 to TIM2_CH1
	GPIO_InitTypeDef gpio_tim2OC;
	gpio_tim2OC.Pin = GPIO_PIN_0;
	gpio_tim2OC.Mode = GPIO_MODE_AF_PP;
	gpio_tim2OC.Pull = GPIO_NOPULL;
	gpio_tim2OC.Alternate = GPIO_AF1_TIM2;
	gpio_tim2OC.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_tim2OC);

	// Enable interrupt request number of timer 2
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_SetPriority(TIM2_IRQn, 14, 0);
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	// Enable timer 2 & GPIOA clock & GPIOB clock
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef gpio_timPWM;
	//Assign PB3 to TIM2_CH2(Red), PB10 to TIM2_CH3(Green), PB2 to TIM2_CH4(Blue)
	gpio_timPWM.Pin = GPIO_PIN_3 | GPIO_PIN_10 | GPIO_PIN_2;
	gpio_timPWM.Mode = GPIO_MODE_AF_PP;
	gpio_timPWM.Pull = GPIO_NOPULL;
	gpio_timPWM.Alternate = GPIO_AF1_TIM2;
	gpio_timPWM.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio_timPWM);
	//Assign PA8 to TIM1_CH1
	gpio_timPWM.Pin = GPIO_PIN_8;
	HAL_GPIO_Init(GPIOA, &gpio_timPWM);
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	//Assign PA11 to CAN_RX, PA12 to CAN_TX
	GPIO_InitTypeDef gpio_CAN1;
	gpio_CAN1.Pin = GPIO_PIN_11 | GPIO_PIN_12;
	gpio_CAN1.Mode = GPIO_MODE_AF_PP;
	gpio_CAN1.Pull = GPIO_NOPULL;
	gpio_CAN1.Alternate = GPIO_AF9_CAN1;
	gpio_CAN1.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &gpio_CAN1);

	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
	HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 15, 0);
	HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 15, 0);
}
