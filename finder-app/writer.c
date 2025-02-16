#include <syslog.h>
#include <stdio.h>

int main (int argc , char *argv [])
{
	if (argc <2)
	return 1;
    //open systemlog with an identifier
    openlog("Writer program" ,LOG_PID | LOG_CONS ,LOG_USER);


    //setup syslogging with LOG_USER 
    FILE *file = fopen(argv[1],"w");  // open the file 


    // check if the file is opened or not and log th error if the file could not be opened.
    if (file == NULL){
        syslog(LOG_ERR,"could not open the file");
        return 1;
    }
    else
    {
        //log that the file has been opened succesfully.
        syslog(LOG_INFO, "the file has been opened succesfully");
        //write the string to the file.
        if (fprintf(file ,"%s", argv[2]) < 0)
        {
            // if the string could not be written log this.
            syslog(LOG_ERR,"Could not write to the file");
            return 1;
        }
        else
        {
            // If the string is written lof this info. 
            syslog(LOG_DEBUG ,"Writing %s to %s" ,argv[2],argv[2]);
        
        }

    }
    if (fclose(file) < 0)
    {
    
        // If the file could not be closed.  
        syslog(LOG_ERR," Could not close the file ");
    
    }
    
    else
    {
    
        // logging that the file has been closed succesfully.
        syslog(LOG_INFO,"The file has been closed succesfully");
    
    }
    closelog();
    return 0;





}
