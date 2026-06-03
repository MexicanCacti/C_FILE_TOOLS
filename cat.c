/*
    1) Difference between file descriptor and a FILE*
        File descriptor is just an integer that is an index into the process's table of open file descriptors. FILE* actually contains metadata about the file, like the file position.

    2) Can read() returns fewer bytes than requested
        Yes, read returns the number of bytes that it had read, this can be less than or equal to the number of bytes requested. 
    
    3) Can write() write fewer bytes than requested
        Yes, write can write fewer bytes than requested. This could occur if maybe a signal handler interrupted the write or there is insufficent space
        * Partial writes, and write failures do occur.

    4) When should you retry after an error
        If an error is encountered, it is best to only retry after an error if some varialbe in the system had changed that could possible prevent that error from occuring again.
        * Depending on the error code, some errors are explicitly retryable.

    5) What does errno represent?
        errno stands for the system error code, it is used by the system to index then print out a message based on the number in errno
        * Errno is a thead-local variable that libraries and system calls can set to indicate when an operation fails. APIs will explicitly say that they set it if needed.
*/


#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

size_t fileListCapacity = 1024;
size_t readBufferCapacity = 1024;
size_t errorBufferCapacity = 1024;
int returnStatus = 0;

void getFileList(char** fileList, size_t* fileListLength, int* argc, char** argv);

// Pass in the string for perror to display along with the error.
void outputError(char* errorInfo);

int main(int argc, char** argv)
{
    char* fileList = malloc(fileListCapacity);
    char* readBuffer = malloc(readBufferCapacity);
    char* errorBuffer = malloc(errorBufferCapacity);

    if(!fileList || !readBuffer || !errorBuffer)
    {
        outputError("Error in inital buffer malloc");
        returnStatus = errno;
        goto cleanup;
    }

    size_t fileListLength = 0;

    // By default, writing to console output & reading from stdin
    int wd = 1;
    int rd = 0;

    ssize_t readBytes = 0;
    ssize_t writtenBytes = 0;

    // Used to store error codes
    int errorCheck = 0;

    getFileList(&fileList, &fileListLength, &argc, argv);

    // getFileList internally displays errors
    if(returnStatus != 0)
    {
        goto cleanup;
    }

    // If fileListLength is 0, then no input files were provided, so take from stdin
    if(fileListLength == 0)
    {   
        do
        {
            readBytes = read(rd, readBuffer, readBufferCapacity);
            if(readBytes < 0)
            {
                outputError("Error reading from stdin");
                returnStatus = errno;
                break;
            }
            
            writtenBytes = 0;
            while(writtenBytes < readBytes)
            {   
                ssize_t bytes = write(wd, readBuffer + writtenBytes, readBytes - writtenBytes);
                if(bytes < 0)
                {
                    outputError("Error writing to stdout");
                    returnStatus = errno;
                    goto cleanup;
                }
                writtenBytes += bytes;
            }

        } while (readBytes != 0);
    }
    else
    {
        size_t index = 0;
        while(index < fileListLength)
        {
            char* fileName = &fileList[index];
            index += strlen(fileName) + 1;
            rd = open(fileName, O_RDONLY);
            if(rd < 0)
            {
                returnStatus = errno;
                errorCheck = sprintf(errorBuffer, "Error Opening %s", fileName);
                errorCheck < 0 ? outputError("Error opening file & could not create error string with filename") : outputError(errorBuffer);
                continue;
            }
            
            do
            {
                readBytes = read(rd, readBuffer, readBufferCapacity);
                if (readBytes < 0)
                {
                    returnStatus = errno;
                    errorCheck = sprintf(errorBuffer, "Error Reading %s", fileName);
                    errorCheck < 0 ? outputError("Error reading file & could not create error string with filename") : outputError(errorBuffer); 
                    goto filecleanup;
                }

                writtenBytes = 0;
                while(writtenBytes < readBytes)
                {
                    ssize_t bytes = write(wd, readBuffer + writtenBytes, readBytes - writtenBytes);
                    if(bytes < 0)
                    {
                        outputError("Error writing to stdout");
                        returnStatus = errno;
                        goto filecleanup;
                    }
                    writtenBytes += bytes;
                }
            } while (readBytes != 0);

            filecleanup:
            errorCheck = close(rd);
            if(errorCheck < 0)
            {
                returnStatus = errno;
                errorCheck = sprintf(errorBuffer, "Error Closing %s", fileName);
                errorCheck < 0 ? outputError("Error closing rd") : outputError(errorBuffer);
                break;
            }

        }
            

    }

cleanup:
    free(fileList);
    free(readBuffer);
    free(errorBuffer);
    return returnStatus;;

}

void getFileList(char** fileList, size_t* fileListLength, int* argc, char** argv)
{
    // Get the list of files to read from, these are stored in argv. Store every file name in fileList!
    for(int i = 1 ; i < *argc; ++i)
    {
        size_t fileNameLength = strlen(argv[i]);
        if(*fileListLength + fileNameLength + 1 > fileListCapacity)
        {
            char* temp = (char*)realloc(*fileList, (*fileListLength + fileNameLength + 1) * 2);
            if(temp == NULL)
            {
                outputError("File List Realloc Error:");
                returnStatus = errno;
                break;
            }
            fileListCapacity = (*fileListLength + fileNameLength + 1) * 2;
            *fileList = temp;
        }

        memcpy(*fileList + *fileListLength, argv[i], strlen(argv[i]));
        *fileListLength += fileNameLength;

        // The list of all the files are seperated by a null term
        (*fileList)[(*fileListLength)++] = '\0';
    }
}

void outputError(char* errorInfo)
{
    perror(errorInfo);
}