/*
 * tessassert.h
 *
 *  Created on: Jun 12, 2014
 *      Author: wliu
 */

#ifndef TESSASSERT_H_
#define TESSASSERT_H_

#include <stdio.h>
// Because there's very low frequency to fail the assert in AddRightEdges(),
// throw exception instead to avoid crash.
#define assert(exp) {if(!(exp)) {printf("assert failed!file=%s,line=%s\n",__FILE__, __FUNCTION__);fflush(stdout);throw -1;}}

#endif /* TESSASSERT_H_ */
