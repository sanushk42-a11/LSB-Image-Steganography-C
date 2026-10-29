#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include"common.h"
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

Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{


    if((strstr(argv[2],".bmp")!=NULL))
    {
        encInfo->src_image_fname=argv[2];
        if((strstr(argv[3],".txt"))!=NULL)
        {
            encInfo->secret_fname=argv[3];
            if(argv[4]==NULL)
            {
                encInfo->stego_image_fname="default.bmp";
            }
            else if((strstr(argv[4],".bmp"))!=NULL)
            {
                encInfo->stego_image_fname=argv[4];    
            }
            else
            {
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        } 
    }
    else
    {
return e_failure;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) != e_success)
        return e_failure;

    printf("Open Files Successful\n");

    if (check_capacity(encInfo) != e_success)
        return e_failure;

    printf("Check Capacity Successful\n");

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) != e_success)
        return e_failure;

    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
        return e_failure;

    printf("Magic String Encoded\n");

    strcpy(encInfo->extn_secret_file, ".txt");

    if (encode_secret_file_extn_size(
            encInfo->extn_secret_file, encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_extn(
            encInfo->extn_secret_file, encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_size(
            encInfo->size_secret_file, encInfo) != e_success)
        return e_failure;

    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;

    if (copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) != e_success)
        return e_failure;

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}



Status check_capacity(EncodeInfo *encInfo)
{

   encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
   encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);

   encInfo->extn_size=4;
   printf("Image size : %d\n",encInfo->image_capacity);
   printf("Sec file size : %ld\n",encInfo->size_secret_file);
   
   if(encInfo->image_capacity>(16+32+(encInfo->extn_size*8)+32+(encInfo->size_secret_file*8)))
   {
    return e_success;
   }
   else
   {
   return e_failure;
   }
}

uint get_file_size(FILE *fptr)
{
long size;
fseek(fptr,0,SEEK_SET);
fseek(fptr,0,SEEK_END);
size=ftell(fptr);
fseek(fptr,0,SEEK_SET);
if(size<0)
return 0;

return (uint)size;    
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char arr[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(arr,54,1,fptr_src_image);
    fwrite(arr,54,1,fptr_dest_image);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        if((data&(1<<i)))
        {
            image_buffer[i]=(image_buffer[i]&0xFE)|1;
        }
        else
        {
            image_buffer[i]=(image_buffer[i]&0xFE)|0;
        }
    }
    return e_success;
}
Status encode_int_to_lsb(int data, char *image_buffer)
{
    for(int i=0;i<32;i++)
    {
        if((data&(1<<i)))
        {
            image_buffer[i]=(image_buffer[i]&0xFE)|1;
        }
        else
        {
            image_buffer[i]=(image_buffer[i]&0xFE)|0;
        }
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
if(encode_data_to_image(magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
{
    return e_success;
}
return e_failure;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
char buffer[8];
for(int i=0;i<size;i++)
{
    fread(buffer,8,1,fptr_src_image);
    if((encode_byte_to_lsb(data[i],buffer))!=e_success)
    {
        return e_failure;
    }
fwrite(buffer,8,1,fptr_stego_image);
}
return e_success;
}

Status encode_secret_file_extn_size(const char *file_extn, EncodeInfo *encInfo)
{ 
   char buffer[32];

    encInfo->extn_size = strlen(file_extn);

    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    encode_int_to_lsb(encInfo->extn_size, buffer);

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
  return e_success;
}

//encode extn 
//encode the secret file size
//encode the secret file data
//copy the remaining data
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    if (encode_data_to_image(file_extn, encInfo->extn_size,encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }

    return e_failure;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    encode_int_to_lsb((int)file_size, buffer);

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    rewind(encInfo->fptr_secret);

    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        if (fread(&ch, 1, 1, encInfo->fptr_secret) != 1)
        {
            return e_failure;
        }

        if (encode_data_to_image(&ch, 1,encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            return e_failure;
        }
    }

    if (ferror(fptr_src))
    {
        return e_failure;
    }

    return e_success;
}
