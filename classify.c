/* classify.c */

#include <string.h>
#include <math.h>

#include "common.h"

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.
    The function returns 1 if the first two bytes of the array start
    with the characters "\xff\d8", i.e. if there is a JPG header
    signature at the beginning of the cluster. The function returns 0,
    otherwise.    
*/
int has_jpg_header(char cluster[])
{
    return (cluster[0] == '\xff' && cluster[1] == '\xd8');
}

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.
    The function returns 1 if there are any (zero or more) '\x00' 
    characters at the end of the cluster, followed by the string 
    "\xff\d9". I.e. it checks if there is a JPG footer signature at 
    the end of the cluster. The function returns 0, otherwise.
    
    It is assumed that the "\xff\xd9" string is not split across
    two clusters, i.e. '\xff' being the last byte of one cluster
    and '\xd9' being the first byte of the next cluster (followed
    by all zero bytes).
*/
int has_jpg_footer(char cluster[])
{
    int current_pos = CLUSTER_SIZE; // Start at the end of the array

    while (current_pos > 0) {
        current_pos--;
        if (cluster[current_pos] == '\x00')  // if there are any zero bytes, ignore them
            continue;
        if (cluster[current_pos] == '\xd9') // if the first non-zero byte from
                                            // the end is '\xd9', check the
                                            // next character, otherwise,
                                            // return 0
            return (current_pos > 0 && 
                    cluster[current_pos-1] == '\xff'); // return 1 if next 
                                                       // character is '\xff'
        return 0;
    }
    
    return 0;
}

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.

    The function tries to determine if the bytes in the array
    are JPG data or not. Looping over all characters, it counts
    the occurrences of all byte values found in the data and stores
    them in a frequency array. If the count of '\xff' characters 
    (a special escape character in JPG compression) is
    larger than 2, then the data is most likely JPG, and the function
    returns 1. Otherwise, the Shannon entropy for the data is computed.
    If the entropy value is high (> 7), which indicates compressed data,
    the function also returns 1. Otherwise, 0 is returned.
*/
int has_jpg_body(char cluster[])
{
    int current_pos = 0;
    int freq[256];
    double entropy;
    unsigned char val;
    int i;
    
    memset(freq, 0, sizeof(freq));  // clear frequency array
    while (current_pos < CLUSTER_SIZE - 1) { // iterate over all characters
        val = (unsigned char)cluster[current_pos];
        freq[val] = freq[val] + 1; // increase the frequency count for the
                                   // current character
        current_pos++;
    }
    if (freq[255] > 2) // Entry 255 is the count for '\xff' 
        return 1;
    entropy = 0; // Start with 0 for entropy value
    // Shannon entropy is caluclated as -Sum{p(c)*log_2(p(c))}
    for (i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            entropy -= (double)freq[i]/CLUSTER_SIZE*log2((double)freq[i]/CLUSTER_SIZE);
        }
    }
    
    return (entropy > 7);
}

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.
    The function returns 1 if the first 15 bytes of the array start
    with the characters "<!DOCTYPE html>", i.e. if there is a HTML header
    signature at the beginning of the cluster. The function returns 0,
    otherwise.
    Note that this function only works for HTML files actually starting
    with the DOCTYPE tag. This severly limits its usefulness to recognize 
    HTML files in general, but is sufficient for this class.
*/
int has_html_header(char cluster[])
{
    return !(memcmp(cluster, "<!DOCTYPE html>", 15));
}

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.
    The function returns 1 if there are any (zero or more) '\x00' 
    characters at the end of the cluster, followed by the string 
    "</html>\n". I.e. it checks if there is a HTML footer signature at 
    the end of the cluster. The function returns 0, otherwise.
    
    It is assumed that the "</html>\n" string is not split across
    two clusters and that there is no extra whitespace or comments
    after the HTML closing tag.
*/
int has_html_footer(char cluster[])
{
    int current_pos = CLUSTER_SIZE;

    while (current_pos > 0) {
        current_pos--;
        if (cluster[current_pos] == '\x00')
            continue;
        if (current_pos > 6)
            return !(memcmp(&cluster[current_pos-7], "</html>\n", 8));
        return 0;
    }
    
    return 0;
}

/* 
    This function takes as its single parameter a pointer to
    an array of characters. The size of the array is assumed to
    be CLUSTER_SIZE bytes.
    The function attempts to determine if the data in the array belongs
    to an HTML file or not. The function iterates over all characters in the
    array and counts two things: the number of non-ascii characters as well 
    as the number of HTML tag bracket characters ('<' and '>').
    If the number of non-ascii characters is smaller than 10, or if the
    ratio of bracket characters to non-ascii characters is larger than 0.1,
    the data is considered to be HTML, and the function returns 1. Otherwise,
    the function returns 0.
*/
int has_html_body(char cluster[])
{
    int current_pos = 0;
    int non_ascii = 0;
    int bracket_count = 0;
    while (current_pos < CLUSTER_SIZE - 1) {
        if (((int) cluster[current_pos]) < 32)
            non_ascii++;
        if (cluster[current_pos] == '<' || cluster[current_pos] == '>')
            bracket_count++;
        current_pos++;
    }
    return (non_ascii < 10) || (((double)bracket_count/(double)non_ascii) > 0.1);
}
