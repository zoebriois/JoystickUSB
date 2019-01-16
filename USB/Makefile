#executable a compiler
EXEC=tutorat

#compilateur et options
CC=gcc -Wall

#la librairie libusb
libusb=-l usb-1.0

#regle principale: make
all: $(EXEC)

fonctions_USB.o: fonctions_USB.c
	$(CC) -c fonctions_USB.c $(libusb)

prog_USB.o: prog_USB.c
	$(CC) -c prog_USB.c $(libusb)

#compilation finale: il faut d'abord que tout les fichiers .o aient été générés
#on met $^ pour "ttes les dep" soit fonctions_USB.o et prog_USB.o
$(EXEC): fonctions_USB.o prog_USB.o
	$(CC) -o $(EXEC) $^ $(libusb)

#regle de nettoyage: clean
clean:
	rm -f *.o *.a

#regle de nettoyage de l'executable: cleanExec
cleanExec:
		rm -f $(EXEC)
