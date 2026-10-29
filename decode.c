
#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    decInfo->fptr_output = NULL;
    return e_success;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if (argv[2] == NULL || strstr(argv[2], ".bmp") == NULL)
        return e_failure;

    decInfo->stego_image_fname = argv[2];
    decInfo->output_fname = NULL;

    if (argv[3] != NULL)
        decInfo->output_fname = argv[3];

    return e_success;
}

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    unsigned char value = 0;

    for (int i = 0; i < 8; i++)
    {
        value |= ((unsigned char)image_buffer[i] & 1U) << i;
    }

    *data = (char)value;
    return e_success;
}

Status decode_int_from_lsb(unsigned int *data, char *image_buffer)
{
    unsigned int value = 0;

    for (int i = 0; i < 32; i++)
    {
        value |= ((unsigned int)((unsigned char)image_buffer[i] & 1U)) << i;
    }

    *data = value;
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    char decoded[16];
    int len = strlen(magic_string);

    for (int i = 0; i < len; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&decoded[i], buffer);
    }

    decoded[len] = '\0';

    if (strcmp(decoded, magic_string) == 0)
        return e_success;

    return e_failure;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    unsigned int size;

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_int_from_lsb(&size, buffer);

    if (size == 0 || size >= MAX_FILE_SUFFIX)
        return e_failure;

    decInfo->extn_size = size;
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&decInfo->extn_secret_file[i], buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    unsigned int size;

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_int_from_lsb(&size, buffer);
    decInfo->size_secret_file = size;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (unsigned int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
            return e_failure;

        decode_byte_from_lsb(&ch, buffer);

        if (fwrite(&ch, 1, 1, decInfo->fptr_output) != 1)
            return e_failure;
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) != e_success)
        return e_failure;

    /* Skip the 54-byte BMP header */
    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
        goto failure;

    if (decode_magic_string(MAGIC_STRING, decInfo) != e_success)
        goto failure;

    if (decode_secret_file_extn_size(decInfo) != e_success)
        goto failure;

    if (decode_secret_file_extn(decInfo) != e_success)
        goto failure;

    if (decode_secret_file_size(decInfo) != e_success)
        goto failure;

    if (decInfo->output_fname == NULL)
    {
        char filename[100];

        snprintf(filename, sizeof(filename), "default%s",
                 decInfo->extn_secret_file);

        decInfo->fptr_output = fopen(filename, "wb");
    }
    else
    {
        decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    }

    if (decInfo->fptr_output == NULL)
        goto failure;

    if (decode_secret_file_data(decInfo) != e_success)
        goto failure;

    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);

    decInfo->fptr_output = NULL;
    decInfo->fptr_stego_image = NULL;

    return e_success;

failure:
    if (decInfo->fptr_output != NULL)
        fclose(decInfo->fptr_output);

    if (decInfo->fptr_stego_image != NULL)
        fclose(decInfo->fptr_stego_image);

    decInfo->fptr_output = NULL;
    decInfo->fptr_stego_image = NULL;

    return e_failure;
}
