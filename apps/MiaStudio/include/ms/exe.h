#ifndef MS_EXE_H
#define MS_EXE_H

/**
 * @file exe.h
 *
 * shared MSFile executables
 */

#include "MSFile.h"


void ms_exe_init(oobj parent);


MSFile *ms_exe_mia_paint(void);


MSFile *ms_exe_mv(void);
MSFile *ms_exe_rm(void);
MSFile *ms_exe_cp(void);



#endif //MS_EXE_H
