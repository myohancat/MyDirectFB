#include "event_loop.h"

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "log.h"

static void parse_option(int argc, char** argv);

static void _signal_handler(int signo)
{
    (void)signo;

    EventLoop::getInstance().terminate();
}

int main(int argc, char** argv)
{
__TRACE_FUNC__;
    parse_option(argc, argv);

    signal(SIGINT,  _signal_handler);
    signal(SIGQUIT, _signal_handler);
    signal(SIGTERM, _signal_handler);

    /* TODO IMPLEMENTS HERE */

    while(EventLoop::getInstance().loop()) { }

    /* TODO IMPLEMENTS HERE */

    return 0;
}

#include <string.h>
#include <getopt.h>
static void show_help(char* cmd)
{
    char* cmdname = strrchr(cmd, '/');
    if (cmdname)
        cmdname ++;
    else
        cmdname = cmd;

    printf("Usage: %s [OPTION]...\n", cmdname);
    printf("-l, --log_level      log level :\n");
    printf("                         [none|error|warn|info|debug|trace]\n");
    printf("-o, --log_output     log output :\n");
#ifdef ANDROID
    printf("                         [stdout|serial|adb]\n");
#else
    printf("                         [stdout|serial]\n");
#endif
    printf("-h, --help           help\n");
}

static void parse_option(int argc, char** argv)
{
    int c;

    LOG_OUTPUT_e eOutput   = LOG_OUTPUT_STDOUT;
    LOG_LEVEL_e  eLogLevel = LOG_LEVEL_TRACE;

    while (1)
    {
        /* name, [no_argument|required_argument|optional_argument], flag, val */
        static struct option long_options[] =
        {
            {"log_level",   required_argument,       0, 'l'},
            {"log_output",  required_argument,       0, 'o'},
            {"help",        no_argument,             0, 'h'},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        c = getopt_long (argc, argv, "hl:o:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'l':
                if (strcasecmp(optarg, "NONE") == 0)
                    eLogLevel = LOG_LEVEL_NONE;
                else if (strcasecmp(optarg, "ERROR") == 0)
                    eLogLevel = LOG_LEVEL_ERROR;
                else if (strcasecmp(optarg, "WARN") == 0)
                    eLogLevel = LOG_LEVEL_WARN;
                else if (strcasecmp(optarg, "INFO") == 0)
                    eLogLevel = LOG_LEVEL_INFO;
                else if (strcasecmp(optarg, "DEBUG") == 0)
                    eLogLevel = LOG_LEVEL_DEBUG;
                else if (strcasecmp(optarg, "TRACE") == 0)
                    eLogLevel = LOG_LEVEL_TRACE;
                else
                {
                    printf("Invalid argument : %s\n", optarg);
                    show_help(argv[0]);
                    exit(-1);
                }
                break;

            case 'o':
                if (strcasecmp(optarg, "STDOUT") == 0)
                    eOutput = LOG_OUTPUT_STDOUT;
                else if (strcasecmp(optarg, "SERIAL") == 0)
                    eOutput = LOG_OUTPUT_SERIAL;
#ifdef ANDROID
                else if (strcasecmp(optarg, "ADB") == 0)
                    eOutput = LOG_OUTPUT_ADB;
#endif
                else
                {
                    printf("Invalid argument : %s\n", optarg);
                    show_help(argv[0]);
                    exit(-1);
                }
                break;

                break;

            case 'h':
                show_help(argv[0]);
                exit(0);

            case '?':
                break;

            default:
                abort ();
        }
    }

    LOG_SetOutput(eOutput);
    LOG_SetLevel(eLogLevel);

#if 0
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }
#endif
}
