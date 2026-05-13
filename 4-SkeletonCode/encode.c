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

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2],".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: Source image file should be .bmp\n");
        return e_failure;
    }
    if (strstr(argv[3],".txt") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("ERROR: Secret file should be .txt\n");
        return e_failure;
    }
    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "default.bmp";
    }
    else
    {
        //validate argv[4] is .bmp or not
        if (strstr(argv[4], ".bmp") == NULL)
    {
        return e_failure;
    }

        encInfo->stego_image_fname = argv[4];
    }
return e_success;
}
uint get_file_size(FILE *fptr)
{
    /* find size of secret.txt*/
    uint size;

    // move file pointer to end
    fseek(fptr, 0, SEEK_END);

    // get current position (this is size)
    size = ftell(fptr);

    // move back to beginning
    fseek(fptr, 0, SEEK_SET);

    return size;
}
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = (long)get_file_size(encInfo->fptr_secret);
    {

    }
    if(encInfo->image_capacity > (16+32+32+32+ (encInfo->size_secret_file * 8)))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
    {
        printf("INFO: Open files is success\n");
        if (check_capacity(encInfo) == e_success)
        {
            printf("Secret data can fit in image file\n");
            // Other encoding steps to be done here
        }
        else
        {
            printf("ERROR: Image Size is not sufficient enough to fit secret data\n");
            return e_failure;
        }
    }
    else
    {
        printf("ERROR: Open files is failure\n");
        return e_failure;
    }


    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("Copy bmp header is a success.\n");
    }
    else
    {
        printf("Copy bmp header is a failure.\n");
        return e_failure;
    }


    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("Encode magic string is a success.\n");
    }
    else
    {
        printf("Encode magic string is a failure.\n");
        return e_failure;
    }



    if (encode_secret_file_extn_size(4, encInfo) == e_success)
    {
        printf("Encode secret file extn size is a success.\n");
    }
    else
    {
        printf("Encode secret file extn size is a failure.\n");
        return e_failure;
    }



    if (encode_secret_file_extn("txt", encInfo) == e_success)
    {
        printf("Encode secret file extn is a success.\n");
    }
    else
    {
        printf("Encode secret file extn is a failure.\n");
        return e_failure;
    }


    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("Encode secret file size is a success.\n");
    }
    else
    {
        printf("Encode secret file size is a failure.\n");
        return e_failure;
    }


    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("Encode secret file data is a success.\n");
    }
    else
    {
        printf("Encode secret file data is a failure.\n");
        return e_failure;
    }
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("Copy remaining image data is a success.\n");
    }
    else
    {
        printf("Copy remaining image data is a failure.\n");
        return e_failure;
    }
    
    return e_success;
}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo->size_secret_file];
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    fread(str, 1, encInfo->size_secret_file, encInfo->fptr_secret);
    encode_data_to_image(str,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb((int)file_size,encInfo);
    return e_success;
}


/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image((char *)file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/* Encode secret file extenstion size */
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(size,encInfo);
    return e_success;
}

/* Encode a byte into LSB of image data array */
Status encode_size_to_lsb(int data, EncodeInfo *encInfo)
{
    char image_buffer[32];
    fread(image_buffer, 1, 32, encInfo->fptr_src_image);
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (31-i)) & 1);
    }
    fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image);
    return e_success;
}

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if (encode_data_to_image((char *)magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }

}
/* Encode function, which does the real encoding */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char image_buffer[8];
    for(int i = 0; i < size; i++)
    {
        if(fread(image_buffer, 1, 8, fptr_src_image) != 8)
        {
            return e_failure;
        }
        encode_byte_to_lsb(data[i], image_buffer);
        if(fwrite(image_buffer, 1, 8, fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }
    return e_success;
    }

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> (7-i)) & 1);
    }
    return e_success;
}
    


/* Copy bmp image header*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // BMP header is 54 bytes
    char bmp_header[54];

    // Go to starting of src image
    fseek(fptr_src_image, 0, SEEK_SET);

    // Read 54 bytes from src image
    if (fread(bmp_header, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    // Write those 54 bytes to dest image
    if (fwrite(bmp_header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    return e_success;
}
