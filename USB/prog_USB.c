#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdbool.h>

_Bool enum_USB_devices(libusb_context **context, libusb_device_handle **handle);
int config_USB_device(libusb_device_handle **handle, int tab_endpoint_add[], int tab_interface_number[]);
void close_USB_device(int nb_interface, libusb_device_handle **handle);

int main()
{
  printf("\n");
  libusb_context *context;
  libusb_device_handle *handle;
  int status=libusb_init(&context);
  if(status!=0){  perror("[main] libusb_init"); exit(-1);}
  int tab_interface_number[10], tab_endpoint_add[4]={0,0,0,0};

  if (enum_USB_devices(&context, &handle) == false)  {
    printf("[main] Le peripherique attendu n'est pas connecte.\n");
    return 0;
  }

  int nb_interface=config_USB_device(&handle, tab_endpoint_add, tab_interface_number);

  // initialisation des variables necessaires
  // --- nombre max de donnes a envoyer ou recevoir
  // --- buffer pour envoyer ou recevoir les donnees
  // --- taille max pour envoyer ou recevoir les donnees (1 octet suffit)
  // --- timeout (ms)
  // --- sauvegarde du caractere entré par l'utilisateur et caractere pour vidage
  // --- stocke le nombre de bits transferes
  int MAX_DATA = 1;
  unsigned char data[MAX_DATA], data1[MAX_DATA], data2[MAX_DATA];
  int size = 1;
  int timeout=20000;
  char caractere='a', c;
  int nb_bits_trans;

  /* PLUS NECESSAIRE (Q 4.4)
  // bit 7 = direction du transfert: on compare les adresses enregistrees pour
  // savoir lequel est le endpoint d'entree et lequel est celui de sortie
  // bit 7 = 1  -> endpoint d'entree
  // sinon      -> endpoint sortie
  printf("[main] Tableau des endpoints interruption sauvés : tab_endpoint_add\n");
  for (int i=0; i<4; i++)  {
    printf("[main]\ttab_endpoint_add[%d]=%d\n", i, tab_endpoint_add[i]);
    if((tab_endpoint_add[i]&0x80)==0x80)  {
      printf("[main]\t\t-> endpoint IN_1\n");
      int endpoint_IN_1=tab_endpoint_add[i];
    }
    else {
      printf("[main]\t\t-> endpoint OUT_1\n");
      int endpoint_OUT_1=tab_endpoint_add[i];
    }
  } */

  int endpoint_IN_1=tab_endpoint_add[0];
  int endpoint_IN_2=tab_endpoint_add[1];
  int endpoint_OUT_1=tab_endpoint_add[2];
  int endpoint_OUT_2=tab_endpoint_add[3];

  // ALLUMER / ETEINDRE LED L
  // tant que l'utilisateur ne tape pas o ou p on redemande
  while((caractere!='o') && (caractere!='p')) {
    printf("[main]\tAppuyez sur o pour allumer la led\n\t\t ou p pour l'eteindre\n");
    scanf("%c", &caractere);
    // on vide l'entree standart (les \n qui peuvent trainer par ex)
    while((c=getchar()!='\n') && (c!=EOF)) {}
    if (caractere=='o') {
      data[0]='1';
      status=libusb_interrupt_transfer(handle,endpoint_OUT_1,data,size,&nb_bits_trans,timeout);
      if((status!=0) && (status!=-7))  { perror("[main] libusb_interrupt_transfer"); printf("[main]  status=%d\n",status);}
    }
    if (caractere=='p') {
      data[0]='0';
      status=libusb_interrupt_transfer(handle,endpoint_OUT_1,data,size,&nb_bits_trans,timeout);
      if((status!=0) && (status!=-7)) { perror("[main] libusb_interrupt_transfer"); printf("[main] status=%d\n",status);}
    }
  }

  // RECEPTION DES CARACTERES: affichage des boutons pressés ou relachés jusqu’à pression sur le bouton d’arrêt
  while(1)  {
    int status=libusb_interrupt_transfer(handle,endpoint_IN_1,data1,size,&nb_bits_trans,timeout);
    int status2=libusb_interrupt_transfer(handle,endpoint_IN_2,data2,size,&nb_bits_trans,timeout);
    if((status!=0) & (status2!=0)){ perror("[main] libusb_interrupt_transfer"); printf("[main] status=%d\t status2=%d\n",status, status2);}
    else  {
      unsigned char data_joy = (unsigned char) data1[0];
      data_joy = data_joy << 5;
      data_joy |= 0x20;
      data_joy &= (unsigned char) data2[0];
      printf("[main] Caractère reçu : %c\n",data_joy);
    }
  }

  close_USB_device(nb_interface, &handle);

  // Fermeture du contexte
  libusb_exit(context);
  return 0;
}
