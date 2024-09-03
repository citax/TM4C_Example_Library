#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/lwiplib.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "drivers/pinout.h"
#include "lwip/udp.h"




//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Ethernet with lwIP (enet_lwip)</h1>
//!
//! This example application demonstrates the operation of the Tiva
//! Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain
//! an Ethernet address.  If DHCP times out without obtaining an address,
//! AutoIP will be used to obtain a link-local address.  The address that is
//! selected will be shown on the UART.
//!
//! UART0, connected to the ICDI virtual COM port and running at 115,200,
//! 8-N-1, is used to display messages from this application. Use the
//! following command to re-build the any file system files that change.
//!
//!     ../../../../tools/bin/makefsfile -i fs -o enet_fsdata.h -r -h -q
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! savannah.nongnu.org/.../
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

//*****************************************************************************
//
// The current IP address.
//
//*****************************************************************************
uint32_t g_ui32IPAddress;

//*****************************************************************************
//
// The system clock frequency.
//
//*****************************************************************************
uint32_t g_ui32SysClock;

//*****************************************************************************
//
// Volatile global flag to manage LED blinking, since it is used in interrupt
// and main application.  The LED blinks at the rate of SYSTICKHZ.
//
//*****************************************************************************
volatile bool g_bLED;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


void udp_echo_init( void );

//*****************************************************************************
//
// Display an lwIP type IP Address.
//
//*****************************************************************************
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    //
    // Convert the IP Address into a string.
    //
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    //
    // Display the string.
    //
    UARTprintf(pcBuf);
}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void
lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    //
    // Get the current IP address.
    //
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    //
    // See if the IP address has changed.
    //
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        //
        // See if there is an IP address assigned.
        //
        if(ui32NewIPAddress == 0xffffffff)
        {
            //
            // Indicate that there is no link.
            //
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            //
            // Display the new IP address.
            //
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTprintf("\nEcho Server is ready.\n");
        }

        //
        // Save the new IP address.
        //
        g_ui32IPAddress = ui32NewIPAddress;
    }

    //
    // If there is not an IP address.
    //
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        //
        // Do nothing and keep waiting.
        //
    }
}

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(SYSTICKMS);

    //
    // Tell the application to change the state of the LED (in other words
    // blink).
    //
    g_bLED = true;
}

//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

    //
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    //
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    //
    // Configure the device pins.
    //
    //*****************************************************************************
    //
    //! Configures the device pins for the standard usages on the EK-TM4C1294XL.
    //!
    //! \param bEthernet is a boolean used to determine function of Ethernet pins.
    //! If true Ethernet pins are  configured as Ethernet LEDs.  If false GPIO are
    //! available for application use.
    //! \param bUSB is a boolean used to determine function of USB pins. If true USB
    //! pins are configured for USB use.  If false then USB pins are available for
    //! application use as GPIO.
    //
    //*****************************************************************************
    //    void
    // PinoutSet(bool bEthernet, bool bUSB)
    PinoutSet(true, false);

    //
    // Configure UART.
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);

    //
    // Clear the terminal and print banner.
    //
    UARTprintf("\033[2J\033[H");
    UARTprintf("Ethernet lwIP udp echo example\n\n");

    //
    // Configure Port N1 for as an output for the animation LED.
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    //
    // Initialize LED to OFF (0)
    //
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~GPIO_PIN_1);

    //
    // Configure SysTick for a periodic interrupt.
    //
    MAP_SysTickPeriodSet(g_ui32SysClock / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        //
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }

    //
    // Tell the user what we are doing just now.
    //
    UARTprintf("Waiting for IP.\n");

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    //
    // Initialize the lwIP library, using DHCP.
    //

    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);



    //
    // Initialize the echo server.
    //Run the udpSendReceive Linux or Windows executable that is shipped with TI-RTOS.
    //The executable is found in:

      //<tirtos_install_dir>\packages\examples\tools\udpSendReceive

      //Usage: ./udpSendReceive <IP-addr> <port> <id> -l[length] -s[sleep in uS]

      //<IP-addr> is the IP address
      //<port>    is the UDP port being listened to (23)
      //<id>      is a unique id for the executable. Printed out when 1000 packets are
      //          transmitted. It allows the user to run multiple instances
      //          of udpSendReceive.

      //Optional:
      //  -l[length]      size of the packet in bytes. Default is 1024 bytes.
      //  -s[sleep in uS] usleep time to between sends. Default is 1000 uSecs.

      //Example:
      //      udpSendReceive 192.168.1.5 23 1 -s100

    //Messages such as the following will begin to appear on the terminal window when
    //a UDP packet has been echoed back:

    //        Starting test with a 1000 uSec delay between transmits
    //        [id 1] count = 1000, time = 12
    //        [id 1] count = 2000, time = 24
    //        [id 1] count = 3000, time = 36
    //
    udp_echo_init();

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    //
    // Loop forever, processing the LED blinking.  All the work is done in
    // interrupt handlers.
    //
    while(1)
    {
        //
        // Wait till the SysTick Interrupt indicates to change the state of the
        // LED.
        //
        while(g_bLED == false)
        {
        }

        //
        // Clear the flag.
        //
        g_bLED = false;

        //
        // Toggle the LED.
        //
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                         (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) ^
                          GPIO_PIN_1));
    }




}

void udp_echo_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct
ip_addr *addr, u16_t port)
{
    if (p != NULL) {
        /* send received packet back to sender */
        udp_sendto(pcb, p, addr, port);
        /* free the pbuf */
        pbuf_free(p);
    }
}


void udp_echo_init(void)
{
    struct udp_pcb * pcb;

    /* get new pcb */
    pcb = udp_new();
    if (pcb == NULL) {
        LWIP_DEBUGF(UDP_DEBUG, ("udp_new failed!\n"));
        return;
    }

    /* bind to any IP address on port 23 */
    if (udp_bind(pcb, IP_ADDR_ANY, 23) != ERR_OK) {
        LWIP_DEBUGF(UDP_DEBUG, ("udp_bind failed!\n"));
        return;
    }

    /* set udp_echo_recv() as callback function
       for received packets */
    udp_recv(pcb, udp_echo_recv, NULL);
}
