#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

void display_error_msg()
{
    printf("\033[1;35m-----------------------------------------------------------------------------------\033[0m\n\n");
    printf("ERROR:\n");
    printf("USAGE:\n");
    printf("Please provide neccessary command line arguments\n");
    printf("To encode pass like : ./a.out -e <sourceImage.bmp> <secretText.txt> [stego.bmp]\n");
    printf("To decode pass like : ./a.out -d <stego.bmp> [data.bmp]\n");
    printf("\033[1;35m-----------------------------------------------------------------------------------\033[0m\n\n");
}
uint get_image_size_for_bmp(FILE *fptr_image)
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

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}




/* Validate encode arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char extn[20];
    int i,k=0;
    // Source image
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
            printf("Source image file not contains '.bmp' extension\n");
            return e_failure;
        }
        encInfo->src_image_fname = argv[2];
    }
    else if(argv[2]==NULL)
    {
        printf("Source Image is not Passed\n");
        return e_failure;
    }

    // Secret file
    if(argv[3]!=NULL) 
    {
        k=0;
        for(i=0;argv[3][i]!='\0';i++)
        {
            if(argv[3][i]=='.')
                break;
        }
        extn[k++]=argv[3][i++];
        for( ;argv[3][i]!='\0';i++)
        {
            extn[k++]=argv[3][i];
        }
        extn[k]='\0';
        if(strcmp(extn,".txt")!=0 && strcmp(extn,".csv")!=0 && strcmp(extn,".c")!=0 && strcmp(extn,".py")!=0 && strcmp(extn,".cpp")!=0 && strcmp(extn,".sh")!=0 && strcmp(extn,".java")!=0)
        {
            printf("Secret file not contains '.txt/.csv/.c/.py/.cpp/.sh/.h/.java' extension\n");
            return e_failure;
        } 
        encInfo->secret_fname = argv[3];  
        strcpy(encInfo->extn_secret_file, extn);
    }
    else if(argv[3]==NULL)
    {
        printf("Secret file is not Passed\n");
        return e_failure;
    }

    // Stego image
    if(argv[4]!=NULL) 
    {
        k=0;
        for(i=0;argv[4][i]!='\0';i++)
        {
            if(argv[4][i]=='.')
                break;
        }
        extn[k++]=argv[4][i++];
        for( ;argv[4][i]!='\0';i++)
        {
            extn[k++]=argv[4][i];
        }
        extn[k]='\0';
        if(strcmp(extn,".bmp")!=0)
        {
            printf("Stego image file not contains '.bmp' extension\n");
            return e_failure;
        }
        encInfo->stego_image_fname=argv[4];
    }
    else if(argv[4]==NULL)
    {
        printf("Stego Image is not passed defaultly created as : %s\n","stego.bmp");
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}


/* check capacity */
Status check_capacity(EncodeInfo *encInfo) // do_encoding
{
     /* Find size of source image file */
     encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);


     /* Find sise of secret file size */
    fseek(encInfo->fptr_secret, 0, SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    if(encInfo->image_capacity < 54 + strlen(MAGIC_STRING)*8 + 32 + strlen(encInfo->extn_secret_file)*8 + 32 + encInfo->size_secret_file*8)
    {
        printf("Source Image file size is less than required for encoding\n");
        return e_failure;
    }
    else
    {
        printf("Source Image file size : %d KB\n",encInfo->image_capacity/1024);
        printf("Secret File size: %ld\n",encInfo->size_secret_file);
        return e_success;
    }
}


/* Copy first 54 Bytes of header from source image into stego image */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    rewind(fptr_src_image);
    if(fread(header,54,1,fptr_src_image))
    {
        fwrite(header,54,1,fptr_dest_image);
        return e_success;
    }
    return e_failure;
}


/* Encode Magic string */
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    if(encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        //printf("Magic string is not encoded\n");
        return e_failure;
    }
    return e_success;
}

/* Generic Encode Logic for magicstring, extension, secretdata */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    unsigned char buf[8];
    for(int i=0;i<size;i++)
    {
        fread(buf, 8 ,1, fptr_src_image);
        buf[8]='\0';
        //printf("%s\n",buf);
        encode_byte_to_lsb(data[i], buf);
        fwrite(buf, 8 ,1, fptr_stego_image);
    }
    return e_success;
}


Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int i=0;
    unsigned int bit;
    for(int pos=7;pos>=0;pos--)
    {
        bit = (data>>pos)&1;
        bit==0 ? (image_buffer[i]=image_buffer[i] & 0xFE) : (image_buffer[i]=image_buffer[i] | 1);
        i++;
    }
}

/* Encode secret file extension size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buf[32];
    fread(buf, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buf);
    if(fwrite(buf, 32, 1, encInfo->fptr_stego_image)==0)
        return e_failure;
    return e_success;
}

/* Encode int datatype  */
Status encode_size_to_lsb(int data, char *image_buffer)
{
    int i=0;
    unsigned int bit;
    for(int pos=31;pos>=0;pos--)
    {
        bit = (data>>pos)&1;
        bit==0 ? (image_buffer[i] = image_buffer[i] & 0xFE) : (image_buffer[i] = image_buffer[i] | 1);
        i++;
    }
}


/* Encode secret file extenstion */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    if(encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    
    char ch;
    char buf[8];
    int flag=0;
    rewind(encInfo->fptr_secret);
    //printf("Position : %ld",ftell(encInfo->fptr_secret));
    
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(&ch, 1, 1, encInfo->fptr_secret);
        //printf("%c",ch);
        fread(buf, 8, 1, encInfo->fptr_src_image);
        buf[8]='\0';
        encode_byte_to_lsb(ch, buf);
        if(fwrite(buf, 8, 1, encInfo->fptr_stego_image)>0)
            flag=1;
    }
    if(flag==1)
        return e_success;
    return e_failure;
}


/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src)>0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
}

/* Do encoding */
Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n\nEncoding starting\n");

    /* Call open file function */
    if(open_files(encInfo)==e_failure)
    {
        printf("File opening is failed\n");
        return e_failure;
    }
    else 
    {
        printf("Files opened successfully\n");
    }

    /* check capacity of files */
    if(check_capacity(encInfo)==e_failure)
    {
        printf("Capacity Checked failed\n");
        return e_failure;
    }
    else 
    {
        printf("Capacity Checked successfully\n");
    }



    /* Copy first 54 Bytes of header from source image into stego image */
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Header not copied into stego image\n");
        return e_failure;
    }
    else
    {
        printf("Header copied into stego image successfylly\n");
    }


    /* Encode Magic string */
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("Magic string is not encoded\n");
        return e_failure;
    }
    else
    {
        printf("Magic string is encoded successfully\n");
    }



    /* Encode secret file extension size */
    if(encode_secret_file_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure)
    {
        printf("Failed for secret file extension size encoding\n");
        return e_failure;
    }
    else
    {
        printf("Encoded secret file extension size successfully\n");
    }

    /* Encode secret file extension */
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("Failed for secret file extension encoding\n");
        return e_failure;
    }
    else
    {
        printf("Encoded secret file extension successfully\n");
    }


    /* Encode secret file size */
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("Failed for secret file size encoding\n");
        return e_failure;
    }
    else
    {
        printf("Encoded secret file size successfully\n");
    }



    /* Encode secret data */
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Failed for secret data encoding\n");
        return e_failure;
    }
    else
    {
        printf("Encoded secret data successfully\n");
    }



    /* Copy remaining image bytes from src to stego image after encoding */
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("Remaining data not encoded\n");
        return e_failure;
    }
    else
    {
        printf("Remaining data encoded successfully\n");
    }

    printf("Encoding completed successfully\n");
    printf("Done...\n");
    /* Close all files */
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

}


