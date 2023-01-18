#include "base64.h"
#include <string.h>

void base64_encode(char *input, char *output) {
	int inputLen = strlen(input);
	int outputLen = base64_enc_len(input);
	//char output[outputLen];

    for (int i = 0, j = 0; i <= inputLen;) {
 
        char octet_a = i <= inputLen ? lookup(input[i++]) : 0;
        char octet_b = i <= inputLen ? lookup(input[i++]) : 0;
        char octet_c = i <= inputLen ? lookup(input[i++]) : 0;
		char octet_d = i <= inputLen ? lookup(input[i++]) : 0;

        unsigned long quartet = (octet_a << 3 * 6) + (octet_b << 2 * 6) + (octet_c << 1 * 6) + (octet_d << 0 * 6);
 
		output[j++] = (quartet >> 2 * 8) & 0xFF;
        output[j++] = (quartet >> 1 * 8) & 0xFF;
        output[j++] = (quartet >> 0 * 8) & 0xFF;
    }
 
    //return output;
}

int base64_enc_len(char *input) {
	int inputLen = strlen(input);
	int n = inputLen%4;
    return ( 3 * ((inputLen + n) / 4));
}

int lookup (char Ch)
{
	for (int i = 0; i<=sizeof(encoding_table); i++)
	{
		if (Ch == encoding_table[i]) return i;
	}
	return 0;
}
