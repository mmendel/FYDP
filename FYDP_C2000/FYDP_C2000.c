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

#include "sma.h"
#include "sma_controller.h"
#include "LED.h"

// Prototype statements for functions found within this file.
__interrupt void sciaRxFifoIsr(void);
__interrupt void deadman_isr(void);

void adc_init(void);
void pwm_init(PWM_Handle myPwm);
void scia_init(void);
void scia_fifo_init(void);
void scia_xmit(int a);
void scia_msg(char * msg);
void scia_float_xmit(float32 f);
void smaUpdate(SMA *sma);
void processCommand();
void setPWM(unsigned char pin, float dutypercent);

// Global variables
ADC_Handle myAdc;
CLK_Handle myClk;
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;
SCI_Handle mySci;
PWM_Handle myPwm1, myPwm2, myPwm3, myPwm4;

volatile uint16_t commandData [5] = {0,0,0,0,0};
volatile bool running = false;
volatile bool deadman = false;
Finger hand[5];
uint16_t adc_val;
float32 angle;
float32 duty_cycle;


#define SAMPLE_TIME 10000//500000
#define CHECK_COMMAND_FREQ 2
#define PWM_PER 2000
#define PWM_DUTY_PERCENT_TO_TICK(x) (x*PWM_PER)/100
#define PWM_DUTY_TICK_TO_PERCENT(x) (x*100)/PWM_PER
#define ADC_TICKS_TO_VOLT(x) (3.3/4095)*x


//Commands
typedef enum
{
	STOPALL = 1,
	STARTALL,
	SETFINGER,
	GETFINGER,
	STREAMSTART,
	STREAMSTOP
} Commands;


void main(void)
{
	uint16_t i;
	uint32_t j;
    CPU_Handle myCpu;
    PLL_Handle myPll;
    WDOG_Handle myWDog;

    // Initialize all the handles needed for this application
    myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myPwm1 = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
    myPwm2 = PWM_init((void *)PWM_ePWM2_BASE_ADDR, sizeof(PWM_Obj));
    myPwm3 = PWM_init((void *)PWM_ePWM3_BASE_ADDR, sizeof(PWM_Obj));
    myPwm4 = PWM_init((void *)PWM_ePWM4_BASE_ADDR, sizeof(PWM_Obj));
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

    // PWM GPIO
    GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_1, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_3, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_4, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_5, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_6, GPIO_PullUp_Disable);
    GPIO_setPullUp(myGpio, GPIO_Number_7, GPIO_PullUp_Disable);

    GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_EPWM1A);
    GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_EPWM1B);
    GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_EPWM2A);
    GPIO_setMode(myGpio, GPIO_Number_3, GPIO_3_Mode_EPWM2B);
    GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_EPWM3A);
    GPIO_setMode(myGpio, GPIO_Number_5, GPIO_5_Mode_EPWM3B);
    GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_EPWM4A);
    GPIO_setMode(myGpio, GPIO_Number_7, GPIO_7_Mode_EPWM4B);

    //SCI GPIO
    GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);
    GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
    GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_ASync);
    GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_SCIRXDA);
    GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_SCITXDA);

    //Deadman
    GPIO_setPullUp(myGpio, GPIO_Number_12, GPIO_PullUp_Enable);
    GPIO_setMode(myGpio, GPIO_Number_12, GPIO_12_Mode_GeneralPurpose);
    GPIO_setDirection(myGpio, GPIO_Number_12, GPIO_Direction_Input);
    GPIO_setQualification(myGpio, GPIO_Number_12, GPIO_Qual_Sample_6);
    GPIO_setQualificationPeriod(myGpio, GPIO_Number_12, 0xFF);
    GPIO_setExtInt(myGpio, GPIO_Number_12, CPU_ExtIntNumber_1);

    //LED
    ConfigureLeds(myGpio);
    SetLedState(POWER, ON);


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

    //Deadman Interrupt
    PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_4, (intVec_t)&deadman_isr);
    PIE_enableInt(myPie, PIE_GroupNumber_1, PIE_InterruptSource_XINT_1);
    CPU_enableInt(myCpu, CPU_IntNumber_1);
    PIE_setExtIntPolarity(myPie, CPU_ExtIntNumber_1, PIE_ExtIntPolarity_RisingAndFallingEdge);
    PIE_enableExtInt(myPie, CPU_ExtIntNumber_1);


    //Init SCIA
    scia_init();        // Init SCI-A
    scia_fifo_init();   // Init SCI-A Fifos

    //Init ADC
    adc_init();

	//Init PWM
    CLK_disableTbClockSync(myClk);
    CLK_enablePwmClock(myClk, PWM_Number_1);
    CLK_enablePwmClock(myClk, PWM_Number_2);
    CLK_enablePwmClock(myClk, PWM_Number_3);
    CLK_enablePwmClock(myClk, PWM_Number_4);
    CLK_enablePwmClock(myClk, PWM_Number_5);
    CLK_enablePwmClock(myClk, PWM_Number_6);
    CLK_enablePwmClock(myClk, PWM_Number_7);
    pwm_init(myPwm1);
    pwm_init(myPwm2);
    pwm_init(myPwm3);
    pwm_init(myPwm4);
    CLK_enableTbClockSync(myClk);


    // Enable interrupts required for this example
    PIE_enableInt(myPie, PIE_GroupNumber_9, PIE_InterruptSource_SCIARX);

    CPU_enableInt(myCpu, CPU_IntNumber_9);
    CPU_enableGlobalInts(myCpu);

    InitializeHand(hand);


	scia_msg("\r\nWelcome to the deXassist system");
	scia_msg("\r\nRemember as always, safety is of utmost importance :)");

	for(;;)
    {

		for (j = 0; j < SAMPLE_TIME/CHECK_COMMAND_FREQ; j++)
    	{
        	//If there is a waiting command process it
    		if (commandData[0] == 1)
    			processCommand();

    		DELAY_US(CHECK_COMMAND_FREQ);
    	}
    	if (running == false)// || deadman == false)
    		continue;

    	//Configure for only one finger
    	for (i = 0; i < 1; i ++)
    	{
    		//smaUpdate(hand[i].pip);
    		smaUpdate(hand[i].mcpv);
    		//Disable the MCP joint
    		//smaUpdate(hand[i].mcph);
    	}
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

__interrupt void deadman_isr(void)
{
	uint16_t value = GPIO_getData(myGpio, GPIO_Number_12);

	if (value == 0)
	{
		deadman = true;
		SetLedState(DEADMAN, ON);
	}
	else
	{
		deadman = false;
		SetLedState(DEADMAN, OFF);
	}

    PIE_clearInt(myPie, PIE_GroupNumber_1);
}

void adc_init()
{
	ADC_enableBandGap(myAdc);
	ADC_enableRefBuffers(myAdc);
	ADC_powerUp(myAdc);
	ADC_enable(myAdc);
	ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);

    ADC_setIntSrc(myAdc, ADC_IntNumber_1, ADC_IntSrc_EOC1);                 //Connect ADCINT1 to EOC1
	ADC_setIntMode(myAdc, ADC_IntNumber_1, ADC_IntMode_ClearFlag);          		//Disable ADCINT1 Continuous mode
	ADC_enableInt(myAdc, ADC_IntNumber_1);                                  		//Enable ADCINT1

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_0, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_0, ADC_SocChanNumber_A1);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_0, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_1, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_1, ADC_SocChanNumber_A2);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_1, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_2, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_2, ADC_SocChanNumber_A3);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_2, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_3, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_3, ADC_SocChanNumber_A4);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_3, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_4, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_4, ADC_SocChanNumber_A6);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_4, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_5, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_5, ADC_SocChanNumber_A7);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_5, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_6, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_6, ADC_SocChanNumber_B1);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_6, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_7, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_7, ADC_SocChanNumber_B2);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_7, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_8, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_8, ADC_SocChanNumber_B3);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_8, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_9, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_9, ADC_SocChanNumber_B4);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_9, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_10, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_10, ADC_SocChanNumber_B6);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_10, ADC_SocSampleWindow_7_cycles);

	ADC_setSocTrigSrc(myAdc, ADC_SocNumber_11, ADC_SocTrigSrc_Sw);
	ADC_setSocChanNumber (myAdc, ADC_SocNumber_12, ADC_SocChanNumber_B7);
	ADC_setSocSampleWindow(myAdc, ADC_SocNumber_13, ADC_SocSampleWindow_7_cycles);
}

void pwm_init(PWM_Handle myPwm)
{
    // Setup TBCLK
    PWM_setCounterMode(myPwm, PWM_CounterMode_Up);         // Count up
    PWM_setPeriod(myPwm, PWM_PER);              			// Set timer period
    PWM_disableCounterLoad(myPwm);                         // Disable phase loading
    PWM_setPhase(myPwm, 0x0000);                           // Phase is 0
    PWM_setCount(myPwm, 0x0000);                           // Clear counter
    PWM_setHighSpeedClkDiv(myPwm, PWM_HspClkDiv_by_2);     // Clock ratio to SYSCLKOUT
    PWM_setClkDiv(myPwm, PWM_ClkDiv_by_2);

    // Set Compare values
    PWM_setCmpA(myPwm, 0);    // Set compare B value
    PWM_setCmpB(myPwm, 0);    // Set compare B value

    // Set actions
    PWM_setActionQual_Zero_PwmA(myPwm, PWM_ActionQual_Set);            // Set PWM1A on Zero
    PWM_setActionQual_CntUp_CmpA_PwmA(myPwm, PWM_ActionQual_Clear);    // Clear PWM1A on event A, up count
    PWM_setActionQual_Zero_PwmB(myPwm, PWM_ActionQual_Set);            // Set PWM1B on Zero
    PWM_setActionQual_CntUp_CmpB_PwmB(myPwm, PWM_ActionQual_Clear);    // Clear PWM1B on event B, up count
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

void smaUpdate(SMA *sma)
{
    //Force start of conversion
    ADC_setIntSrc(myAdc, ADC_IntNumber_1, sma->uADCPin);                 //Connect ADCINT1 to EOC1

    //ADC_forceConversion(myAdc, ADC_SocNumber_0);
    ADC_forceConversion(myAdc, sma->uADCPin);
    //Wait for end of conversion.
    while(ADC_getIntStatus(myAdc, ADC_IntNumber_1) == 0) {
    }
    // Clear ADCINT1
    ADC_clearIntFlag(myAdc, ADC_IntNumber_1);
    //Get ADC val
    adc_val =  ADC_readResult(myAdc, sma->uADCPin);

    switch (sma->jointId)
    {
    	case PIP:
    	    angle = VOLTAGE_TO_ANGLE_PIP(ADC_TICKS_TO_VOLT(adc_val));
    		break;
    	case MCP_VERTICAL:
    	    angle = VOLTAGE_TO_ANGLE_MCP(ADC_TICKS_TO_VOLT(adc_val));
    		break;
    	default:
    	    angle = VOLTAGE_TO_ANGLE(ADC_TICKS_TO_VOLT(adc_val));
    		break;

    }

    if (sma->stream == true)
    	scia_float_xmit(angle);

    //LedPosistion(angle);

    duty_cycle = ControllerStep(sma, angle);
    setPWM(sma->uPwm, duty_cycle);
}

void setPWM(unsigned char pin, float dutypercent)
{
	if (dutypercent < 0)
		dutypercent = 0;
	else if (dutypercent > 100)
		dutypercent = 100;

	uint16_t ticks = PWM_DUTY_PERCENT_TO_TICK(dutypercent);

	switch (pin)
	{
		case 0:
		    PWM_setCmpA(myPwm1, ticks);
			break;
		case 1:
		    PWM_setCmpB(myPwm1, ticks);
			break;
		case 2:
			PWM_setCmpA(myPwm2, ticks);
			break;
		case 3:
			PWM_setCmpB(myPwm2, ticks);
			break;
		case 4:
			PWM_setCmpA(myPwm3, ticks);
			break;
		case 5:
			PWM_setCmpB(myPwm3, ticks);
			break;
		case 6:
			PWM_setCmpA(myPwm4, ticks);
			break;
		case 7:
			PWM_setCmpB(myPwm4, ticks);
			break;
		default:
			break;
	}
}

void processCommand()
{
	uint16_t ref;
	switch (commandData[1])
	{
		case STOPALL:
			running = false;
		    PWM_setCmpA(myPwm1, 0);
		    PWM_setCmpB(myPwm1, 0);
		    PWM_setCmpA(myPwm2, 0);
		    PWM_setCmpB(myPwm2, 0);
		    PWM_setCmpA(myPwm3, 0);
		    PWM_setCmpB(myPwm3, 0);
		    PWM_setCmpA(myPwm4, 0);
		    PWM_setCmpB(myPwm4, 0);
		    SetLedState(RUNNING, OFF);
			break;
		case STARTALL:
			running = true;
			SetLedState(RUNNING, ON);
			break;
		case SETFINGER:
			SetReference(hand, commandData[2], commandData[3], commandData[4]);
			break;
		case GETFINGER:
			ref = GetReference(hand, commandData[2], commandData[3]);
			scia_xmit(ref);
			break;
		case STREAMSTART:
			DataStreamStart(hand, commandData[2], commandData[3]);
			SetLedState(STREAM, ON);
			break;
		case STREAMSTOP:
			DataStreamStop(hand, commandData[2], commandData[3]);
			SetLedState(STREAM, OFF);
			break;
		default:
			break;

	}

	commandData[0] = 0;
}


//===========================================================================
// No more.
//===========================================================================

