#include <stdio.h>
#include <stdlib.h>

#define OUT 0
#define IN 1

#define INIT OUT

int count_word(char *filename);
_Bool splite(char c, char prev_char);
_Bool is_letter(char c);

int count_word(char *filename) {
    int status = INIT;
    int word_count = 0;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    char c;
    char prev_char = ' ';

    while ((c = fgetc(fp)) != EOF) {
        if (splite(c, prev_char)) {
            // if the char is not alphabet, set status to OUT
            status = OUT;
        } else {
            if (OUT == status) {
                // if the char is alphabet and if the original status is OUT, 
                // set the status to be IN and work_count++
                status = IN;
                word_count++;
            }
        }
        prev_char = c;
    }

    fclose(fp);
    return word_count;
}

_Bool splite(char c, char prev_char) {

    if (is_letter(c)) {
        return 0;
    }

    // handle case where char is ' or -, 
    // ' - is valid word only its prev char is also a letter
    // TODO: the char follow by ' should also be a letter
    if ((c == '\'' || c == '-') && is_letter(prev_char)) {
        return 0;
    }

    return 1;
}

_Bool is_letter(char c) {
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z')
    ) {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2){
        return -1;
    }

    int word_count = count_word(argv[1]);
    printf("word count: %d", word_count);

}
