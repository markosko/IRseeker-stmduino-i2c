#include <Wire.h>
#include <elapsedMillis.h>
elapsedMillis milis;

//#include <RunningMedian.h>

#define I2C_ADDRESS 0x10 / 2


/* Copy of STM32 functions from STM32CubeIDE  */
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C2_Init(void);
/*   */

HardwareSerial Serial3(PC11, PC10); // Serial3 with pins PC11 and PC10 is 5V tolerant


int inputs[12];
int current_max = 0;
volatile unsigned int position = 4;
volatile unsigned int avgposition = 4;
volatile byte registerOffset;

volatile int sendBytes;

volatile bool printValues;



char printHex(int numberToPrint)
{
  if (numberToPrint >= 16){
    printHex(numberToPrint / 16);
  }
  return ("0123456789ABCDEF"[numberToPrint % 16]);
}


uint32_t masks[] = {
    0, // 0
    1, // 1
    2, // 2
    3, // 3
    4, // 4
    5, // 5
    6, // 6
    7, // 7
    9, // 8
    10,// 9
    11,//10
    12,//11
};
void reset_inputs(){
    for(int i=0;i<12;i++){
        inputs[i] = 0;
    }
}
void read_inputs(){

    milis = 0;


    while(milis <2){
        uint32_t value =  (GPIOB->IDR & 0b0001111011111111);
    
        for(int i=0;i<12;i++){
      
            inputs[i] += 1 - ((value & (1 << masks[i])) >> masks[i]);
        }
    }

}

unsigned int find_max(){


  unsigned int current_max = 0;
    unsigned int c = 0;
    
    for(int i=0;i<12;i++){

        if(inputs[i] > current_max){
            c = i+1;
            current_max = inputs[i];

        }
    }

    return c;
}


void receiveEvent(int howMany)
{
  while (1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    //Serial3.print(c);         // print the character
  }
  registerOffset = Wire.read();    // receive byte as an integer
  //Serial3.println(x);         // print the integer
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  switch(registerOffset){
      case 0:
        Wire.write(0); // respond with message of 6 bytes
        break;

      case 0x42:
      case 0x49:
          Wire.write((position & 0xFF)); // respond with message of 6 bytes
          Wire.write(0); // respond with message of 6 bytes
          Wire.write(0); // respond with message of 6 bytes
          Wire.write(0); // respond with message of 6 bytes
          Wire.write(0); // respond with message of 6 bytes
          Wire.write(0); // respond with message of 6 bytes
        break;
      case 0x08:
        Wire.write("HiTechncNewIRDir");
        break;
  }
  
  
  // as expected by master
}


void setup()
{
  
  
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  //MX_TIM1_Init();
  MX_I2C2_Init();
  
  
  Wire.begin(8);                // join i2c bus with address #8
  
  reset_inputs();
  read_inputs();
  position = find_max();
  avgposition = position;
  Serial3.println(position);
  delay(100);
  
  
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event
}

void loop()
{
    reset_inputs();
    read_inputs();
    position = find_max();
    avgposition = position;
    //Serial3.println(position);
    //delay(100);
}


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 PB12 PB3 PB4
                           PB5 PB6 PB7 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}



I2C_HandleTypeDef hi2c2;


UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;



/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{


  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10B0DCFB;
  hi2c2.Init.OwnAddress1 = I2C_ADDRESS;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }


}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{


  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

}


