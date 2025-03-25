/**
  **************************************************************************
  * @file     usart.c
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    usart program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to 
  * download from Artery official website is the copyrighted work of Artery. 
  * Artery authorizes customers to use, copy, and distribute the BSP 
  * software and its related documentation for the purpose of design and 
  * development in conjunction with Artery microcontrollers. Use of the 
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "usart.h"
#include "osal_irq.h"

usart_group_type usart_group_struct;

void OTA_USART_IRQHandler(void)
{
    uint8_t reval;

    if((RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)) && 
       (RESET != usart_flag_get(USART1, USART_FLAG_RBNE))){
        reval = (uint8_t)usart_data_receive(USART1);
        if(usart_group_struct.count > (USART_REC_LEN - 1)) {
            usart_group_struct.count = 0;
            usart_group_struct.head = 0;
            usart_group_struct.tail = 0;
        }
        else {
            usart_group_struct.count++;
            usart_group_struct.buf[usart_group_struct.head++] = reval;
            if(usart_group_struct.head > (USART_REC_LEN - 1)) {
                usart_group_struct.head = 0;
            }
        }
    }
}

void ota_uart_init(uint32_t baudrate)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_3);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, baudrate);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);

    nvic_irq_enable(USART1_IRQn, 0, 0);

    /* enable USART1 receive interrupt */
    usart_interrupt_enable(USART1, USART_INT_RBNE);

    OsSetVector(USART1_IRQn, OTA_USART_IRQHandler, NULL);
}

uint8_t uart_data_take(void)
{
    uint8_t val;

    usart_group_struct.count--;
    val = usart_group_struct.buf[usart_group_struct.tail++];
    if(usart_group_struct.tail > (USART_REC_LEN - 1)) {
        usart_group_struct.tail = 0;
    }
    return val;
}
