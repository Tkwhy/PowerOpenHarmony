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

/**
  * @brief  usart1 interrupt handler.
  * @param  none
  * @retval none
  */
void USART1_IRQHandler(void)
{
    uint16_t reval;
    //time_ira_cnt = 0;  /* clear upgrade time out flag */
    if(usart_flag_get(USART1, USART_RDBF_FLAG) != RESET) {
        reval = usart_data_receive(USART1);
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

void USART2_IRQHandler(void)
{
    uint16_t reval;
    //time_ira_cnt = 0;  /* clear upgrade time out flag */
    if(usart_flag_get(USART2, USART_RDBF_FLAG) != RESET) {
        //usart_flag_clear(USART2, USART_RDBF_FLAG);
        reval = usart_data_receive(USART2);
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

/**
  * @brief  init usart.
  * @param  none
  * @retval none
  */
void uart1_init(uint32_t baudrate)
{
    gpio_init_type gpio_init_struct;
    /* enable the usart and it's io clock */
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);

    /* configure the usart1_tx  pa9 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_9;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOA, &gpio_init_struct);

    /* configure the usart1_rx  pa10 */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_10;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOA, &gpio_init_struct);

    /*configure usart nvic interrupt */
    nvic_irq_enable(USART1_IRQn, 0, 0);

    /*configure usart param*/
    usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
    usart_enable(USART1, TRUE);
    OsSetVector(USART1_IRQn, USART1_IRQHandler, NULL);
}

void uart2_init(uint32_t baudrate)
{
    gpio_init_type gpio_init_struct;

    /* enable the USART2 and gpio clock */
    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    /* configure the USART2 tx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_5;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOD, &gpio_init_struct);

    /* configure the USART2 rx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_6;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOD, &gpio_init_struct);

    gpio_pin_remap_config(USART2_MUX, TRUE);

    /* configure USART2 param */
    usart_init(USART2, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART2, TRUE);
    usart_receiver_enable(USART2, TRUE);

    /* config usart nvic interrupt */
    OsSetVector(USART2_IRQn, USART2_IRQHandler);
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(USART2_IRQn, 1, 0);

    /* enable USART2 interrupt */
    usart_interrupt_enable(USART2, USART_RDBF_INT, TRUE);  /* usart receive data buffer full interrupt */
    usart_enable(USART2, TRUE);
}

uint8_t uart_data_take(void)
{
    uint8_t val;
    usart_interrupt_enable(USART1, USART_RDBF_INT, FALSE);
    usart_group_struct.count--;
    usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
    val = usart_group_struct.buf[usart_group_struct.tail++];
    if(usart_group_struct.tail > (USART_REC_LEN - 1)) {
        usart_group_struct.tail = 0;
    }
    return val;
}
