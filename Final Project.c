/*Final Project - 1.c:Temperature and Humidity Display 
*
*/

/*******************************************************************************
  * File Name          : Final project - 1.c
  * Description        : Temperature and Humidity Display 
  * 
  * Author             : SAI TEJA ANAPARTHY
  * Date               : 04/12/2021				 
  ******************************************************************************
  */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "main.h"
#include "math.h"



static TIM_HandleTypeDef tim17; // Handler For Timer
uint32_t state = 0;
uint32_t cLimit = 0;
uint32_t toggle = 0;
uint32_t count = 0;
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;
float Temperature = 0;
float Humidity = 0;
uint8_t Presence = 0;

void timerInit(void);
void tDelay(uint32_t delayVal);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void TIM17_IRQHandler(void);
void timerStart(void);
void timerStop(void);
void DHT11_Start (void)
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint32_t DHT11_Check_Response (void);
uint32_t DHT11_Read (void);
// FUNCTION      : void GPIOInit (void) 
// DESCRIPTION   : To initialize the GPIO.
// PARAMETERS    : N/A 
// RETURNS       : N/A

void lab5Init(void *data)  {
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.Pin = (GPIO_PIN_1 | GPIO_PIN_8);  
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
GPIO_InitStruct.Alternate = 0;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


}
// FUNCTION      : void GPIOInit (void) 
// DESCRIPTION   : To initialize the GPIO.
// PARAMETERS    : N/A 
// RETURNS       : N/A
void timerInit(void)
{
    if (state == 0) 
    {
        __HAL_RCC_TIM17_CLK_ENABLE(); //Enabling the clock for the gpio

        tim17.Instance = TIM17;
        tim17.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
        tim17.Init.CounterMode = TIM_COUNTERMODE_UP;
        tim17.Init.Period = 0xffff;
        tim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        tim17.Init.RepetitionCounter = 0;
        HAL_TIM_Base_Init(&tim17);
        HAL_TIM_Base_Start(&tim17);
        HAL_NVIC_SetPriority(TIM17_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(TIM17_IRQn);
    }
}

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_1
// FUNCTION      : void DHT11_Start (void) 
// DESCRIPTION   : To initialize the Sensor.
// PARAMETERS    : N/A 
// RETURNS       : N/A
void DHT11_Start (void)
{
	Set_Pin_Output (DHT11_PORT, DHT11_PIN);  // set the pin as output
	HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 0);   // pull the pin low
	tDelay (18000);   // wait for 18ms
    HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);   // pull the pin high
	tDelay (20);   // wait for 20us
	Set_Pin_Input(DHT11_PORT, DHT11_PIN);    // set as input
}
// FUNCTION      : uint32_t DHT11_Check_Response (void)
// DESCRIPTION   : To check the response of sensor.
// PARAMETERS    : N/A 
// RETURNS       : N/A
uint32_t DHT11_Check_Response (void)
{
	uint32_t Response = 0;
	tDelay (40);
	if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
	{
		tDelay (80);
		if ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN))) Response = 1;
		else Response = -1; // 255
	}
	while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));   // wait for the pin to go low

	return Response;
}
// FUNCTION      : uint32_t DHT11_Read (void)
// DESCRIPTION   : To Read the Sensor.
// PARAMETERS    : N/A 
// RETURNS       : N/A
uint32_t DHT11_Read (void)
{
	uint32_t i=0;
	uint32_t j=0;
	for (j=0;j<8;j++)
	{
		while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));   // wait for the pin to go high
		tDelay (40);   // wait for 40 us
		if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));  // wait for the pin to go low
	}
	return i;
}
// FUNCTION      : void TIM7_IRQHandler(void)
// DESCRIPTION   : This Function will initialize the handler for interrupt
// PARAMETERS    : N/A
// RETURNS       : N/A
void TIM17_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&tim17);
}

void tDelay(uint32_t delayVal)
{
    if(state == 1)  
    {  

        uint32_t B = 0;
        uint32_t J = 0;
        uint32_t P = 0;

        B = delayVal / 65535;
        J = delayVal % 65535;

        for(P=0; P<B; P++)
        {
            TIM17->CNT = 0;
            
            while(TIM17->CNT < 65535)
            {
                asm volatile ("nop\n");
            }
        }
            TIM17->CNT = 0;
            
            while(TIM17->CNT < J)
            {
                asm volatile ("nop\n");
            }
    }
}
// FUNCTION      : void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// DESCRIPTION   : thisfunction which will handle the delay 
// PARAMETERS    : TIM_HandleTypeDef *htim
// RETURNS       : N/A

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(state == 2) 
    {  
            count++;
        if(count > cLimit){
            count = 0;
            toggle= ~ toggle;
   
            if(toggle){
                HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
            }
             
        }
    }
}
void timerStart(void)
{
    HAL_TIM_Base_Start_IT(&tim17);
}

ParserReturnVal_t CmdtimerInit(int mode)
{

    if(mode != CMD_INTERACTIVE) return CmdReturnOk;
    timerInit(); 
    printf("Timer Initialized \n");
    return CmdReturnOk;
}
ADD_CMD("TimerInt",CmdtimerInit,"          Initialize Timer")

ParserReturnVal_t CmdtimeDelay(int mode)
{
    uint32_t delay,rc;
    if(mode != CMD_INTERACTIVE) return CmdReturnOk;
    rc = fetch_uint32_arg(&delay);

    if(rc)
    {
        printf("You Must Supply Delay Value\n");
        return CmdReturnBadParameter1;
    }
    state = 1;
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,1);
    tDelay(delay);
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,0);
    return CmdReturnOk;

}
ADD_CMD("timeDelay",CmdtimeDelay," Delay Value            Start Timer For Polling")
void pwmfun(uint32_t channel_no, float percent_value)
{
  
  if(percent_value <= 100)
    {
      printf("For %f percent\t\n",percent_value);
      percent_value = ((percent_value/100.0)*1000);              //Convert 0-100 percent value to 0-1000 scale to feed it to TIM1->CCR1
     
      if((channel_no!=1) && (channel_no !=2) && (channel_no!=3))  //Conditional check
{
 printf("wrong channel_no selected\n");
 return;
} 
      if(channel_no == 1)
      {

{
 TIM1->CR1 &= ~TIM_CR1_CEN;
 TIM1->CCR1 = percent_value;
 TIM1->CR1 |= TIM_CR1_CEN;

}
      }
      if(channel_no == 2)
{

{
 TIM1->CR1 &= ~TIM_CR1_CEN;
 TIM1->CCR2 = percent_value;
 TIM1->CR1 |= TIM_CR1_CEN;

}
 
}
      if(channel_no == 3)
{

{
 TIM1->CR1 &= ~TIM_CR1_CEN;
 TIM1->CCR3 = percent_value;
 TIM1->CR1 |= TIM_CR1_CEN;
}
}
    }
  else
    {
      printf("percent_value should be in between 0-100 only\n");
      return;
    }
}

ParserReturnVal_t Cmdpwminitialisation(int mode)
{

  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  pwm_initialisatin();
  return CmdReturnOk;
}

ADD_CMD("pwminit",Cmdpwminitialisation,"initialises  pwm")

ParserReturnVal_t Cmdpwm(int mode)
{

  int32_t channel_no;
  float percent_value;
 
  int rc;

  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  printf("pwm starting...\n");
  rc=fetch_int32_arg(&channel_no);
  if(rc)
    {
      printf("Mention supply channel_no\n");
      return CmdReturnBadParameter1;
    }
  rc=fetch_float_arg(&percent_value);
  if(rc)
    {
      printf("Mention supply percent_value\n");
      return CmdReturnBadParameter1;
    }
  pwmfun(channel_no,percent_value);
  return CmdReturnOk;
}

ADD_CMD("pwm",Cmdpwm,"<channel_no> <percent_value 0-100%>        setting pwm ")

ParserReturnVal_t Cmdstart(int mode)
{
    uint32_t val,rc;
    if(mode != CMD_INTERACTIVE) return CmdReturnOk;
    rc = fetch_uint32_arg(&val);
    if(rc)
    {
        printf("You Must Supply Delay Value\n");
        return CmdReturnBadParameter1;
    }
    cLimit=val;
    state = 2;
    timerStart(); 
    return CmdReturnOk;  
}
ADD_CMD("start",Cmdstart,"Delay Value             Start Timer Interrupt")

ParserReturnVal_t CmdRead(int mode)
{
    uint32_t val,rc;
    if(mode != CMD_INTERACTIVE) return CmdReturnOk;
    rc = fetch_uint32_arg(&val);
    if(rc)
    {   
          DHT11_Start();
	  Presence = DHT11_Check_Response();
	  Rh_byte1 = DHT11_Read ();
	  Rh_byte2 = DHT11_Read ();
	  Temp_byte1 = DHT11_Read ();
	  Temp_byte2 = DHT11_Read ();
	  SUM = DHT11_Read();
	  TEMP = Temp_byte1;
	  RH = Rh_byte1;
	  Temperature = (float) TEMP;
	  Humidity = (float) RH;
	  Temperature = ((TEMP)/1.0);
	  Humidity = ((RH)/1.0);
	  printf("Temperature of Food-Storage is %.2f\n",Temperature);
          printf("Humidity of Food-Storage is %.2f\n",Humidity);
         return CmdReturnBadParameter1;
    }
    return CmdReturnOk;  
}
ADD_CMD("Read",CmdRead,"READ TEMPERATURE AND HUMIDITY")

	

