#ifndef BASE64_H
#define BASE64_H

static char encoding_table[] = {'\0', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'n', 'c', ' ', 'g', 'i',
                                    'j', 's', 'a', 'y', 'u', 'v', '9', 'b', 'c', 'd', '?', 'o', '!', '+', '-', ':', '/', '#', '*', '_', '\t'};

int base64_enc_len(char  *input);

void base64_encode(char *input, char *output);

int lookup (char);

#endif