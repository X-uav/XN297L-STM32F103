
#include "config.h"             //包含所有的驱动头文件
#include "config.h"

#define PRESSED_KEY(key) (GPIO_ReadInputDataBit(GPIOB,key)==0)


const uint16_t ARM_KEY = GPIO_Pin_3;
const uint16_t ALT_KEY = GPIO_Pin_8;
const uint16_t CAL_KEY = GPIO_Pin_11;
const uint16_t ONLINE_KEY = GPIO_Pin_1;
const uint16_t USER1_KEY = GPIO_Pin_12;
const uint16_t USER2_KEY = GPIO_Pin_13;
const uint16_t USER3_KEY = GPIO_Pin_14;
const uint16_t USER4_KEY = GPIO_Pin_15;
const uint16_t USER5_KEY = GPIO_Pin_9;

u8 onlineKeyFlag = 0;
u8 armKeyFlag = 0;
u8 calibrationKeyFlag = 0;
u8 altKeyFlag = 0;
u8 user1Flag = 0;
u8 user2Flag = 0;
u8 user3Flag = 0;
u8 user4Flag = 0;
u8 user5Flag = 0;
/********************************************
              Key初始化函数
功能：
1.配置Key接口IO输入方向
********************************************/
void KeyInit(void)
{
    GPIO_TypeDef *t_gpio;
    uint16_t t_pin;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    /* config the extiline(PB1,PB3,PA8) clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);
    /* config the NVIC(arm-->PB3) */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* config the NVIC(althold-->PA8) */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

#ifdef NEWC03
    t_gpio = GPIOB;
    t_pin = GPIO_Pin_11;
#else
    t_gpio = GPIOA;
    t_pin = GPIO_Pin_13;
#endif
    /* Configure “Calibrate”-->PA13 as output push-pull */
    GPIO_InitStructure.GPIO_Pin =  t_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(t_gpio, &GPIO_InitStructure);

    /* Configure "online"-->PB1 as output push-pull */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure “arm”-->PB3 as output push-pull */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure "altHold"-->PA8 as output push-pull */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //arm-->PB3
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;            //设定外部中断1
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  //设定中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //设定下降沿触发模式
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    //althold-->PA8
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;              //设定外部中断1
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //设定中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //设定下降沿触发模式
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

}


//arm按键，控制电机是否待机转动
void EXTI3_IRQHandler(void) {
    if(EXTI_GetITStatus(EXTI_Line3) != RESET) //确保是否产生了EXTI Line中断
    {
        armKeyFlag = 1;
        EXTI_ClearITPendingBit(EXTI_Line3);     //清除中断标志位
    }
}


void EXTI9_5_IRQHandler(void) {
    //althold按键，控制IMU校准
    if(EXTI_GetITStatus(EXTI_Line8) != RESET) //确保是否产生了EXTI Line中断
    {
        altKeyFlag = 1;
        EXTI_ClearITPendingBit(EXTI_Line8);     //清除中断标志位
    }

}


void checkKey(void)
{
    //中断计数
    static uint8_t keyCount = 0;
    //查询计数
    static uint8_t queryOnlineCount = 0;
    static uint8_t queryCalCount = 0;

    //在线模式控制
    //在在线模式下，其他按键不可用
    //退出对频只用在线按键能用
    if(PRESSED_KEY(ONLINE_KEY))
    {
        onlineKeyFlag = 1;
        queryOnlineCount++;
        if(queryOnlineCount>15)
        {
            queryOnlineCount = 10;
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);

        }
    }
    else
    {
        if(onlineKeyFlag && queryOnlineCount > 8)
        {
            onlineKeyFlag = 0;
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
#ifdef AMERICAN_RC_MODE
            if(remoteData.motor[PITCH] > 1600 && remoteData.motor[ROLL] > 1600)
            {
                remoteData.cmd |= NEWADDRESS;
                remoteData.cmd &= (~ONLINE);

            }
#else
            if(remoteData.motor[THROTTLE] > 1600 && remoteData.motor[ROLL] > 1600)
            {
                remoteData.cmd |= NEWADDRESS;
                remoteData.cmd &= (~ONLINE);
            }
#endif
            else
            {
                if(remoteData.cmd & NEWADDRESS)
                {
                    modifyOnceFlag = 1;
                    remoteData.cmd &= (~NEWADDRESS);
                }
                else
                {
                    if(remoteData.cmd & ONLINE)
                    {
                        remoteData.cmd &= (~ONLINE);
                        remoteData.cmd &= (~MOTOR);
                    }
                    else
                    {
                        remoteData.cmd |= ONLINE;
                        remoteData.motor[PITCH] = 1500;
                        remoteData.motor[ROLL] = 1500;
                        remoteData.motor[YAW] = 1500;
                        remoteData.motor[THROTTLE] = 1000;
                    }
                }
            }

            remoteData.cmd &= (~ARM);
            remoteData.cmd &= (~ALTHOLD);
            remoteData.cmd &= (~OFFLINE);

        }
        //按键松开，计数清0
        queryOnlineCount = 0;
    }

    //对频或者在线模式，则忽略其他按键
    if((remoteData.cmd & ONLINE) || (remoteData.cmd & NEWADDRESS))
    {
        armKeyFlag = 0;
        altKeyFlag = 0;
        calibrationKeyFlag = 0;
        user1Flag = 0;
        user2Flag = 0;
        user3Flag = 0;
        user4Flag = 0;
        user5Flag = 0;

        return;
    }

    //校准按键，发送后清除校准标志位
#ifdef NEWC03
    if(PRESSED_KEY(CAL_KEY))
#else
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_13)==0)
#endif
    {
        calibrationKeyFlag = 1;
        queryCalCount++;
        if(queryCalCount>10)
        {
            queryCalCount = 10;
            GPIO_ResetBits(GPIOB, GPIO_Pin_10);
        }
    }
    else
    {
        if(calibrationKeyFlag && queryCalCount > 8)
        {
            calibrationKeyFlag = 0;
            remoteData.cmd |= CALIBRATION;
            GPIO_SetBits(GPIOB, GPIO_Pin_10);
        }
        //按键松开，计数清0
        queryCalCount = 0;
    }

    //用户自定义按键1
    if(PRESSED_KEY(USER1_KEY))
    {
        user1Flag = 1;
    }
    else
    {
        if(user1Flag)
        {
            user1Flag = 0;
            remoteData.key = 1;
        }

    }
    //用户自定义按键2
    if(PRESSED_KEY(USER2_KEY))
    {
        user2Flag = 1;
    }
    else
    {
        if(user2Flag)
        {
            user2Flag = 0;
            remoteData.key = 2;
        }
    }
    //用户自定义按键3
    if(PRESSED_KEY(USER3_KEY))
    {
        user3Flag = 1;
    }
    else
    {
        if(user3Flag)
        {
            user3Flag = 0;
            remoteData.key = 3;
        }
    }
    //用户自定义按键4
    if(PRESSED_KEY(USER4_KEY))
    {
        user4Flag = 1;
    }
    else
    {
        if(user4Flag)
        {
            user4Flag = 0;
            remoteData.key = 4;
        }
    }
    //用户自定义按键5
    if(PRESSED_KEY(USER5_KEY))
    {
        user5Flag = 1;
    }
    else
    {
        if(user5Flag)
        {
            user5Flag = 0;

            if(remoteData.cmd & OFFLINE)
            {
                remoteData.cmd &= (~OFFLINE);
            }
            else
            {
                remoteData.cmd |= OFFLINE;
            }
        }

    }

    //中断判断（定高和解锁）
    if(armKeyFlag || altKeyFlag)
    {
        keyCount++;
        if(keyCount<3)
        {
            return;
        }
        else
        {
            keyCount = 0;
        }
    }


    //上/解锁
    if(armKeyFlag)
    {
        armKeyFlag = 0;
        if(remoteData.cmd & ARM)
        {

            remoteData.cmd &= (~ARM);
        }
        else
        {
            if(remoteData.motor[THROTTLE] < 1150)
            {
                remoteData.cmd |= ARM;
            }
        }
    }
    //开/关定高模式
    if(altKeyFlag)
    {
        altKeyFlag = 0;
        if(remoteData.cmd & ALTHOLD)
        {

            remoteData.cmd &= (~ALTHOLD);
        }
        else
        {

            remoteData.cmd |= ALTHOLD;
        }

    }



}



