all : cliente.o gestor.o verificador.o
	gcc cliente.o -o cliente -lpthread -lcurses
	gcc gestor.o -o gestor -lpthread
	gcc verificador.o -o verificador

cliente : cliente.o
	gcc cliente.o -o cliente -lpthread -lcurses

gestor : gestor.o
	gcc gestor.o -o gestor -lpthread

verificador : verificador.o
	gcc verificador.o -o verificador

cliente.o : cliente.c incl/estruturas.h incl/cliente.h
	gcc cliente.c incl/estruturas.h incl/cliente.h -c 

gestor.o : gestor.c incl/estruturas.h incl/gestor.h
	gcc gestor.c incl/estruturas.h incl/gestor.h -c 

verificador.o : verificador.c
	gcc verificador.c -c

clean :
	rm cliente.o cliente incl/cliente.h.gch gestor.o gestor incl/gestor.h.gch verificador.o verificador incl/estruturas.h.gch
