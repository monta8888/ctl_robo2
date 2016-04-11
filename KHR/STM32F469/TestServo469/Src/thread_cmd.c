/**
  ******************************************************************************
  * @file    thread_cmd.c 
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

extern UART_HandleTypeDef huart6;
extern osThreadId    ServoMain;
extern osThreadId    CmdRcv;
extern osMessageQId  RcvUSARTMsgBox;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t CmdBuf[CMD_BUF_SIZE];
uint32_t CmdBufRPtr=0;

uint32_t tim1=0;
uint32_t tim2=0;
uint32_t tim4cnt=0;

#ifdef USING_LCD
uint32_t line_num=20;
uint8_t  desc[24];
#endif // USING_LCD

uint8_t TxD[3];
double theta = 0.0; 

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
#ifdef CTL_GR001
void OnTorque(uint8_t ID, uint8_t data) {
  uint8_t TxData[9];
  uint8_t CheckSum = 0;

  TxData[0] = 0xFA; // H e a d e r
  TxData[1] = 0xAF; // H e a d e r
  TxData[2] = ID;   // I D
  TxData[3] = 0x00; // F l a g s
  TxData[4] = 0x24; // A d d r e s s
  TxData[5] = 0x01; // L e n g t h
  TxData[6] = 0x01; // C o u n t
  TxData[7] = data; // D a t a
  // チェックサム計算 // ID～DATAまでのXOR
  for(int i=2; i<=7; i++){ CheckSum = CheckSum ^ TxData[i]; }
  TxData[8] = CheckSum; // S u m
  // パケットデータ送信
  HAL_UART_Transmit(&huart6, (uint8_t*)TxData, 9, 100);
}

void GetSVinfo(uint8_t ID) {
  uint8_t TxData[8];
  uint8_t CheckSum = 0;

  TxData[0] = 0xFA; // H e a d e r
  TxData[1] = 0xAF; // H e a d e r
  TxData[2] = ID;   // I D
  TxData[3] = 0x0F; // F l a g s
  TxData[4] = 0x2A; // A d d r e s s
  TxData[5] = 0x02; // L e n g t h
  TxData[6] = 0x00; // C o u n t
  // チェックサム計算 // ID～DATAまでのXOR
  for(int i=2; i<=6; i++){ CheckSum = CheckSum ^ TxData[i]; }
  TxData[7] = CheckSum; // S u m
  // パケットデータ送信
  HAL_UART_Transmit(&huart6, (uint8_t*)TxData, 9, 100);
}

void MovSV(uint8_t ID, int32_t Angle, uint32_t Speed) {
  uint8_t TxData[12];
  uint8_t CheckSum = 0;

  TxData[0] = 0xFA; // H e a d e r
  TxData[1] = 0xAF; // H e a d e r
  TxData[2] = ID;   // I D
  TxData[3] = 0x00; // F l a g s
  TxData[4] = 0x1E; // A d d r e s s
  TxData[5] = 0x04; // L e n g t h
  TxData[6] = 0x01; // C o u n t
  // Angle
  TxData[7] = (unsigned char)0x00FF & Angle; // L o w b y t e
  TxData[8] = (unsigned char)0x00FF & (Angle >> 8); // H i byte
  // Speed
  TxData[9] = (unsigned char)0x00FF & Speed; // L o w b y t e
  TxData[10] = (unsigned char)0x00FF & (Speed >> 8); // Hi byte
  // チェックサム計算 // ID～DATAまでのXOR
  for(int i=2; i<=10; i++){ CheckSum = CheckSum ^ TxData[i]; }
  TxData[11] = CheckSum; // S u m
  // パケットデータ送信
  HAL_UART_Transmit(&huart6, (uint8_t*)TxData, 12, 100);
}
#elif defined(CTL_KHR)
void GetSvTemp(uint8_t ID) {
  uint8_t TxData[2];

  TxData[0] = 0xa0 | ID;
  TxData[1] = 0x04; // Temparature

  // パケットデータ送信
  HAL_UART_Transmit(&huart6, (uint8_t*)TxData, 2, 100);
}

void MovSV(uint8_t ID, int32_t Pos) {

  TxD[0] = 0x80 | ID;
//  TxD[1] = (uint8_t)(Pos >> 7 & 0x7F);
//  TxD[2] = (uint8_t)(Pos & 0x7F);
  TxD[1] = (uint8_t)(Pos / 128); // 上位7bit 
  TxD[2] = (uint8_t)(Pos % 128); // 下位7bit 
  printf("TRANS:%x %x %x\r\n", TxD[0], TxD[1], TxD[2]);

  // パケットデータ送信
  HAL_UART_Transmit(&huart6, (uint8_t*)TxD, 3, 100);
}
#endif

/**
  * @}
  */ 
uint32_t cmd_check_first(uint32_t rptr) {

#ifdef CTL_GR001
  if(CmdBuf[rptr]==0xfd) {
#elif defined(CTL_KHR)
  if(CmdBuf[rptr]==TxD[0]) {
#endif
    CmdBufRPtr = rptr;
    return 1;
  }
  else {
    return 0;
  }
}

/**
  * @}
  */ 
uint32_t cmd_check_rptr(uint32_t rptr, uint32_t offset) {

  if ((rptr+offset) >= CMD_BUF_SIZE) {
    return ((rptr+offset) - CMD_BUF_SIZE);
  }
  else {
    return ((rptr+offset));
  }
}

#ifdef USING_LCD
/**
  * @}
  */ 
void check_displayline(uint32_t *line)
{
  uint32_t i;

  if (*line > 300) {
    for(i=20; i<320; i+=10) {
      BSP_LCD_DisplayStringAt(0, i, (uint8_t *)"                     ", LEFT_MODE);
    }
    *line = 20;
  }
  else {
    *line += 10;
  }
}
#endif // USING_LCD

/**
  * @}
  */ 
void task_cmd_recieve(void const *argument) {

  uint8_t i=0;
#ifdef CTL_GR001
  uint8_t BufADDR, BufLEN, BufCNT, BufDATA1, BufDATA2;
#elif defined(CTL_KHR)
  uint8_t BufDATA1, BufDATA2;
#endif
  uint32_t    cptr_pre=0;
  osEvent     evt;

#ifdef USING_UART_DMA
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE); /* Enable the UART Transmit data register empty Interrupt */
  HAL_UART_Receive_DMA(&huart6, (uint8_t *)CmdBuf, CMD_BUF_SIZE);
#else // USING_UART_DMA
  HAL_UART_Receive_IT(&huart6, (uint8_t *)CmdBuf, CMD_BUF_SIZE);
#endif // USING_UART_DMA
  for(;;) {
    evt = osMessageGet(RcvUSARTMsgBox, osWaitForever);  // wait for message
    if (evt.status == osEventMessage) {
      cptr_pre = 0;
      switch (i) {
      case 0: // (0xFD or 0x8x)
        if(cmd_check_first(CmdBufRPtr)) { i=1; }
        else { CmdBufRPtr = cmd_check_rptr(CmdBufRPtr,1); } /* 文字化けの場合無視する */
        break;
#ifdef CTL_GR001
      case 1: // 0xDF
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(cmd_check_first(cptr_pre))   { i=1; } /* 文字欠けで次CMD先頭が来てしまった場合 */
        else if(CmdBuf[cptr_pre]==0xDF) { i=2; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 2: // 0x01
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(cmd_check_first(cptr_pre))   { i=1; } /* 文字欠けで次CMD先頭が来てしまった場合 */
        else if(CmdBuf[cptr_pre]==0x01) { i=3; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 3: // 0x00
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(cmd_check_first(cptr_pre))   { i=1; } /* 文字欠けで次CMD先頭が来てしまった場合 */
        else if(CmdBuf[cptr_pre]==0x00) { i=4; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 4:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufADDR = CmdBuf[cptr_pre];
        i=5;
        break;
      case 5:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufLEN = CmdBuf[cptr_pre];
        i=6;
        break;
      case 6:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufCNT = CmdBuf[cptr_pre];
        i=7;
        break;
      case 7:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufDATA1 = CmdBuf[cptr_pre];
        i=8;
        break;
      case 8:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufDATA2 = CmdBuf[cptr_pre];
        i=9;
        break;
      case 9:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
        tim2 = tim4cnt;
        printf("TIM[%d] ADDR:%x LEN:%x CNT:%x DATA:%x%x\r\n", (tim2-tim1), BufADDR, BufLEN, BufCNT, BufDATA2, BufDATA1);
        i=0;
        BufADDR=0;
        BufLEN=0;
        BufCNT=0;
        BufDATA1=0;
        BufDATA2=0;
        break;
#elif defined(CTL_KHR)
      case 1: // 
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(cmd_check_first(cptr_pre))     { i=1; } /* 文字欠けで次CMD先頭が来てしまった場合 */
        else if(CmdBuf[cptr_pre]==TxD[1]) { i=2; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 2: // 
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(cmd_check_first(cptr_pre))     { i=1; } /* 文字欠けで次CMD先頭が来てしまった場合 */
        else if(CmdBuf[cptr_pre]==TxD[2]) { i=3; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 3: // 
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        if(CmdBuf[cptr_pre]==TxD[0]) { i=4; }
        else {
          CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
          i=0;
        }
        break;
      case 4: // 
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        BufDATA1 = CmdBuf[cptr_pre];
        i=5;
        break;
      case 5:
        cptr_pre = cmd_check_rptr(CmdBufRPtr,i);
        CmdBufRPtr = cmd_check_rptr(cptr_pre,1);
        BufDATA2 = CmdBuf[cptr_pre];
        tim2 = tim4cnt;
#ifdef USING_LCD
        sprintf((char *)desc,"TIM[%d] DATA:%x", (tim2-tim1), BufDATA1);
        BSP_LCD_DisplayStringAt(0, line_num, (uint8_t *)desc, LEFT_MODE);
        check_displayline(&line_num);
#endif // USING_LCD
        printf("TIM[%d] %x %x %x DATA:%x %x\r\n", (tim2-tim1), TxD[0], TxD[1], TxD[2], BufDATA1, BufDATA2);
        i=0;
        BufDATA1=0;
        BufDATA2=0;
        break;
#endif
      default:
        break;
      }
    }
  }
}

/**
  * @}
  */ 
void task_main(void const *argument) {

#ifdef CTL_GR001
  OnTorque(0xFF, 0x01); // ID=1(0x01),torque=OFF(0x00),ON(0x01),BRAKE(0x02)
#endif // CTL_GR001

  osDelay(1000);

  while(1){
    BSP_LED_Toggle(LED3);
    BSP_LED_Toggle(LED4);

    tim1 = tim4cnt;

#ifdef CTL_GR001
//    MovSV(1,300,100); // I D = 1～20 , GoalPosition = 30.0deg(300) , Time = 1.0sec(100)
//    GetSVinfo(1);
//    osDelay(500);
    MovSV(0xFF,-300,100); // ID = ALL(0xFF) , GoalPosition = -30.0deg(-300) , Time = 1.0sec
#elif defined(CTL_KHR)
//    GetSvTemp(0x00); // ID=0
//    MovSV(0x00,7500); // ID=0, Position=7500
MovSV(0x00,7500+(uint32_t)(1000*sin(theta)));
theta += 0.05;
#endif

    osDelay(1000);

#ifdef USING_LCD
    sprintf((char *)desc,"TIM_CNT0:%d", tim4cnt);
    BSP_LCD_DisplayStringAt(0, line_num, (uint8_t *)desc, LEFT_MODE);
    check_displayline(&line_num);
#endif // USING_LCD
    printf("TIM_CNT:%d\r\n", tim4cnt);
  }
}
