#include "incl/gestor.h"

int MAXMSG;
int MAXNOT;
int filter = 1; //  FilterON = 1      FilterOFF = 0
int g_fifo_fd, c_fifo_fd, g2_fifo_fd, c2_fifo_fd, gconf_fifo_fd, cconf_fifo_fd; //descritores de ficheiros (pipes)
char word[WORD_SIZE];//para o verificador
MENSAGEM msg[10]; /*mensagem do tipo MENSAGEM*/
ORDEM ordem[10];
UTILIZADOR user[100];
int i=0;
int n=0;
int y=0;
RESPOSTAGESTOR respGest; /*mensagem do tipo RESPOSTAGESTOR*/
PERGUNTAGESTOR pergGest;
int e, res, cmdcliente;
char c_fifo_fname[50], c2_fifo_fname[50], cconf_fifo_fname[50];
char res_listartopicos[50];
char listartpcs[20];
char trash[99];

void eliminaMensagem(){
    fprintf(stderr,"\n----Mensagem eliminada----\ntópico: %s\ntitulo: %s\ncorpo: %s", msg[i-1].topico, msg[i-1].titulo, msg[i-1].corpo);
    strcpy(msg[i-1].titulo,"");
    strcpy(msg[i-1].corpo,"");
    MAXMSG++;
    //fprintf(stderr, "%d", i-1);
}

void* estaOnline(void * arg){
    while(1){
            //OBTEM unlink(GESTORCONF_FIFO);
            res = read(gconf_fifo_fd, & pergGest, sizeof(pergGest));
            if(res < sizeof(pergGest)){
                fprintf(stderr, "!!!!! Recebida pergunta incompleta [bytes lidos: %d] !!!!!\n", res);
                continue; //não responde ao cliente (Qual cliente?)
            }//*FIM* OBTEM MENSAGEM

            strcpy(respGest.resposta, "***** O gestor continua online *****\n");

            // OBTEM FILENAME DO FIFO PARA A RESPOSTA
            sprintf(cconf_fifo_fname, CLIENTECONF_FIFO, pergGest.pid_cliente);

            //ABRE FIFO DO CLIENTE P/ WRITE
            cconf_fifo_fd = open(cconf_fifo_fname, O_WRONLY);
            if(cconf_fifo_fd == -1){
                perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
            }else{
                fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");


                //ENVIA RESPOSTA
                res= write(cconf_fifo_fd, & respGest, sizeof(respGest));
                if (res == sizeof(respGest)){
                    fprintf(stderr,"***** REspondi a um cliente que eu estava online *****\n");
                }else{
                    perror("!!!!! erro a escrever a resposta !!!!!\n");
                }
                close(cconf_fifo_fd); //fecha logo o fifo do cliente!
                fprintf(stderr,"***** FIFO cliente fechado *****\n");
                
            }
    }//fim do ciclo while(1)
}

// void listarUtilizadores(){
//     for(int u=0; u<5; u++){
//         printf("%s", user[u].username);
//     }
// }


// void listarTopicos(){
//     int b;
//     int r;
//     for(r=0; r<5; r++){
//         for(int b=0; b<r; b++){
//             if(strcasecmp(msg[r].topico, msg[b].topico)==0){
//                 break;
//             }
//         }
//         if(r==b){
//             printf("%s\n", msg[r].topico); //caso n encontre
//         }
        
//     }
// }

void trataSinal(int e) {
    printf("**** O Gestor irá terminar (Interrupção via comando shutdown) ****\n\n");
    close(g_fifo_fd);
    close(g2_fifo_fd);
    close(gconf_fifo_fd);
    unlink(GESTOR_FIFO);
    unlink(GESTOR2_FIFO);
    unlink(GESTORCONF_FIFO);
    exit(EXIT_SUCCESS); /* para terminar o processo */
}

int vouVerificar(){
    int valorRetornado;
    char estado[9999];
    pid_t childpid;

    int writepipe[2] = {-1. - 1}; // pipe (write) gestor > verificador
    int readpipe[2] = {-1. - 1}; // pipe de retorno (read) verificador > gestor

    writepipe[READ_END] = -1;
    if (pipe(readpipe) < 0 || pipe(writepipe) < 0) {
        printf("!!!!! FATAL: comunication not allowed !!!!!\n");
        return 0;
    }

    int pid = fork();

    switch (pid) {
        case -1:
            printf("!!!!! Erro ao criar o FORK. pid<0 !!!!!\n");
            return 0;
            break;

        case 0:
            //printf("Estou no filho! Vou fechar o readpipe[READ_END]!\n");
            close(readpipe[READ_END]);
            //printf("Estou no filho! Fechei o readpipe[READ_END] e vou fechar o writepipe[WRITE_END]!\n");
            close(writepipe[WRITE_END]);
            //printf("Estou no filho! Fechei o writepipe[WRITE_END] e vou fazer o dup2 do STDIN !\n");
            dup2(writepipe[READ_END], STDIN_FILENO);
            //printf("Estou no filho! Fiz o dup2 do STDIN e vou fazer o dup2 do STDOUT!\n");
            dup2(readpipe[WRITE_END], STDOUT_FILENO);
            //printf("Estou no filho! Fiz o dup2 do STDOUT e vou fechar o writepipe[READ_END]!\n");
            close(writepipe[READ_END]);
            //printf("Estou no filho! Fechei o writepipe[READ_END] e vou fechar o readpipe[WRITE_END]!\n");
            close(readpipe[WRITE_END]);
            //printf("Estou no filho! Fechei o readpipe[WRITE_END] e a proxima linha é o execl()!\n");

            execl("./verificador", "./verificador", "incl/palavrasProibidas.txt", (char*) NULL);
            //printf("Estou no filho! O execl já passou. UPSSSS!\n");

            break;

        default:
            //printf("Estou no pai! Vou fechar o writepipe[READ_END]!\n");
            close(writepipe[READ_END]);
            //printf("Estou no pai! Fechei o writepipe[READ_END] e vou fechar o readpipe[WRITE_END]!\n");
            close(readpipe[WRITE_END]);

            //printf("Estou no pai! Vou escrever coisas no write!\n");
            write(writepipe[WRITE_END], word, strlen(word));
            write(writepipe[WRITE_END], "\n##MSGEND##\n", 12);
            //printf("Estou no pai! Escrevi coisas no write mas não fiz fechei!\n");
            close(writepipe[WRITE_END]);
            //printf("Estou no pai! Escrevi coisas no write e já o fechei!\n");

            //printf("Estou no pai! O nbytes não tem nada!\n");
            int nbytes = read(readpipe[READ_END], estado, sizeof (estado));
            //printf("Estou no pai! O nbytes está a lojar um valor!\n");
            close(readpipe[READ_END]);
            //printf("Vou fazer Wait(NULL)!\n");
            wait(NULL);
            //printf("Fiz Wait(NULL)!\n");
            //printf("Output nybtes, estado: %d, %s", nbytes, estado);
            //printf("Output estado: %s", estado);
            valorRetornado = atoi(estado);
            printf("Output valorRetornado: %d\n", valorRetornado);

            break;
    }
}

void menuAdministrador(){
    char adminCmd[40];
    while (1) {

        printf("root@MSGDIST:~# ");
        scanf("%s", adminCmd);

        if (strcasecmp(adminCmd, "shutdown") == 0) {
            kill(getpid(), SIGUSR1);
        }

        if (strcasecmp(adminCmd, "filteron") == 0) {
            filter = 1;
            printf("Palavras Proibidas ligado\n");
        }

        if (strcasecmp(adminCmd, "filteroff") == 0) {
            filter = 0;
            printf("Palavras Proibidas desligado\n");
        }
        
        if (strcasecmp(adminCmd, "users") == 0) {
            //listarUtilizadores();
        }

        if (strcasecmp(adminCmd, "topics") == 0) {
            // listarTopicos();
        }

        if (strcasecmp(adminCmd, "msg") == 0) {
            printf("Listar Mensagens\n");
        }

        //topic topico em questao
        //del mensagem-em-questao
        //kickusername em questao

        if (strcasecmp(adminCmd, "prune") == 0) {
            printf("Eliminar todos os tópicos que não tenham mensagens\n");
        }

        if (strcmp(adminCmd, "verifica") == 0) {
            printf("PALAVRA: ");
            scanf("%s", word);
            vouVerificar();
        }
        
    }

}

void* atendeComandosCliente(void * arg){
    while(1){
        //OBTEM COMANDO
        cmdcliente = read(g2_fifo_fd, & ordem[0], sizeof(ordem[0]));
        if(cmdcliente < sizeof(ordem[0])){
                fprintf(stderr, "!!!!! Recebida ordem do cliente incompleta [bytes lidos: %d] !!!!!\n", res);
                continue; //não responde ao cliente (Qual cliente?)
        }

        if(strcasecmp(ordem[0].comandocli, "listatopicos")==0){
            strcpy(respGest.resposta,"");
            for(int n=0; n<5; n++){
                int j;
                for(j=0; j<n; j++){
                    if(strcasecmp(msg[n].topico, msg[j].topico)==0){
                        break;
                    }
                }
                if(n==j){
                    strcat(respGest.resposta, msg[n].topico); //caso n encontre
                    strcat(respGest.resposta, "\n"); //caso n encontre
                }
            }

            sprintf(c2_fifo_fname, CLIENTE2_FIFO, ordem[0].pid_cliente);
            c2_fifo_fd = open(c2_fifo_fname, O_WRONLY);
            if(c2_fifo_fd == -1){
                perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
            }else{
                fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");
                cmdcliente= write(c2_fifo_fd, & respGest, sizeof(respGest));
                if (cmdcliente == sizeof(respGest)){
                    fprintf(stderr,"***** Escreveu a resposta *****\n");
                }else{
                    perror("!!!!! erro a escrever a resposta !!!!!\n");
                }
                close(c2_fifo_fd); //fecha logo o fifo do cliente!
                fprintf(stderr,"***** FIFO cliente fechado *****\n");
            }
        }

        if(strcasecmp(ordem[0].comandocli, "listatitulodetopicos")==0){
            
            size_t length = strlen(ordem[0].topico);
            if (ordem[0].topico[length-1] == '\n'){
                ordem[0].topico[length-1] = '\0';
            }
            strcpy(respGest.resposta,"");
            for(int i=0; i<=5; i++){
                if(strcasecmp(ordem[0].topico, msg[i].topico)==0){
                    strcat(respGest.resposta, msg[i].titulo); //caso n encontre
                    strcat(respGest.resposta, "\n"); //caso n encontre
                }
            }
            sprintf(c2_fifo_fname, CLIENTE2_FIFO, ordem[0].pid_cliente);
            c2_fifo_fd = open(c2_fifo_fname, O_WRONLY);
            if(c2_fifo_fd == -1){
                perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
            }else{
                fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");
                cmdcliente= write(c2_fifo_fd, & respGest, sizeof(respGest));
                if (cmdcliente == sizeof(respGest)){
                    fprintf(stderr,"***** Escreveu a resposta *****\n");
                }else{
                    perror("!!!!! erro a escrever a resposta !!!!!\n");
                }
                close(c2_fifo_fd); //fecha logo o fifo do cliente!
                fprintf(stderr,"***** FIFO cliente fechado *****\n");
            }
        }

        if(strcasecmp(ordem[0].comandocli, "listamsgdetitulodetopicos")==0){
            
            size_t length = strlen(ordem[0].topico);
            if (ordem[0].topico[length-1] == '\n'){
                ordem[0].topico[length-1] = '\0';
            }
            size_t length1 = strlen(ordem[0].titulo);
            if (ordem[0].titulo[length1-1] == '\n'){
                ordem[0].titulo[length1-1] = '\0';
            }

            strcpy(respGest.resposta,"");

            for(int i=0; i<=5; i++){
                if(strcasecmp(ordem[0].topico, msg[i].topico)==0){
                    if(strcasecmp(ordem[0].titulo, msg[i].titulo)==0){
                        strcat(respGest.resposta, msg[i].corpo); //caso n encontre
                        strcat(respGest.resposta, "\n"); //caso n encontre
                    }
                }
            }
            sprintf(c2_fifo_fname, CLIENTE2_FIFO, ordem[0].pid_cliente);
            c2_fifo_fd = open(c2_fifo_fname, O_WRONLY);
            if(c2_fifo_fd == -1){
                perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
            }else{
                fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");
                cmdcliente= write(c2_fifo_fd, & respGest, sizeof(respGest));
                if (cmdcliente == sizeof(respGest)){
                    fprintf(stderr,"***** Escreveu a resposta *****\n");
                }else{
                    perror("!!!!! erro a escrever a resposta !!!!!\n");
                }
                close(c2_fifo_fd); //fecha logo o fifo do cliente!
                fprintf(stderr,"***** FIFO cliente fechado *****\n");
            }
        }

        // if(strcasecmp(ordem[0].comandocli, "subscrevetopico")==0){
            

        //     size_t length = strlen(ordem[0].topico);
        //     if (ordem[0].topico[length-1] == '\n'){
        //         ordem[0].topico[length-1] = '\0';
        //     }

        //     strcpy(respGest.resposta,"Tópico subscrito\n");

        //     for(int i=0; i<=5; i++){
        //         if(strcasecmp(ordem[0].topico, msg[i].topico)==0){
        //             while(subs[y].subscricoes!=NULL){
        //                 y++;
        //             }
        //             strcat(subs[y].username, ordem[0].utilizador);
        //             strcat(subs[y].subscricoes, ordem[0].topico);
                    
        //             strcat(respGest.resposta, subs[y].subscricoes); //caso n encontre
        //             strcat(respGest.resposta, "\n"); //caso n encontre
        //             break;
        //         }
        //     }
        //     sprintf(c2_fifo_fname, CLIENTE2_FIFO, ordem[0].pid_cliente);
        //     c2_fifo_fd = open(c2_fifo_fname, O_WRONLY);
        //     if(c2_fifo_fd == -1){
        //         perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
        //     }else{
        //         fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");
        //         cmdcliente= write(c2_fifo_fd, & respGest, sizeof(respGest));
        //         if (cmdcliente == sizeof(respGest)){
        //             fprintf(stderr,"***** Escreveu a resposta *****\n");
        //         }else{
        //             perror("!!!!! erro a escrever a resposta !!!!!\n");
        //         }
        //         close(c2_fifo_fd); //fecha logo o fifo do cliente!
        //         fprintf(stderr,"***** FIFO cliente fechado *****\n");
        //     }
        // }

    }
}

void* atendeEscreveMsg(void * arg){
    while(1){
            //OBTEM Munlink(GESTOR_FIFO);ENSAGEM
            res = read(g_fifo_fd, & msg[i], sizeof(msg[i]));
            if(res < sizeof(msg[i])){
                fprintf(stderr, "!!!!! Recebida pergunta incompleta [bytes lidos: %d] !!!!!\n", res);
                continue; //não responde ao cliente (Qual cliente?)
            }
            fprintf(stderr, "\n************Mensagem %d ****************\nTópico: [%s]\nTitulo: [%s]\nCorpo: [%s]\nDuracao: [%d]\n****************************** \n",i , msg[i].topico, msg[i].titulo, msg[i].corpo, msg[i].duracao);
            //*FIM* OBTEM MENSAGEM

            if(filter == 1){
                // VERIFICA MENSAGEM
                strcpy(respGest.resposta, "***** Mensagem foi analisada e foi aprovada *****\n"); //caso n encontre
                char* novaString;
                novaString=strtok(msg[i].corpo, " ");
                while (novaString!=NULL){
                    printf("%s\n", novaString);
                    strcpy (word, novaString);
                    vouVerificar();
                    novaString=strtok(NULL, " ");
                }
                // *FIM* VERIFICA MENSAGEM;
            }else if(filter == 0){
                strcpy(respGest.resposta, "***** Mensagem não foi analisada e foi aprovada *****\n");
            }

            // OBTEM FILENAME DO FIFO PARA A RESPOSTA
            sprintf(c_fifo_fname, CLIENTE_FIFO, msg[i].pid_cliente);

            //ABRE FIFO DO CLIENTE P/ WRITE
            c_fifo_fd = open(c_fifo_fname, O_WRONLY);
            if(c_fifo_fd == -1){
                perror("!!!!! Erro no open - Ninguem quis a resposta !!!!!\n");
            }else{
                fprintf(stderr,"***** FIFO cliente aberto para WRITE *****\n");


                //ENVIA RESPOSTA
                res= write(c_fifo_fd, & respGest, sizeof(respGest));
                if (res == sizeof(respGest)){
                    fprintf(stderr,"***** Escreveu a resposta *****\n");
                }else{
                    perror("!!!!! erro a escrever a resposta !!!!!\n");
                }
                close(c_fifo_fd); //fecha logo o fifo do cliente!
                fprintf(stderr,"***** FIFO cliente fechado *****\n");
                msg[i].msgID=i;
                
                int pid2 = fork();

                switch (pid2) {
                    case -1:
                        printf("!!!!! Erro ao criar o FORK. pid<0 !!!!!\n");
                        return 0;
                        break;

                    case 0:
                        alarm(msg[i].duracao);
                        break;

                    default:
                        break;
                }
                i++;
            }
    }//fim do ciclo while(1)
}

int main(int argc, char *argv[]) {

    signal(SIGUSR1, trataSinal);
    signal(SIGALRM, eliminaMensagem);
    int c;
    opterr = 0;
    pthread_t tid, tid2, tid3;
    

    while (((c = getopt(argc, argv, "m:n:")) != -1) || argv[2] == NULL)
        switch (c) {
            case 'm':
                MAXMSG = atoi(optarg);
                
                break;
            case 'n':
                MAXNOT = atoi(optarg);
                
                break;
            case '?':
                if (optopt == 'm')
                    fprintf(stderr, "Opcao -%c requer um argumento.\n", optopt);
                else if (optopt == 'n')
                    fprintf(stderr, "Opcao -%c requer um argumento.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Opcao desconhecida`-%c'.\n", optopt);
                else
                    fprintf(stderr, "Caracter desconhecido `\\x%x'.\n", optopt);
                return 1;
            default:
                printf("Precisa de receber um argumento!!\n");
                abort();
        }

        if (MAXMSG < 1) {
            printf("Numero invalido de armazenamento de mensagens\n");
            return -1;
        }

        if (MAXNOT < 1) {
            printf("Numero invalido de Palavras proibidas\n");
            return -1;
        }
        
    printf("***WARNING*** : O numero maximo de mensagens a armazenar e: %d\n", MAXMSG);
    printf("***WARNING*** : O numero maximo de Palavras proibidas e: %d\n", MAXNOT);


    if(signal(SIGUSR1, trataSinal) == SIG_ERR){
        perror("!!!!! Não foi possivel configurar o sinal SIGUSR1 !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***Sinal SIGUSR1 configurado!***\n");

    res = mkfifo(GESTOR_FIFO, 0777);
    if(res == -1){
        perror("!!!!! mkfifo do FIFO do gestor deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do Gestor criado!***\n");

    res = mkfifo(GESTOR2_FIFO, 0777);
    if(res == -1){
        perror("!!!!! mkfifo do FIFO do gestor deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do Gestor criado!***\n");

    res = mkfifo(GESTORCONF_FIFO, 0777);
    if(res == -1){
        perror("!!!!! mkfifo do FIFO do gestor deu erro !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do Gestor criado!***\n");


    g_fifo_fd = open(GESTOR_FIFO, O_RDWR);
    if(g_fifo_fd == -1){
        perror("!!!!! Erro ao abrir o FIFO do gestor (RDWR/blocking) !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do gestor aberto para READ (+WRITE) bloqueante ***\n");

    g2_fifo_fd = open(GESTOR2_FIFO, O_RDWR);
    if(g2_fifo_fd == -1){
        perror("!!!!! Erro ao abrir o FIFO do gestor (RDWR/blocking) !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do gestor aberto para READ (+WRITE) bloqueante ***\n");

    gconf_fifo_fd = open(GESTORCONF_FIFO, O_RDWR);
    if(gconf_fifo_fd == -1){
        perror("!!!!! Erro ao abrir o FIFO do gestor (RDWR/blocking) !!!!!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "***FIFO do gestor aberto para READ (+WRITE) bloqueante ***\n");

    pthread_create(&tid3, NULL, estaOnline, NULL);
    pthread_create(&tid, NULL, atendeEscreveMsg, NULL);
    pthread_create(&tid2, NULL, atendeComandosCliente, NULL);
    menuAdministrador();
    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);



    exit(0);
}

