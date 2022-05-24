#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#ifdef ANDROID
static LOG_OUTPUT_e       gLogOutput = LOG_OUTPUT_ADB;
#else
static LOG_OUTPUT_e       gLogOutput = LOG_OUTPUT_STDOUT;
#endif

static LOG_LEVEL_e        gLogLevel  = LOG_LEVEL_DEBUG;
static bool               gLogWithTime = true;

static pthread_mutex_t    gLock = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_LOG_OUT()    pthread_mutex_lock(&gLock)
#define UNLOCK_LOG_OUT()  pthread_mutex_unlock(&gLock)

static const char* cur_time_str(char* buf)
{
    timeval tv;
    gettimeofday(&tv, 0);
    time_t curtime = tv.tv_sec;
    tm *t = localtime(&curtime);

    sprintf(buf, "[%02d:%02d:%02d.%03ld] ", t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec/1000);

    return buf;
}

void LOG_SetOutput(LOG_OUTPUT_e eOutput)
{
    gLogOutput = eOutput;
}

LOG_OUTPUT_e LOG_GetOutput()
{
    return gLogOutput;
}

void LOG_SetLevel(LOG_LEVEL_e eLevel)
{
    gLogLevel = eLevel;
}

LOG_LEVEL_e LOG_GetLevel()
{
    return gLogLevel;
}

static FILE* output_device()
{
    static FILE* _outDev = NULL;

    if(gLogOutput == LOG_OUTPUT_SERIAL)
    {
        if (_outDev == NULL)
            _outDev = fopen("/dev/ttyS0", "w");
    }

    if(_outDev)
        return _outDev;

    return stdout;
}

void LOG_Print(int priority, const char* color, const char *fmt, ...)
{
    va_list ap;

    if(priority > gLogLevel)
    {
        return;
    }
#ifdef ANDROID
    if (gLogOutput == LOG_OUTPUT_ADB)
    {
        int android_prio;
        char buf[4*1024];

        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);

        switch(priority)
        {
            case LOG_LEVEL_NONE:
                android_prio = ANDROID_LOG_SILENT;
                break;
            case LOG_LEVEL_ERROR:
                android_prio = ANDROID_LOG_ERROR;
                break;
            case LOG_LEVEL_WARN:
                android_prio = ANDROID_LOG_WARN;
                break;
            case LOG_LEVEL_INFO:
                android_prio = ANDROID_LOG_INFO;
                break;
            case LOG_LEVEL_DEBUG:
                android_prio = ANDROID_LOG_DEBUG;
                break;
            case LOG_LEVEL_TRACE:
                android_prio = ANDROID_LOG_VERBOSE;
                break;
            default:
                android_prio = ANDROID_LOG_UNKNOWN;
                break;
        }

        __android_log_write(android_prio, TAG, buf);
    }
    else
#endif
    {
        FILE*   fp;

        LOCK_LOG_OUT();

        fp = output_device();

        if(color)
            fputs(color, fp);

        if(gLogWithTime)
        {
            char timestr[32];
            fputs(cur_time_str(timestr), fp);
        }

        va_start(ap, fmt);
        vfprintf(fp, fmt, ap);
        va_end(ap);

        if(color)
            fputs(ANSI_COLOR_RESET, fp);

        fflush(fp);

        UNLOCK_LOG_OUT();
    }
}

//#define ISPRINTABLE(c)  (((c)>=32 && (c)<=126) || ((c)>=161 && (c)<=254))
#define ISPRINTABLE(c)  (((c)>=32 && (c)<=126))

void LOG_Dump(int priority, const void* ptr, int size)
{
    FILE* fp;
    char buffer[1024];
    int ii, n;
    int offset = 0;
    const unsigned char* data = (const unsigned char*)ptr;

    if(priority > gLogLevel)
        return;

    LOCK_LOG_OUT();

    fp = output_device();

    while(offset < size)
    {
        char* p = buffer;
        int remain = size - offset;

        n = sprintf(p, "0x%04x  ", offset);
        p += n;

        if(remain > 16)
            remain = 16;

        for(ii = 0; ii < 16; ii++)
        {
            if(ii == 8)
                strcpy(p++, " ");

            if(offset + ii < size)
               n = sprintf(p, "%02x ", data[offset + ii]);
            else
               n = sprintf(p, "   ");

            p += n;
        }

        strcpy(p++, " ");
        for(ii = 0; ii < remain; ii++)
        {
            if(ISPRINTABLE(data[offset + ii]))
                sprintf(p++, "%c", data[offset + ii]);
            else 
                strcpy(p++, ".");
        }
        strcpy(p++, "\n");

        offset += 16;

        fputs(buffer, fp);
    }

    fflush(fp);

    UNLOCK_LOG_OUT();
}
