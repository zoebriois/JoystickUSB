#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

// informations sur le peripherique recherche
// celui de polytech
//#define device_vendor 0x02341
//#define device_id 0x043

// celui de Zoe
#define device_vendor 0x01a86
#define device_id 0x07523


// pour compiler : -l usb-1.0

/*  Fonction qui enumere tout les peripheriques USB connectes a un ordinateur */
_Bool enum_USB_devices(libusb_context **context, libusb_device_handle **handle)
{
  _Bool device_found=false;
  printf("[enum_USB_devices] START\n");
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
      if(status!=0) {
        perror("[enum_USB_devices] libusb_open"); exit(-1);
      }
    }
  }
  libusb_free_device_list(list,1);
  printf("[enum_USB_devices] END\n\n");
  return device_found;
}


/* Fonction de configuration du peripherique detecte
3 tableaux:
tab_endpoint_add qui stocke l'adresse du premier endpoint d'interruption correspondant a l'indice
tab_interface_number qui stocke le numero de l'interface d'indice i
tab_nb_endpoint qui stocke le nombre de endpoint pour l'interface d'indice i
*/
int config_USB_device(libusb_device_handle **handle, int tab_endpoint_add[], int tab_interface_number[])
{
  printf("[config_USB_device] START\n");
  // recuperation de la premiere configuration (celle d'indice 0)
  struct libusb_config_descriptor *config0;
  struct libusb_device *device = libusb_get_device(*handle);
  uint8_t indice = 0;
  int status = libusb_get_config_descriptor(device, indice, &config0);
  if(status!=0){ perror("[config_USB_device] libusb_get_config_descriptor");}


  //  affichage de la valeur de cette configuration
  printf("[config_USB_device] Descriptor type de la configuration: %d\n", config0->bDescriptorType);


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
  if(status!=0){
    perror("[config_USB_device] libusb_set_configuration"); exit(-1);
  }

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
  for(int i=0; i<nb_interface; i++) {
    // on parcourt tout les endpoints de l'interface
    int i_endpoint=0;
    _Bool endpoint_trouve=false;
    printf("[config_USB_device] On parcourt l'interface numero %d, qui possede %d endpoint(s):\n", i, tab_nb_endpoint[i]);
    for (int j=0; j<tab_nb_endpoint[i]; j++)  {
      // si il est de type interruption (cad 2 premiers bits de bmAttributes == 1)
      if((config0[0].interface[i].altsetting[0].endpoint[j].bmAttributes&3)==3) {
        // on stocke l'adresse du endpoint dans le tableau
        tab_endpoint_add[i_endpoint]=config0[0].interface[i].altsetting[0].endpoint[j].bEndpointAddress;
        i_endpoint++;
        endpoint_trouve=true;
        printf("[config_USB_device] - endpoint numero %d est de type interruption (adresse = %d)\n", j, tab_endpoint_add[i_endpoint]);
      }
      else {
        printf("[config_USB_device] - endpoint numero %d n'est pas de type interruption\n", j);
      }

      // pour avoir que le premier
      if (endpoint_trouve==true) {
        break;
      }
    }
    printf("[config_USB_device] = pour l'interface %d on a trouve %d endpoint(s) de type interruption \n", i, i_endpoint);
  }

  printf("[config_USB_device] END\n\n");
  // on retourne le nombre d'interface qui a present est correct
  return nb_interface;
}

/* Fonction qui libere le peripherique detecte */
void close_USB_device(int nb_interface, libusb_device_handle **handle){
  printf("[close_USB_device] START\n");
  struct libusb_device *device=libusb_get_device(*handle);
  struct libusb_config_descriptor *config;
  int num_inter_i;
  int status=libusb_get_active_config_descriptor(device, &config);
  if(status!=0){perror("[close_USB_device] libusb_get_active_config_descriptor");}
  // affichage du numero et de l'indice de l'interface liberee
  else{
    printf("[close_USB_device] Il y a %d interfaces a liberer:\n", nb_interface);
    for(int i=0;i<nb_interface;i++) {
      // relache de l'interfae
      status=libusb_release_interface(*handle,num_inter_i);
      if(status!=0){perror("[close_USB_device] libusb_release_interface");}
      num_inter_i=config->interface[i].altsetting[0].bInterfaceNumber;
      printf("[close_USB_device]  - interface liberee: id = %d, numero = %d\n",i, num_inter_i);
    }
  }

  // fermeture de la poignee
  libusb_close(*handle);
  libusb_free_config_descriptor(config);
  printf("[close_USB_device] END\n\n");
}
