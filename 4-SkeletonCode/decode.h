#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Decode Info Structure */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output file info */
    char output_fname[100];
    FILE *fptr_output;

    int is_output_file_given; // Flag to check if output file name is given

    /* Secret file info */
    int secret_file_extn_size;
    char secret_file_extn[20];
    int secret_file_size;

} DecodeInfo;

/* Decode function prototypes */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

Status decode_byte_from_lsb(char *data, char *image_buffer);
Status decode_size_from_lsb(int *data, char *image_buffer);

#endif
