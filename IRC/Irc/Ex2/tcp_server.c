/*******************************************************************************
 * SERVIDOR no porto 9000, à escuta de novos clientes.  Quando surjem
 * novos clientes os dados por eles enviados são lidos e descarregados no ecran.
 *******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define SERVER_PORT 9000
#define BUF_SIZE	1024
#define IP "127.0.0.1"

void process_client(int fd, char* s);
void erro(char *msg);
int dados(int client_fd, int* dados);
void soma(int client_fd, int* dados);
void media(int client_fd, int* dados);
int isNumber(char* string);

int main() {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	char s[50];
	//ip[20];

	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(SERVER_PORT);

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	erro("na funcao socket");
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
	erro("na funcao bind");
	if( listen(fd, 5) < 0)
	erro("na funcao listen");
	client_addr_size = sizeof(client_addr);

	printf("Servidor hosted em %s:%d\n",inet_ntoa(addr.sin_addr),SERVER_PORT);
	while (1) {
		//clean finished child processes, avoiding zombies
		//must use WNOHANG or would block whenever a child process was working
		while(waitpid(-1,NULL,WNOHANG)>0);
		//wait for new connection
		client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
		if (client > 0) {
			if (fork() == 0) {
				close(fd);
				//inet_ntop(AF_INET,&((addr.sin_addr).s_addr),ip,INET_ADDRSTRLEN);
				process_client(client,s);
				exit(0);
			}
		close(client);
		}
	}
	return 0;
}

void process_client(int client_fd,char* s)
{
	int nread;
	char buffer[BUF_SIZE];
	int numeros[10];
	int check;
	int recebeuDados = 0;
	nread = read(client_fd, buffer, BUF_SIZE-1);
	//buffer[BUF_SIZE-2] = '\0';
	buffer[nread] = '\0';

	while (strcmp(buffer,"quit") != 0){
	 	puts(buffer);
		if(strcmp(buffer,"DADOS")==0){

			/*write(client_fd, buffer, 1 + strlen(buffer));

			fflush(stdout);
			*/
			check = dados(client_fd, numeros);
			if (check == 1)
				recebeuDados = 1;
		} 

		else if(strcmp(buffer,"SOMA")==0 && recebeuDados) soma(client_fd, numeros);
		
		else if(strcmp(buffer,"MEDIA")==0 && recebeuDados) media(client_fd, numeros);
		
		else write(client_fd, "Comando Invalido", 5); 
		memset(buffer, 0, sizeof(buffer));
		nread = read(client_fd, buffer, BUF_SIZE-1);
		puts(buffer);
		buffer[nread] = '\0';
		fflush(stdin);
    	fflush(stdout);
	}
	
	close(client_fd);
}

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}

int dados(int client_fd, int* numeros){
	
	int nread;
	int number, i;
	int temp[10];
	char buffer[BUF_SIZE];

	fflush(stdin);
	fflush(stdout);
	//fflush(mylife);
	//memset(buffer, 0, sizeof(buffer));

	for (i = 0; i < 10; i++){
		nread = read(client_fd,buffer,BUF_SIZE-1);
		buffer[nread] = '\0';

		number = isNumber(buffer);
		if (number == 0){
			write(client_fd,"Operacao invalida",18);
			fflush(stdout);
			return 0;
		}
		write(client_fd,"Operacao completa",18);
		temp[i] = atoi(buffer);
		fflush(stdin);
	}
	
	for(i = 0; i < 10; i++)
		numeros[i] = temp[i];
	//write(client_fd,"Operacao completa",18);
	return 1;
}

void soma(int client_fd, int* dados){
	
	int total = 0;
	char string[20];

	for (int i = 0 ; i < 10; i++)
		total += dados[i];

	sprintf(string,"%d\n",total);
	write(client_fd, string, strlen(string)+1);
}

void media(int client_fd, int* dados){
	
	int total = 0;
	char string[20];

	for (int i = 0 ; i < 10; i++)
		total += dados[i];

	total = total / 10;

	sprintf(string,"%d\n",total);
	write(client_fd, string, strlen(string)+1);
}

int isNumber(char* string){

	int i;

	for(i = 0; i < strlen(string); i++){
		if (string[i] < '0' || string[i] > '9')
			return 0;
	} 
	return 1;
}
