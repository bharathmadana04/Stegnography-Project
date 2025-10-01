#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"


Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    char extn[20];
    char *filename=malloc(20);
    int i,k=0;
    // Stego image
    if(argv[2]!=NULL) 
    {
        k=0;
        for(i=0;argv[2][i]!='\0';i++)
        {
            if(argv[2][i]=='.')
                break;
        }
        extn[k++]=argv[2][i++];
        for( ;argv[2][i]!='\0';i++)
        {
            extn[k++]=argv[2][i];
        }
        extn[k]='\0';
        if(strcmp(extn,".bmp")!=0)
        {
            printf("Stego image file not contains '.bmp' extension\n");
            return e_failure;
        }
        decInfo->stego_image_fname = argv[2];
    }
    else if(argv[2]==NULL)
    {
        printf("Stego Image is not Passed\n");
        return e_failure;
    }


    // Secret file
    if(argv[3]!=NULL) 
    {
        char *token=strtok(argv[3],".");
        strcpy(filename, token);
        strcat(filename,".txt");
        decInfo->secret_fname = filename;     
    }
    else if(argv[3]==NULL)
    {
        printf("Data file is not passed defaultly created as : 'data.txt'\n");
        decInfo->secret_fname = strdup("data.txt");
    }
    return e_success;
}

/* Get File pointers for i/p and o/p files */
Status open_files_decode(DecodeInfo *decInfo)
{
    /* Stego image file */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("Unable to open File 'output image file'");
        return e_failure;
    }


    /* Secret data output file */
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if (decInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

    	return e_failure;
    }
    return e_success;
}


/* Get output image size */
uint get_stego_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}



/* Skip Header 54 bytes */
Status skip_header(DecodeInfo *decInfo)
{
    rewind(decInfo->fptr_stego_image);
    /* Skip 54 bytes */
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);
    return e_success;
}



/* Decode Magic String */
Status decode_magic_string(char *data,DecodeInfo *decInfo)
{
    char buf[8];
    unsigned char ch;
    int j,k=0;
    for(int i=0;i<2;i++)
    {
        ch = 0x0000;
        j=0;
        fread(buf, 8, 1, decInfo->fptr_stego_image);
        for(int pos=7;pos>=0;pos--)
        {
            buf[j]&1==1 ? (ch=ch | 1<<pos) : ch;
            j++;
        }
        data[k++]=ch;
    }
}

/* Generic function for data (text content) decode */
Status decode_image_to_data(char *data,int size,DecodeInfo *decInfo)
{

}

/* Decode data file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buf[32];
    fread(buf, 32, 1, decInfo->fptr_stego_image);
    unsigned int ch=0;
    int size;
    int i=0;
    for(int pos=31;pos>=0;pos--)
    {
        buf[i]&1==1 ? (ch = ch | 1<<pos) : ch;
        i++;
    }
    decInfo->size_secret_file_extn  = ch;
    if(ch>0)
        return e_success;
    return e_failure;
}



/* Decode data file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo) 
{
    char buf[8];
    unsigned char ch;
    int j,k=0;
    for(int i=0;i<decInfo->size_secret_file_extn;i++)
    {
        ch = 0x0000;
        j=0;
        fread(buf, 8, 1, decInfo->fptr_stego_image);
        for(int pos=7;pos>=0;pos--)
        {
            buf[j]&1==1 ? (ch=ch | 1<<pos) : ch;
            j++;
        }
        decInfo->extn_secret_file[k++]=ch;
    }
}

/* Decode secret data file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buf[32];
    fread(buf, 32, 1, decInfo->fptr_stego_image);
    unsigned int ch=0;
    int size;
    int i=0;
    for(int pos=31;pos>=0;pos--)
    {
        buf[i]&1==1 ? (ch = ch | 1<<pos) : ch;
        i++;
    }
    decInfo->size_secret_file  = ch;
    if(ch>0)
        return e_success;
    return e_failure;

}

/* Decode secret data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buf[8];
    unsigned char ch;
    int j,k=0;
    for(int i=0;i<decInfo->size_secret_file;i++)
    {
        ch = 0x0000;
        j=0;
        fread(buf, 8, 1, decInfo->fptr_stego_image);
        for(int pos=7;pos>=0;pos--)
        {
            buf[j]&1==1 ? (ch=ch | 1<<pos) : ch;
            j++;
        }
        //printf("%c",ch);
        fwrite(&ch, 1, 1, decInfo->fptr_secret);
        //fprintf(decInfo->fptr_secret,"%c",ch);
    }
    return e_success;
}




/* Start Decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n\nEntered decoding\n");


    /* Open files */
    if(open_files_decode(decInfo) == e_failure)
    {
        printf("Failure in file opening\n");
        return e_failure;
    }
    else
    {
        printf("Files opened successfully\n");
    }
    
    
    /* Read output image file size */
    if(decInfo->size_stogo_file = get_stego_image_size_for_bmp(decInfo->fptr_stego_image) )
    {
        if(decInfo->size_stogo_file == e_failure)
        {
            printf("Unable to read the stego image size\n");
            return e_failure;
        }
        printf("Stego image size : %d KB\n",(int)decInfo->size_stogo_file/1024);
    }
    else
    {
        ;
    }


    /* Skip header */
    if(skip_header(decInfo) == e_failure)
    {
        printf("Header not skipped\n");
        return e_failure;
    }
    else
    {
        printf("Header skipped successfully\n");
    }

    /* Check Magic string contained in the stego image or not */
    char magic_string[2];
    if(decode_magic_string(magic_string,decInfo) == e_failure)
    {
        printf("Stego image not contains Magic String : '#*'\n");
        return e_failure;
    }
    else 
    {
        printf("Decoded magic string successfully : %s\n",magic_string);
    }


    /* Decode data file extension size */
    if(decode_secret_file_extn_size(decInfo) == e_failure)
    {
        printf("Failed in decoding data file extension size\n");
        return e_failure;
    }
    else
    {
        printf("Decoded data file extension size successfully : %ld\n",decInfo->size_secret_file_extn);
    }


    /* Decode data file extension */
    if(decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("Failed in decoding data file extension\n");
        return e_failure;
    }
    else
    {
        printf("Decoded data file extension successfully : %s\n",decInfo->extn_secret_file);
    }


    /* Decode secret data file size */
    if(decode_secret_file_size(decInfo) == e_failure)
    {
        printf("Failed in decoding data file size\n");
        return e_failure;
    }
    else
    {
        printf("Decoded data file size successfully : %ld\n",decInfo->size_secret_file);
    }


    /* Decode secret data */
    if(decode_secret_file_data(decInfo) == e_failure)
    {
        printf("Failed in decoding secret data\n");
        return e_failure;
    }
    else
    {
        printf("Decoded secret data succssfully\n");
    }

    printf("Decoding successfully completed\n");
    printf("Done...\n");

    /* Rename the data file from the decoded extension */
    char file[20], extn[10];
    strcpy(file, decInfo->secret_fname);
    strcpy(extn, decInfo->extn_secret_file);
    char *token=strtok(file,".");
    strcpy(file, token);
    strcat(file, extn);
    printf("%s\n",file);
    rename(decInfo->secret_fname, file);

    fclose(decInfo->fptr_secret);
    fclose(decInfo->fptr_stego_image);

    
    
}