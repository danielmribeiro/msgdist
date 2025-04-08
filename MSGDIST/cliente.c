#include "incl/cliente.h"

int g_fifo_fd, c_fifo_fd, g2_fifo_fd, c2_fifo_fd, cconf_fifo_fd, gconf_fifo_fd;
MENSAGEM msg;
UTILIZADOR user; //do tipo UTILIZADOR
RESPOSTAGESTOR respGest;
PERGUNTAGESTOR pergGest;
ORDEM ordem;
char c_fifo_fname[25], c2_fifo_fname[25], cconf_fifo_fname[25];
int lerRes, confirma;
char lertopicos[1000];

void trataSinal(int i) {
    printf("O Cliente irá terminar (Interrupção via comando shutdown)\n\n");
    close(c_fifo_fd);
    close(g_fifo_fd);
    close(cconf_fifo_fd);
    close(gconf_fifo_fd);
    unlink(cconf_fifo_fname);
    unlink(c_fifo_fname);
    close(c2_fifo_fd);
    close(g2_fifo_fd);
    unlink(c2_fifo_fname);
    exit(EXIT_SUCCESS); /* para terminar o processo */
}


void estaOnline(){
    strcpy(pergGest.pergunta, "estou vivo");
    write(gconf_fifo_fd, & pergGest, sizeof(pergGest));
    //OBTEM A RESPOSTA
    confirma = read(cconf_fifo_fd, & respGest, sizeof(respGest));
    //printf("\n%s\n", respGest.resposta);
    //printf("\n[bytes lidos: %d]", lerRes);
    alarm(10);
}


// void subscreverTopico(){
//     memset(ordem.topico, '\0', TAM_TOPICO);
//     strcpy(ordem.comandocli, "subscrevetopico");
//     strcpy(ordem.utilizador, user.username);
//     printf("Tópico: ");
//     fgets(ordem.topico, sizeof(ordem.topico), stdin);
//     size_t length1 = strlen(ordem.topico);
//     if (ordem.topico[length1-1] == '\n')
//         ordem.topico[length1-1] = '\0';
//     write(g2_fifo_fd, & ordem, sizeof(ordem));
//     lerRes = read(c2_fifo_fd, & respGest, sizeof(respGest));
//     printf("\n-----Resposta:-----\n%s\n", respGest.resposta);
// }


void listarMsgDeTituloDeTopico(){
    memset(ordem.topico, '\0', TAM_TOPICO);
    memset(ordem.titulo, '\0', TAM_TITULO);
    strcpy(ordem.comandocli, "listamsgdetitulodetopicos");
    printf("Tópico: ");
    fgets(ordem.topico, sizeof(ordem.topico), stdin);
    size_t length1 = strlen(ordem.topico);
    if (ordem.topico[length1-1] == '\n')
        ordem.topico[length1-1] = '\0';
    printf("Titulo: ");
    fgets(ordem.titulo, sizeof(ordem.titulo), stdin);
    size_t length2 = strlen(ordem.titulo);
    if (ordem.titulo[length2-1] == '\n')
        ordem.titulo[length2-1] = '\0';
    write(g2_fifo_fd, & ordem, sizeof(ordem));
    lerRes = read(c2_fifo_fd, & respGest, sizeof(respGest));
    printf("\n-----Mensagem:-----\n%s\n", respGest.resposta);
}

void listarTituloDeTopico(){
    memset(ordem.topico, '\0', TAM_TOPICO);
    strcpy(ordem.comandocli, "listatitulodetopicos");
    printf("Tópico: ");
    fgets(ordem.topico, sizeof(ordem.topico), stdin);
    size_t length1 = strlen(ordem.topico);
    if (ordem.topico[length1-1] == '\n')
        ordem.topico[length1-1] = '\0';
    write(g2_fifo_fd, & ordem, sizeof(ordem));
    lerRes = read(c2_fifo_fd, & respGest, sizeof(respGest));
    printf("\n-----Titulos de ' %s ':-----\n%s\n", ordem.topico, respGest.resposta);
}

void listartopicos(){
    strcpy(ordem.comandocli, "listatopicos");
    write(g2_fifo_fd, & ordem, sizeof(ordem));
    //OBTEM A RESPOSTA
    lerRes = read(c2_fifo_fd, & respGest, sizeof(respGest));
    printf("\n-----Tópicos:-----\n%s\n", respGest.resposta);
    //printf("\n[bytes lidos: %d]", lerRes);
}


void escreveMsg(){        
    memset(msg.topico, '\0', TAM_TOPICO);
    memset(msg.titulo, '\0', TAM_TITULO);
    memset(msg.corpo, '\0', TAM_CORPO);
    memset(&msg.duracao, '\0', sizeof(msg.duracao));

        char duracao[sizeof(msg.duracao)];
        //OBTEM A PERGUNTA
        fflush(stdin);
        printf("Topico:");
        fgets(msg.topico, sizeof(msg.topico), stdin);
        size_t length1 = strlen(msg.topico);
        if (msg.topico[length1-1] == '\n')
            msg.topico[length1-1] = '\0';
        printf("Titulo:");
        fgets(msg.titulo, sizeof(msg.titulo), stdin);
        size_t length2 = strlen(msg.titulo);
        if (msg.titulo[length2-1] == '\n')
            msg.titulo[length2-1] = '\0';


        //initscr();
        //int height, width, start_y, start_x;
        //height = 20;
        //width =50;
        //start_y = start_x = 10;

        //WINDOW * win = newwin(height, width, start_y, start_x);
        //refresh();

        //box(win, 0, 0);
        //mvwprintw(win, 0, 23, "Corpo:");
        printf("Corpo:");
        //int charsDoCorpo = getch();
        //getch();
        //wrefresh(win);
        fgets(msg.corpo, sizeof(msg.corpo), stdin);
        size_t length3 = strlen(msg.corpo);
        if (msg.corpo[length3-1] == '\n')
            msg.corpo[length3-1] = '\0';
        //endwin();
        printf("Duracao:");
        fgets(duracao, sizeof(msg.duracao), stdin);
        msg.duracao = atoi(duracao);
        //ENVIA A PERGUNTA
        write(g_fifo_fd, & msg, sizeof(msg));
        //OBTEM A RESPOSTA
        lerRes = read(c_fifo_fd, & respGest, sizeof(respGest));
        if(lerRes == sizeof(respGest)){
            printf("\n Resposta: %s", respGest.resposta);
        }else{
            printf("\nSem resposta ou resposta incompreensivel [bytes lidos: %d]", lerRes);
        }

}

void menuUtilizador(){
    char cmd[40];
    while(1){
        fflush(stdin);
        printf("%s: ", user.username);
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strlen(cmd)-1] = '\0';
        

        if (strcasecmp(cmd, "shutdown") == 0) {
            kill(getpid(), SIGUSR1);
        }

        if (strcasecmp(cmd, "escrevermsg") == 0) {
            escreveMsg();
        }

        if (strcasecmp(cmd, "listar topicos") == 0) {
            listartopicos();
        }
        
        if (strcasecmp(cmd, "procura titulo") == 0) {
            listarTituloDeTopico();
        }
        
        if (strcasecmp(cmd, "procura mensagem") == 0) {
            listarMsgDeTituloDeTopico();
        }

        // if (strcasecmp(cmd, "subscrever") == 0) {
        //     subscreverTopico();
        // }
        

    }

}

int main(int argc, char **argv) {
    signal(SIGUSR1, trataSinal);
    signal(SIGALRM, estaOnline);
    alarm(10);
    int c;


    opterr = 0;

    while (((c = getopt(argc, argv, "u:")) != -1) || argv[1] == NULL)
        switch (c) {
            case 'u':
                strcpy(user.username, optarg);
               
                break;
            case '?':
                if (optopt == 'u')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                printf("Precisa de receber um argumento!!\n");
                abort();
        }
    printf("Olá %s. Bem-vindo ao MSGDIST.\n", user.username);

    //cria o fifo do cliente
    msg.pid_cliente = getpid();
    sprintf(c_fifo_fname, CLIENTE_FIFO, msg.pid_cliente);
    if(mkfifo(c_fifo_fname, 0777) == -1){
        perror("!!!! mkfifo FIFO cliente deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do cliente criado ***\n");

    //cria o fifo 2 do cliente
    ordem.pid_cliente = getpid();
    sprintf(c2_fifo_fname, CLIENTE2_FIFO, ordem.pid_cliente);
    if(mkfifo(c2_fifo_fname, 0777) == -1){
        perror("!!!! mkfifo FIFO cliente deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do cliente criado ***\n");

    //cria o fifo confirmacao do cliente
    pergGest.pid_cliente = getpid();
    sprintf(cconf_fifo_fname, CLIENTECONF_FIFO, pergGest.pid_cliente);
    if(mkfifo(cconf_fifo_fname, 0777) == -1){
        perror("!!!! mkfifo FIFO cliente deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do cliente criado ***\n");

    //abre o FIFO do servidor p/escrita
    g_fifo_fd = open(GESTOR_FIFO, O_WRONLY); /* bloqueante */
    if(g_fifo_fd == -1){
        fprintf(stderr, "!!!!! O servidor nao está a correr !!!!!\n");
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do servidor aberto WRITE / BLOCKING ***\n"); 


    //abre o FIFO2 do servidor p/escrita
    g2_fifo_fd = open(GESTOR2_FIFO, O_WRONLY); /* bloqueante */
    if(g2_fifo_fd == -1){
        fprintf(stderr, "!!!!! O servidor nao está a correr !!!!!\n");
        unlink(c2_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do servidor aberto WRITE / BLOCKING ***\n"); 


    //abre o FIFO confirmacao do servidor p/escrita
    gconf_fifo_fd = open(GESTORCONF_FIFO, O_WRONLY); /* bloqueante */
    if(gconf_fifo_fd == -1){
        fprintf(stderr, "!!!!! O servidor nao está a correr !!!!!\n");
        unlink(cconf_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "*** FIFO do servidor aberto WRITE / BLOCKING ***\n"); 


    c_fifo_fd = open(c_fifo_fname, O_RDWR);
    if(c_fifo_fd == -1){
        perror("!!!!! Erro nao abrir o FIFO do cliente !!!!!\n");
        close(g_fifo_fd);
        unlink(c_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do cliente aberto para READ (+WRITE) BLOCK ***\n");

    c2_fifo_fd = open(c2_fifo_fname, O_RDWR);
    if(c2_fifo_fd == -1){
        perror("!!!!! Erro nao abrir o FIFO do cliente !!!!!\n");
        close(g2_fifo_fd);
        unlink(c2_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do cliente aberto para READ (+WRITE) BLOCK ***\n");

    cconf_fifo_fd = open(cconf_fifo_fname, O_RDWR);
    if(cconf_fifo_fd == -1){
        perror("!!!!! Erro nao abrir o FIFO do cliente !!!!!\n");
        close(gconf_fifo_fd);
        unlink(cconf_fifo_fname);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do cliente aberto para READ (+WRITE) BLOCK ***\n");


    menuUtilizador();

}
