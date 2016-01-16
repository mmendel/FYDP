//#############################################################################
//
//  File:   FYDP_C2000.c
//
//  Title:  Initail deXassist system.
//
//
//#############################################################################
// deXassist
//
//#############################################################################

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#include "f2802x_common/include/adc.h"
#include "f2802x_common/include/clk.h"
#include "f2802x_common/include/flash.h"
#include "f2802x_common/include/gpio.h"
#include "f2802x_common/include/pie.h"
#include "f2802x_common/include/pll.h"
#include "f2802x_common/include/pwm.h"
#include "f2802x_common/include/sci.h"
#include "f2802x_common/include/wdog.h"


// Prototype statements for functions found within this file.
__interrupt void sciaRxFifoIsr(void);
void adc_init(void);
void pwm_init(void);
void scia_init(void);
void scia_fifo_init(void);
void scia_xmit(int a);
void scia_msg(char * msg);
void scia_float_xmit(float32 f);

// Global variables
ADC_Handle myAdc;
CLK_Handle myClk;
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;
SCI_Handle mySci;
PWM_Handle myPwm1;

volatile uint16_t commandData [5] = {0,0,0,0,0};

void main(void)
{
    CPU_Handle myCpu;
    PLL_Handle myPll;
    WDOG_Handle myWDog;
    uint16_t adc_val;
    float32 volt;

    // Initialize all the handles needed for this application
    myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    mySci = SCI_init((void *)SCIA_BASE_ADDR, sizeof(SCI_Obj));
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));

    // Perform basic system initialization
    WDOG_disable(myWDog);
    CLK_enableAdcClock(myClk);
    (*Device_cal)();

    //Select the internal oscillator 1 as the clock source
    CLK_setOscSrc(myClk, CLK_OscSrc_Internal);

    // Setup the PLL for x12 /2 which will yield 50Mhz = 10Mhz * 10 / 2
    PLL_setup(myPll, PLL_Multiplier_12, PLL_DivideSelect_ClkIn_by_2);

    // Disable the PIE and all interrupts
    PIE_disable(myPie);
    PIE_disableAllInts(myPie);
    CPU_disableGlobalInts(myCpu);
    CPU_clearIntFlags(myCpu);

// If running from flash copy RAM only functions to RAM
#ifdef _FLASH
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

    // Initialize GPIO
    GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);
    GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_EPWM1A);
    GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_ASync);
    GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_SCIRXDA);
    GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_SCITXDA);

    // Setup a debug vector table and enable the PIE
    PIE_setDebugIntVectorTable(myPie);
    PIE_enable(myPie);

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
    EALLOW;    // This is needed to write to EALLOW protected registers
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
    ((PIE_Obj *)myPie)->SCIRXINTA = &sciaRxFifoIsr;
    EDIS;   // This is needed to disable write to EALLOW protected registers

    // Register interrupt handlers in the PIE vector table
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_9, PIE_SubGroupNumber_1,
                              (intVec_t)&sciaRxFifoIsr);

    //Init SCIA
    scia_init();        // Init SCI-A
    scia_fifo_init();   // Init SCI-A Fifos

    //Init ADC
    adc_init();

	//Init PWM
    pwm_init();

    // Enable interrupts required for this example
    PIE_enableInt(myPie, PIE_GroupNumber_9, PIE_InterruptSource_SCIARX);

    CPU_enableInt(myCpu, CPU_IntNumber_9);
    CPU_enableGlobalInts(myCpu);

	scia_msg("\r\nWelcome to the deXassist system");
	scia_msg("\r\nRemember as always, safety is of utmost importance :)");

	for(;;)
    {
    	DELAY_US(500000);

        //Force start of conversion on SOC0
        ADC_forceConversion(myAdc, ADC_SocNumber_0);
        //Wait for end of conversion.
        while(ADC_getIntStatus(myAdc, ADC_IntNumber_1) == 0) {
        }

        // Clear ADCINT1
        ADC_clearIntFlag(myAdc, ADC_IntNumber_1);

        //Get ADC val
        adc_val =  ADC_readResult(myAdc, ADC_ResultNumber_0);

        volt = (3.3/4095)*adc_val;
        //scia_xmit(adc_val);
        scia_float_xmit(volt);
    }
}

__interrupt void sciaRxFifoIsr(void)
{
	//Command Strucutre is as follows
	//BYTE 1 = ':' -> Beggining of command
	//BYTE 2 = Command ID
	//BYTE 3 = Finger (Upper 4 Bits) Joint (lower 4 Bits)
	//BYTE 4 = Value (uint)
    if(SCI_getRxFifoStatus(mySci) != SCI_FifoLevel_Empty)
    {
    	commandData[0] = SCI_getData(mySci) == ':';
    	commandData[1] = SCI_getData(mySci);
    	commandData[2] = SCI_getData(mySci);
    	commandData[3] = commandData[2] & 0xF;
    	commandData[2] = commandData[2] >> 4;
    	commandData[4] = SCI_getData(mySci);
    }

    // Clear Overflow flag
    SCI_clearRxFifoOvf(mySci);

    // Clear Interrupt flag
    SCI_clearRxFifoInt(mySci);

    // Issue PIE ack
    PIE_clearInt(myPie, PIE_GroupNumber_9);

    return;
}

void adc_init()
{
	ADC_enableBandGap(myAdc);
	ADC_enableRefBuffers(myAdc);
	ADC_powerUp(myAdc);
	ADC_enable(myAdc);
	ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);

	ADC_enableInt(myAdc, ADC_IntNumber_1);                                  		//Enable ADCINT1
	ADC_setIntMode(myAdc, ADC_IntNumber_1, ADC_IntMode_ClearFlag);          		//Disable ADCINT1 Continuous mode
	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_0, ADC_SocTrigSrc_Sw);    				//set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_0, ADC_SocChanNumber_A4);    		//Set SOC0 channel select to ADCINA5
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_0, ADC_SocSampleWindow_7_cycles);   //Set SOC0 acquisition period to 7 ADCCLK
}

void pwm_init()
{
    CLK_disableTbClockSync(myClk);

    CLK_enablePwmClock(myClk, PWM_Number_1);

    // Setup TBCLK
    PWM_setCounterMode(myPwm1, PWM_CounterMode_Up);         // Count up
    PWM_setPeriod(myPwm1, 2000);              			    // Set timer period
    PWM_disableCounterLoad(myPwm1);                         // Disable phase loading
    PWM_setPhase(myPwm1, 0x0000);                           // Phase is 0
    PWM_setCount(myPwm1, 0x0000);                           // Clear counter
    PWM_setHighSpeedClkDiv(myPwm1, PWM_HspClkDiv_by_2);     // Clock ratio to SYSCLKOUT
    PWM_setClkDiv(myPwm1, PWM_ClkDiv_by_2);

    // Set Compare values
    PWM_setCmpA(myPwm1, 400);    // Set compare A value

    // Set actions
    PWM_setActionQual_Zero_PwmA(myPwm1, PWM_ActionQual_Set);            // Set PWM1A on Zero
    PWM_setActionQual_CntUp_CmpA_PwmA(myPwm1, PWM_ActionQual_Clear);    // Clear PWM1A on event A, up count
    CLK_enableTbClockSync(myClk);
}

void scia_init()
{
    CLK_enableSciaClock(myClk);

    // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    SCI_disableParity(mySci);
    SCI_setNumStopBits(mySci, SCI_NumStopBits_One);
    SCI_setCharLength(mySci, SCI_CharLength_8_Bits);

    // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    SCI_enableTx(mySci);
    SCI_enableRx(mySci);
    SCI_enableTxInt(mySci);
    SCI_enableRxInt(mySci);

    //SCI_enableLoopBack(mySci);

    // SCI BRR = LSPCLK/(SCI BAUDx8) - 1
#if (CPU_FRQ_60MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)194);
#endif
#if (CPU_FRQ_50MHZ)
    SCI_setBaudRate(mySci, SCI_BaudRate_9_6_kBaud);
#elif (CPU_FRQ_40MHZ)
    SCI_setBaudRate(mySci, (SCI_BaudRate_e)129);
#endif

    SCI_enable(mySci);

    return;
}

void scia_fifo_init()
{
    SCI_enableFifoEnh(mySci);
    SCI_resetTxFifo(mySci);
    SCI_clearTxFifoInt(mySci);
    SCI_resetChannels(mySci);
    SCI_setTxFifoIntLevel(mySci, SCI_FifoLevel_Empty);

    SCI_resetRxFifo(mySci);
    SCI_clearRxFifoInt(mySci);
    SCI_setRxFifoIntLevel(mySci, SCI_FifoLevel_4_Words);
    SCI_enableRxFifoInt(mySci);

    return;
}

void scia_xmit(int a)
{
    while(SCI_getTxFifoStatus(mySci) != SCI_FifoStatus_Empty)
    {
    }

    SCI_putDataBlocking(mySci, a);
}

void scia_msg(char * msg)
{
    int i;
    i = 0;
    while(msg[i] != '\0')
    {
        scia_xmit(msg[i]);
        i++;
    }
}

void scia_float_xmit(float f)
{
	int *p;
	char a1,a2,a3,a4;
    p = (int *)&f;

    a1=__byte(p,3); // HIGH byte - byte 4

    a2=__byte(p,2); // byte 3

    a3=__byte(p,1); // byte 2

    a4=__byte(p,0); // LOW byte - byte 1
    scia_msg(":");
    scia_xmit(a1);
    scia_xmit(a2);
    scia_xmit(a3);
    scia_xmit(a4);
    scia_msg("\r\n");


}


//===========================================================================
// No more.
//===========================================================================

