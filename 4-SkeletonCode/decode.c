#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "decode.h"
#include "common.h"

/* Read and validate decode args */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // argv[2] must be stego.bmp
    if (argv[2] == NULL)
    {
        printf("ERROR: Please pass stego image file\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL)
    {
        printf("ERROR: Stego image must be .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    // output file optional
    if (argv[3] != NULL)
    {
        strcpy(decInfo->output_fname, argv[3]);
        decInfo->is_output_file_given = 1;
    }
    else
    {
        // default name (extension will be added later)
        strcpy(decInfo->output_fname, "decode");
        decInfo->is_output_file_given = 0;
    }

    return e_success;
}

/* Open stego image file */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open stego image file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

/* Decode main function */
Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) != e_success)
    {
        printf("ERROR: open_decode_files failed\n");
        return e_failure;
    }

    printf("INFO: Open files success\n");

    /* Step 1: skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Step 2: decode magic string */
    if (decode_magic_string(decInfo) != e_success)
    {
        printf("ERROR: Magic string not matched. Not a valid stego image.\n");
        return e_failure;
    }
    printf("INFO: Magic string matched\n");

    /* Step 3: decode extn size */
    if (decode_secret_file_extn_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension size\n");
        return e_failure;
    }
    printf("INFO: Extension size = %d\n", decInfo->secret_file_extn_size);

    /* Step 4: decode extn */
    if (decode_secret_file_extn(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode extension\n");
        return e_failure;
    }
    printf("INFO: Extension = %s\n", decInfo->secret_file_extn);

    /* Create output file name with extension */
    if (decInfo->is_output_file_given == 0)
    {
        strcat(decInfo->output_fname, ".");
        strcat(decInfo->output_fname, decInfo->secret_file_extn);
    }

    /* Open output file */
    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to create output file %s\n", decInfo->output_fname);
        return e_failure;
    }

    /* Step 5: decode secret file size */
    if (decode_secret_file_size(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret file size\n");
        return e_failure;
    }
    printf("INFO: Secret file size = %d bytes\n", decInfo->secret_file_size);

    /* Step 6: decode secret file data */
    if (decode_secret_file_data(decInfo) != e_success)
    {
        printf("ERROR: Failed to decode secret file data\n");
        return e_failure;
    }

    printf("INFO: Decoding completed successfully\n");
    printf("INFO: Output file = %s\n", decInfo->output_fname);

    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);

    return e_success;
}

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char decoded_char;
    char buffer[8];

    for (int i = 0; i < (int)strlen(MAGIC_STRING); i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&decoded_char, buffer);

        if (decoded_char != MAGIC_STRING[i])
            return e_failure;
    }

    return e_success;
}

/* Decode secret file extension size (32 bits) */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];

    fread(image_buffer, 1, 32, decInfo->fptr_stego_image);

    int size = 0;
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);
    }

    decInfo->secret_file_extn_size = size;

    printf("INFO: Extension size = %d\n", decInfo->secret_file_extn_size);

    return e_success;
}

/* Decode secret file extension string */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];

    for (int i = 0; i < decInfo->secret_file_extn_size; i++)
    {
        fread(image_buffer, 1, 8, decInfo->fptr_stego_image);

        char ch;
        decode_byte_from_lsb(&ch, image_buffer);

        decInfo->secret_file_extn[i] = ch;
    }

    decInfo->secret_file_extn[decInfo->secret_file_extn_size] = '\0';

    printf("INFO: Decoded extension = %s\n", decInfo->secret_file_extn);

    return e_success;
}


/* Decode secret file size (32 bits) */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    int size;

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_size_from_lsb(&size, buffer);

    decInfo->secret_file_size = size;
    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&ch, buffer);

        if (fwrite(&ch, 1, 1, decInfo->fptr_output) != 1)
            return e_failure;
    }

    return e_success;
}

/* Decode 1 byte from 8 bytes of image data */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0;
    for (int i = 0; i < 8; i++)
    {
        *data = (*data << 1) | (image_buffer[i] & 1);  // MSB-first
    }
    return e_success;
}

/* Decode 32-bit size from 32 bytes of image data */
Status decode_size_from_lsb(int *data, char *image_buffer)
{
    *data = 0;

    for (int i = 0; i < 32; i++)
    {
        *data = (*data << 1) | (image_buffer[i] & 1);
    }

    return e_success;
}
