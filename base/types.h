/**
 * @author kyungin.kim < myohancat@naver.com >
 * my simple event loop source code
 */
#ifndef __TYPES_H_
#define __TYPES_H_

#include <stdbool.h>
#include <stdint.h>

#ifndef NELEM
#define NELEM(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))
#endif /* NELEM */

#ifndef NULL
#define NULL      ((void*)0)
#endif /* NULL */


#endif /* __TYPES_H_ */
