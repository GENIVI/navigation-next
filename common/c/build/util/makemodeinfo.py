import os
import sys
import subprocess


def makeModeInfo(modeType):
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    # Set CCC mode - hybrid or offboard
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    curPath = os.path.abspath(".")
    branchName = os.path.split(curPath)[1]
    use_onboard_file = open('../../nbservices/' + branchName + '/include/private/useonboard.h',"w")
    if modeType == "onboard":
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n//#define OFFBOARD_SERVICE_ONLY\n#endif')
    else:
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n#define OFFBOARD_SERVICE_ONLY\n#endif')
    use_onboard_file.close()

CCC_MODE = "offboard"
if len(sys.argv) > 1:
    CCC_MODE = sys.argv[1].lower()

makeModeInfo(CCC_MODE)

