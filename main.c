/*
Name : Madana Bharath
Date : 27/09/2025
Title : Stegnography
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *argv)
{
    if(strcmp(argv,"-e") == 0)
        return e_encode;
    else if(strcmp(argv,"-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    int type;

    /* check operation type */
    if(argc>1)
    {
        type = check_operation_type(argv[1]);
        if(type == e_encode)
        {
            if(read_and_validate_encode_args(argv,&encInfo) == e_failure)
            {
                display_error_msg();
                return e_failure;
            }
            else
                do_encoding(&encInfo);
        }
        else if(type == e_decode)
        {
            if(read_and_validate_decode_args(argv,&decInfo) == e_failure)
            {
                display_error_msg();
                return e_failure;
            }
            else
                do_decoding(&decInfo);
        }
        else
        {
            display_error_msg();
        }
    }
    else
    {
        display_error_msg();
    }
    return 0;
}
