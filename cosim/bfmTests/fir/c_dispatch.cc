//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************
#include <unistd.h>
#include "v2c_cmds.h"
#include "access.h"
#include "c_dispatch.h"
#include "c_module.h"
#include "cep_apis.h"
#include "cep_adrMap.h"
#include "simPio.h"
/*
 * main 
 */
int main(int argc, char *argv[])
{

  /* ===================================== */
  /*   SETUP SECTION FOR SIMULATION        */
  /* ===================================== */
  unsigned long seed;
  sscanf(argv[1],"0x%x",&seed);  
  printf("Seed = 0x%x\n",seed);
  int errCnt = 0;
  int verbose = 0x1f;

  /* ===================================== */
  /*   spawn all the paralle threads       */
  /* ===================================== */
  int activeSlot=0; // only 1 board 
  //
  // ============================  
  // fork all the tests here
  // ============================  
  //
  shPthread thr;
  //
  // max number of cores not include the system thread
  //
  int maxHost = MAX_CORES; // number of cores/threads
  //
  // each bit is to turn on the given core (bit0 = core0, bit1=core1, etc..)
  //
  long unsigned int mask = 1 << (seed & 0x3);
  //
  // Set the active CPU mask before spawn the threads...
  //
  thr.SetActiveMask(mask);
  //
  // c_module is the threead to run
  //
  for (int i=0;i<maxHost;i++) {
    if ((long unsigned int)(1 << i) & mask) {
      thr.ForkAThread(activeSlot,i,verbose, seed * (1+i), c_module);
    }
  }
  //
  // lastly: Added system thread always
  //
  thr.AddSysThread(SYSTEM_SLOT_ID,SYSTEM_CPU_ID);
  //
  DUT_WRITE_DVT(DVTF_TOGGLE_CHIP_RESET_BIT,DVTF_TOGGLE_CHIP_RESET_BIT, 1);
  //
  // Enable vector capture-ing
  //  DUT_WRITE_DVT(DVTF_FIR_CAPTURE_EN_BIT,DVTF_FIR_CAPTURE_EN_BIT,1);
  // ============================
  // Turn on the wave here
  // ============================  
  //
  int cycle2start=0;
  int cycle2capture=-1; // til end
  int wave_enable=1;
  #ifndef NOWAVE
  dump_wave(cycle2start, cycle2capture, wave_enable);
  #endif
  //
  // ============================  
  // wait until all the threads are done
  // ============================  
  //
  int Done = 0;
  while (!Done) {
    Done = thr.AllThreadDone();
    sleep(2);
  }
  //
  //
  /* ===================================== */
  /*   END-OF-TEST CHECKING                */
  /* ===================================== */
  errCnt += thr.GetErrorCount();
  if (errCnt != 0) {
    LOGE("======== TEST FAIL ========== %x\n",errCnt);
  } else {
    LOGI("%s ======== TEST PASS ========== \n",__FUNCTION__);
  }
  //
  // shutdown HW side
  //
  thr.Shutdown();
  return(errCnt);
}
