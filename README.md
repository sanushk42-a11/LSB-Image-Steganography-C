LSB Image Steganography in C

A command-line tool written in C that hides a secret text file inside a BMP image using Least Significant Bit (LSB) steganography, and extracts it back with no data loss.

How it works

Every byte of a BMP image's pixel data has a least significant bit. Changing that bit alters the pixel value by at most 1, which is invisible to the human eye. This tool stores the secret data one bit at a time in those bits.

Encoding steps:

Validate the input files (BMP image and secret file) Check that the image has enough capacity for the secret data Copy the BMP header unchanged Encode a magic string (used to confirm the image contains hidden data) Encode the secret file size Encode the secret file data bit by bit into the image bytes Copy the remaining image data unchanged

Decoding reverses the process: it checks the magic string, reads the size, then rebuilds the secret file.

Features Encode and decode from the command line Capacity check before encoding Magic string validation on decode Lossless recovery (verified with diff) Works with 24-bit BMP images Build gcc test_encode.c encode.c decode.c -o stego Usage

Encode:

./stego -e <input.bmp> <secret.txt> [output.bmp]

Decode:

./stego -d <stego.bmp> [output.txt]

Example:

./stego -e beautiful.bmp secret.txt stego.bmp ./stego -d stego.bmp decoded_secret.txt diff secret.txt decoded_secret.txt

No output from diff means the decoded file is identical to the original.

Sample output Selected Encoding Read and validate Success Open Files Successful width = 1024 height = 768 Image size : 2359296 Sec file size : 25 Check Capacity Successful Magic String Encoded Encoding Successfull Selected Decoding Decoding Successful Screenshots

Add your images here:

Original vs stego image Terminal output Concepts used

C programming, bitwise operations, file handling, BMP file format, structures, command-line arguments, modular design (separate encode and decode modules).

Project structure common.h shared definitions types.h custom types and status codes encode.c/.h encoding logic decode.c/.h decoding logic test_encode.c main program and argument parsing
