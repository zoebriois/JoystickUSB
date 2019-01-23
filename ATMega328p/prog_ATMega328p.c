#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>		        // pour les registres entrees/sorties
#include <util/delay.h>       // pour _delay_ms
#define CPU_FREQ  16000000L   // on considere la frequence du CPU a 16Mhz

/* Fonction d'initialisation du port serie */
void init_serial(int speed) {
  UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;
  UCSR0B = (1<<TXEN0 | 1<<RXEN0);
  UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);
  UCSR0A &= ~(1 << U2X0);
}

/* Fonction d'initialisation des entrees */
void input_init(void){
  DDRD &= 0x83;
  PORTD |= 0x7c;
}

/* Fonction d'initialisation des sorties: PIN 8 a 13 pour les leds */
void output_init(void)  {
  DDRB |= 0x3F;         // pin 8 a 13 en sortie
}

/* Fonction qui verifie si le port serie est disponible */
unsigned char serial_available(void) {
  return (UCSR0A&(1<<RXC0));
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

/* Fonction qui allume les led en fonction de la valeur
  (on utilise les 6 bits de poids faible)                 */
unsigned char maj_led(unsigned char data_led, unsigned char data_serial){
  /*
  // si on voulait afficher un score sur les leds
  int reste, j=0;
  unsigned value_binaire;
  for(int i=value; value >= 1; i++) {
    j++;
    reste = value % 2;
    value = value / 2;
    if (reste == 1) {
      value_binaire == (value_binaire | (1u<<(j-1))); // pour mettre le bit n°j de value_binaire a 1
    }
    else {
      value_binaire == (value_binaire | ~(1u<<(j-1))); // pour mettre le bit n°j de value_binaire a 1
    }
  }
  PORTB &= value_binaire;
  */
  int n;
  if (data_serial == 'o') {
    data_serial = 'A';
  }
  else if (data_serial == 'p') {
      data_serial = 'a';
  }
  n= (int) data_serial;
  // utilisation de A à F pour allumer une LED
  if (65 <= n <= 70) {
    data_led |= (1u << (n-65));
  }
  // utilisation de a à f pour éteindre une LED
  if (97 <= n <= 102) {
    data_led &= ~(1u << (n-97));
}
  return data_led;
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
  unsigned char data_button, data_serial, data_led;
  data_led = 0x00;

  while(1)
	{
    // Si on recoit quelque chose
    // --- on recupere ce qu'on a recu
    // --- on conserve uniquement les 5 bits de poids faible
    // --- bits 765 == 001
    // --- on l'envoi sur le port serie
    // --- on a bien :
    //      001 11101 = =     D3
    //      001 11011 = ;     D4
    //      001 10111 = 7     D5
    //      001 01111 = /     D6
    //      001 11110 = >     gros bouton

    if(input_get()) {
      data_button = PIND;
      data_button=data_button>>2;
      data_button=data_button & 0b00111111;
			data_button=data_button | 0b00100000;
      send_serial(data_button);
    }


    // Si le port serie est disponible
    // --- on recupere une valeur sur le port serie
    // --- si ça commence par 0b 0... .... c'est un caractere donc on calcule ce qu'on veut faire sur la led
    // --- sinon c'est directement la valeur des leds
    // --- on allume/eteint les leds en fonction
    if (serial_available()) {
      data_serial=get_serial();
      if ((data_serial&0x80)==0){
        data_led = maj_led(data_led, data_serial);
      }
      else {
        data_led=data_serial;
      }
      PORTB = data_led;
    }
    _delay_ms(20);
  }
  return 0;
}
