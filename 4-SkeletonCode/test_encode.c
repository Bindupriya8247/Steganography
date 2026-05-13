#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"


int main(int argc, char *argv[])
{
    EncodeInfo E1;
    if (argc < 2)
    {
        printf("Usage: %s <-e|-d>\n", argv[0]);
        return 1;
    }
    int res = check_operation_type(argv);
    if(res == e_encode)
    {
        printf("Selected encoding\n");
        if (read_and_validate_encode_args(argv, &E1) == e_success)
        {
            printf("INFO : Read and validate encode_args is success\n");
            if (do_encoding(&E1) == e_success)
            {
                printf("INFO : Encoding is done successfully\n");
            }
            else
            {
                printf("ERROR : Encoding failure\n");
                return 0;
            }
        }
        else
        {
            printf("INFO : Read and validate encode_args is failure\n");
            return 0;
        }

    }
    else if(res == e_decode)
    {
        DecodeInfo decInfo;
        printf("Selected decoding\n");

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("INFO : Read and validate decode_args is success\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("INFO : Decoding is done successfully\n");
            }
            else
            {
                printf("ERROR : Decoding failure\n");
                return 0;
            }
        }
        else
        {
            printf("INFO : Read and validate decode_args is failure\n");
            return 0;
        }

    }
    else
    {
        printf("Unsupported operation\n");
        printf("Usage:\nFor encoding : ./a.out -e beautiful.bmp secret.txt [stego.bmp]\n");
        printf("For decoding : ./a.out -d stego.bmp [output.txt]\n");
        return 0;
    }

    return 0;

}

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}


