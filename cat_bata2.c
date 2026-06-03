#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

size_t fileListCapacity = 1024;
size_t readBufferCapacity = 1024;
static volatile int keepCatting = 1;

int getFileList(char** fileList, size_t* fileListLength, int* argc, char** argv);
void catFile(char* fileName);
void outputError(char* errorInfo);

void intHandler(int dummy)
{
    keepCatting = 0;
}

/*
    Input: A list of file to take read from
    Cat outputs the contents of every file listed in the standard output
*/
int main(int argc, char** argv)
{
    char* fileList = malloc(fileListCapacity);
    size_t fileListLength = 0;

    int wd = 1;
    int rd = 0;


    int errorCheck = getFileList(&fileList, &fileListLength, &argc, argv);

    if(errorCheck != 0)
    {
        
        goto cleanup;
    }

    // stdin
    if(fileListLength == 0)
    {
        signal(SIGINT, intHandler);
        while(keepCatting)
        {
            cat(&rd, &wd);
        }
    }
    else
    {
        // Now iterate through the file list, attempt to open each one, then write to the console
        size_t index = 0;
        while(index < fileListLength)
        {
            char* fileName = &fileList[index];
            int rd = open(fileName, O_RDONLY);
            cat(&rd, &wd);
            errorCheck = close(fileName);
            
            if(errorCheck != 0)
            {

            }

            index += strlen(fileName) + 1;
        }
    }

cleanup:
    free(fileList);


    return errno;
}

int getFileList(char** fileList, size_t* fileListLength, int* argc, char** argv)
{
    // Get the list of files to read from, these are stored in argv. Store every file name in fileList!
    for(int i = 1 ; i < *argc; ++i)
    {
        size_t fileNameLength = strlen(argv[i]);
        if(*fileListLength + fileNameLength + 1 > fileListCapacity)
        {
            char* temp = (char*)realloc(*fileList, *fileListLength + fileNameLength * 2);
            if(temp == NULL)
            {
                outputError("File List Realloc Error:");
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

    return errno;
}


void cat(int* rd, int* wd)
{
    char* readBuffer = malloc(readBufferCapacity);
    char* errorBuffer = malloc(readBufferCapacity);
    
    if(rd < 0)
    {   
        int blah = sprintf(errorBuffer, "\nError Opening %s", fileName);
        outputError(errorBuffer);
        goto cleanupcat;
    }

    ssize_t readOffset = 1;
    ssize_t writeOffset = 1;
    while(readOffset > 0)
    {
        readOffset = read(rd, readBuffer, readBufferCapacity);

        if(readOffset < 0)
        {
            int blah = sprintf(errorBuffer, "\nError Reading %s", fileName);
            outputError(errorBuffer);
            break;
        }

        writeOffset = write(wd, readBuffer, readOffset);
        if(writeOffset < 0)
        {
            outputError("Error writing to console");
            break;
        }

    }
    

cleanupcat:
    if(rd > 0) close(rd);
    free(readBuffer);
}

void catFile(char* fileName)
{
    int wd = 1;
    char* readBuffer = malloc(readBufferCapacity);
    int rd = open(fileName, O_RDONLY);
    char* errorBuffer = malloc(readBufferCapacity);
    
    if(rd < 0)
    {   
        int blah = sprintf(errorBuffer, "\nError Opening %s", fileName);
        outputError(errorBuffer);
        goto cleanupcat;
    }

    ssize_t readOffset = 1;
    ssize_t writeOffset = 1;
    while(readOffset > 0)
    {
        readOffset = read(rd, readBuffer, readBufferCapacity);

        if(readOffset < 0)
        {
            int blah = sprintf(errorBuffer, "\nError Reading %s", fileName);
            outputError(errorBuffer);
            break;
        }

        writeOffset = write(wd, readBuffer, readOffset);
        if(writeOffset < 0)
        {
            outputError("Error writing to console");
            break;
        }

    }
    

cleanupcat:
    if(rd > 0) close(rd);
    free(readBuffer);
}

void outputError(char* errorInfo)
{
    perror(errorInfo);
}