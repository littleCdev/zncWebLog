//
//  base64.c
//  zncWebLog
//
//  Created by Littlecheetah on 21.05.15.
//  Copyright (c) 2015 Littlecheetah. All rights reserved.
//

#define ext_bas64
#include "base64.h"



char *base64_encode( char *data,
                    int input_length,
                    int *output_length) {
    
    *output_length = 4 * ((input_length + 2) / 3);
    
    int i=0;
    int j=0;
    char *encoded_data = malloc(*output_length+1);
    if (encoded_data == NULL) return NULL;
    
    for (i = 0, j = 0; i < input_length;) {
        
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;
        
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }
    
    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = '\0';
    
    return encoded_data;
}


char *base64_decode(char *data,
                             int input_length,
                             int *output_length) {
    int i = 0;
    int j=0;
    
   	static char *decoding_table = NULL;
    decoding_table = malloc(256);
    
    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
    
    if (input_length % 4 != 0) return NULL;
    
    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;
    
    char *decoded_data = malloc(*output_length+1);
    if (decoded_data == NULL) return NULL;
    
    memset(decoded_data, 0x00, *output_length);
    
    for (i = 0, j = 0; i < input_length;) {
        
        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(int)data[i++]];
        
        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);
        
        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    decoded_data[*output_length] = '\0';
    free(decoding_table);
    return decoded_data;
}

/*
void build_decoding_table() {
    int i = 0;
    decoding_table = malloc(256);
    
    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}*/
