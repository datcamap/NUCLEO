/*
 * main_app.c
 *
 *  Created on: May 12, 2025
 *      Author: datad
 */

#include "main_app.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define PI 3.14

void Error_handler(void);
void SystemClock_Config(uint8_t clock_freq);
void Uart2_Init(void);
void GPIO_Config(void);
void Timer6_Config(void);
void Timer2_Config(void);
void Timer1_Config(void);
void CAN1_Config(void);
void CAN_Transmit(void);
void CAN_Receive(void);

typedef struct {
	uint8_t uart_recv_cplt : 1;
	uint8_t tim_capture_cplt : 1;
}MyFlaags;

GPIO_InitTypeDef gpioled;
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htiem6;
TIM_HandleTypeDef htiem2;
TIM_HandleTypeDef htiem1;
CAN_HandleTypeDef hcan1;
char mess[100] = "HELLOworld\n\r";
char prefix[100] = "123465790";
uint8_t recv_data;
MyFlaags flgss = {.uart_recv_cplt=FALSE, .tim_capture_cplt=FALSE};
uint8_t countUART = 0;
uint8_t countTIM = 0;
uint32_t timstamp[2] = {0, 0};
uint32_t tim2Tick[2] = {0, 0};
uint32_t pulse1Hz = 1250000;
uint32_t servo_angle = 0;
uint64_t capture_diff;
double duty_cycl[3] = {0.5, 0.5, 0.5};
const double omega = 0.05*PI;
const double color_phase[3] = {0, PI*2/3, PI*4/3};
double bright_scala = 0.2;
double timelapsed = 0;
int8_t addorsub[3] = {1, 1, 1};
uint32_t ccrvalue;

int main(void)
{
	HAL_Init();
	SystemClock_Config(SYS_CLOCK_FREQ_50_MHZ);
	Uart2_Init();
	GPIO_Config();
	CAN1_Config();
	if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING | \
			CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_BUSOFF | CAN_IT_ERROR) != HAL_OK)
	{
		Error_handler();
	}
	if (HAL_CAN_Start(&hcan1) != HAL_OK)
	{
		Error_handler();
	}
	CAN_Transmit();
	Timer6_Config();
	Timer2_Config();
	Timer1_Config();

	HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);

	// Start timer
	HAL_TIM_Base_Start_IT(&htiem6);
	HAL_TIM_Base_Start_IT(&htiem2);
	// Start timer 2 PWM mode on Channel 2 3 4
	HAL_TIM_PWM_Start(&htiem2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htiem2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htiem2, TIM_CHANNEL_4);
	// Start timer 1 PWM mode on Channel 1
	HAL_TIM_PWM_Start(&htiem1, TIM_CHANNEL_1);

	while (1)
	{
#if 0
		while (!flgss.uart_recv_cplt)
		{
			HAL_UART_Receive_IT(&huart2, &recv_data, 1);
		}
		sprintf(prefix, "You typed: ");
		HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);
		sprintf(prefix, "\n \r");
		HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
		flgss.uart_recv_cplt = FALSE;
#endif
		if (flgss.tim_capture_cplt)
		{
			double input_freq;
			double timeresolution;
			if (timstamp[0] < timstamp[1])
			{
				capture_diff = timstamp[1] - timstamp[0];
			}
			else
			{
				capture_diff = htiem2.Init.Period - timstamp[0] + timstamp[1] + 1;
			}

			timeresolution = 1 / ((double)HAL_RCC_GetPCLK1Freq() * 2 / (double)(htiem2.Init.Prescaler + 1));
			input_freq = 1 / (timeresolution * (double)(capture_diff + htiem2.Init.Period*tim2Tick[0]));
			sprintf(prefix, "Channel 3 clock: %fHz \n \r", input_freq);
			HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
			tim2Tick[0] = 0;
			countTIM = 0;
			flgss.tim_capture_cplt = FALSE;
		}
		// Go to bed
		__WFI();
	}
	for ( ; ; );

	return 0;
}

void SystemClock_Config(uint8_t clock_freq)
{
	RCC_OscInitTypeDef oscinit;
	RCC_ClkInitTypeDef clkinit;
	uint32_t Flatency;

	oscinit.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	oscinit.HSEState = RCC_HSE_BYPASS;
	oscinit.PLL.PLLState = RCC_PLL_ON;
	oscinit.LSEState = RCC_LSE_ON;
	oscinit.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	switch (clock_freq)
	{
	case SYS_CLOCK_FREQ_50_MHZ:
	{
		oscinit.PLL.PLLM = 4;
		oscinit.PLL.PLLN = 100;
		oscinit.PLL.PLLP = 4;
		oscinit.PLL.PLLQ = 2;
		oscinit.PLL.PLLR = 2;

		clkinit.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
					RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		clkinit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		clkinit.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clkinit.APB1CLKDivider = RCC_HCLK_DIV2;
		clkinit.APB2CLKDivider = RCC_HCLK_DIV1;

		Flatency = FLASH_ACR_LATENCY_1WS;
		break;
	}
	case SYS_CLOCK_FREQ_80_MHZ:
	{
		oscinit.PLL.PLLM = 8;
		oscinit.PLL.PLLN = 160;
		oscinit.PLL.PLLP = 2;
		oscinit.PLL.PLLQ = 2;
		oscinit.PLL.PLLR = 2;

		clkinit.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
					RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		clkinit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		clkinit.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clkinit.APB1CLKDivider = RCC_HCLK_DIV2;
		clkinit.APB2CLKDivider = RCC_HCLK_DIV1;

		Flatency = FLASH_ACR_LATENCY_2WS;
		break;
	}
	case SYS_CLOCK_FREQ_120_MHZ:
	{
		oscinit.PLL.PLLM = 8;
		oscinit.PLL.PLLN = 120;
		oscinit.PLL.PLLP = 1;
		oscinit.PLL.PLLQ = 2;
		oscinit.PLL.PLLR = 2;

		clkinit.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
					RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		clkinit.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		clkinit.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clkinit.APB1CLKDivider = RCC_HCLK_DIV2;
		clkinit.APB2CLKDivider = RCC_HCLK_DIV2;

		Flatency = FLASH_ACR_LATENCY_3WS;
		break;
	}
	default:
	{
		clkinit.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | \
					RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
		clkinit.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
		clkinit.AHBCLKDivider = RCC_SYSCLK_DIV1;
		clkinit.APB1CLKDivider = RCC_HCLK_DIV2;
		clkinit.APB2CLKDivider = RCC_HCLK_DIV2;

		Flatency = FLASH_ACR_LATENCY_1WS;
		break;
	}
	}

	if (HAL_RCC_OscConfig(&oscinit) != HAL_OK)
	{
		Error_handler();
	}

	if (HAL_RCC_ClockConfig(&clkinit, Flatency) != HAL_OK)
	{
		Error_handler();
	}
}

void Uart2_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	if (HAL_UART_Init( &huart2) != HAL_OK)
	{
		Error_handler();
	}
}

void GPIO_Config(void)
{
	GPIO_InitTypeDef gpiobutt;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	gpioled.Pin = GPIO_PIN_5;
	gpioled.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOA, &gpioled);

	gpiobutt.Pin = GPIO_PIN_13;
	gpiobutt.Mode = GPIO_MODE_IT_FALLING;
	gpiobutt.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &gpiobutt);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
}

void CAN1_Config(void)
{
	hcan1.Instance = CAN1;
	hcan1.Instance->MSR |= 1;
	hcan1.Init.Mode = CAN_MODE_LOOPBACK;
	hcan1.Init.AutoBusOff = DISABLE;
	hcan1.Init.AutoRetransmission = ENABLE;
	hcan1.Init.AutoWakeUp = DISABLE;
	hcan1.Init.ReceiveFifoLocked = DISABLE;
	hcan1.Init.TimeTriggeredMode = DISABLE;
	hcan1.Init.TransmitFifoPriority = DISABLE;

	// CAN bit timing
	hcan1.Init.Prescaler = 5;
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan1.Init.TimeSeg1 = CAN_BS1_8TQ;
	hcan1.Init.TimeSeg1 = CAN_BS2_1TQ;

	if (HAL_CAN_Init(&hcan1) != HAL_OK)
	{
		Error_handler();
	}

	// Filter configuration
	CAN_FilterTypeDef can1_filter;
	can1_filter.FilterActivation = ENABLE;
	can1_filter.FilterBank = 0;
	can1_filter.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter.FilterIdHigh = 0x0000;
	can1_filter.FilterIdLow = 0x0000;
	can1_filter.FilterMaskIdHigh = 0x0000;
	can1_filter.FilterMaskIdLow = 0x0000;
	can1_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter.FilterScale = CAN_FILTERSCALE_32BIT;

	if (HAL_CAN_ConfigFilter(&hcan1, &can1_filter) != HAL_OK)
	{
		Error_handler();
	}
}

void CAN_Transmit(void)
{
	CAN_TxHeaderTypeDef txHeader;
	uint32_t txMailbox;
	uint8_t message[5] = {'L','M','F','A','O'};

	txHeader.DLC = 5;
	txHeader.IDE = CAN_ID_STD;
	txHeader.StdId = 0x69;
	txHeader.RTR = CAN_RTR_DATA;

	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, message, &txMailbox) != HAL_OK)
	{
		Error_handler();
	}
}

void Timer6_Config(void)
{
	// Calculation to generate interrupt every 1000ms
	htiem6.Instance = TIM6;
	htiem6.Init.Prescaler = 799;
	htiem6.Init.Period = 62500-1;
	if (HAL_TIM_Base_Init(&htiem6) != HAL_OK)
	{
		Error_handler();
	}
}

void Timer2_Config(void)
{
	TIM_IC_InitTypeDef htime2_IC;
	TIM_OC_InitTypeDef htime2_OC;
	TIM_OC_InitTypeDef htime2_PWM[3];
	memset(&htime2_IC, 0, sizeof(htime2_IC));
	memset(&htime2_OC, 0, sizeof(htime2_OC));
	memset(&htime2_PWM, 0, sizeof(htime2_PWM));

	// Basic timer initialization
	htiem2.Instance = TIM2;
	htiem2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htiem2.Init.Prescaler = 10-1;		// Timer clock will be 5Mhz
	htiem2.Init.Period = 10000-1;		// Pulse frequency is 50Hz
	if (HAL_TIM_Base_Init(&htiem2) != HAL_OK)
	{
		Error_handler();
	}
	/*// Input capture mode initialize
	if (HAL_TIM_IC_Init(&htiem2) != HAL_OK)
	{
		Error_handler();
	}
	htime2_IC.ICFilter = 0x0;
	htime2_IC.ICPolarity = TIM_ICPOLARITY_RISING;
	htime2_IC.ICPrescaler = TIM_ICPSC_DIV1;
	htime2_IC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	if (HAL_TIM_IC_ConfigChannel(&htiem2, &htime2_IC, TIM_CHANNEL_2) != HAL_OK)
	{
		Error_handler();
	}

	// Output capture mode initialize
	if (HAL_TIM_OC_Init(&htiem2) != HAL_OK)
	{
		Error_handler();
	}
	htime2_OC.OCMode = TIM_OCMODE_TOGGLE;
	htime2_OC.OCPolarity = TIM_OCPOLARITY_HIGH;
	htime2_OC.Pulse = pulse1Hz;
	if (HAL_TIM_OC_ConfigChannel(&htiem2, &htime2_OC, TIM_CHANNEL_3) != HAL_OK)
	{
		Error_handler();
	}*/

	// PWM mode initialize
	if (HAL_TIM_PWM_Init(&htiem2) != HAL_OK)
	{
		Error_handler();
	}
	for (int i=0; i<3; i++)
	{
		uint32_t chnnl[3] = {TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4};
		htime2_PWM[i].OCMode = TIM_OCMODE_PWM1;
		htime2_PWM[i].OCPolarity = TIM_OCPOLARITY_HIGH;
		htime2_PWM[i].Pulse = (htiem2.Init.Period * duty_cycl[i]) /100;
		if (HAL_TIM_PWM_ConfigChannel(&htiem2, htime2_PWM + i, chnnl[i]) != HAL_OK)
		{
			Error_handler();
		}
	}
}

void Timer1_Config(void)
{
	TIM_OC_InitTypeDef htime1_PWM;
	memset(&htime1_PWM, 0, sizeof(htime1_PWM));

	// Basic timer initialization
	htiem1.Instance = TIM1;
	htiem1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htiem1.Init.Prescaler = 99;			// Timer clock will be 500Khz
	htiem1.Init.Period = 10000 - 1;		// Pulse frequency is 50Hz
	if (HAL_TIM_PWM_Init(&htiem1) != HAL_OK)
	{
		Error_handler();
	}

	htime1_PWM.OCMode = TIM_OCMODE_PWM1;
	htime1_PWM.OCPolarity = TIM_OCPOLARITY_HIGH;
	uint32_t pulse1ms = (double)htiem1.Init.Period*0.1;
	uint32_t pulse2ms = (double)htiem1.Init.Period*0.2;
	htime1_PWM.Pulse = (double)servo_angle/180 * (pulse2ms - pulse1ms) + pulse1ms;
	if (HAL_TIM_PWM_ConfigChannel(&htiem1, &htime1_PWM, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_handler();
	}
}

void Error_handler(void)
{
	sprintf(prefix, "Error! \n \r");
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	sprintf(mess, "SYSCLK	: %ldHz \n\r", HAL_RCC_GetSysClockFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);
	sprintf(mess, "AHBCLK	: %ldHz \n\r", HAL_RCC_GetHCLKFreq());
	HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);
	sprintf(mess, "APB1CLK	: %ldHz \n\r", HAL_RCC_GetPCLK1Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);
	sprintf(mess, "APB2CLK	: %ldHz \n\r", HAL_RCC_GetPCLK2Freq());
	HAL_UART_Transmit(&huart2, (uint8_t*)mess, (uint16_t)strlen(mess), HAL_MAX_DELAY);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		HAL_GPIO_TogglePin(GPIOA, gpioled.Pin);
	}
	else if (htim->Instance == TIM2)
	{
		if (tim2Tick[1] == 1)
		{
			double timer2period = 1 / ((double)HAL_RCC_GetPCLK1Freq()/(double)(htiem2.Init.Prescaler + 1));
			timelapsed += (double)(__HAL_TIM_GET_COUNTER(&htiem2) + htiem2.Init.Period*tim2Tick[1])*timer2period;
			uint32_t rgb_PWM[3];
			for (uint8_t i=0; i<3; i++)
			{
				duty_cycl[i] = fabs(1/sin(PI/3)*sin(omega*timelapsed + color_phase[i]));
				duty_cycl[i] = (duty_cycl[i] <= 0)?0:duty_cycl[i];	// Clamp
				duty_cycl[i] = (duty_cycl[i] >= 1)?1:duty_cycl[i];	// Clamp
				rgb_PWM[i] = bright_scala*(double)htiem2.Init.Period*duty_cycl[i];
			}
			// Set new duty cycle
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_2,rgb_PWM[0]);
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_3,rgb_PWM[1]);
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_4,rgb_PWM[2]);

			tim2Tick[1] = 0;
		}

		tim2Tick[0]++;
		tim2Tick[1]++;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (!flgss.tim_capture_cplt)
	{
		timstamp[countTIM] = __HAL_TIM_GET_COMPARE(&htiem1, TIM_CHANNEL_1);
		if (countTIM == 1)
		{
			flgss.tim_capture_cplt = TRUE;
		}
		else if (countTIM == 0)
		{
			countTIM++;
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2)
	{
		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
		{
			ccrvalue = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_1,(ccrvalue + pulse1Hz));
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(recv_data == '\r')
	{
		flgss.uart_recv_cplt = TRUE;
		mess[countUART] = 0;
		countUART = 0;
	}
	else
	{
		mess[countUART] = recv_data;
		countUART++;
	}
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	sprintf(prefix, "Transmit message from Mailbox 0 \n \r");
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	sprintf(prefix, "Transmit message from Mailbox 1 \n \r");
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	sprintf(prefix, "Transmit message from Mailbox 2 \n \r");
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef rxHeader;
	char msg[50];

	if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, (uint8_t*)msg) != HAL_OK)
	{
		Error_handler();
	}

	sprintf(prefix, "Received message: %s \n \r", msg);
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef rxHeader;
	char msg[50];

	if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, (uint8_t*)msg) != HAL_OK)
	{
		Error_handler();
	}

	sprintf(prefix, "Received message: %s \n \r", msg);
	HAL_UART_Transmit(&huart2, (uint8_t*)prefix, (uint16_t)strlen(prefix), HAL_MAX_DELAY);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	Error_handler();
}

int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar((*ptr++));
    }
    return len;
}

