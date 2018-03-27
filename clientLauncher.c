
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/wait.h>

// #define T 10
// #define N 20 max number of concurrent clients / max to be launched ?

#define SERVER_PORT 3000 //port server

/* codul de eroare returnat de anumite apeluri */
extern int errno;

pthread_t thread_id;
int active_clients;

int getConn(void);

void * track_active_clients(void * arg);

int main(int argc, char** argv)
{	
	srand(time(NULL));
	int i;
	int T = 10;
	int N;

	/*do{
		T = rand() % 120;
	}while(!T);
	printf("\n[LAUNCHER] T : %d\n", T); fflush(stdout);*/

	int server_socket = getConn();
	
	if(write(server_socket, &T, sizeof(int)) < 0)
	{
		perror("[clientLauncher] Eroare la write()\n");
		exit(errno);
	}

	if(read(server_socket, &N, sizeof(int)) < 0)
	{
		perror("[clientLauncher] Eroare la read()\n");
		exit(errno);
	}

	close(server_socket);
	
	int current_load_size;

	pid_t pid;

	pthread_create(&thread_id, NULL, track_active_clients, NULL);

	for(;;)
	{	
		if(active_clients < N)
		{	
			current_load_size = rand() % (N - active_clients) + 1;
			active_clients += current_load_size;
			printf("\n[LAUNCHER] active_clients : %d\n", active_clients); fflush(stdout);
			
			for(i = 0; i < current_load_size; ++i) 
			{
				pid = fork();
				if(pid == -1)
				{
					perror("fork");  
					exit(1);
				} 
				else if(pid == 0)
				{	
					execl("cLunch01","cLunch01",(char*)NULL);
				
					printf ("\nEroare exec() !\n"); fflush(stdout);
					exit (1);
				}
			}
		}
	}	
	exit(0);
}


void * track_active_clients(void * arg)
{
	while(1)
	{
		if(wait(NULL) != -1) --active_clients;
	}
}


int getConn(void)
{
int sd;			// descriptorul de socket
struct sockaddr_in server;	// structura folosita pentru conectare

/* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[client] Eroare la socket().\n");
      return errno;
    }
  

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* portul de conectare */
  server.sin_port = htons (SERVER_PORT);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
	perror ("[client]Eroare la connect().\n");
	return errno;
  }

  return sd;
}  