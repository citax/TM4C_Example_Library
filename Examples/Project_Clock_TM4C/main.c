#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint16_t Second;
uint32_t SysClock;

void delay_with_nop(uint32_t count) {
    while(count--) {
        __asm("NOP");
    }
}

int main(void)
{

    SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                           SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    SysClock = SysCtlClockGet();

    while(1){

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);

        SysCtlDelay(120000000);

        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00);

        SysCtlDelay(120000000);

        Second++;

    }

}
