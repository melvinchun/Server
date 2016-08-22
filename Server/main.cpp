#include <iostream>
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
#include <sstream>
#include <map>

using namespace std;

/* Puerto TCP de HTTP */
#define TCP_PORT   8000

void *atender_solicitud(void *);


int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";

    //thread t1(task1, "Hello");

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

    }
    return 0;
}

bool writeDataToClient(int sckt, const void *data, int datalen)
{
    const char *pdata = (const char*) data;

    while (datalen > 0){
        int numSent = send(sckt, pdata, datalen, 0);
        if (numSent <= 0){
            if (numSent == 0){
                printf("The client was not written to: disconnected\n");
            } else {
                perror("The client was not written to");
            }
            return false;
        }
        pdata += numSent;
        datalen -= numSent;
    }

    return true;
}

bool writeStrToClient(int sckt, const char *str)
{
    return writeDataToClient(sckt, str, strlen(str));
}

void *atender_solicitud(void *arg) {


    int 	*mysocfd = (int*) arg;
    char 	data[100];
    //int 	i;
    cout << "Listo para leer datos del cliente" << endl;
    //while (1) {
        /* Lee la data del socket */
    read(*mysocfd, data, 100);
    cout << data << endl;
    //}


    std::string request = data;
    std::istringstream iss(request);

    std::string method;
    std::string query;
    std::string protocol;


    if(!std::getline(std::getline(std::getline(iss, method, ' '), query, ' '), protocol))
    {
        std::cout << "ERROR: parsing request\n";
        return 0;
    }

    // reset the std::istringstream with the query string

    iss.clear();
    iss.str(query);

    std::string url;

    if(!std::getline(iss, url, '?')) // remove the URL part
    {
        std::cout << "ERROR: parsing request url\n";
        return 0;
    }

    // store query key/value pairs in a map
    std::map<std::string, std::string> params;

    std::string keyval, key, val;

    while(std::getline(iss, keyval, '&')) // split each term
    {
        std::istringstream iss(keyval);

        if(std::getline(std::getline(iss, key, '='), val))
            params[key] = val;
    }

    std::cout << "protocol: " << protocol << '\n';
    std::cout << "method  : " << method << '\n';
    std::cout << "url     : " << url << '\n';

    std::map<std::string, std::string>::iterator i;

    for(i = params.begin(); i != params.end(); ++i)
        std::cout << "param   : " << i->first << " = " << i->second << '\n';

    std::stringstream ss;
    ss << "/Users/Alvarez/Downloads/Server/mi_web" << url;
    const char *path = ss.str().c_str();

    long fsize;
    FILE *fp = fopen(path, "rb");
    if (!fp){
        perror("The file was not opened");
    }

    printf("The file was opened\n");

    if (fseek(fp, 0, SEEK_END) == -1){
        perror("The file was not seeked");
        exit(1);
    }

    fsize = ftell(fp);
    if (fsize == -1) {
        perror("The file size was not retrieved");
        exit(1);
    }
    rewind(fp);

    char *msg = (char*) malloc(fsize);
    if (!msg){
        perror("The file buffer was not allocated\n");
        exit(1);
    }

    if (fread(msg, fsize, 1, fp) != 1){
        perror("The file was not read\n");
        exit(1);
    }
    fclose(fp);

    printf("The file size is %ld\n", fsize);

    if (!writeStrToClient(*mysocfd, "HTTP/1.1 200 OK\r\n")){
        close(*mysocfd);
        return 0;
    }

    char clen[40];
    sprintf(clen, "Content-length: %ld\r\n", fsize);
    if (!writeStrToClient(*mysocfd, clen)){
        close(*mysocfd);
        return 0;
    }

    if (!writeStrToClient(*mysocfd, "Content-Type: image/png\r\n")){
        close(*mysocfd);
        return 0;
    }

    if (!writeStrToClient(*mysocfd, "Connection: close\r\n\r\n") == -1){
        close(*mysocfd);
        return 0;
    }

    //if (!writeStrToClient(new_socket, "<html><body><H1>Hello world</H1></body></html>")){
    if (!writeDataToClient(*mysocfd, msg, fsize)){
        close(*mysocfd);
        return 0;
    }
    printf("The file was sent successfully\n");
    close(*mysocfd);

    return 0;
}
