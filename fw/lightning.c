#include <stdio.h>
#include <string.h>
#include "stm8s.h"

#define FALSE 0
#define TRUE 1

//#define DEBUG_ENABLE

/* mirror lockup will stop when there is no 
 * shutter trigger in 25 seconds on Canon 350D
 * */
#define MIRROR_LOCKUP_AUTO_RESTART

#define BG_INTERVAL 5000           // get background light level every BG_INTERVAL sec

#ifdef MIRROR_LOCKUP_AUTO_RESTART
#define MIRROR_TIMEOUT 30500
#endif

#ifdef STM8S105                 // config for STM8S-DISCOVERY board
#define CH_BACKGROUND 4
#define CH_HIGHPASS 9
#define SHUTTER_PORT GPIOB
#define SHUTTER_PIN 0x80        //PB7
#define UART UART2
#define UART_SR_TXE UART2_SR_TXE
#define UART_CR2_TEN UART2_CR2_TEN
#define TXD_PORT GPIOD
#define TXD_PIN 0x20            //PD5
#define ML_PORT GPIOD
#define ML_PIN 0x80
#elif defined(STM8S103)         // config for my STM8S103 board
#define CH_BACKGROUND 5
#define CH_HIGHPASS 6
#define SHUTTER_PORT GPIOC
#define SHUTTER_PIN 0x80        //PB7
#define UART UART1
#define UART_SR_TXE UART1_SR_TXE
#define UART_CR2_TEN UART1_CR2_TEN
#define TXD_PORT GPIOD
#define TXD_PIN 0x20            //PD5
#define ML_PORT GPIOB
#define ML_PIN 0x20
#endif

volatile bool lightning_trg = FALSE;
bool mirror_lockup_enable = FALSE;
uint16_t *buf_ptr;
volatile uint16_t time_cnt = 0;

#ifdef MIRROR_LOCKUP_AUTO_RESTART
volatile bool do_mirror_lockup = FALSE;
volatile uint16_t mirror_cnt = 0;
#endif

void delay_ms(uint16_t t)
{
    time_cnt = t;
    while(time_cnt > 0);
}

#ifdef DEBUG_ENABLE
bool bg_flag = FALSE;
uint16_t tmp;
/* ========================================================================== */
/**
 * @brief   putchar send a char to UART2 for printf func
 *
 * @param   c char to send
 *
 * @return  
 */
/* ========================================================================== */
int putchar(int c)
{
    while(!(UART->SR & UART_SR_TXE));
    UART->DR = (uint8_t)c;
    return 0;
}
#endif

/* ========================================================================== */
/**
 * @brief   main main func
 */
/* ========================================================================== */
void main(void) 
{
    uint32_t i = 0;

    CLK->CKDIVR = 0x00;                             // Set the frequency to 16 MHz
    CLK->PCKENR1 = 0x88;                            // Enable TIM1 & UART2 peripherals
    CLK->PCKENR2 = 0x08;                            // Enable ADC1 peripherals

    /* setting up shutter io */
    SHUTTER_PORT->DDR |= SHUTTER_PIN;
    SHUTTER_PORT->CR1 |= SHUTTER_PIN;
    SHUTTER_PORT->CR2 |= SHUTTER_PIN;
    SHUTTER_PORT->ODR = 0;

    /* mirror lockup pin setup */
    ML_PORT->CR1 |= ML_PIN;                         //input , pull up, no interrupt

#ifdef DEBUG_ENABLE
    TXD_PORT->DDR |= TXD_PIN;                       // Put TX line on
    TXD_PORT->CR1 |= TXD_PIN;
    TXD_PORT->CR2 |= TXD_PIN;

    UART->CR2 = UART_CR2_TEN;                       // Allow TX & RX
    UART->BRR2 = 0x0B; UART->BRR1 = 0x08;           // 115200 baud
#endif

    buf_ptr = (uint16_t *)&ADC1->DB0RH;             //set a pointer to ADC data buffer
    /* set adc1 to continue buffered mode to get background light level 
     * and set it to high threathold for lightning trigger*/
    ADC1->CR1 &= ~ADC1_CR1_ADON;                    //power off
    ADC1->CR2 |= ADC1_CR2_ALIGN;                    //right alignment
    ADC1->CR1 |= ADC1_CR1_CONT;                     //Continuous
    ADC1->CR3 |= ADC1_CR3_DBUF;                     //buffer
    ADC1->CSR |= (ADC1_CSR_CH & CH_BACKGROUND);     // set input channel
    ADC1->CSR |= ADC1_CSR_EOCIE;                    //set end of conversion interrupt
    ADC1->TDRL = 0xFF;                              //disable sthmit trigger
    ADC1->TDRH = 0xFF;
    ADC1->HTRL = ADC1_HTRL_RESET_VALUE;
    ADC1->HTRH = ADC1_HTRH_RESET_VALUE;
    ADC1->LTRL = ADC1_LTRL_RESET_VALUE;
    ADC1->LTRH = ADC1_LTRH_RESET_VALUE;
    ADC1->CR1 |= ADC1_CR1_ADON;                     //power on
    ADC1->CR1 |= ADC1_CR1_ADON;                     //start conversion

    /*
     * set up 1ms timer interrupt
     * to get background light level every BG_INTERVAL ms
     */
    TIM1->ARRH = (uint8_t)(500 >> 8);               //auto reload value
    TIM1->ARRL = (uint8_t)500;
    TIM1->PSCRH = 0;                                //prescaler
    TIM1->PSCRL = (uint8_t)31;
    TIM1->IER |= TIM1_IER_UIE;                      //enable update interrupt
    TIM1->RCR = 0;                                  //disable repeat counter
    TIM1->CR1 = TIM1_CR1_RESET_VALUE;
    TIM1->CR1 |= TIM1_CR1_ARPE;                     //auto reload
    TIM1->CR1 |= TIM1_CR1_CEN;

    enableInterrupts();
    delay_ms(1000);

    // read mirror lockup switch
    mirror_lockup_enable = ((ML_PORT->IDR & ML_PIN) == 0);

    if (mirror_lockup_enable) {
        //mirror lockup enable
        SHUTTER_PORT->ODR |= SHUTTER_PIN;
        delay_ms(100);
        SHUTTER_PORT->ODR &= ~SHUTTER_PIN;
    }

    do {
        /* trigger camera */
        if (lightning_trg) {
            SHUTTER_PORT->ODR |= SHUTTER_PIN;
#ifdef DEBUG_ENABLE
            //print trigger light level
            printf("lightning = %d !\n\r", tmp);
#endif
            delay_ms(100);
            SHUTTER_PORT->ODR &= ~SHUTTER_PIN;

            if (mirror_lockup_enable) {
                // here, shutter duration should less than 2000 ms
                delay_ms(2000);
                //mirror lockup enable
                SHUTTER_PORT->ODR |= SHUTTER_PIN;
                delay_ms(100);
                SHUTTER_PORT->ODR &= ~SHUTTER_PIN;
#ifdef MIRROR_LOCKUP_AUTO_RESTART
                //reset mirror timeout counter
                mirror_cnt = MIRROR_TIMEOUT;
#endif
            }

            lightning_trg = FALSE;
            ADC1->CR1 |= ADC1_CR1_ADON;                         //power on
            ADC1->CR1 |= ADC1_CR1_ADON;                         //start conversion
        }


#ifdef DEBUG_ENABLE
        //print background light level
        if (bg_flag) {
            bg_flag = FALSE;
            printf("background = %d\n\r", tmp);
        }
#endif

#ifdef MIRROR_LOCKUP_AUTO_RESTART
        if (mirror_lockup_enable && do_mirror_lockup) {
            do_mirror_lockup = FALSE;
            //mirror lockup enable
            SHUTTER_PORT->ODR |= SHUTTER_PIN;
            delay_ms(100);
            SHUTTER_PORT->ODR &= ~SHUTTER_PIN;
            //reset mirror timeout counter
            mirror_cnt = MIRROR_TIMEOUT;
        }
#endif
    } while(1);
}

/* ========================================================================== */
/**
 * @brief   tim1_update tim1 ISR , 1 sec timer, count BG_INTERVAL sec to
 *                      get background light level
 *
 * @param   none
 */
/* ========================================================================== */
void tim1_update(void) __interrupt(ISR_TIM1_OVF)
{
    static uint16_t sec = 0;

    if (time_cnt > 0)
        time_cnt--;

#ifdef MIRROR_LOCKUP_AUTO_RESTART
    if (mirror_cnt > 0) {
        if (--mirror_cnt == 0)
            do_mirror_lockup = TRUE;
    }
#endif

    if (sec++ > BG_INTERVAL) {
        sec = 0;
        /*************************************************
         * set adc1 to buffered continue mode
         * and get background light level
         * **********************************************/
        ADC1->CR1 &= ~ADC1_CR1_ADON;                    //power off
        ADC1->CR3 |= ADC1_CR3_DBUF;                     //buffer mode
        //ADC1->CSR &= ~ADC1_CSR_CH;
        //ADC1->CSR |= (ADC1_CSR_CH & CH_BACKGROUND);   //select background channel
        //ADC1->CSR |= ADC1_CSR_EOCIE;                  //enable end of conversion isr
        //ADC1->CSR &= ~ADC1_CSR_AWDIE;                 //disable AWD
        ADC1->CSR = 0x24;
        ADC1->CR1 |= ADC1_CR1_ADON;                     //power on
        ADC1->CR1 |= ADC1_CR1_ADON;                     //start conversion
    }

    TIM1->SR1 &= ~(TIM1_SR1_UIF);                       //clear update flag
}

/* ========================================================================== */
/**
 * @brief   adc ADC1 ISR, background light level calculate and setting
 *              lightning trigger level, when lightning detected, trigger
 *              the camera shutter
 *
 * @param   none
 */
/* ========================================================================== */
void adc(void) __interrupt(ISR_ADC1)
{
    uint8_t i;
    uint32_t sum;

    if (ADC1->CSR & ADC1_CSR_AWD) {                     //AWD interrupt
        ADC1->CSR &= ~ADC1_CSR_AWDIE;                   //disable AWD interrupt
        /*************************************************
         * detected lightning bolt
         * and trigger camera shutter
         * **********************************************/
        if (lightning_trg == FALSE) {
            SHUTTER_PORT->ODR |= SHUTTER_PIN;           //trigger camera shutter
            ADC1->CR1 &= ~ADC1_CR1_ADON;                    //power off
            lightning_trg = TRUE;
#ifdef DEBUG_ENABLE
            tmp = *(uint16_t*)&ADC1->DRH;               //store the trigger value for print
#endif
        }
        ADC1->CSR &= ~ADC1_CSR_AWD;                     //clear interrupt flag
        ADC1->CSR &= ~ADC1_CSR_EOC;                     //clear interrupt flag
        ADC1->CSR |= ADC1_CSR_AWDIE;                    //enable AWD interrupt
    } else if (ADC1->CSR & ADC1_CSR_EOC) {              //end of conversion
        ADC1->CR1 &= ~ADC1_CR1_ADON;                    //power off

        /***************************************************
         * everage 8 sample to be the background light level
         **************************************************/
        sum = 0;
        for(i = 0; i < 8; i++)
            sum += buf_ptr[i];
        sum >>= 3;
        sum += 5;

#ifdef DEBUG_ENABLE
        tmp = sum;                                      //store everage value for print
        bg_flag = TRUE;                                 //print flag
#endif
        /*************************************************
         * set adc1 to non-buffered continue mode
         * and trigger by light level greater that 
         * high threathold
         * **********************************************/
        ADC1->CR3 &= ~ADC1_CR3_DBUF;                        //buffer mode
        //ADC1->CSR &= ~ADC1_CSR_EOC;                       //clear interrupt flag
        //ADC1->CSR &= ~ADC1_CSR_CH;
        //ADC1->CSR |= (ADC1_CSR_CH & CH_HIGHPASS);         //select highpass channel
        //ADC1->CSR &= ~ADC1_CSR_EOCIE;                     //set end of conversion interrupt
        //ADC1->CSR |= ADC1_CSR_AWDIE;                      //enable AWD interrupt
        ADC1->CSR = 0x19;
        ADC1->HTRL = (sum & 3);                             //set AWD
        ADC1->HTRH = (sum >> 2);
        ADC1->CR1 |= ADC1_CR1_ADON;                         //power on
        ADC1->CR1 |= ADC1_CR1_ADON;                         //start conversion
    }
}
