#define __STATIC__
#define CHECKPOINTING_HAS_NAMESPACE 1
#include "src/Checkpointing.cpp"

#include "src/Restore.cpp"

#include "src/Save.cpp"

#include "src/MMap.cpp"
#include "src/Area.cpp"
#include "src/Static.cpp"
#include "src/MemMaps.cpp"
#include "src/Process.cpp"
#include "src/FileMap.cpp"
#include "src/FileDesc.cpp"
#include "src/FileHandlers.cpp"

// Checkpoint
#include "src/LibC.cpp"
#include "src/Thread.cpp"
#include "src/SysCalls.cpp"
#include "src/MainThread.cpp"
#include "src/FutexState.cpp"
#include "src/ThreadsLock.cpp"

// The tests
#include "src/File_test.cpp"
#include "src/MemMaps_test.cpp"
#include "src/Process_test.cpp"
#include "src/Chkpt_test.cpp"
