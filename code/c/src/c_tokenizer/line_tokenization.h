/* c_tokenizer/src/line_tokenization.h */
#ifndef LINE_TOKENIZATION_SENTRY
#define LINE_TOKENIZATION_SENTRY

#include "word_list.h"

#include <stdio.h>

enum tokenizer_option { tokenizer_opt_off = 0, tokenizer_opt_on = 1 };

void set_tokenization_options(enum tokenizer_option use_spec_chars);
int tokenize_input_line_to_word_list(FILE *f, 
        struct word_list **out_words, int *eol_char);

#endif
