#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "fonts.h"
#include "ssd1306.h"

uint8_t rx_data[8];
char buffer[4];
int pwm = 0;

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
void handleReceivedData(uint8_t* rx_data);

class ServoMotor
{
	public:
		ServoMotor(TIM_HandleTypeDef &htim, uint32_t channel)
										: htim(htim), channel(channel) {}

		void pwmWrite(int angle)
		{
			int pwmValue = map(angle, 0, 180, 250, 1250);
			__HAL_TIM_SET_COMPARE(&htim, channel, pwmValue);
			HAL_Delay(3);
		}

	private:
		TIM_HandleTypeDef& htim;
		uint32_t channel;

		int map(int value, int fromLow, int fromHigh, int toLow, int toHigh)
		{
			return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
		}
};

void handleReceivedData(uint8_t* rx_data)
{
	ServoMotor Servo1(htim2, TIM_CHANNEL_1); //PA0  Range: 0 to 180
	ServoMotor Servo2(htim2, TIM_CHANNEL_2); //PA1  Range: 20 to 170
	ServoMotor Servo3(htim2, TIM_CHANNEL_4); //PA3  Range: 30 to 150
	ServoMotor Servo4(htim3, TIM_CHANNEL_1); //PA11  Range: Closed: 70-75 to Open: 105-110

	uint16_t angle = 0;
	char identifier = rx_data[0];  // Find the identifier position
	size_t terminatorPosition = 0;  // Find the terminator position
	uint8_t stop = 0;
	while (rx_data[terminatorPosition] != 'A' && rx_data[terminatorPosition] != 'B'
	    								&& rx_data[terminatorPosition] != 'C')
	{
	   terminatorPosition++;
	   if (terminatorPosition >= 8)
	   {

		   return;  // Invalid data format, handle error or ignore data
	   }
	}
	switch(terminatorPosition)
	{
		case 4: stop = 3; break;
		case 3: stop = 2; break;
		case 2: stop = 1; break;
	}
	char terminator = rx_data[terminatorPosition];

	char anglearray[4];

	memcpy(anglearray, &rx_data[1], stop);
	memcpy(buffer, &rx_data[1], stop);

	angle = atoi(anglearray);  // Convert ASCII digits to integer
	//pwm = map(angle, 0, 180, 250, 1250);
	//memcpy(buffer, anglearray, sizeof(anglearray));
	//memcpy(buffer, &rx_data[1], stop);

	SSD1306_GotoXY(0,0);
	SSD1306_Puts(buffer, &Font_11x18, SSD1306_COLOR_WHITE);
	SSD1306_UpdateScreen();

	switch(identifier)
		{
			case 'M':
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
				SSD1306_GotoXY(0,0);
				SSD1306_Puts("Motor: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 0);
				SSD1306_Puts("1", &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_GotoXY (0, 25);
				SSD1306_Puts("Angle: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 25);
				SSD1306_Puts(&buffer, &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_UpdateScreen();
				break;
			case 'N':
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
				SSD1306_GotoXY(0,0);
				SSD1306_Puts("Motor: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 0);
				SSD1306_Puts("2", &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_GotoXY (0, 25);
				SSD1306_Puts("Angle: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 25);
				SSD1306_Puts(&buffer, &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_UpdateScreen();
				break;
			case 'O':
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm);
				SSD1306_GotoXY(0,0);
				SSD1306_Puts("Motor: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 0);
				SSD1306_Puts("3", &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_GotoXY (0, 25);
				SSD1306_Puts("Angle: ", &Font_11x18, SSD1306_COLOR_WHITE);
				SSD1306_GotoXY (70, 25);
				SSD1306_Puts(&buffer, &Font_11x18, SSD1306_COLOR_WHITE);

				SSD1306_UpdateScreen();
				break;
			case 'A':
				memcpy(gripnum, &rx_data[1], 1);
				griptest = atoi(gripnum);

				if(griptest == 1)
				{
					grip = map(110, 0, 180, 250, 1250);
					__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, grip);
					SSD1306_GotoXY(0,0);
					SSD1306_Puts("Gripper", &Font_11x18, SSD1306_COLOR_WHITE);

					SSD1306_GotoXY (0, 25);
					SSD1306_Puts("Open", &Font_11x18, SSD1306_COLOR_WHITE);

					SSD1306_UpdateScreen();
				}
				else
				{
					grip = map(70, 0, 180, 250, 1250);
					__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, grip);

					SSD1306_GotoXY(0,0);
					SSD1306_Puts("Gripper", &Font_11x18, SSD1306_COLOR_WHITE);

					SSD1306_GotoXY (0, 25);
					SSD1306_Puts("Closed", &Font_11x18, SSD1306_COLOR_WHITE);

					SSD1306_UpdateScreen();
				}
				break;


	//    memcpy(buffer, &rx_data[0], 8);
	//	SSD1306_GotoXY(0,0);
	//	SSD1306_Puts(&buffer, &Font_11x18, 1);
	//	SSD1306_UpdateScreen();


			}

}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //T2C1
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); //T1C2
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); //T2C3
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //T1C4

  HAL_UART_Receive_IT(&huart1,rx_data,8);

//  SSD1306_Init();

  ServoMotor Servo1(htim2, TIM_CHANNEL_1); //PA0  Range: 0 to 180
  ServoMotor Servo2(htim2, TIM_CHANNEL_2); //PA1  Range: 20 to 170
  ServoMotor Servo3(htim2, TIM_CHANNEL_4); //PA3  Range: 30 to 150
  ServoMotor Servo4(htim3, TIM_CHANNEL_1); //PA6  Range: Closed: 70-75 to Open: 105-110

  while (1)
  {
	  handleReceivedData(rx_data);


//	  uint16_t angle = 0;
//	  	char identifier = rx_data[0];  // Find the identifier position
//
//	  	if(identifier == 'M' || identifier == 'N' || identifier == 'O')
//	  				{
//	  			  for(int i=0; i<=180;i++)
//	  			  {
//	  				  Servo4.pwmWrite(i);
//	  				  HAL_Delay(15);
//	  			  }
//	  			  for(int i=180; i>=0;i--)
//	  			  {
//	  				  Servo4.pwmWrite(i);
//	  				  HAL_Delay(15);
//	  			  }
//	  				}
//	  				else
//	  				{
//
//
//	  				}
//		memcpy(buffer, &rx_data, sizeof(rx_data));
//		SSD1306_GotoXY(0,0);
//		SSD1306_Puts(buffer, &Font_11x18, SSD1306_COLOR_WHITE);
//		SSD1306_UpdateScreen();

  }


}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART1)
  {
	SSD1306_Clear();
	memset(rx_data, 0, sizeof(rx_data));
    HAL_UART_Receive_IT(&huart1,rx_data,8);
//    handleReceivedData(rx_data);
  }
}
/* USER CODE END 4 */


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif /* USE_FULL_ASSERT */
