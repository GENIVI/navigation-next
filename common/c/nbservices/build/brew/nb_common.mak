#-------------------------------------------------------------------------------
# Common definitions
#-------------------------------------------------------------------------------

SOL_DIR	   = ..\..
BREW_SDK_INC = C:\brew315\sdk\inc
BREW_INC = C:\brew315\inc
ARM_DIR = C:\dev\coretech\nimnbipal\main\thirdparty\arm\windows
ADSK_DIR = C:\dev\coretech\nimabpal\main\thirdparty\winmobile\lbsdriver\include
NIMNBIPAL_INC = C:\dev\coretech\nimnbipal\main\include
NIMABPAL_INC = C:\dev\coretech\nimabpal\main\include
NIMCORE_INC = C:\dev\coretech\nimcoreservices\main\include

#-------------------------------------------------------------------------------
# Include file search path options
#-------------------------------------------------------------------------------

INC = -I $(BREW_SDK_INC) \
      -I $(BREW_INC) \
      -I $(ARM_DIR) \
      -I $(ADSK_DIR) \
      -I $(NIMNBIPAL_INC) \
      -I $(NIMNBIPAL_INC)\brew \
      -I $(NIMABPAL_INC) \
      -I $(NIMABPAL_INC)\brew \
      -I $(NIMCORE_INC) \
      -I $(NIMCORE_INC)\brew \
      -I $(NIMCORE_INC)\util \
      -I $(NIMCORE_INC)\tps \
      -I $(NIMCORE_INC)\dbg \
      -I $(NIMCORE_INC)\gps \
      -I $(NIMCORE_INC)\network \
      -I $(NIMCORE_INC)\brew \
      -I $(SOL_DIR)\include \
      -I $(SOL_DIR)\include\brew \
      -I $(SOL_DIR)\include\private \
      -I $(SOL_DIR)\src\data \
      -I $(SOL_DIR)\src\geocode \
      -I $(SOL_DIR)\src\main \
      -I $(SOL_DIR)\src\map \
      -I $(SOL_DIR)\src\nav \
      -I $(SOL_DIR)\src\nbcommon \
      -I $(SOL_DIR)\src\network \
      -I $(SOL_DIR)\src\proxpoi \
      -I $(SOL_DIR)\src\qalog \
      -I $(SOL_DIR)\src\spatial \
      -I $(SOL_DIR)\src\util
  	
  	
#-------------------------------------------------------------------------------
# Compiler/assembler debug options
#-------------------------------------------------------------------------------

DBG= -DNDEBUG

#-------------------------------------------------------------------------------
# Network Server Selection Option
#-------------------------------------------------------------------------------

NETWORK= -DLAB_NETWORK

#-------------------------------------------------------------------------------
# QA Logging Option
#-------------------------------------------------------------------------------

#QALOG= -DENABLE_QA_LOGGING

#-------------------------------------------------------------------------------
# Debug UI Option
#-------------------------------------------------------------------------------

#DEBUGUI= -DENABLE_DEBUG_UI

#-------------------------------------------------------------------------------
# LRU Caching
#-------------------------------------------------------------------------------

CACHE= -DENABLE_VOICE_LRU

#-------------------------------------------------------------------------------
# GPS Settings
#-------------------------------------------------------------------------------
GPS= -DGPS_EMUPOSDET
