#ifndef _VOXDEFS_H_
#define _VOXDEFS_H_

#include "flite.h"
#ifdef __cplusplus
extern "C" {
#endif

#define VOXNAME cmu_us_rms
#define REGISTER_VOX register_cmu_us_rms
#define UNREGISTER_VOX unregister_cmu_us_rms
#define VOXHUMAN "rms"
#define VOXGENDER "unknown"
#define VOXVERSION 1.0

cst_voice *register_cmu_us_rms(const char *voxdir);
void unregister_cmu_us_rms(cst_voice *vox);

#ifdef __cplusplus
}
#endif
#endif /*_VOXDEFS_H_*/