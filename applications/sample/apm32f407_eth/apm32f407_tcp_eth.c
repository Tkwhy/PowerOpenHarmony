#include "apm32f407_tcp_eth.h"
// #include "sys_arch.h"
/** lwip network interface structure for ethernetif */
struct netif UserNetif;
/** MAC address */
uint8_t SetMACaddr[6] = {0, 0, 0, 0, 0, 8};

void LwIP_Init(void);

void ETH_IRQHandler(void)
{
    if (ETH_ReadDMAIntFlag(ETH_DMA_INT_RX))
    {
        LOS_SemPost(s_BinarySem_Handle); // 释放信号量
        ETH_ClearDMAIntFlag(ETH_DMA_INT_RX);
    }
}

void eth_task(void)
{
    LOS_HwiCreate(ETH_IRQn, 0, 0, ETH_IRQHandler, 0);
    ConfigEthernet();
    LwIP_Init();
    while (1)
    {
        // printf("Hello world!\r\n");
        APM_EVAL_LEDToggle(LED2);
        LOS_TaskDelay(1000); // 1s 延时
    }
}
VOID EthTaskSample(VOID)
{
    UINT32 ret;
    UINT32 taskID1;
    TSK_INIT_PARAM_S stTask = {0};
    printf("TaskSample: eth_task create start...\n");
    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)eth_task;
    stTask.uwStackSize = 4096;
    stTask.pcName = "EthTask";
    stTask.usTaskPrio = 6;
    ret = LOS_TaskCreate(&taskID1, &stTask);
    if (ret != LOS_OK)
    {
        printf("Task1 create failed\n");
    }
}
void LwIP_Init(void)
{
    struct ip4_addr ipaddr;
    struct ip4_addr netmask;
    struct ip4_addr gw;

    /** Initializes the dynamic memory heap */
    mem_init();

    /** Initializes the m+emory pools */
    memp_init();

    // 使用静态IP
    IP4_ADDR(&ipaddr, 169, 254, 71, 22);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 169, 254, 71, 1);

    /** Config MAC Address */
    ETH_ConfigMACAddress(ETH_MAC_ADDRESS0, SetMACaddr);

    /** Add a network interface to the list of lwIP netifs */
    netif_add(&UserNetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

    /** Registers the default network interface */
    netif_set_default(&UserNetif);

    /** When the netif is fully configured this function must be called */
    netif_set_up(&UserNetif);

    /** Use Com printf static IP address*/
    printf("\n  Static IP address   \n");
    printf("IP: %d.%d.%d.%d\n", 169, 254, 71, 22);
    printf("NETMASK: %d.%d.%d.%d\n", 255, 255, 255, 0);
    printf("Gateway: %d.%d.%d.%d\n", 169, 254, 71, 1);
}
// SYS_RUN(EthTaskSample);