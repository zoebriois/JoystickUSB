#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>		        // pour les registres entrees/sorties
#define CPU_FREQ  16000000L   // on considere la frequence du CPU a 16Mhz

/* Fonction d'initialisation du port serie */
void init_serial(int speed) {
  UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;
  UCSR0B = (1<<TXEN0 | 1<<RXEN0);
  UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);
  UCSR0A &= ~(1 << U2X0);
}

/* Fonction qui ecrit sur le port serie */
void send_serial(unsigned char c) {
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

/* Fonction qui lit le port serie (1 octet) */
unsigned char get_serial(void) {
  loop_until_bit_is_set(UCSR0A, RXC0);
  return UDR0;
}

/* Fonction qui verifie si le port serie est disponible */
unsigned char serial_available(void) {
  return (UCSR0A&(1<<RXC0));
}

/* Fonction d'initialisation des sorties: PIN 8 a 13 pour les leds */
void output_init(void)  {
  DDRB |= 0x3F;
}

/* Fonction qui allume les led en fonction de la valeur */
void output_set(unsigned char value){
  int reste, j=0;
  unsigned value_binaire;
  for(int i=value; value >= 1; i++) {
    j++;
    reste = value % 2;
    value = value / 2;
    if (reste == 1) {
      value_binaire == value_binaire | (1u<<(j-1)); // pour mettre le bit n°j de value_binaire a 1
    }
    else {
      value_binaire == value_binaire | ~(1u<<(j-1))); // pour mettre le bit n°j de value_binaire a 1
    }
  }
  PORTB &= value_binaire;
}

/* Fonction d'initialisation des entrees */
void input_init(void){
  DDRD &= 0x83;
  PORTD |= 0x7c;
}

/* Fonction qui regarde si un des boutons a ete appuye */
unsigned char input_get(void){
  return ((PIND&0x7c)!=0x7c)?1:0;
}

int main(void){
  // Initialisations
  init_serial(9600);
  input_init();
  output_init();
  unsigned char recu;
  char_serial=0;
  while(1)
	{
    // Si on recoit quelque chose
    // --- on recupere ce qu'on a recu
    // --- on l'envoi sur le port serie
    if(input_get()) {
      recu = PIND;
      send_serial(recu);
    }

    // Si le port serie est disponible
    // --- on recupere une valeur sur le port serie
    // --- on l'affiche sur les leds
    if (serial_available()) {
      recu=get_serial();
      output_set(value);
    }

    _delay_ms(20);
  }
  return 0;
}
