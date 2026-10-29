#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "decode.h"

int main(int argc,char *argv[])
{
    EncodeInfo encode;
   //validate the cmd args
   //./a.out -e secret.txt beautiful.bmp(optional)
  //-e->encoding
  //-d->decoding
  //arg count shouls be atleast 4 (if not-> print error)
  int ret=check_operation_type(argv);
  if(ret==e_encode)
  {
    printf("Selected Encoding\n");
   if(read_and_validate_encode_args(argv,&encode)==e_success)
   {
    printf("Read and validate Success\n");
   if((do_encoding(&encode))==e_success)
   {
    printf("Encoding Successfull\n");
   }
   else
   {
    printf("Encoding Failed\n");
   }
  }
   else
   {
    printf("Read and validate Failure\n");
    return 0;
   }
  }  
  else if(ret == e_decode)
{
    printf("Selected Decoding\n");

    DecodeInfo decode;

    if(argc < 3 || argc > 4)
    {
        printf("Invalid number of arguments\n");
        return 1;
    }

    if(read_and_validate_decode_args(argv, &decode) == e_success)
    {
        if(do_decoding(&decode) == e_success)
            printf("Decoding Successful\n");
        else
            printf("Decoding Failed\n");
    }
    else
    {
        printf("Invalid decoding arguments\n");
    }
}
  else
  {
    printf("Invalid Option\n");
  }
return 0;
}

OperationType check_operation_type(char *argv[])
{
    //check argv[1] is -e
    if((strcmp(argv[1], "-e"))==0)
    {
        return e_encode;
    } 
    else if((strcmp(argv[1], "-d"))==0)
    {
     return e_decode;
    }
    return e_unsupported;
}