#include <sam.h>
#include <stdio.h>
int m;
int c1 = 17972;
int c2 = 40;
// int c1 = 2000;
// int c2=0;
#define P27 1u<<27



volatile int targetweight = 25;

void generate_pump_signal__init__() {
    PMC->PMC_PCER0 = 1u << ID_TC0; // I let TCLK0 pass durch multiplexer

    PIOB->PIO_PER = 1u << 27;  // pin controlled by pio
    PIOB->PIO_OER = 1u << 27;  // pin as output
    PIOB->PIO_CODR = 1u << 27; // pin low
}

void generate_pump_signal() {
    /**
     * Termin 3 Aufgabe 1
     * Generate a signal at the frequency of 50Hz
    */
    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS; // Counter Clock Disable Command

    TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_WAVE |                        // Waveform mode is enabled
                                TC_CMR_WAVSEL_UP_RC |                // UP mode with automatic trigger on RC Compare
                                TC_CMR_TCCLKS_TIMER_CLOCK1 |         // MCK/2
                                TC_CMR_EEVT_XC0 |                    // Use xc0 as external event instead of TIOB so that TIOB can also be used as an output.
                                TC_CMR_BCPB_SET | TC_CMR_BCPC_CLEAR; // Toogle TIAB output on RC Compare

    TC0->TC_CHANNEL[0].TC_RC = SystemCoreClock / 2 / 50; // 50Hz, so we divide by the frequency we want
    TC0->TC_CHANNEL[0].TC_RB = TC0->TC_CHANNEL[0].TC_RC  >> 1;

    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG; // Counter Clock Enable and Software Trigger Command
    PIOB->PIO_PDR = 1u << 27; // three columns, I want to work with column B, not with Parallel IO
    PIOB->PIO_ABSR = 1u << 27; // select peripheral b
}

void scale_initialization() {
    unsigned int cnt;
    float freq_PC25, freq_PC28; // freq1 = PA7; freq2 = PA4
    PMC->PMC_PCER1 = 1u << (ID_TC6 % 32) |  1u << (ID_TC7 % 32);
    PIOC->PIO_PDR = 1u << 25 | 1u << 28;
    PIOC->PIO_ABSR = 1u << 25 | 1u << 28;; // select peripheral b//7

    // timer setup
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;               // Counter Clock Disable
    TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | // Clock selected: internal MCK/2 clock signal (from PMC)
                                TC_CMR_LDRA_RISING |         // Load CV to RA when a rising edge on TIOA occurs
                                TC_CMR_LDRB_RISING |         // Load CV to RB when a rising edge on TIOA occurs (after ra loading occured)
                                TC_CMR_LDBSTOP;              // Counter clock is stopped when RB loading occurs.
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN;                // Counter Clock Enable


    // timer setup
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKDIS;               // Counter Clock Disable
    TC2->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | // Clock selected: internal MCK/2 clock signal (from PMC)
                                TC_CMR_LDRA_RISING |         // Load CV to RA when a rising edge on TIOA occurs
                                TC_CMR_LDRB_RISING |         // Load CV to RB when a rising edge on TIOA occurs (after ra loading occured)
                                TC_CMR_LDBSTOP;              // Counter clock is stopped when RB loading occurs.
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN;                // Counter Clock Enable
}

double calcWeight() {
    volatile int	periodenDauer1, periodenDauer2;

    float freq_PC25, freq_PC28; // freq1 = PA7; freq2 = PA4

    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;       // software trigger
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_SWTRG;       // software trigger

    while (~TC2->TC_CHANNEL[0].TC_SR & TC_SR_LDRBS); // wait until rb loading occurs  lower period
    while (~TC2->TC_CHANNEL[1].TC_SR & TC_SR_LDRBS); // wait until rb loading occurs  higher period

    periodenDauer1 = TC2->TC_CHANNEL[0].TC_RB - TC2->TC_CHANNEL[0].TC_RA;
    freq_PC25 = SystemCoreClock/2/periodenDauer1;

    periodenDauer2 = TC2->TC_CHANNEL[1].TC_RB - TC2->TC_CHANNEL[1].TC_RA;
    freq_PC28 = SystemCoreClock/2/periodenDauer2;

    return (c1 * (freq_PC25 / freq_PC28 - 1) - c2);
}

char* intToString(int n){
    char* output;
    char arr[12]={0};
    output = arr;
    output += 11;
    int neg=0;
    if(n<0){
        neg=1;
        n = (n*-1);
    }
    while(n>0){
        *(--output) = '0' + n % 10;
        n/=10;
    }
    if(neg == 1){
        *(--output) = '-';
    }
    for(int i = 0; output[i] != '\0'; i++)
        printf("%c ", *(output+i));

    return output;
}



void put_char(char character) {
    int counter = 0;

    while(~USART1->US_CSR & US_CSR_TXRDY) {
        counter++;
    }

    USART1->US_THR = character;
}


void put_string(char *string) {
    unsigned int counter = 0;
    while(1) {
        char c = string[counter];
        while(~USART1->US_CSR & US_CSR_TXRDY);
        USART1->US_THR = c;
        if(c == '\000') {
            put_char(0xD);
            put_char(0xA);
            break;
        }
        counter++;
    }
}

void put_string_no_endl(char *string) {
    unsigned int counter = 0;
    while(1) {
        char c = string[counter];
        while(~USART1->US_CSR & US_CSR_TXRDY);
        USART1->US_THR = c;
        if(c == '\000') {
            break;
        }
        counter++;
    }
}


void usart__init__() {
    PMC->PMC_PCER0 = 1u << ID_USART1;

    USART1->US_CR = US_CR_RXDIS | US_CR_TXDIS | US_CR_RSTRX | US_CR_RSTTX; // Receiver and transmitter resetted and disabled
    USART1->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS;                     // Disables the PDC receiver & transmitter channel requests. (DMA)

    USART1->US_MR = US_MR_NBSTOP_1_BIT |   // 1 stop bit
                    US_MR_PAR_NO |         // No parity
                    US_MR_CHRL_8_BIT |     // Character length is 8 bits
                    US_MR_CHMODE_NORMAL |  // Normal channel mode
                    US_MR_USCLKS_MCK;      // Master Clock is selected

    USART1->US_BRGR = SystemCoreClock / 16 / 9600;

    USART1->US_CR = US_CR_RXEN | US_CR_TXEN;         // Receiver and transmitter enabled

    USART1->US_TCR = 0;
    PIOA->PIO_PDR = PIO_PA13A_TXD1 | PIO_PA12A_RXD1;
}
int if_button_pressed1() {
    /* Von Termin 2 Aufgabe 4*/
    /*
    */
    PMC->PMC_PCER0 = 0x01 << ID_PIOD | 1 << ID_PIOB; //Activating the PMC, in order to work with inputs
    PIOB->PIO_OER = P27;
    return ((PIOD->PIO_PDSR & 0x1) != 0x1) ;
}

int if_button_pressed2() {
    /* Von Termin 2 Aufgabe 4*/
    /*
    *  That's also important:
    *
    */
    PMC->PMC_PCER0 = 0x01 << ID_PIOD | 1 << ID_PIOB; //Activating the PMC, in order to work with inputs
    PIOB->PIO_OER = P27;
    return ((PIOD->PIO_PDSR & 0x2) != 0x2) ;
}

double measureMass() {
    unsigned int cnt;
    float freq_PC25, freq_PC28; // freq1 = PA7; freq2 = PA4
    PMC->PMC_PCER1 = 1u << (ID_TC6 % 32) |  1u << (ID_TC7 % 32);
    PIOC->PIO_PDR = 1u << 25 | 1u << 28;
    PIOC->PIO_ABSR = 1u << 25 | 1u << 28;; // select peripheral b//7

    // timer setup
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;               // Counter Clock Disable
    TC2->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | // Clock selected: internal MCK/2 clock signal (from PMC)
                                TC_CMR_LDRA_RISING |         // Load CV to RA when a rising edge on TIOA occurs
                                TC_CMR_LDRB_RISING |         // Load CV to RB when a rising edge on TIOA occurs (after ra loading occured)
                                TC_CMR_LDBSTOP;              // Counter clock is stopped when RB loading occurs.
    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN;                // Counter Clock Enable


    // timer setup
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKDIS;               // Counter Clock Disable
    TC2->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | // Clock selected: internal MCK/2 clock signal (from PMC)
                                TC_CMR_LDRA_RISING |         // Load CV to RA when a rising edge on TIOA occurs
                                TC_CMR_LDRB_RISING |         // Load CV to RB when a rising edge on TIOA occurs (after ra loading occured)
                                TC_CMR_LDBSTOP;              // Counter clock is stopped when RB loading occurs.
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN;                // Counter Clock Enable

    double m;
    double a, b;
    // C28 TIOA 7
    // measurements

    TC2->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;       // software trigger
    TC2->TC_CHANNEL[1].TC_CCR = TC_CCR_SWTRG;       // software trigger

    while (~TC2->TC_CHANNEL[0].TC_SR & TC_SR_LDRBS); // wait until rb loading occurs  lower period
    while (~TC2->TC_CHANNEL[1].TC_SR & TC_SR_LDRBS); // wait until rb loading occurs  higher period

    a = TC2->TC_CHANNEL[0].TC_RB - TC2->TC_CHANNEL[0].TC_RA;
    // freq_PC25 = SystemCoreClock/2/cnt;
    b = TC2->TC_CHANNEL[1].TC_RB - TC2->TC_CHANNEL[1].TC_RA;
    // freq_PC28 = SystemCoreClock/2/cnt;

    // m = c1 * (freq_PC25 / freq_PC28 - 1) - c2;
    m = c1 * (b / a - 1) - c2;

    return m;
}


int average(volatile int *weight_pointer, int weightOfCub){
	
	int i=0, sum=0;
	
	
	for(i;i<5;i++){
		int tmp = measureMass();
		if(tmp <= 2) return 0;
		if(tmp >= (targetweight + weightOfCub)) return -1;
		else{
			sum += tmp;
			*weight_pointer = tmp;
		}
	}
	
    /** add 0.5 to ged the next higher gram (if 0.7g it will cut it off), its better to get a slightly less weight. So we get no overflow */
    // GLOBAL_WEIGHT = sumOfWeights/numberOfMeasurements + 0.5;
	return (sum/5) + 0.5;
}

get_char_from_usart(char sign) {
    while(1) {
        while(~USART1->US_CSR & US_CSR_RXRDY);
        char c = USART1->US_RHR;

        if(c == sign) return c;
    }
}


int main() {


    int Toleranz = 5;


        
    while(1) {

        volatile int weightOfCup    =	0;
        volatile int weight	        =	0;
        volatile int weight2	    =	0; // because of average method

        usart__init__();

        scale_initialization();

        generate_pump_signal__init__();


        put_string("*******************************************");
        put_string("Wilkommen zu Eingebettete Systeme");

        put_string("Kalibrieren Sie den Becher... (Dafuer liegen Sie den Tasse auf die Waage und druecken Sie Taste C auf der Tastatur)");
        get_char_from_usart('C');
        weightOfCup = measureMass();

        put_string("Weight of the Cup: ");
        put_string(intToString(weightOfCup));

        put_string("Druecken Sie P auf der Tastatur, um den Tasse mit Wasser zu fuehlen: ");
        get_char_from_usart('P');


        weight = measureMass();
        if(weight + Toleranz < weightOfCup  - Toleranz) {
            put_string("Cup was taken away. System stopped");
        } else {



            generate_pump_signal();
            
            while((weight = average(&weight2, weightOfCup)) > 0) {
                put_string(intToString(weight));
            }

            if (weight == 0) {
                put_string("UNCOMPLETED! Cup picked up to early");
                put_string_no_endl("NETTO: ");
                put_string_no_endl(intToString(weight2 - weightOfCup));
                put_string("g \n");
            } else {
                put_string_no_endl("NETTO: ");
                put_string_no_endl(intToString(weight2 - weightOfCup));
                put_string("g \n");
            }

                TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS; // Counter Clock Disable Command
                PIOB->PIO_PER = 1; // three columns, I want to work with column B, not with Parallel IO

                put_string("Take the cup away");
                while(measureMass() <= 1);

                for (size_t i = 0; i < 100000; i++);

                // put_string("Take the cup away.");

                // while(measureMass() > 2);
                // {
                //     /* code */
                // }
                

            }
            for (size_t i = 0; i < 100000; i++);
            

            
    }

    return 0;
}

// int main() {

//     volatile int weightOfCup    =	0;
//     volatile int weight	        =	0;
//     int Toleranz = 5;
//     while(1) {

//         usart__init__();

//         scale_initialization();

//         generate_pump_signal__init__();


//         put_string("Welcome to Embedded Systems");

//         put_string("Calibrate the cub... (Dafuer liegen Sie den Tasse auf die Waage und druecken Sie Taste C auf der Tastatur)");
//         get_char_from_usart('C');
//         weightOfCup = measureMass();

//         put_string("Weight of the Cup: ");
//         put_string(intToString(weightOfCup));

//         put_string("Druecken Sie P auf der Tastatur, um den Tasse mit Wasser zu fuehlen: ");
//         get_char_from_usart('P');


//         weight = measureMass();
//         if(weight + Toleranz < weightOfCup  - Toleranz) {
//             put_string("Cup was taken away. System stopped");
//         } else {

//             generate_pump_signal();

//             // int lastFiveMeasurements[5];
//             // lastFiveMeasurements[0] = 0;
//             // lastFiveMeasurements[1] = 0;
//             // lastFiveMeasurements[2] = 0;
//             // lastFiveMeasurements[3] = 0;
//             // lastFiveMeasurements[4] = 0;
//             // int idx = 0;
//             // int avg = 0;
//             while(1) {
//                 weight = measureMass();
//                 if(weight + Toleranz < weightOfCup  - Toleranz) {
//                     put_string("Cup was taken away. System stopped");
//                     break;
//                 }  
//                 if((weight - Toleranz) >= (weightOfCup + targetweight  + Toleranz)) {
//                     put_string("System finished pumping. [FINAL WEIGHT (REAL)]");
//                     put_string(intToString(weight));

//                     for (size_t i = 0; i < 1000000; i++)
//                     {
//                         /* code */
//                     }
                    
//                     break;
//                 }
//                 // lastFiveMeasurements[idx % 5] = weight;
//                 // int sum = 0;
//                 // for(int i = 0; i < 5; i++)
//                 //     sum += lastFiveMeasurements[i];
//                 // avg = sum / 5;
//                 // idx++;

//                 put_string("[CURRENT WEIGHT (REAL)]");
//                 put_string(intToString(weight));

//             }

//             TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS; // Counter Clock Disable Command
//             PIOB->PIO_PER = 1; // three columns, I want to work with column B, not with Parallel IO

//         }

            
//     }

//     return 0;
// }

