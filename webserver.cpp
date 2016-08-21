#include <stdlib.h>
 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <string.h>
 #include <sys/uio.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <iostream>
 #include <string>

 using namespace std;
 /* Puerto TCP de HTTP */
 #define TCP_PORT   8000

 void *atender_solicitud(void *);

 int main(){
 	int 	sockfd, newsockfd;
 	socklen_t clilen;
 	struct sockaddr_in cli_addr, serv_addr;
  pthread_t hiloCliente;
 	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
 		cout << "No se pudo crear el socket server" << endl;
 		exit(0);
 	}
 	serv_addr.sin_family = AF_INET;
 	serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
 	serv_addr.sin_port = htons(TCP_PORT);
 	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
 		cout << "No se pudo hacer el binding al socket" << endl;
 		 exit(0);
 	}
  	// La operacion que escucha por requisiciones de conexion
 	listen(sockfd, 5);
  cout<<"Server Listo y escuchando..."<<endl;
 	clilen = (socklen_t)sizeof(cli_addr);
 	for(;;){
 		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
 		if(newsockfd < 0) {
 			cout << "No se pudo crear el socket del cliente" << endl;
 			exit(0);
 		}
 		/* Crea un nuevo hilo por requisicion de conexion */
 		pthread_create(&hiloCliente, NULL, atender_solicitud, (void *)&newsockfd);
    cout<<"Cliente creado"<<endl;
 	}
 	return 0;
 }

 /*
 Rutina para leer la solicitud
 */
 void *atender_solicitud(void *arg){
 	int 	*mysocfd = (int*) arg;
 	char 	data[100];
 	int 	i;
 	cout << "Listo para leer datos del cliente" << endl;
 	while (1) {
 		/* Lee la data del socket */
 		read(*mysocfd, data, 100);
 		cout << data << endl;
 	}
 	close(*mysocfd);
 }
