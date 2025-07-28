#include <stdio.h>       // For printf(), perror()
#include <stdlib.h>      // For exit() , malloc ()
#include <string.h>      // For memset(), strlen(), etc.
#include <unistd.h>      // For close()
#include <sys/types.h>   // For data types like socket()
#include <sys/socket.h>  // For socket(), bind(), connect(), etc.
#include <netinet/in.h>  // For sockaddr_in, htons(), etc.
#include <arpa/inet.h>   // For inet_addr(), htons(), etc.
#include <syslog.h>      // For syslogs (openlog syslog ).
#include <signal.h>      // for sigaction 
#include <fcntl.h>
#include <sys/stat.h>



// Define the port number that will be used by this program.
#define PORT_NUMBER  9000
#define bool  int
#define FALSE 0
#define TRUE  1

int server_fd = -1 ;
int client_fd  =-1 ;
char  * buffer = NULL;

/*
    Handler that exit the program gracefully after 
    1- logging that  the signal has been catched 
    2- shutting down the client fd to tell the client that we will stop recieving  and sending any data 
    3- closing both the client and the server fd to free their resources.
    4- closing the log.
    5- deleting the /var/tmp/aesdsocketdata. 

*/
void signal_handler (int sig)
{

    syslog (LOG_INFO , "Caught signal, exiting");
    shutdown(client_fd,SHUT_RDWR);
    close (client_fd);
    close (server_fd);
    closelog();
    remove("/var/tmp/aesdsocketdata");
    free(buffer);
    exit(0);

}
bool is_packet_complete (FILE * fp)
{
        // Seek to the last byte
    if (fseek(fp, -1, SEEK_END) != 0) {
        fclose(fp);
        return FALSE;  // Probably empty file, so no newline
    }

    int last_char = fgetc(fp);
    fclose(fp);

    if (last_char == '\n'){
        return TRUE;
    }
    else{
        return FALSE;
    }

}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent exits
        exit(EXIT_SUCCESS);
    }

    // Child continues
    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // Redirect file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);


    // Change working directory (optional)
    chdir("/");

    // Set file mode creation mask to 0
    umask(0);
}
int  main (int argc , char * argv[])
{



    // Now proceed to normal socket server setup
    /*
    Assign the handlers to the signal SIGINT and SIGTERM 

    */


    signal(SIGINT,signal_handler);
    signal(SIGTERM,signal_handler);

    /* Create the socket server.
       AF_INET indicates that we are using IPV4.
       SOCK_STREAM to use tcp (transmission control protocol).
       0 to choose the default protocol suitable for the socket type we chose. 
    */ 

    server_fd = socket(AF_INET ,SOCK_STREAM,0);
    // Create the client file descriptor to be used in the accept function.
    buffer = malloc(1024);
    // Open the log to be able to use write in the syslog file using syslog function.
    openlog ("Socket server.",LOG_PID | LOG_CONS ,LOG_USER) ;



    // Check  if the socket has been created succesfully .
    if (server_fd == -1){
        printf("%s","creating the socket has failed");
        return -1 ;
    }

    // Specifying the details of the socket that we will create.

    struct sockaddr_in server_addr ;
    struct sockaddr_in client_addr ;

    // Saving the size of add inside a variable as this variable is needed in accept function to be modified 
    socklen_t addrlen = sizeof(client_addr);

     // This line specify that we will use IPV4 as our family
    server_addr.sin_family = AF_INET ; 
    
    // This line to attach the socket to port 9000
    server_addr.sin_port = htons(PORT_NUMBER);  
    /*
    // Convert the IP Address to binary format and save it in the address attribute of the addr struct
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr); 
    */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);       // 0.0.0.0


    // binding the socket to the IP we want.  
    if ( bind (server_fd,(struct sockaddr * )&server_addr , sizeof(server_addr)  ) < 0){
        syslog(LOG_ERR,"%s" ,"Binding has failed");
        return -1;
     }
    bool run_as_daemon = FALSE;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0) {
            run_as_daemon = TRUE;
        }
    }

    if (run_as_daemon) {
        daemonize();
    }
    // Listening for upcoming connections.
    if ( listen (server_fd ,1 )    ){
        syslog (LOG_ERR,"%s", "listening has failed ");
        return -1;

    }


    // Create a buffer that stores the ip address of the client.
    char client_IP [INET_ADDRSTRLEN]; 
    while (1){

        // Accept the upcoming connection request.
        client_fd = accept (server_fd,(struct sockaddr *) &client_addr , &addrlen);

        if (client_fd < 0)
        {
            syslog (LOG_ERR,"%s","Accepting the connection has been failed");
            return -1;
        }
        char connection_lost = FALSE ;
        
        // Get The ip address of the client.  
        inet_ntop(AF_INET,&client_addr.sin_addr,client_IP , INET_ADDRSTRLEN);

        // Log the IP Address of the client.
        syslog(LOG_INFO,"Accepted connection from %s",client_IP);
        while (!connection_lost){

            int bytes_read = read(client_fd,buffer,1024-1);
            // variable to indicate if the pakcet sended is complete
            bool packet_completed = FALSE;


            if (bytes_read > 0)
            {
                FILE * fp;

                buffer[bytes_read] = '\0';  // Null-terminate manually!
                if (buffer[bytes_read-1] == '\n')
                {

                    packet_completed = TRUE ;
                
                }


                // Dividing the recieved packets according to new line character
                char *token = strtok(buffer, "\n");


                // Append each data packet keep in mind that  data packets are  seperated by new  line 
                while (token !=NULL)
                {
                    // Open the file that we want to append to
                    fp = fopen("/var/tmp/aesdsocketdata", "a+");	
                    if (fp == NULL)
                    {
                        syslog(LOG_ERR ,"opening the file has failed");
                        return -1;
                    }
                    


                    // Write a line to the file
                    fprintf(fp, "%s", token);

                    // Move to the next data packet to be appended to the file.
                    token = strtok(NULL,"\n");
                    if ( packet_completed || token != NULL)
                    {
                        fprintf(fp ,"\n");
                        // Return to the beginning of the file to begin reading
                        fflush(fp);
                        rewind(fp);
                        while ( fgets(buffer,sizeof(buffer),fp) != NULL )
                        {
                        // Send the file content to the client. 
                        send(client_fd,buffer,strlen(buffer),0);
                        }
                        fseek(fp, 0, SEEK_END);

                    }

            }
            if (packet_completed == TRUE){
            	close (client_fd);
            	connection_lost = TRUE;
            	
            }
            packet_completed = FALSE;
            fclose(fp);
            
        }
            else if (bytes_read == 0){

                syslog(LOG_INFO , "Closed connection from %s", client_IP);
                connection_lost = TRUE ;
                // Close the client file descriptor to be able to connect to another client 
                // without leaking resources. 
                close (client_fd);

                }
            else {
                printf ("%s","Reading from the client has failed ");
                return -1;

                }
            }
    }


                   




}
