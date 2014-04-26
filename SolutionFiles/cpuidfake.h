////// MTMon.h ///////////////////////////////////////// © 2010-01-22 AgF //
//
//            Header file for CpuidFake program
//
// This header file contains class declarations, function prototypes, 
// constants and other definitions for this program.
//
// © 2010 GNU General Public License www.gnu.org/licenses
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <windef.h>
#include <winnt.h>
#include <winsvc.h>
#include <stdlib.h>
#include <stdio.h>

// maximum number of threads. Must be 4 or 8.
#define MAXTHREADS  4

// list of input/output data structures for MSR driver
#define MAX_QUE_ENTRIES 32                  // maximum number of entries in queue

// define Model Specific Register numbers
#define MSRID0 0x1204        // This MSR contains control bits and family, model, etc.
#define MSRID1 0x1206        // This MSR contains part of the vendor string
#define MSRID2 0x1207        // This MSR contains part of the vendor string
#define VENDOR_BIT (1 << 8)  // This bit in MSRID0 enables alternative vendor string


// Class definition. Queue of MSR commands
class CMSRInOutQue {
public:
   // constructor
   CMSRInOutQue();
   // put record in queue
   int put (EMSR_COMMAND msr_command, unsigned int register_number,
      unsigned int value_lo, unsigned int value_hi = 0);
   // list of entries
   SMSRInOut queue[MAX_QUE_ENTRIES];
   // get size of queue
   int GetSize () {return n;}
protected:
   // number of entries
   int n;
};


// class CMSRDriver encapsulates the interface to the driver MSRDriver32/64.sys
// which is needed for privileged access to set the model specific registers. 
// This class loads, unloads and sends commands to MSRDriver
class CMSRDriver {
public:
	CMSRDriver();                            // constructor
	~CMSRDriver();                           // destructor
   LPCTSTR GetDriverName();                 // get name of driver
   // send commands to driver to read or write MSR registers
	int AccessRegisters(void * pnIn, int nInLen, void * pnOut, int nOutLen);
   // send commands to driver to read or write MSR registers
   int AccessRegisters(CMSRInOutQue & q);
   // read performance monitor counter
   // send command to driver to read one MSR register
   long long MSRRead(int r);
   // send command to driver to write one MSR register
   int MSRWrite(int r, long long val);
   // send command to driver to read one control register
   size_t CRRead(int r);
   // send command to driver to write one control register
   int CRWrite(int r, size_t val);
	int InstallDriver();                     // install MSRDriver
	int UnInstallDriver();                   // uninstall MSRDriver
	int LoadDriver();                        // load MSRDriver
	int UnloadDriver();                      // unload MSRDriver
protected:
   int Need64BitDriver();                   // tell whether we need 32 bit or 64 bit driver
   SC_HANDLE GetScm();                      // Make scm handle
	SC_HANDLE scm;
	SC_HANDLE service;
	HANDLE hDriver;
   LPCTSTR DriverFileName;
   LPCTSTR DriverSymbolicName;
   char DriverFileNameE[MAX_PATH], DriverFilePath[MAX_PATH];
};


// bitfield CPUID0EAX defines CPUID function 0 eax
struct CPUID0EAX {
   unsigned int stepping : 4;     // Processor stepping
   unsigned int model    : 4;     // Processor model
   unsigned int family   : 4;     // Processor family
   unsigned int res1     : 4;     // unused
   unsigned int Emodel   : 4;     // Processor extended model
   unsigned int Efamily  : 8;     // Processor extended family
   unsigned int res2     : 4;     // unused
};

// structure QWord handles 64-bit integer as two 32-bit integers
struct QWord {
   int lo;                                 // Low 32 bits
   int hi;                                 // High 32 bits
};


// class CCpuidManipulate handles CPUID manipulation
class CCpuidManipulate {
public:
   CCpuidManipulate();                      // constructor
   void LockProcessor();                    // Make program and driver use the same processor number
   int  StartDriver();                      // Install and load driver
   void ReadCurrentFamily();                // Read current values for family, model, etc.
   void ReadCurrentVendor();                // Read current value for vendor string
   void ReadCurrentMSRID0();                // Read current values for bits 0-31 of MSRID0
   void SetFamily(int fam);                 // Prepare new value for family
   void SetEFamily(int efam);               // Prepare new value for extended family
   void SetModel(int mod);                  // Prepare new value for model
   void SetEModel(int emod);                // Prepare new value for extended model
   void SetFamilyModel(int fam, int efam, int mod, int emod); // Prepare new values for family, model, etc.
   void SetFamilyModel(unsigned int eax);   // Prepare new values for family, model, etc., as a bitfield
   void SetVendorString(const char * s);    // Prepare new vendor ID string
   void OriginalVendorString();             // Prepare for original vendor ID string
   void DoChanges();                        // Run driver to do the prepared changes
   void RunQueue();                         // run commands in queue1
   void Uninstall();                        // Uninstall driver
   void Put1 (int num_threads,              // put record into multiple start queues
      EMSR_COMMAND msr_command, unsigned int register_number,
      unsigned int value_lo, unsigned int value_hi = 0);
   CMSRDriver msr;                          // interface to MSR access driver
protected:
   CMSRInOutQue queue1[MAXTHREADS];         // que of MSR commands to do by RunQueue()
   int ProcessorNumber;                     // main thread processor number in multiprocessor systems
   int ChangesRequired;                     // 1: change family and model, 2: change vendor string, 4: original vendor string
   union {                                  // three 64-bit Model Specific Registers
      long long MSR;                        // as 64-bit register
      QWord q;                              // as 32-bit parts
      CPUID0EAX eax[2];                     // as bitfield
   } reg[3];
};

// class CpuID reads and prints current CPUID information
class CpuID {
public:
   static int  IsItVIA();                   // Is it a VIA processor?
   static void PrintVendor();               // Print vendor string
   static void PrintFullName();             // Print name string
   static void PrintFamilyAndModel();       // Print family and model number
   static void PrintAll();                  // Print vendor, name, family and model
};

// class Menu handles user interface
class CMenu {
public:
   CMenu(CCpuidManipulate & m);             // constructor
   void ShowIntro();                        // show introductory text
   void ShowPrompt();                       // show prompt according to state
   void ReadInput();                        // read user input after prompt
   void ParseInput();                       // interpret input
   void DispatchInput();                    // do command according to input
   void Run();                              // run prompt-input loop
protected:
   CCpuidManipulate & manip;                // reference to CCpuidManipulate instance
   char InputText[64];                      // user input
   int  state;                              //   0:  initial
                                            //   1:  show main menu
                                            // 100: V menu: other vendor string
                                            // 200: F menu: other family and model (dec)
                                            // 300: H menu: other family and model (hex)
                                            // 800: Do change
                                            // 801: Uninstall
                                            // 900: exit
                                            // 901: print new CPUID and exit
   int InputValue;
};

// Class ThreadHandler: Create threads
class ThreadHandler {
public:
   ThreadHandler();       // constructor
   void Start(int Num);   // start threads
   void Stop();           // wait for threads to finish
   ~ThreadHandler();      // destructor
protected:
   int NumThreads;
   HANDLE hThreads[MAXTHREADS];
   int ThreadData[MAXTHREADS];
};
