/*
 * File:  maestro.c
 * Author: Melanie Samayoa
 *
 * Descripcion: Comunicación SPI, con contador en maestro que incrementa cada segundo y envia
 * dato al escalvo y el esclavo lo muestra en el PORTD.
 */

//Configuracion 1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// Configuracion 2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 1000000      
#define FLAG_SPI 0xFF         

uint8_t regis;          
void setup(void);

void __interrupt() isr (void){
    if(PIR1bits.ADIF){                  
        if(ADCON0bits.CHS == 0){      
            regis = ADRESH;       
        }
        PIR1bits.ADIF = 0;          
    } 
    return;
}
void main(void) {
    setup();
    while(1){
        
        if(ADCON0bits.GO == 0){   
            __delay_us(41);
            ADCON0bits.GO = 1;      
        }

        PORTAbits.RA7 = 1;     
        SSPBUF = regis;    
        while(!SSPSTATbits.BF){}     
        PORTAbits.RA7 = 0;           

        PORTAbits.RA6 = 1;           
        PORTAbits.RA7 = 1;           
        __delay_ms(11);             
        PORTAbits.RA7 = 0;     
       
        SSPBUF = FLAG_SPI;           
        while(!SSPSTATbits.BF){}    
        PORTB = SSPBUF;              
        PORTAbits.RA6 = 0;           
    }
    return;
}

void setup(void){       
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    
    ANSEL = 0b001;              // AN0 como entrada anal?gicas
    ANSELH = 0x00;              // I/O digitales
        
    TRISA = 0b00000001;         // AN0 como entrada
    TRISC = 0b00010000;         // SDI entrada, SCK y SD0 como salida
    PORTCbits.RC4 = 0;
    TRISB = 0x00;               // PORTD como salida
    PORTA = 0x00;               // Limpieza del PORTA
    PORTC = 0x00;               // Limpieza del PORTC
    PORTB = 0x00;               // Limpieza del PORTD

    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de perifericos
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales

    ADCON0bits.ADCS = 0b01;         // Fosc/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selecci?n de canal AN0
    ADCON1bits.ADFM = 0;            // Configuraci?n de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitaci?n del modulo ADC
    __delay_us(40);                 // Display de sample time
        
    SSPCONbits.SSPM = 0b0000;   // SPI Maestro, Reloj -> Fosc/4 (250kbits/s)
    SSPCONbits.CKP = 0;         // Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       

    SSPSTATbits.CKE = 1;        
    SSPSTATbits.SMP = 1;       
    SSPBUF = regis;       
}