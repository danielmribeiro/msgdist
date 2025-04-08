#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#include <pthread.h>

//FIFO do Gestor
#define GESTOR_FIFO "/tmp/gest_fifo"
#define GESTOR2_FIFO "/tmp/gest2_fifo"
#define GESTORCONF_FIFO "/tmp/gestconf_fifo"

//FIFO do Cliente
#define CLIENTE_FIFO "/tmp/cli_%d_fifo"
#define CLIENTE2_FIFO "/tmp/cli2_%d_fifo"
#define CLIENTECONF_FIFO "/tmp/cliconf_%d_fifo"

//Utilizador
#define TAM_USERNAME 20
#define TAM_SUBS 100

//Mensagem
#define TAM_MSGID 30
#define TAM_TOPICO 50
#define TAM_TITULO 100
#define TAM_CORPO 1000

//Resposta do Gestor
#define TAM_RESPOSTA 100000

//Pergunta ao Gestor
#define TAM_PERGUNTA 40

//Tamanho de palavra a passar no verificador
#define WORD_SIZE 1000


typedef struct{
	char username[TAM_USERNAME];
	char subscricoes[TAM_SUBS];
}UTILIZADOR;

typedef struct{
	pid_t pid_cliente;
	int msgID;
	char topico[TAM_TOPICO];
	char titulo[TAM_TITULO];
	char corpo[TAM_CORPO];
	int duracao;
}MENSAGEM;

typedef struct{
	pid_t pid_cliente;
	char comandocli[90];
	char topico[TAM_TOPICO];
	char titulo[TAM_TITULO];
	//char extraInfo[TAM_USERNAME];
}ORDEM;

typedef struct{
	char resposta[TAM_RESPOSTA];
}RESPOSTAGESTOR;

typedef struct{
	pid_t pid_cliente;
	char pergunta[TAM_PERGUNTA];
}PERGUNTAGESTOR;
