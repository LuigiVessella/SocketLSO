/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <string.h>
#include <postgresql/libpq-fe.h>
#include <sys/time.h>

struct arg_struct {
    int client_sock;
    PGconn * dbConn;
};
 
//the thread function
void *connection_handler(void *args);
PGconn *dbConnection();
void executeQuery(PGconn *conn, char * user);
void checkUserDb(PGconn *conn, char * user);
void executePhotolesQuery(PGconn * conn, char * query);
char * executeSelectPhotolesQuery(PGconn * conn, char * query);

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    PGconn *dbConn = NULL; 
    struct arg_struct args;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);
    
    //instauro la connessione al db
    args.dbConn =  dbConnection();
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (args.client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if(pthread_create( &thread_id , NULL ,  connection_handler , (void*) &args) != 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *arguments)
{
    struct arg_struct *args = arguments; 
    //Get the socket descriptor
    int sock = args->client_sock;
    int read_size;
    char *message , client_message[2000];
     
    //Receive a message from client
    while( (read_size = recv(sock, client_message , 3000 * sizeof(char) , 0)) > 0 )
    {
		
        //end of string marker
		client_message[read_size] = '\0';


        checkUserDb(args->dbConn, client_message);

        if(strstr(client_message, "select")) {
            printf("si tratta della select");
            char * resultRecords = executeSelectPhotolesQuery(args->dbConn, client_message);
            //printf("ho ricevuto: \n%s\n", resultRecords);
            send(sock , resultRecords , strlen(resultRecords), MSG_CONFIRM);
            write(sock, "\n", strlen("\n"));
    
        }
		
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    
    
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }

    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    // Client closed socket so clean up
    close(sock);
    return 0;
} 

PGconn *dbConnection() {

    printf("dbConnection invoked\n");
    PGconn *conn;

   char *stringConn = "host=projectpotholes.postgres.database.azure.com dbname=postgres port=5432 user=adminpotholes password=potholes2.";
   
   conn = PQconnectdb(stringConn);                
    
   if(PQstatus(conn) == CONNECTION_BAD) {
      printf("unable to connect\n");
      conn = NULL;
   }
   else{
      printf("connesso al db\n"); 
   }

   return conn;

}
void checkUserDb(PGconn *conn, char * user) { 
    printf("checkUser invoked\n");
   
    if(strlen(user) > 10 ) {
        executePhotolesQuery(conn, user);
    }

    else {
        PGresult *res;
        char * query_pt1 = "select 1 from utente where nome = '";
        char * query_pt2 = "'";
        char * query = (char *)malloc(1 + strlen(query_pt1) + strlen(user) + strlen(query_pt2));

        strcpy(query, query_pt1);
        strcat(query, user);
        strcat(query, query_pt2);

        printf("query: %s\n", query);
        
        res = PQexec(conn, query);
        
        int rows = PQntuples(res);
        if(rows > 0 ) {
            printf("utente esiste\n");
            
        }
        else {
            printf("utente non presente\n");
            executeQuery(conn, user);
        }
    }
    
}

void executeQuery(PGconn *conn, char * user) {
    printf("executeUserInsert invoked\n");
    PGresult *res;
    char * status;
    char * query_pt1 = "insert into utente(nome)values('";
    char * query_pt2 = "')";
    char * query = (char *)malloc(1 + strlen(query_pt1) + strlen(user) + strlen(query_pt2));

    strcpy(query, query_pt1);
    strcat(query, user);
    strcat(query, query_pt2);
    //printf("la query: %s\n", query);
   
    res = PQexec(conn, query);

    status = PQresStatus(PQresultStatus(res));
    
    printf("%s\n", status);
}

void executePhotolesQuery(PGconn * conn, char * query){

    printf("executeBucaInsert invoked\n");
    PGresult *res;
    char * status;
    res = PQexec(conn, query);
    status = PQresStatus(PQresultStatus(res));
    printf("%s\n", status);

}

char * executeSelectPhotolesQuery(PGconn * conn, char * query) {

    printf("selectBuche invoked\n");

    PGresult *res;
    char * status;
    int col, row;

    int dimension = 0;

    res = PQexec(conn, query);
    status = PQresStatus(PQresultStatus(res));
    printf("%s\n", status);

    int res_count = PQntuples(res);
    printf("ho %d records\n", res_count);
    for(row = 0; row<res_count; row++) {
        for(col = 0; col < 5; col++) {
            dimension = dimension + strlen(PQgetvalue(res, row,col));
        }
   
    }
   

    char * resultRecords = (char *) malloc(50 + dimension);
    
    for(row = 0; row<res_count; row++) {
        for(col = 0; col < 5; col++) {
            strcat(resultRecords, PQgetvalue(res, row,col));
            
        }
       strcat(resultRecords, "\n");
    }

   
    return resultRecords;
}