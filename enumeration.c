#include "enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

// pour compiler : -l libusb-1.0
libusb_device_handle *handle;
libusb_context *context;

void list_USB_devices()
{
    // Initialisation de la bibliotheque
    int status=libusb_init(&context);
    if(status!=0) {perror("libusb_init"); exit(-1);}
    libusb_device **list;
    ssize_t count=libusb_get_device_list(context,&list);
    if(count<0) 
    {
        perror("libusb_get_device_list"); exit(-1);
    }
    ssize_t i=0;
    for(i=0;i<count;i++)
    {
        libusb_device *device=list[i];
        struct libusb_device_descriptor desc;
        int status=libusb_get_device_descriptor(device,&desc);
        if(status!=0) 
            continue;
        uint8_t bus=libusb_get_bus_number(device);
        uint8_t address=libusb_get_device_address(device);
        printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
        printf("Vendor ID 0x0%x\n",desc.idVendor);
        printf("Product ID 0x0%x\n",desc.idProduct);
        
        // Si c'est ce qu'on cherche on l'ouvre
        if(desc.idVendor == 0x03eb && desc.idProduct == 0x02fef)    {
            printf("Device found @ (Bus:Address) %d:%d\n", bus, address);
            device_found(device);
            exit(1);
        }
    }
    libusb_free_device_list(list,1);
    
}

void device_found(libusb_device *device)
{
    printf("The good device has been found.\n");
    int status=libusb_open(device,&handle);
    if(status!=0){ perror("libusb_open"); exit(-1); }
    
 
}
    
int main()
{
    

    
    list_USB_devices();
    
    // Fermeture du contexte et de la poigneeœ
    libusb_exit(context);
    libusb_close(handle);
    return 0;
}
    
