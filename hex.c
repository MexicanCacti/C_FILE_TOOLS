#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

size_t readBufferCapacity = 1024;
size_t fileListCapacity = 1024;
int returnStatus = 0;
size_t bytesPerLine = 8;

void getFileList(char** fileList, size_t* fileListLength, int* argc, char*** argv);
void outputError(char* errorInfo);

// hexdump - display file contents in hexadecimal, decimal, octal, or ascii
// For the purposes of this project, file contents will be displayed only in hexadecimal
// Each hex digit represents 4 bits, each bytes is 8 bits
int main(int argc, char** argv)
{
    char* fileList = malloc(fileListCapacity);
    size_t fileListLength = 0;
    char* readBuffer = malloc(readBufferCapacity);

    if(fileList == NULL)
    {
        returnStatus = errno;
        outputError("Error allocating fileList array");
        free(fileList);
        return returnStatus;
    }

    if(readBuffer == NULL)
    {
        returnStatus = errno;
        outputError("Error allocating readBuffer array");
        free(fileList);
        free(readBuffer);
        return returnStatus;
    }

    getFileList(&fileList, &fileListLength, &argc, &argv);
    if(returnStatus != 0){
        free(fileList);
        free(readBuffer);
        return returnStatus;
    }

    int rd = 0;

    int errorCheck = 0;

    // Note ssize_t only used when negative vals possible, otherwise size_t
    ssize_t readBytes = 0;  


    if(fileListLength > 0)
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
                outputError(fileName);
                continue;
            }

            printf("%s", fileName);
            unsigned int bytesRead = 0;
            do
            {
                readBytes = read(rd, readBuffer, readBufferCapacity);

                if(readBytes < 0)
                {
                    returnStatus = errno;
                    outputError(fileName);
                    break;
                }

                for(size_t b = 0; b < readBytes; ++b)
                {
                    if(bytesRead % bytesPerLine == 0)
                    {
                        printf("\n%x\t", bytesRead);
                    }
                    ++bytesRead;
                    unsigned char byte = readBuffer[b];
                    unsigned char upper = byte >> 4;
                    unsigned char lower = byte & 0x0F;

                    printf("%X%X ", upper, lower);
                    
                }
            } while (readBytes != 0);

            errorCheck = close(rd);
            if(errorCheck < 0)
            {
                returnStatus = errno;
                perror(fileName);
                break;
            }

            printf("\n");
        }

    }


    free(fileList);
    free(readBuffer);

    return returnStatus;

}

void getFileList(char** fileList, size_t* fileListLength, int* argc, char*** argv)
{

    for(int i = 1; i < *argc ; ++i)
    {
        size_t fileNameLength = strlen((*argv)[i]);
        if(*fileListLength + fileNameLength + 1 > fileListCapacity)
        {
            char* temp = (char*)realloc(*fileList, (*fileListLength + fileNameLength + 1) * 2);
            if(temp == NULL)
            {
                outputError("Filelist realloc error:");
                returnStatus = errno;
                return;
            }
            fileListCapacity = (*fileListLength + fileNameLength + 1) * 2;
            *fileList = temp;
        }

        memcpy(*fileList + *fileListLength, (*argv)[i], fileNameLength);
        *fileListLength += fileNameLength;

        (*fileList)[(*fileListLength)++] = '\0';
    }
}

void outputError(char* errorInfo)
{
    perror(errorInfo);
}