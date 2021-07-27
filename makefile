CC = gcc
MCGRUDER_DIR = ./McGruder/
LIONEL_DIR = ./Lionel/
OUT = $(MCGRUDER_DIR)McGruder $(LIONEL_DIR)Lionel
OBJ1 = McGruder.o McGruder_Configuration.o McGruder_Connection.o McGruder_List.o
OBJ2 = Lionel.o Lionel_Configuration.o Lionel_Connection.o Lionel_List.o Lionel_Constellations_List.o Paquita.o Paquita_List.o
CFLAGS = -lpthread -lm -ggdb -Wall -Wextra
TARCMD = tar -cf
TARFILE = G8_lluisMasdeu_oriolRamis.tar
TARCOMPS = $(MCGRUDER_DIR) $(LIONEL_DIR) makefile Memoria.pdf

all: $(OUT)

$(MCGRUDER_DIR)McGruder: $(OBJ1)
	$(CC) $(OBJ1) -o $(MCGRUDER_DIR)McGruder $(CFLAGS)

McGruder.o: $(MCGRUDER_DIR)McGruder.c $(MCGRUDER_DIR)McGruder_Configuration.h $(MCGRUDER_DIR)McGruder_Connection.h
	$(CC) $(MCGRUDER_DIR)McGruder.c -c $(CFLAGS)

McGruder_Configuration.o: $(MCGRUDER_DIR)McGruder_Configuration.c $(MCGRUDER_DIR)McGruder_Configuration.h
	$(CC) $(MCGRUDER_DIR)McGruder_Configuration.c -c $(CFLAGS)

McGruder_Connection.o: $(MCGRUDER_DIR)McGruder_Connection.c $(MCGRUDER_DIR)McGruder_Connection.h $(MCGRUDER_DIR)semaphore_v2.h
	$(CC) $(MCGRUDER_DIR)McGruder_Connection.c -c $(CFLAGS)

McGruder_List.o: $(MCGRUDER_DIR)McGruder_List.c $(MCGRUDER_DIR)McGruder_List.h
	$(CC) $(MCGRUDER_DIR)McGruder_List.c -c $(CFLAGS)

$(LIONEL_DIR)Lionel: $(OBJ2)
	$(CC) $(OBJ2) -o $(LIONEL_DIR)Lionel $(CFLAGS)

Lionel.o: $(LIONEL_DIR)Lionel.c $(LIONEL_DIR)Lionel_Configuration.h $(LIONEL_DIR)Lionel_Connection.h $(LIONEL_DIR)Lionel_List.h $(LIONEL_DIR)Lionel_Constellations_List.h $(LIONEL_DIR)Paquita.h $(LIONEL_DIR)Paquita_List.h
	$(CC) $(LIONEL_DIR)Lionel.c -c $(CFLAGS)

Lionel_Configuration.o: $(LIONEL_DIR)Lionel_Configuration.c $(LIONEL_DIR)Lionel_Configuration.h
	$(CC) $(LIONEL_DIR)Lionel_Configuration.c -c $(CFLAGS)

Lionel_Connection.o: $(LIONEL_DIR)Lionel_Connection.c $(LIONEL_DIR)Lionel_Connection.h
	$(CC) $(LIONEL_DIR)Lionel_Connection.c -c $(CFLAGS)

Lionel_List.o: $(LIONEL_DIR)Lionel_List.c $(LIONEL_DIR)Lionel_List.h
	$(CC) $(LIONEL_DIR)Lionel_List.c -c $(CFLAGS)

Lionel_Constellations_List.o: $(LIONEL_DIR)Lionel_Constellations_List.c $(LIONEL_DIR)Lionel_Constellations_List.h
	$(CC) $(LIONEL_DIR)Lionel_Constellations_List.c -c $(CFLAGS)

Paquita.o: $(LIONEL_DIR)Paquita.c $(LIONEL_DIR)Paquita.h $(LIONEL_DIR)Paquita_List.h
	$(CC) $(LIONEL_DIR)Paquita.c -c $(CFLAGS)

Paquita_List.o: $(LIONEL_DIR)Paquita_List.c $(LIONEL_DIR)Paquita_List.h
		$(CC) $(LIONEL_DIR)Paquita_List.c -c $(CFLAGS)

clean:
	rm *.o $(MCGRUDER_DIR)McGruder $(LIONEL_DIR)Lionel

tar:
	$(TARCMD) $(TARFILE) $(TARCOMPS)
