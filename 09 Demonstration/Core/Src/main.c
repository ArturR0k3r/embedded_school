/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"ssd1306.h"
#include<stdio.h>
#include"button.h"
#include "tetris.h"
#include "ssd1306.h"
#include "button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
button BTNUP, BTNLEFT, BTNRIGHT, BTNDOWN;
void printPointer(uint8_t ptr)
{
	ssd1306_SetCursor(0, ptr * 10);
	ssd1306_WriteString(">", Font_7x10, White);
}
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define MPU6050_ADDR 0xD0

#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75

int16_t Accel_X_RAW = 0;
int16_t Accel_Y_RAW = 0;
int16_t Accel_Z_RAW = 0;

int16_t Gyro_X_RAW = 0;
int16_t Gyro_Y_RAW = 0;
int16_t Gyro_Z_RAW = 0;

float Ax, Ay, Az, Gx, Gy, Gz;

void MPU6050_Init(void)
{
	uint8_t check;
	uint8_t Data;

	// check device ID WHO_AM_I

	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);

	if (check == 104) // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &Data, 1,
				1000);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		Data = 0x07;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1,
				1000);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ± 2g
		Data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1,
				1000);

		// Set Gyroscopic configuration in GYRO_CONFIG Register
		// XG_ST=0,YG_ST=0,ZG_ST=0, FS_SEL=0 -> ± 250 °/s
		Data = 0x00;
		HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &Data, 1,
				1000);
	}

}

void MPU6050_Read_Accel(void)
{
	uint8_t Rec_Data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 6,
			1000);

	Accel_X_RAW = (int16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Accel_Y_RAW = (int16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Accel_Z_RAW = (int16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	/*** convert the RAW values into acceleration in 'g'
	 we have to divide according to the Full scale value set in FS_SEL
	 I have configured FS_SEL = 0. So I am dividing by 16384.0
	 for more details check ACCEL_CONFIG Register              ****/

	Ax = Accel_X_RAW / 16384.0;
	Ay = Accel_Y_RAW / 16384.0;
	Az = Accel_Z_RAW / 16384.0;
}

void MPU6050_Read_Gyro(void)
{
	uint8_t Rec_Data[6];

	// Read 6 BYTES of data starting from GYRO_XOUT_H register

	HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, GYRO_XOUT_H_REG, 1, Rec_Data, 6,
			1000);

	Gyro_X_RAW = (int16_t) (Rec_Data[0] << 8 | Rec_Data[1]);
	Gyro_Y_RAW = (int16_t) (Rec_Data[2] << 8 | Rec_Data[3]);
	Gyro_Z_RAW = (int16_t) (Rec_Data[4] << 8 | Rec_Data[5]);

	/*** convert the RAW values into dps (°/s)
	 we have to divide according to the Full scale value set in FS_SEL
	 I have configured FS_SEL = 0. So I am dividing by 131.0
	 for more details check GYRO_CONFIG Register              ****/

	Gx = Gyro_X_RAW / 131.0;
	Gy = Gyro_Y_RAW / 131.0;
	Gz = Gyro_Z_RAW / 131.0;
}
void menuOverview()
{
	ssd1306_Fill(Black);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	HAL_Delay(250);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	HAL_Delay(250);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString(" BLINK == Hello World ", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString(" in embedded so hello", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("     from BSW Tech", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("    This is a brief ", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("   introduction of", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("    DinoDev Board", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString(" This Board contains", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("         4 sensors", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("BMP280 Pressure", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("   Temperature", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("connected to I2C1", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("MPU6050 IMU ", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("ACCELOROMETER", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("GYROSCOPE", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("connected to I2C1", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("DHT11 Humidity ", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("Temperature", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("connected to PB8", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("LDR to get ", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("Luminosity level", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("Also 4 user buttons", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("And 3 LEDS", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Board is driven by", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 10);
	ssd1306_WriteString("CortexM3 STM32 use", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 20);
	ssd1306_WriteString("It and only border", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 30);
	ssd1306_WriteString("is your imagination", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(0, 40);
	ssd1306_WriteString("", Font_6x8, White);
	HAL_Delay(500);
	ssd1306_UpdateScreen();
	HAL_Delay(1000);
	if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
	{
		return;
	}
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	uint16_t AD_RES = 0, Vamb, DC_Multiplier;
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_ADC1_Init();
	MX_TIM1_Init();
	/* USER CODE BEGIN 2 */

	ssd1306_Init();
	MPU6050_Init();
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString("Initialized", Font_7x10, White);
	ssd1306_UpdateScreen();
	// Calibrate The ADC On Power-Up For Better Accuracy
	HAL_ADCEx_Calibration_Start(&hadc1);

	// Read The Sensor Once To Get The Ambient Level
	// & Calculate The DutyCycle Multiplier
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1);
	Vamb = HAL_ADC_GetValue(&hadc1);
	DC_Multiplier = 65535 / (4096 - Vamb);
	uint16_t LDR_VAL = 0;
	char buf[16] =
	{ 0 };
//initializing buttons
	BTNUP.PIN = KEY2_Pin;
	BTNUP.PORT = KEY2_GPIO_Port;
	BTNUP.StateFlag = 0;
	BTNLEFT.PIN = KEY1_Pin;
	BTNLEFT.PORT = KEY1_GPIO_Port;
	BTNLEFT.StateFlag = 0;
	BTNRIGHT.PIN = KEY3_Pin;
	BTNRIGHT.PORT = KEY3_GPIO_Port;
	BTNRIGHT.StateFlag = 0;
	BTNDOWN.PIN = KEY4_Pin;
	BTNDOWN.PORT = KEY4_GPIO_Port;
	BTNDOWN.StateFlag = 0;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_Delay(1000);  // wait for 1 sec

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		static uint8_t pointer = 0;
		ssd1306_Fill(Black);
		ssd1306_SetCursor(7, 0);
		ssd1306_WriteString("DATA", Font_7x10, White);
		ssd1306_SetCursor(7, 10);
		ssd1306_WriteString("TETRIS", Font_7x10, White);
		ssd1306_SetCursor(7, 20);
		ssd1306_WriteString("IMU", Font_7x10, White);
		ssd1306_SetCursor(7, 30);
		ssd1306_WriteString("LDR", Font_7x10, White);
		printPointer(pointer);
		if (Button_Check(BTNDOWN))
		{
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
			TIM1->CCR1 = 65535;
			if (pointer > -1 && pointer < 3)
			{
				pointer++;
			}
		}
		else if (Button_Check(BTNUP))
		{
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			TIM1->CCR1 = 0;
			if (pointer > 0 && pointer < 4)
			{
				pointer--;
			}
		}
		else if (Button_Check(BTNRIGHT) && Button_Check(BTNLEFT))
		{
			return;
		}
		else if (Button_Check(BTNRIGHT))
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			TIM1->CCR1 = 0;
			switch (pointer)
			{
			case 0:
			{
				menuOverview();
				break;
			}
			case 1:
			{
				Tetris_GameLoop();
				break;
			}
			case 2:
			{

				// read the Accelerometer and Gyro values
				MPU6050_Read_Accel();
				MPU6050_Read_Gyro();

				// print the Acceleration and Gyro values on the LCD 20x4
				ssd1306_Fill(Black);
				ssd1306_SetCursor(0, 0);
				ssd1306_WriteString("Ax=", Font_7x10, White);
				sprintf(buf, "%.2f", Ax);
				ssd1306_WriteString(buf, Font_7x10, White);
				ssd1306_WriteString("g ", Font_7x10, White);

				ssd1306_SetCursor(0, 10);
				ssd1306_WriteString("Ay=", Font_7x10, White);
				sprintf(buf, "%.2f", Ay);
				ssd1306_WriteString(buf, Font_7x10, White);
				ssd1306_WriteString("g ", Font_7x10, White);

				ssd1306_SetCursor(0, 20);
				ssd1306_WriteString("Az=", Font_7x10, White);
				sprintf(buf, "%.2f", Az);
				ssd1306_WriteString(buf, Font_7x10, White);
				ssd1306_WriteString("g ", Font_7x10, White);

				ssd1306_SetCursor(0, 30);
				ssd1306_WriteString("Gx=", Font_7x10, White);
				sprintf(buf, "%.2f", Gx);
				ssd1306_WriteString(buf, Font_7x10, White);

				ssd1306_SetCursor(0, 40);
				ssd1306_WriteString("Gy=", Font_7x10, White);
				sprintf(buf, "%.2f", Gy);
				ssd1306_WriteString(buf, Font_7x10, White);

				ssd1306_SetCursor(0, 50);
				ssd1306_WriteString("Gz=", Font_7x10, White);
				sprintf(buf, "%.2f", Gz);
				ssd1306_WriteString(buf, Font_7x10, White);
				ssd1306_UpdateScreen();
				HAL_Delay(250);  // wait for a while
				break;
			}
			case 3:
			{
				// Start ADC Conversion
				HAL_ADC_Start(&hadc1);
				// Poll ADC1 Perihperal & TimeOut = 1mSec
				HAL_ADC_PollForConversion(&hadc1, 1);
				// Read The ADC Conversion Result & Map It To PWM DutyCycle
				AD_RES = HAL_ADC_GetValue(&hadc1);
				LDR_VAL = (AD_RES - Vamb) * DC_Multiplier;
				HAL_Delay(1);
				ssd1306_WriteString("LDR value = ", Font_7x10, White);
				sprintf(buf, "%d", LDR_VAL);
				ssd1306_WriteString(buf, Font_7x10, White);
				ssd1306_UpdateScreen();
				if (LDR_VAL > 20000)
				{
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
				}
				if (LDR_VAL > 30000)
				{
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
				}
				if (LDR_VAL > 40000)
				{
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
				}
			}
			}
		}
		ssd1306_UpdateScreen();

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit =
	{ 0 };

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
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig =
	{ 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
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

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig =
	{ 0 };
	TIM_OC_InitTypeDef sConfigOC =
	{ 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig =
	{ 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 0;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 65535;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : KEY3_Pin KEY1_Pin KEY4_Pin */
	GPIO_InitStruct.Pin = KEY3_Pin | KEY1_Pin | KEY4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
	GPIO_InitStruct.Pin = LED1_Pin | LED2_Pin | LED3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : KEY2_Pin */
	GPIO_InitStruct.Pin = KEY2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(KEY2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
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
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
