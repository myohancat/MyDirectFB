#ifndef __INI_H_
#define __INI_H_

#include <stdio.h>

typedef int (*IniCB_Fn)(const char* section, const char* name, const char* value);

int parse_ini(FILE* file, IniCB_Fn cb);

#endif /* __INI_H_ */
