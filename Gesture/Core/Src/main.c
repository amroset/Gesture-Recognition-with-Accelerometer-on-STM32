/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "app.h"
#include "CycleCounter.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_i2c.h"
#include "lsm6dsl_reg.h"
#include "math.h"
#include "float.h"
#include "data.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
I2C_HandleTypeDef hi2c1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LSM6DSL_I2C_ADDR 0x6A
#define FILTER_SIZE 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Buffer for accelerometer data
int16_t accel_data[3];
qdata quantized_data[32];
uint8_t sequence_for_pred[96];


// Circular buffer to store past accelerometer readings
typedef struct {
    float x[FILTER_SIZE];
    float y[FILTER_SIZE];
    float z[FILTER_SIZE];
    uint8_t index;
} accel_buffer_t;

// Structure to hold filtered accelerometer data
typedef struct {
    float x;
    float y;
    float z;
} filtered_accel_t;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
__attribute__((weak)) void _write(char* ptr)
{
HAL_UART_Transmit(&huart1, (uint8_t*)ptr, strlen(ptr), HAL_MAX_DELAY);

}

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
	 HAL_I2C_Mem_Write((I2C_HandleTypeDef *)handle, LSM6DSL_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, HAL_MAX_DELAY);
	 return 0;
}
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
	 HAL_I2C_Mem_Read((I2C_HandleTypeDef *)handle, LSM6DSL_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, bufp, len, HAL_MAX_DELAY);
	    return 0;
}
static void platform_delay(uint32_t ms) {
	HAL_Delay(ms);
}

// Function to calculate the mean
float calculate_mean(float data[], int n) {
    float sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum / n;
}

// Function to calculate the standard deviation
float calculate_std(float data[], int n, float mean) {

    if (isnan(mean)){
        fprintf(stderr, "Error: Mean is NAN.\n");
        return NAN;
    }

    float sum_squared_diff = 0;
    for (int i = 0; i < n; i++) {
        sum_squared_diff += pow(data[i] - mean, 2);
    }
    return sqrt(sum_squared_diff / n); // Population standard deviation
    
}

// Function to quantize a single double value to int8
uint8_t quantize_float(float value, float scale, int zero_point) {
    float quantized_float = round((value - (-zero_point * scale)) / scale);
    if (quantized_float > 255) return 255;
    if (quantized_float < 0) return 0;
    return (uint8_t)quantized_float;
}

// Function to calculate the moving average of an array
float moving_average(float* data, uint8_t size) {
    float sum = 0.0;
    for (uint8_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}

// Initialize the circular buffer
void init_buffer(accel_buffer_t* buffer) {
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        buffer->x[i] = 0.0;
        buffer->y[i] = 0.0;
        buffer->z[i] = 0.0;
    }
    buffer->index = 0;
}

// Update the circular buffer with new accelerometer readings
void update_buffer(accel_buffer_t* buffer, float x, float y, float z) {
    buffer->x[buffer->index] = x;
    buffer->y[buffer->index] = y;
    buffer->z[buffer->index] = z;
    buffer->index = (buffer->index + 1) % FILTER_SIZE;
}


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
stmdev_ctx_t dev_ctx;
extern int latency;
extern int try_var;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  // Initialize the LSM6DSL driver
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = &hi2c2;

  int result_app = 0;
    //printf("Number of cycles required is: %d\n", latency);

  // Initialize the sensor
  uint8_t whoamI = 0;
  lsm6dsl_device_id_get(&dev_ctx, &whoamI);
  if (whoamI != LSM6DSL_ID) {
	  _write("ERROR, accelerometer not found\r\n");

    while (1);
  }

  //_write("Accelerometer initialized successfully\r\n");

  // Enable accelerometer (104 Hz, ±8g)
  lsm6dsl_xl_data_rate_set(&dev_ctx, LSM6DSL_XL_ODR_12Hz5);
  lsm6dsl_xl_full_scale_set(&dev_ctx, LSM6DSL_8g);

    //printf("Inference starting...\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  float sequence[96];
  float magnitude_buffer[9];
  int i = 0, k = 3;
  char msg[100];
  uint8_t movement = 0;

  accel_buffer_t accel_buffer;
  filtered_accel_t filtered_accel;

  init_buffer(&accel_buffer);


  while (1)
  {
	 // Read accelerometer data
	 lsm6dsl_acceleration_raw_get(&dev_ctx, accel_data);

	 // Convert raw values to g
	 float x = accel_data[0] * 0.244f / 1000 * 9.81f;
	 float y = accel_data[1] * 0.244f / 1000 * 9.81f;
	 float z = accel_data[2] * 0.244f / 1000 * 9.81f;

   update_buffer(&accel_buffer, x, y, z);
   
   filtered_accel.x = moving_average(accel_buffer.x, FILTER_SIZE);
   filtered_accel.y = moving_average(accel_buffer.y, FILTER_SIZE);
   filtered_accel.z = moving_average(accel_buffer.z, FILTER_SIZE);
     

	 magnitude_buffer[i] = filtered_accel.x;
	 magnitude_buffer[i+1] = filtered_accel.y;
	 magnitude_buffer[i+2] = filtered_accel.z;

   float magnitude_x = 0, magnitude_y = 0, magnitude_z = 0, magnitude = 0;

   if(!movement) {
    i+=3;

    if(i==9) {
     i = 0;
     magnitude_x = 0;
     magnitude_y = 0;
     magnitude_z = 0;
     magnitude = 0;

		 for(int j = 0; j < 3; j++) {
			
				 magnitude_x += magnitude_buffer[j*3];
			  
         magnitude_y += magnitude_buffer[j*3 + 1];

         magnitude_z += magnitude_buffer[j*3 + 2];
       
		 }

     magnitude_x /= 3;
     magnitude_y /= 3;
     magnitude_z /= 3;


     magnitude = sqrt(magnitude_x*magnitude_x + magnitude_y*magnitude_y + magnitude_z*magnitude_z); 
	    //we start collecting the sequence if we have not been still for too long
      if(magnitude > 1.1 * 9.81f) {
          sequence[0] = magnitude_buffer[6];
          sequence[1] = magnitude_buffer[7];
          sequence[2] = magnitude_buffer[8];
        //_write("Movement recorded\r\n");
          movement = 1;
      }


	  } 
  }

   else {
     sequence[k] = filtered_accel.x;
		 sequence[k+1] = filtered_accel.y;
		 sequence[k+2] = filtered_accel.z;
		 k+=3;
   }

	 if(k == 96) { //sequence ready
     //_write("sequence is ready\r\n");
		 k = 0;
     movement = 0;

		 //take mean and std
     float sequence_x[32];
     float sequence_y[32];
     float sequence_z[32];

     for(int j = 0; j < 32; j++) {
      sequence_x[j] = sequence[j*3];
      sequence_y[j] = sequence[j*3 + 1];
      sequence_z[j] = sequence[j*3 + 2];
     }

     //standardization
     
     float mean_x = calculate_mean(sequence_x, 32);
     float mean_y = calculate_mean(sequence_y, 32);
     float mean_z = calculate_mean(sequence_z, 32);

     float std_x = calculate_std(sequence_x, 32, mean_x);
     float std_y = calculate_std(sequence_y, 32, mean_y);
     float std_z = calculate_std(sequence_z, 32, mean_z);

    //  _write("new sequence: \r\n");

    //  for(int j = 0; j < 32; j++) {
    //     snprintf(msg, sizeof(msg), "X: %.3f g, Y: %.3f g, Z: %.3f g\r\n", sequence_x[j], sequence_y[j], sequence_z[j]);
    //     _write(msg);
    //  } 

     for(int j = 0; j < 32; j++) {
        sequence_x[j] = (sequence_x[j] - mean_x) / std_x; 
        sequence_y[j] = (sequence_y[j] - mean_y) / std_y;
        sequence_z[j] = (sequence_z[j] - mean_z) / std_z;   
    }

    // HAL_UART_Transmit(&huart1, "0,", strlen("0,"), HAL_MAX_DELAY);

    // char buffer[100];
    // HAL_UART_Transmit(&huart1, "\"[", strlen("\"["), HAL_MAX_DELAY);
    // for(int j = 0; j < 32; j++) {
    //   if(j != 31)
    //     snprintf(buffer, sizeof(buffer), "%.6f, ", sequence_x[j]);
    //   else
    //     snprintf(buffer, sizeof(buffer), "%.6f", sequence_x[j]);
    //   HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

    // }
    // HAL_UART_Transmit(&huart1, "]\",", strlen("]\","), HAL_MAX_DELAY);

    // HAL_UART_Transmit(&huart1, "\"[", strlen("\"["), HAL_MAX_DELAY);
    // for(int j = 0; j < 32; j++) {
    //   if(j != 31)
    //     snprintf(buffer, sizeof(buffer), "%.6f, ", sequence_y[j]);
    //   else
    //     snprintf(buffer, sizeof(buffer), "%.6f", sequence_y[j]);
    //   HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
    // }
    // HAL_UART_Transmit(&huart1, "]\",", strlen("]\","), HAL_MAX_DELAY);

    // HAL_UART_Transmit(&huart1, "\"[", strlen("\"["), HAL_MAX_DELAY);
    // for(int j = 0; j < 32; j++) {
    //   if(j != 31)
    //     snprintf(buffer, sizeof(buffer), "%.6f, ", sequence_z[j]);
    //   else
    //     snprintf(buffer, sizeof(buffer), "%.6f", sequence_z[j]);
    //   HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
    // }
    // HAL_UART_Transmit(&huart1, "]\"\r\n,", strlen("]\"\r\n"), HAL_MAX_DELAY);


    //quantization

    float scale = 0.021;
    uint8_t zero_point = 131;


    for (int j = 0; j < 32; j++) {
        quantized_data[j].x = quantize_float(sequence_x[j], scale, zero_point);
        quantized_data[j].y = quantize_float(sequence_y[j], scale, zero_point);
        quantized_data[j].z = quantize_float(sequence_z[j], scale, zero_point);

        //  snprintf(msg, sizeof(msg), "X: %d g, Y: %d g, Z: %d g\r\n", quantized_x[j], quantized_y[j], quantized_z[j]);
	      // _write(msg);
    }
    // _write("new sequence: \r\n");

    //  for(int j = 0; j < 32; j++) {
    //     snprintf(msg, sizeof(msg), "X: %.3f -> %d, Y: %.3f -> %d, Z: %.3f ->%d\r\n", sequence_x[j], quantized_data[j].x, sequence_y[j], quantized_data[j].y, sequence_z[j], quantized_data[j].z);
    //     _write(msg);
    //  } 
  
  //now we need to combine everything into a single array of 96 elements.
    for(int j = 0; j < 32; j++) {
      sequence_for_pred[j*3] = quantized_data[j].x;
      sequence_for_pred[j*3 + 1] = quantized_data[j].y;
      sequence_for_pred[j*3 + 2] = quantized_data[j].z;
    }

   //Print values via USART
	//  snprintf(msg, sizeof(msg), "X: %.3f g, Y: %.3f g, Z: %.3f g\r\n", quantized_x[j], quantized_y[j], quantized_z[j]);
	//  _write(msg);
  
     result_app = application(sequence_for_pred);

     HAL_Delay(1000);

  }
    //result_app = application(sequence_for_pred);

    HAL_Delay(15);

	}


	 // Use the values (e.g., print or process further)
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  printf("Number of cycles required is: %d\n", latency);
//	  printf("try_var = %d\n", try_var);
}
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
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
