#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "msg.h"    /* For the message struct */

using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the allocated message queue
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
    /* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
             so manually or from the code).
        2. Use ftok("keyfile.txt", 'a') in order to generate the key.
        3. Use the key in the TODO's below. Use the same key for the queue
            and the shared memory segment. This also serves to illustrate the difference
            between the key and the id used in message queues and shared memory. The id
            for any System V objest (i.e. message queues, shared memory, and sempahores)
            is unique system-wide among all SYstem V objects. Two objects, on the other hand,
            may have the same key.
     */
     key_t key = ftok("keyfile.txt", 'a');
  //  key_t key = ftok("nebil.txt", 'a');

     
     if(key < 0)
     {
         perror("ftok");
        exit(-1);
     }
    

    
    /* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
    shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT);
    
    if(shmid < 0)
    {
        perror("Error occured from  shmget");
        exit(-1);
    }
    /* TODO: Attach to the shared memory */
    sharedMemPtr = (char*) shmat(shmid, NULL, 0);
    
                          
    /* TODO: Attach to the message queue */
    msqid = msgget(key, 0666 | IPC_CREAT);
    
    if(msqid < 0)
    {
        perror("Error occured from msgget");
        exit(-1);
    }
    /* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
    
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    /* TODO: Detach from shared memory */
    if(shmdt(sharedMemPtr) < 0)
    {
        perror("Error occured from shmdt");
        exit(-1);
    }
    
}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char* fileName)
{
 
    

    /* A buffer to store message we will send to the receiver. */
    message sndMsg;
    
    /* A buffer to store message received from the receiver. */
    ackMessage rcvMsg;
    
    /* The number of bytes sent */
    unsigned long numBytesSent = 0;
    
    
    /* Open the file for reading */
    FILE* fp = fopen(fileName, "r");
    
    /* Was the file open? */
    if(!fp)
    {
        perror("Error occured from fopen");
        exit(-1);
    }
    
    /* Read the whole file */
    while(!feof(fp))
    {
        /* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.
          * fread will return how many bytes it has actually read (since the last chunk may be less
          * than SHARED_MEMORY_CHUNK_SIZE).
          */
        if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
        {
            perror("Error occured from fread");
            exit(-1);
        }
        
        /* TODO: count the number of bytes sent. */
        numBytesSent = sndMsg.size;
        /* TODO: Send a message to the receiver telling him that the data is ready
         * to be read (message of type SENDER_DATA_TYPE).

        
         */

        sndMsg.mtype = SENDER_DATA_TYPE;
        
         if(msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0) < 0)
        {
            perror("Error occured from msgsnd");
            exit(-1);
        }
        /* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
          * that he finished saving the memory chunk.
          */
      //  printf("======= 11 ============= \n");

        
//        do
//         {
//             msgrcv(msqid, &rcvMsg, sizeof(ackMessage) - sizeof(long), RECV_DONE_TYPE, 0);
//
//
//
//         }
//         while(rcvMsg.mtype != RECV_DONE_TYPE);
//
        
        if (rcvMsg.mtype != RECV_DONE_TYPE) {
            
            printf("Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us \n");

        }
        
    
        cout <<"rcvMsg.mtype " << rcvMsg.mtype << endl;
        cout <<"RECV_DONE_TYPE " << RECV_DONE_TYPE << endl;
        
        while(rcvMsg.mtype != RECV_DONE_TYPE){
            
            
            if(msgrcv(msqid, &rcvMsg, sizeof(ackMessage) - sizeof(long), RECV_DONE_TYPE, 0) < 0)
            {
                perror("Error occured from  msgrcv");
                exit(-1);
            }
        }

        
    }
    


  //  printf("======= 33 ============= \n");

    
    /** TODO: once we are out of the above loop, we have finished sending the file.
       * Lets tell the receiver that we have nothing more to send. We will do this by
       * sending a message of type SENDER_DATA_TYPE with size field set to 0.
      */
    sndMsg.size = 0;
    sndMsg.mtype = SENDER_DATA_TYPE;
    if(msgsnd(msqid, &sndMsg, sizeof(message) - sizeof(long), 0) < 0)
        {
            perror("Error occured from msgsnd");
            exit(-1);
        }
        
    /* Close the file */
    fclose(fp);
    
    return numBytesSent;
}

/**
 * Used to send the name of the file to the receiver
 * @param fileName - the name of the file to send
 */
void sendFileName(const char* fileName)
{
    /* Get the length of the file name */
    int fileNameSize = strlen(fileName);

    /* TODO: Make sure the file name does not exceed the
     * the maximum buffer size in the fileNameMsg
     * struct. If exceeds, then terminate with an error.
     */
    if(fileNameSize > (sizeof(fileNameMsg)-sizeof(long)))
    {
        perror("FILE NAME SIZE IS LONG ");
        exit(-1);
    }
    /* TODO: Create an instance of the struct representing the message
     * containing the name of the file.
     */
    fileNameMsg msg;
    /* TODO: Set the message type FILE_NAME_TRANSFER_TYPE */
    msg.mtype = FILE_NAME_TRANSFER_TYPE;
    /* TODO: Set the file name in the message */
    strncpy(msg.fileName, fileName, fileNameSize + 1);
    /* TODO: Send the message using msgsnd */
    if(msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) < 0)
    {
        perror("ERROR OCCURED FROM msgsnd");
        exit(-1);
    }
    
    printf("========== FILENAME SENT ===========\n");

}


int main(int argc, char** argv)
{
    
    /* Check the command line arguments */
    if(argc < 2)
    {
        fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
        exit(-1);
    }
        
    /* Connect to shared memory and the message queue */
    init(shmid, msqid, sharedMemPtr);
    
    /* Send the name of the file */
        sendFileName(argv[1]);
        

    /* Send the file */
    fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));
    
    /* Cleanup */
    cleanUp(shmid, msqid, sharedMemPtr);
    

        
    return 0;
}
