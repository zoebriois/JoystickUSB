#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

// informations sur le peripherique recherche
// celui de base de polytech
//#define device_vendor 0x02341
//#define device_id 0x0001

// celui apres modification de l'ATMega16u2
#define device_vendor 0x1604
#define device_id 0x2305

// pour compiler : -l usb-1.0

/*  Fonction qui enumere tout les peripheriques USB connectes a un ordinateur */
_Bool enum_USB_devices(libusb_context **context, libusb_device_handle **handle)
{
  _Bool device_found=false;
  libusb_device **list;
  ssize_t count=libusb_get_device_list(*context,&list);
  if(count<0) { perror("[enum_USB_devices] libusb_get_device_list"); exit(-1);}
  ssize_t i=0;
  // pour chaque peripherique on affiche ses informations
  for(i=0;i<count;i++)  {
    libusb_device *device=list[i];
    struct libusb_device_descriptor desc;
    int status=libusb_get_device_descriptor(device, &desc);
    if(status!=0)
    continue;
    uint8_t bus=libusb_get_bus_number(device);
    uint8_t address=libusb_get_device_address(device);
    printf("[enum_USB_devices] Device Found @ (Bus:Address) %d:%d\n",bus,address);
    printf("[enum_USB_devices] Vendor ID 0x0%x\n",desc.idVendor);
    printf("[enum_USB_devices] Product ID 0x0%x\n",desc.idProduct);

    // si le peripherique est celui cherche
    if(desc.idVendor == device_vendor && desc.idProduct == device_id)    {
      device_found=true;
      printf("[enum_USB_devices] Searched device found @ (Bus:Address) %d:%d\n", bus, address);
      // --- on stocke la poignee
      status=libusb_open(device,handle);
      if(status!=0) { perror("[enum_USB_devices] libusb_open"); exit(-1);}
    }
  }
  libusb_free_device_list(list,1);
  return device_found;
}


/* Fonction de configuration du peripherique detecte
  3 tableaux:
    tab_endpoint_int_add qui stocke l'adresse des endpoints d'interruptions
    tab_interface_number qui stocke le numero de l'interface d'indice i
    tab_nb_endpoint qui stocke le nombre de endpoint pour l'interface d'indice i
*/
int config_USB_device(libusb_device_handle **handle, int tab_endpoint_int_add[], int tab_interface_number[])
{
  // recuperation de la premiere configuration (celle d'indice 0)
  struct libusb_config_descriptor *config0;
  struct libusb_device *device = libusb_get_device(*handle);
  uint8_t indice = 0;
  int status = libusb_get_config_descriptor(device, indice, &config0);
  if(status!=0){ perror("[config_USB_device] libusb_get_config_descriptor");}

  //  affichage de la valeur de cette configuration
  printf("\n[config_USB_device] Valeur de la configuration: %d\n", config0->bConfigurationValue);

  // reclamation de toutes les interfaces pour notre usage
  int nb_interface = config0->bNumInterfaces;
  printf("[config_USB_device] La configuration d'indice %d possede %d interface(s):\n", indice, nb_interface);
  int tab_nb_endpoint[nb_interface];

  // iteration sur le nombre d'interface
  for (int i=0; i<nb_interface; i++)  {
    // on stocke le numero de l'interface dans un tableau en fonction de son indice
    tab_interface_number[i]=config0->interface[i].altsetting[0].bInterfaceNumber;
    printf("[config_USB_device]   - interface d'indice %d, de numéro %d\n",i, tab_interface_number[i]);
    // on stocke dans un second tableau le nombre de endpoins pour une interface d'indice i
    tab_nb_endpoint[i]=config0->interface[i].altsetting[0].bNumEndpoints;
  }

  // iteration sur le nombre d'interface
  for (int i=0; i<nb_interface; i++)  {
    // si le mechant noyau est passe avant nous
    if(libusb_kernel_driver_active(*handle,tab_interface_number[i])){
      printf("[config_USB_device] ! Le mechant noyau etait passe\n");
      status=libusb_detach_kernel_driver(*handle,tab_interface_number[i]);
      if(status!=0){ perror("[config_USB_device] libusb_detach_kernel_driver"); exit(-1); }
    }
  }

  // installation de cette configuration comme configuration courante
  int configuration=config0->bConfigurationValue;
  status=libusb_set_configuration(*handle,configuration);
  if(status!=0){    perror("[config_USB_device] libusb_set_configuration"); exit(-1);}

  for (int i=0; i<nb_interface; i++)  {
    // on reclame chaque interface
    status=libusb_claim_interface(*handle,tab_interface_number[i]);
    if(status!=0){ perror("[config_USB_device] libusb_claim_interface");}
    else  {
      // affichage de l'indice et du numero de chaque interface reclamee
      printf("[config_USB_device] -> interface d'indice %d bien reclamee\n",i);
    }
  }

  // iteration sur le nombre d'interface BIS
  int k=0;
  for(int i=0; i<nb_interface; i++) {
    // on parcourt tout les endpoints de l'interface
    int i_endpoint_int=0;
    /* PLUS NECESSAIRE (Q 4.4)
    _Bool endpoint_trouve=false;
    */
    printf("\n[config_USB_device] On parcourt l'interface numero %d, qui possede %d endpoint(s):\n", i, tab_nb_endpoint[i]);
    for (int j=0; j<tab_nb_endpoint[i]; j++)  {
      // si il est de type interruption (cad 2 premiers bits de bmAttributes == 1)
      if((config0[0].interface[i].altsetting[0].endpoint[j].bmAttributes&3)==3) {
        printf("[config_USB_device] - endpoint numero %d | type = interruption | adresse = %d\n", j, config0[0].interface[i].altsetting[0].endpoint[j].bEndpointAddress);
        i_endpoint_int++;
        /* PLUS NECESSAIRE (Q 4.4)
        // si c'est pas le premier
        if(endpoint_trouve==true){
          printf("[config_USB_device]\t-> on a déja sauvé un endpoint interruption pour cette interface \n[config_USB_device]\t\t-> on le sauve pas dans le tableau\n");
        }
        // si c'est le premier
        else{
          printf("[config_USB_device]\t-> 1er endpoint interruption de cette interface \n[config_USB_device]\t\t-> on le sauve dans le tableau\n");

          // on stocke l'adresse du endpoint dans le tableau
          tab_endpoint_int_add[i]=config0[0].interface[i].altsetting[0].endpoint[j].bEndpointAddress;
          endpoint_trouve=true;
        }
        */
        // on stocke l'adresse du endpoint dans le tableau
        tab_endpoint_int_add[k]=config0[0].interface[i].altsetting[0].endpoint[j].bEndpointAddress;
      }
      else {
        printf("[config_USB_device] - endpoint numero %d | type = non interruption | adresse = %d\n", j, config0[0].interface[i].altsetting[0].endpoint[j].bEndpointAddress);
      }
      k++;
    }
    /* PLUS NECESSAIRE (Q 4.4)
    printf("[config_USB_device] = pour l'interface %d on a trouvé %d endpoint(s) de type interruption dont %s sauvé en mémoire\n\n", i, i_endpoint_int, endpoint_trouve ? "1" : "0");
    */
    printf("[config_USB_device] = pour l'interface %d on a trouvé %d endpoint(s) de type interruption\n\n", i, i_endpoint_int);
  }

  return nb_interface;
}

/* Fonction qui libere le peripherique detecte */
void close_USB_device(int nb_interface, libusb_device_handle **handle){
  struct libusb_device *device=libusb_get_device(*handle);
  struct libusb_config_descriptor *config;
  int num_inter_i;
  int status=libusb_get_active_config_descriptor(device, &config);
  if(status!=0){perror("[close_USB_device] libusb_get_active_config_descriptor");}
  // affichage du numero et de l'indice de l'interface liberee
  else{
    printf("\n[close_USB_device] Il y a %d interfaces a liberer:\n", nb_interface);
    for(int i=0;i<nb_interface;i++) {
      // relache de l'interfae
      num_inter_i=config->interface[i].altsetting[0].bInterfaceNumber;
      status=libusb_release_interface(*handle,num_inter_i);
      if(status!=0){perror("[close_USB_device] libusb_release_interface");}
      printf("[close_USB_device]  - interface liberee: id = %d, numero = %d\n",i, num_inter_i);
    }
  }

  // fermeture de la poignee
  libusb_close(*handle);
  libusb_free_config_descriptor(config);
}
