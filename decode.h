#ifndef DECODE_H
#define DECODE_H

#include "types.h"

typedef struct _DecodeInfo
{

    /* Data File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[10];
    char secret_data[1024*8];
    long size_secret_file_extn;
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    long size_stogo_file;

} DecodeInfo;

uint get_stego_image_size_for_bmp(FILE *fptr_image);

Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status open_files_decode(DecodeInfo *decInfo);

Status skip_header(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(char *data,DecodeInfo *decInfo);

Status decode_image_to_data(char *data,int size,DecodeInfo *decInfo);

/* Decode data file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo) ;

/* Decode secret data */
Status decode_secret_file_data(DecodeInfo *decInfo);
#endif