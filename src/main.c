#include <stdio.h>

#include <ncurses.h>
/* undefine instr(x) from ncurses so it doesn't conflict with the one from perl */
#undef instr

#include <EXTERN.h>
#include <perl.h>

#define BUF_SIZE 128
#define TAB_WIDTH 4

/* key codes */
#define KEY_ENTER2 10

typedef struct text_line {
    int length;
    /* char* text; */
    SV* text;
} text_line;

typedef struct textbuffer {
    int num_lines;
    text_line* lines;
} textbuffer;

typedef struct editor {
    int screen_x;
    int screen_y;
    int cursor_x;
    int cursor_y;
    textbuffer tb;
} editor;

editor E;
PerlInterpreter* my_perl;

void perl_read_file(char* filename) {
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(sv_2mortal(newSVpv(filename, 0)));
    PUTBACK;
    int count = call_pv("slurp_file", G_ARRAY);
    SPAGAIN;

    E.tb.num_lines = count;
    E.tb.lines = malloc(count * sizeof(text_line));

    for (int i = 0; i < count; i++) {
        /* SV* sv = POPs; */
        /* char* str = SvPV_nolen(sv); */
        /* E.tb.lines[count - 1 - i].length = strlen(str); /\* popped in reverse order *\/ */
        /* E.tb.lines[count - 1 - i].text = malloc((strlen(str) + 1) * sizeof(char)); */
        /* strcpy(E.tb.lines[count - 1 - i].text, str); */

        E.tb.lines[count - 1 - i].text = newSVsv(POPs);
        E.tb.lines[count - 1 - i].length = strlen(SvPV_nolen(E.tb.lines[count - 1 - i].text)); /* popped in reverse order */
    }


    PUTBACK;
    FREETMPS;
    LEAVE;
}

void init_perl(int argc, char** argv, char** env) {
    char *my_argv[] = { "", "src/perl/utility.pl", NULL };

    my_perl = perl_alloc();
    perl_construct(my_perl);
    PERL_SYS_INIT3(&argc,&argv,&env);

    perl_parse(my_perl, NULL, 2, my_argv, (char **)NULL);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    perl_run(my_perl);
}

void end_perl() {
    perl_destruct(my_perl);
    perl_free(my_perl);
    PERL_SYS_TERM();
}

void init_editor(char* filename) {
    E.cursor_x = 0;
    E.cursor_y = 0;
    getmaxyx(stdscr, E.screen_y, E.screen_x);

    perl_read_file(filename);
}

void end() {
    end_perl();
    endwin();
}

void backspace() {
    WINDOW* win = stdscr;
    int x = getcurx(win);
    int y = getcury(win);

    if (x != 0) {
        mvwdelch(win, y, x - 1);
    } else {
        // delete row
    }

}

void process_char(int c) {

    WINDOW* win = stdscr;
    int x = getcurx(win);
    int y = getcury(win);

    switch (c) {
        case CTRL('q'):
            end();
            exit(0);
            break;
        case '\t':
            for (int i = 0; i < TAB_WIDTH; i++) {
                insch(' ');
            }
            move(y, x + TAB_WIDTH);
            break;
        case KEY_DOWN:
            if (y < E.tb.num_lines - 1) {
                if (y < E.screen_y - 1) {
                    if (x <= E.tb.lines[y + 1].length) {
                        move(y + 1, x);
                    } else {
                        move(y + 1, E.tb.lines[y + 1].length);
                    }
                } else {
                    /* scrl(1); */
                }
            }
            break;
        case KEY_UP:
            if (y > 0) {
                if (x <= E.tb.lines[y - 1].length) {
                    move(y - 1, x);
                } else {
                    move(y - 1, E.tb.lines[y - 1].length);
                }
            } else {
                /* scrl(-1); */
            }
            break;
        case KEY_HOME:
            move(y, 0);
            break;
        case KEY_END: {
            int limit_x;
            if (E.tb.lines[y].length < E.screen_x - 1) {
                limit_x = E.tb.lines[y].length;
            } else {
                limit_x = E.screen_x - 1;
            }
            move(y, limit_x);
            break;
        }
        case KEY_LEFT:
            move(y, x - 1);
            break;
        case KEY_RIGHT:
            if (x < E.tb.lines[y].length) {
                move(y, x + 1);
            }
            break;
        case KEY_ENTER:
        case KEY_ENTER2:
            addch('\n');
            break;
        case KEY_BACKSPACE:
            backspace();
            break;
        case KEY_DC:
            move(y, x + 1);
            backspace();
            break;
        default:
            insch(c);
            move(y, x + 1);
            //addch(c);
            //printw("%d", c);
    }
}

int main(int argc, char** argv, char** env) {

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    init_perl(argc, argv, env);

    char* filename = "test.txt";
    init_editor(filename);

    int limit_y;
    if (E.tb.num_lines > E.screen_y) {
        limit_y = E.screen_y;
    } else {
        limit_y = E.tb.num_lines;
    }
    for (int i = 0; i < limit_y; i++) {
        /* addnstr(E.tb.lines[i].text, E.screen_x); */
        addnstr(SvPV_nolen(E.tb.lines[i].text), E.screen_x);
        move(i + 1, 0);
    }

    addstr("Hello\n");
    printw("screen x: %d, screen y: %d\n", E.screen_x, E.screen_y);

    move(0,0);
    refresh();

    int c = '\0';
    while (1) {
        c = getch();
        process_char(c);
        refresh();
    }

    end();
    return 0;
}
