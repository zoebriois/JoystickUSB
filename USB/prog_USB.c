#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdbool.h>

_Bool enum_USB_devices(libusb_context **context, libusb_device_handle **handle);
int config_USB_device(libusb_device_handle **handle, int tab_endpoint_add[], int tab_interface_number[]);
void close_USB_device(int nb_interface, libusb_device_handle **handle);

int main()
{
  libusb_context *context;
  libusb_device_handle *handle;
  int status=libusb_init(&context);
  if(status!=0)
  {perror("[main] libusb_init"); exit(-1);}
  int tab_interface_number[10], tab_endpoint_add[10]={0,0,0,0,0,0,0,0,0,0};

  if (enum_USB_devices(&context, &handle) == false)  {
    printf("[main] Le peripherique attendu n'est pas connecte.\n");
    return 0;
  }

  int nb_interface=config_USB_device(&handle, tab_endpoint_add, tab_interface_number);

  printf("[main] Affichage du tableau tab_endpoint_add\n");

  for (int i=0; i<10; i++)  {
    printf("[main] tab_endpoint_add[%d]=%d\n", i, tab_endpoint_add[i]);
  }
  // QUESTION 4.4. gestion de la manette
  // initialisation des variables necessaires
  // --- nombre max de donnes a envoyer ou recevoir
  // --- buffer pour envoyer ou recevoir les donnees
  // --- taille max pour envoyer ou recevoir les donnees (1 octet suffit)
  // --- timeout (ms)
  // --- endpoints
  // --- sauvegarde du caractere entré par l'utilisateur et caractere pour vidage
  // --- stocke le nombre de bits transferes
  int MAX_DATA = 1;
  unsigned char data[MAX_DATA];
  int size = 1;
  int timeout=10000;
  int endpoint_IN, endpoint_OUT;
  char caractere='b', c;
  int nb_bits_trans;

  // bit 7 = direction du transfert: on compare les adresses enregistrees pour
  // savoir lequel est le endpoint d'entree et lequel est celui de sortie
  // bit 7 = 1  -> endpoint d'entree
  // sinon      -> endpoint sortie
  if((tab_endpoint_add[0]&0x80)==0x80)  {
		endpoint_IN=tab_endpoint_add[0];
	}
	else {
		endpoint_OUT=tab_endpoint_add[0];
	}

	if((tab_endpoint_add[1]&0x80)==0x80) {
		endpoint_IN=tab_endpoint_add[1];
	}
	else {
		endpoint_OUT=tab_endpoint_add[1];
	}
  printf("[main] On obtient: endpoint_OUT= %d \t endpoint_IN= %d\n", endpoint_OUT, endpoint_IN);
  // ALLUMER / ETEINDRE LED
/*
  // tant que l'utilisateur ne tape pas a ou e on redemande
  while((caractere!='a') && (caractere!='e')) {
    printf("[main]\tAppuyez sur a pour allumer la led\n\t\t ou e pour l'eteindre");
    scanf("%c", &caractere);
    // on vide l'entree standart (les \n qui peuvent trainer par ex)
    while((c=getchar()!='\n') && (c!=EOF)) {}
    // pour allumer
    if (caractere =='a'){
      data[0]='1';
      printf("[main] on a allume la LED\n");
      status=libusb_interrupt_transfer(&handle,endpoint_OUT,data,size,&nb_bits_trans,timeout);
			if(status!=0)  {
        perror("[main] libusb_interrupt_transfer");
      }
    }
    // pour eteindre
    if (caractere == 'e'){
      data[0]='0';
      printf("[main] on a eteint la LED\n");
      status=libusb_interrupt_transfer(&handle,endpoint_OUT,data,size,&nb_bits_trans,timeout);
      if(status!=0)  {
        perror("[main] libusb_interrupt_transfer");
      }
    }
  }
*/
  // RECEPTION DES CARACTERES: affichage des boutons pressés ou relachés jusqu’à pression sur le bouton d’arrêt
  while(1)  {
  		status=libusb_interrupt_transfer(handle,endpoint_IN,data,size,&nb_bits_trans,timeout);
  		if(status!=0){ perror("[main] libusb_interrupt_transfer");}
  		else  {
  			printf("[main] Caractère reçu : %c\n",data[0]);
  		}
  	}


  close_USB_device(nb_interface, &handle);

  // Fermeture du contexte
  libusb_exit(context);
  return 0;
}
