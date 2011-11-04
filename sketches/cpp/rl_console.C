
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

// use readline with flex
#ifdef YY_INPUT
#undef YY_INPUT
#endif
#define YY_INPUT(buf, result, max_size) \
    result = readline_to_lex(buf, max_size);

const char* prompt = "[test] ";

int readline_to_lex (char* buf, int max_size)
{//readline has no max buffer size, but lex does
    //a static buffer for readline's results
    static std::string line_buffer;
    static const char *pos = 0;
    static size_t chars_left = 0;

    if (chars_left == 0) {
        //read new line
        line_buffer = readline(prompt);
        line_buffer.push_back('\n');
        pos = line_buffer.c_str();
        chars_left = line_buffer.size();
    }

    //copy part of line buffer to lex buffer
    size_t size = (max_size > chars_left ? chars_left : max_size);
    memcpy (buf, pos, size);
    pos        += size;
    chars_left -= size;

    return size;
}

int main ()
{
    char * line_read = NULL;
    while (true) {

        //free memory if line is already allocated
        if (line_read) {
            free (line_read);
            line_read = NULL;
        }

        //read new line
        line_read = readline (prompt);

        //save non-empty lines to history
        if (line_read && *line_read) {
            add_history (line_read);
        }

        //write line back to screen
        std::cout << line_read << std::endl;
    }
}

