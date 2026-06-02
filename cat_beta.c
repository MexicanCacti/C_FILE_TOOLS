#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int useFileOutput = 0;

// In a loop, read into the buffer then write out whats currently in the buffer to the write file descriptor
int doReadWrite(int* rd, int* wd)
{
    int statusCode = 0;
    ssize_t readBufferSize = 1024;
    char readBuffer[readBufferSize];

    ssize_t readOffset = 1;
    ssize_t writeOffset = 1;

    readOffset = read(0, &readBuffer, readBufferSize);

    while(readOffset > 0)
    {
        readOffset = read(*rd, &readBuffer, readBufferSize);
        
        if(readOffset < 0)
        {
            perror("Read Error:");
            statusCode = errno;
            return statusCode;
        }

        writeOffset = write(*wd, &readBuffer, readOffset);
        if(writeOffset < 0)
        {

            perror("Write Error:");
            statusCode = errno;
            return statusCode;
        }

    }

    return statusCode;
}

/*
    A cat implementation that takes two args in the commandline
        1. The file name to read from (by default creates a blah.txt file to read from)
        2. The file name to write to (optional, by default writes to console)

*/
int main(int argc, char** argv)
{
    int errorCheck = 0;
    char* inputFileNamePointer = malloc(1024);
    strcpy(inputFileNamePointer, "blah.txt");

    char* outputFileNamePointer = malloc(1024);

    if(argc > 1)
    {
        memcpy(inputFileNamePointer, argv[1], 1024);
    }

    if(argc > 2)
    {
        memcpy(outputFileNamePointer, argv[2], 1024);
        useFileOutput = 1;
    }

    int rd;
    rd = open(inputFileNamePointer, O_RDONLY, 00700);
    
    // Couldn't open file for reading
    if(rd < 0)
    {
        perror("Input file Error");
        goto cleanup;
    }
    
    int wd = 1;
    // By default use console output, otherwise go ahead and open the file to write to & save in file descriptor
    if(useFileOutput)
    {
        wd = open(outputFileNamePointer, O_CREAT | O_WRONLY, 00700);
    }

    if(wd < 0)
    {
        perror("Output file Error");
        goto cleanup;
    }
    
    errorCheck = doReadWrite(&rd, &wd);

cleanup:
    // File names no longer needed, free the memory
    free(inputFileNamePointer);
    free(outputFileNamePointer);
    int closeCheck = 0;

    closeCheck = close(rd);

    if(useFileOutput)
    {
        closeCheck = close(wd);
    }

done:
    return errorCheck;

}