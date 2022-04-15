/* classify.h */

#ifndef __CS361_CLASSIFY__
#define __CS361_CLASSIFY__

int has_jpg_header(char cluster[]);
int has_jpg_footer(char cluster[]);
int has_html_header(char cluster[]);
int has_html_footer(char cluster[]);
int has_jpg_body(char cluster[]);
int has_html_body(char cluster[]);

#endif
