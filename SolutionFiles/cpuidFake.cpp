//                       CpuidFake.cpp                    © 2010-01-22 Agner Fog
//
//               Program for modifying CPUID on VIA Nano processor
//
//
// This program can modify the CPUID vendor string, family and model number
// on a VIA Nano processor. Does not work on other processors.
//
// The purpose is to test if benchmark programs and other CPU-intensive programs
// work faster when the CPUID is changed.
// 
// If you are running on 64-bit Windows Vista or Windows 7 or later then you
// must press F8 during computer startup and choose 
// "Disable driver signature enforcement".
// Run this program as administrator.
//
// The driver MSRDriver32.sys or MSRDriver64.sys is installed on first use.
// It is highly recommended to uninstall this driver when you are finished using
// this program. Run the program and select 'U' on the menu to uninstall the driver.
//
// Built with MS Visual Studio 2008. Unicode off.
//
// (c) 2010 GNU General Public License www.gnu.org/licenses
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include "MSRDriver.h"
#include "cpuidfake.h"

//    Does you compiler support intrinsic functions?
//    If not, comment out the next line and use the inline assembly version
//    of __cpuid defined below.
#include <intrin.h>

#define VISTA_OR_ABOVE  0              // 0 if Windows XP or earlier

//////////////////////////////////////////////////////////////////////////////
//
//             __cpuid function
//
//////////////////////////////////////////////////////////////////////////////

// Define __cpuid function if intrinsics not supported:
#ifndef __INTRIN_H_
void __cpuid (int output[4], int functionnumber) {
	// This version is for 32-bit, MASM syntax:
	__asm {
		mov eax, functionnumber;
		cpuid;
		mov esi, output;
		mov [esi],    eax;
		mov [esi+4],  ebx;
		mov [esi+8],  ecx;
		mov [esi+12], edx;
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
//             global variables
//
//////////////////////////////////////////////////////////////////////////////
// Thread synchronizer
int NumThreads = 1;                    // number of threads

// processornumber for each thread
int ProcNum[MAXTHREADS] = {0};


//////////////////////////////////////////////////////////////////////
//
//        Main
//
//////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
   int i, t;
   int MainThreadProcNum = 0;

   if (!CpuID::IsItVIA()) {
      // Other processor, would give blue screen error
      printf("\nCurrent processor:\n");
      CpuID::PrintAll();
      printf("\n\nThis program works only for VIA Nano processors\n");
      // Optional: wait for key press
      //getch();
      //return 1;
   }

   // Limit number of threads
   if (NumThreads > MAXTHREADS) {
      NumThreads = MAXTHREADS;
      printf("\nToo many threads");
   }
   if (NumThreads < 1) NumThreads = 1;

   DWORD_PTR ProcessAffMask = 0, SystemAffMask = 0;
   // Get mask of possible CPU cores
   GetProcessAffinityMask(GetCurrentProcess(), &ProcessAffMask, &SystemAffMask);

   // Fix a processornumber for each thread.
   // I don't know if it is necessary to set the CPUID in each processor kernel
   // on future multi-kernel VIA processors.
#if VISTA_OR_ABOVE
   MainThreadProcNum = GetCurrentProcessorNumber();
#endif
   for (t=0, i=NumThreads-1; t < NumThreads; t++, i--) {
      // make processornumbers different, and last thread = MainThreadProcNum:
      //ProcNum[t] = MainThreadProcNum ^ i;
      ProcNum[t] = i;
      if ((ProcessAffMask & ((DWORD_PTR)1 << ProcNum[t])) == 0) {
         // not enough processors
         printf("\nProcessor %i not available. Mask = 0x%X\n", 
            ProcNum[t], ProcessAffMask);
         return 1;
      }
   }

   // Write CPUID information before change
   printf("\nCurrent CPUID:\n");
   CpuID::PrintAll();
   printf("\n");

   // Create CCpuidManipulate instance
   CCpuidManipulate manipulator;

   // Create and run menus
   CMenu menu(manipulator);
   menu.Run();

   // Optional: wait for key press
   printf("\npress any key");
   getch();

   // Exit
   return 0;
}


//////////////////////////////////////////////////////////////////////
//
//        CMSRDriver class member functions
//
//  This is the interface to the driver that can access MSR registers
//
//////////////////////////////////////////////////////////////////////

// Constructor
CMSRDriver::CMSRDriver() {
   
   // Define Driver filename
   if (Need64BitDriver()) {
      DriverFileName = "MSRDriver64";
   }
   else {
      DriverFileName = "MSRDriver32";
   }
   
   // Define driver symbolic link name
   DriverSymbolicName = "\\\\.\\slMSRDriver";

   // Get the full path of the driver file name
   strcpy(DriverFileNameE, DriverFileName);
   strcat(DriverFileNameE, ".sys");           // append .sys to DriverName
   ::GetFullPathName(DriverFileNameE, MAX_PATH, DriverFilePath, NULL);

   // Initialize
   service = NULL;
   hDriver = NULL;
   scm     = NULL;
}

SC_HANDLE CMSRDriver::GetScm() {
   // Make scm handle
   if (scm) return scm;  // handle already made

   // Open connection to Windows Service Control Manager (SCM)
   scm = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
   if(scm == NULL) {
      int e = ::GetLastError();
      if (e == ERROR_ACCESS_DENIED) {
         printf("\nAccess denied. Please run as administrator\n");
      }
      else if (e == 120) {  // function not implemented
         printf("\nFunction not implemented on this operating system. Windows 2000 or later required.\n");
      }
      else {
         printf("\nCannot load Windows Service Control Manager\nError no. %i", e);
      }
   }
   return scm;
}

// Destructor
CMSRDriver::~CMSRDriver() {
   // Unload driver if not already unloaded and close SCM handle

   //if (hDriver) UnloadDriver();
   if (service) {
      ::CloseServiceHandle(service); service = NULL;   
   }
   if (scm) {
      // Optionally unload driver
      // UnloadDriver();
      // Don't uninstall driver, you may need reboot before you can install it again
      // UnInstallDriver();
      ::CloseServiceHandle(scm); scm = NULL;
   }
}

int CMSRDriver::InstallDriver() {
   // install MSRDriver
   if(GetScm() == NULL) return -1;
      
   // Install driver in database
   service = ::CreateService(scm, DriverFileNameE, "MSR driver",
      SERVICE_START + SERVICE_STOP + DELETE, SERVICE_KERNEL_DRIVER,
      SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, DriverFilePath, 
      NULL, NULL, NULL, NULL, NULL);

   if(service == NULL) {
      int e = ::GetLastError();
      printf("\nCannot install driver %s\nError no. %i", DriverFileNameE, e);
   }
   else {
      printf("\nFirst time: Installing driver %s\n", DriverFileNameE);
   }
   return 0;
}

int CMSRDriver::UnInstallDriver() {
   // uninstall MSRDriver
   int r = 0, e = 0;
   GetScm();
   if (service == 0) {
      service = ::OpenService(scm, DriverFileNameE, SERVICE_ALL_ACCESS);
   }
   if(service == 0) {
      e = ::GetLastError();
      if (e == 1060) {
         printf("\nDriver %s already uninstalled or never installed\n", DriverFileNameE);
      }
      else {
         printf("\nCannot open service, failed to uninstall driver %s\nError no. %i", DriverFileNameE, e);
      }
   }
   else {
      r = ::DeleteService(service);
      if (r == 0) {
         e = ::GetLastError();
         printf("\nFailed to uninstall driver %s\nError no. %i", DriverFileNameE, e);
         if (e == 1072) printf("\nDriver already marked for deletion\n");
      }
      else {
         printf("\nUninstalling driver %s\n", DriverFileNameE);
      }
      r = ::CloseServiceHandle(service);
      if (r == 0) {
         e = ::GetLastError();
         printf("\nCannot close service\nError no. %i", e);
      }
      service = NULL;
   }
   return e;
}

// Load driver
int CMSRDriver::LoadDriver() {
   int r = 0, e = 0;
   // Open a service handle if not already open
   if (service == 0) {
      service = ::OpenService(GetScm(), DriverFileNameE, SERVICE_ALL_ACCESS);
   }
   if(service == 0) {
      e = ::GetLastError();

      switch (e) { // Any other error than driver not installed
      case 1060: // Driver not installed. Install it
         break;
      case 6:    // access denied
         printf("\nAccess denied\n");
         break;
      default:  // Any other error
         printf("\nCannot open service, failed to load driver %s\nError no. %i", DriverFileNameE, e);
      }
      return e;
   }
      
   // Start the service
   r = ::StartService(service, 0, NULL);
   if (r == 0) {
      e = ::GetLastError();
      switch (e) {
      case ERROR_FILE_NOT_FOUND:  // .sys file not found
         printf("\nDriver file %s not found\n", DriverFileNameE);
         break;

      case 577:
         // driver not signed (Vista and Windows 7)
         printf("\nThe driver %s is not signed by Microsoft\n"
			 "Please restart computer. Press F8 during startup and select 'Disable Driver Signature Enforcement'\n", 
			 DriverFileNameE);
         break;
      
      case 1056:
         // Driver already loaded. Ignore
         //printf("\nDriver already loaded\n");
         e = 0;
         break;

      case 1058:
         printf("\nError: Driver disabled\n");
         break;
      
      case 3: // path not found
         printf("\nCannot load driver %s\nPath not found\n", DriverFileNameE);
         break;

      default:
         printf("\nCannot load driver %s\nError no. %i", DriverFileNameE, e);
      }
   }
   if (e == 0) {
      // Get handle to driver
      hDriver = ::CreateFile(DriverSymbolicName, GENERIC_READ + GENERIC_WRITE,
         0, NULL, OPEN_EXISTING, 0, NULL);
         
      if(hDriver == NULL || hDriver == INVALID_HANDLE_VALUE) {
         hDriver = NULL;
         e = ::GetLastError();
         printf("\nCannot load driver\nError no. %i", e);
      }
   }
   return e;
}

// Unload driver
int CMSRDriver::UnloadDriver() {
   int r = 0, e = 0;
   if(GetScm() == NULL) {
      return -6;
   }
   
   if (hDriver) {
      r = ::CloseHandle(hDriver); hDriver = NULL;
      if (r == 0) {
         e = ::GetLastError();
         printf("\nCannot close driver handle\nError no. %i", e);
         return e;
      }
      printf("\nUnloading driver");
   }
   
   if (service) {
      SERVICE_STATUS ss;
      r = ::ControlService(service, SERVICE_CONTROL_STOP, &ss);
      if (r == 0) {
         e = ::GetLastError();
         if (e == 1062) {
            printf("\nDriver not active\n");
         }
         else {
            printf("\nCannot close driver\nError no. %i", e);
         }
         return e;
      }
   }
   return 0;
}

// Access MSR or control registers
int CMSRDriver::AccessRegisters(void * pnIn, int nInLen, void * pnOut, int nOutLen) {
   if (nInLen <= 0) return 0;

   const int DeviceType = 0x22;        // FILE_DEVICE_UNKNOWN;
   const int Function = 0x800;
   const int Method = 0;               // METHOD_BUFFERED;
   const int Access = 1 | 2;           // FILE_READ_ACCESS | FILE_WRITE_ACCESS;
   const int IOCTL_MSR_DRIVER = DeviceType << 16 | Access << 14 | Function << 2 | Method;

   DWORD len = 0;

   // This call results in a call to the driver rutine DispatchControl()
   int res = ::DeviceIoControl(hDriver, IOCTL_MSR_DRIVER, pnIn, nInLen,
      pnOut, nOutLen, &len, NULL);
   if (!res) { 
      // Error
      int e = GetLastError();
      if (e == 6) {
         // invalid handle ?
      }
      else {
         printf("\nCan't access driver. error %i", e);
      }
      return e;
   }

   // Check return error codes from driver
   SMSRInOut * outp = (SMSRInOut*)pnOut;
   for (int i = 0; i < nOutLen/(INT)sizeof(SMSRInOut); i++) {
      if (outp[i].msr_command == PROC_SET && outp[i].val[0]) {
         printf("\nSetting processor number in driver failed, error 0x%X", outp[i].val[0]);
      }
   }

   return 0;
}


// Access MSR or control registers by queue
int CMSRDriver::AccessRegisters(CMSRInOutQue & q) {
   // Number of bytes in/out
   int n = q.GetSize() * sizeof(SMSRInOut);  
   if (n <= 0) return 0;
   return AccessRegisters(q.queue, n, q.queue, n);
}


// Get driver name
LPCTSTR CMSRDriver::GetDriverName() {
   return DriverFileName;
}

// Read a MSR register
long long CMSRDriver::MSRRead(int r) {
   SMSRInOut a;
   a.msr_command = MSR_READ;
   a.register_number = r;
   a.value = 0;
   AccessRegisters(&a,sizeof(a),&a,sizeof(a));
   return a.val[0];
}

// Write a MSR register
int CMSRDriver::MSRWrite(int r, long long val) {
   SMSRInOut a;
   a.msr_command = MSR_WRITE;
   a.register_number = r;
   a.value = val;
   return AccessRegisters(&a,sizeof(a),&a,sizeof(a));
}

// Read a control register cr0 or cr4
size_t CMSRDriver::CRRead(int r) {
   if (r != 0 && r != 4) return -11;
   SMSRInOut a;
   a.msr_command = CR_READ;
   a.register_number = r;
   a.value = 0;
   AccessRegisters(&a,sizeof(a),&a,sizeof(a));
   return size_t(a.value);
}

// Write a control register cr0 or cr4
int CMSRDriver::CRWrite(int r, size_t val) {
   if (r != 0 && r != 4) return -12;
   SMSRInOut a;
   a.msr_command = CR_WRITE;
   a.register_number = r;
   a.value = val;
   return AccessRegisters(&a,sizeof(a),&a,sizeof(a));
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
int CMSRDriver::Need64BitDriver() {
   // Tell whether we need 32 bit or 64 bit driver.
   // Return value:
   // 0: running in 32 bits Windows
   // 1: running 32 bits mode in 64 bits Windows
   // 2: running 64 bits mode in 64 bits Windows

#ifdef _WIN64
   return 2;
#else

   LPFN_ISWOW64PROCESS fnIsWow64Process = 
      (LPFN_ISWOW64PROCESS)GetProcAddress(
         GetModuleHandle("kernel32"),"IsWow64Process");
   
   if (fnIsWow64Process) {
      
      BOOL bIsWow64 = FALSE;
      
      if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)) {
         return 0;
      }
      return bIsWow64;
   }
   return 0;
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
//        CMSRInOutQue class member functions
//
//  This is a queue of commands to the MSR driver
//
//////////////////////////////////////////////////////////////////////////////

// Constructor
CMSRInOutQue::CMSRInOutQue() {
   n = 0;
}

// Put data record in queue
int CMSRInOutQue::put (EMSR_COMMAND msr_command, unsigned int register_number,
                       unsigned int value_lo, unsigned int value_hi) {

   if (n >= MAX_QUE_ENTRIES) return -10;

   queue[n].msr_command = msr_command;
   queue[n].register_number = register_number;
   queue[n].val[0] = value_lo;
   queue[n].val[1] = value_hi;
   n++;
   return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
//        CCpuidManipulate class member functions
//
//  This sets up and manipulates the MSRs that control family and vendor string
//
//////////////////////////////////////////////////////////////////////////////

// Constructor
CCpuidManipulate::CCpuidManipulate() {
   // Set everything to zero
    reg[0].MSR = reg[1].MSR = reg[2].MSR = 0;
    ProcessorNumber = 0;
    ChangesRequired = 0;
}

void CCpuidManipulate::ReadCurrentFamily() {
   // Read current values for family, model, etc.
   int abcd[4];
   __cpuid (abcd, 1); // Get current family, model, etc. from cpuid(1).eax
   reg[0].q.hi = abcd[0];
}

void CCpuidManipulate::ReadCurrentVendor() {
   // Read current value for vendor string
   int abcd[4];
   __cpuid (abcd, 0); // Get current vendor string
   reg[1].q.lo = abcd[1];
   reg[1].q.hi = abcd[3];
   reg[2].q.lo = abcd[2];
   reg[2].q.hi = 0;
}

void CCpuidManipulate::ReadCurrentMSRID0() {
   // Read current values for bits 0-31 of MSRID0
   reg[0].q.lo = (int)msr.MSRRead(MSRID0);  // read control bits (undocumented, don't know what they are for)
}


void CCpuidManipulate::SetFamily(int fam) {
   // Prepare new value for family
   reg[0].eax[1].family = fam;
   reg[0].eax[1].stepping = 0;
   ChangesRequired |= 1;          // change family and model
}
 
void CCpuidManipulate::SetEFamily(int efam) {
   // Prepare new value for extended family
   reg[0].eax[1].Efamily = efam;
   ChangesRequired |= 1;          // change family and model
}
 
void CCpuidManipulate::SetModel(int mod) {
   // Prepare new value for model
   reg[0].eax[1].model = mod;
   ChangesRequired |= 1;          // change family and model
}
 
void CCpuidManipulate::SetEModel(int emod) {
   // Prepare new value for extended model
   reg[0].eax[1].Emodel = emod;
   ChangesRequired |= 1;          // change family and model
}

void CCpuidManipulate::SetFamilyModel(int fam, int efam, int mod, int emod) {
   // Prepare new values for family, model, etc. to be set later
   reg[0].eax[1].family = fam;
   reg[0].eax[1].Efamily = efam;
   reg[0].eax[1].model = mod;
   reg[0].eax[1].Emodel = emod;
   reg[0].eax[1].stepping = 0;
   ChangesRequired |= 1;          // change family and model
}

void CCpuidManipulate::SetFamilyModel(unsigned int eax) {
   // Prepare new values for family, model, etc., as a bitfield
   reg[0].q.hi = eax;
   ChangesRequired |= 1;          // change family and model
}

void CCpuidManipulate::SetVendorString(const char * s) {
   // Prepare new vendor ID string to be set later
   union {
      char name[16];
      int n[4];
   } t;
   int i, j = 1;
   // make sure name is 12 characters
   for (i = 0; i < 12; i++) {
      if (*s == 0) j = 0; // end of string
      t.name[i] = j ? *s : 0x20;
      s++;
   }
   reg[1].q.lo = t.n[0];
   reg[1].q.hi = t.n[1];
   reg[2].q.lo = t.n[2];
   reg[2].q.hi = 0;
   ChangesRequired |= 2;          // change vendor string
}

void CCpuidManipulate::OriginalVendorString() {
   // Prepare for original vendor ID string
   SetVendorString("CentaurHauls"); // change vendor string registers to make sure they are not empty
   ChangesRequired |= 4;          // disable alternative vendor string
}

void CCpuidManipulate::DoChanges() {
   // Run driver to do the prepared changes
   if (!(ChangesRequired & 1)) {
      // family and model unchanged
      ReadCurrentFamily();
   }
   if (!(ChangesRequired & 6)) {
      // vendor string unchanged
      ReadCurrentVendor();
   }
   ReadCurrentMSRID0();  // Read current values for bits 0-31 of MSRID0. Make sure they are unchanged except bit 8
   if (ChangesRequired & 4) {
      // original vendor
      reg[0].q.lo &= ~VENDOR_BIT;     // disable alternative vendor string
   }
   else {
      // changed vendor
      reg[0].q.lo |= VENDOR_BIT;      // bit 8 enables alternative vendor string
   }

   // set MSR commands in queue for driver
   Put1(NumThreads, MSR_WRITE, MSRID0, reg[0].q.lo, reg[0].q.hi);
   if (reg[1].MSR) {
      Put1(NumThreads, MSR_WRITE, MSRID1, reg[1].q.lo, reg[1].q.hi);
      Put1(NumThreads, MSR_WRITE, MSRID2, reg[2].q.lo, reg[2].q.hi);
   }
   RunQueue();  // run these commands with a single call to the driver
}

void CCpuidManipulate::LockProcessor() {
   // Make program and driver use the same processor number if multiple processors
   // Enable RDMSR instruction
   int thread, procnum;

   // We must lock the driver call to the desired processor number
   for (thread = 0; thread < NumThreads; thread++) {
      procnum = ProcNum[thread];
      // lock driver to the same processor number as thread
      queue1[thread].put(PROC_SET, 0, procnum);
      // enable RDPMC instruction (for this processor number)
      //queue1[thread].put(PMC_ENABLE, 0, 0);
   }
}

int CCpuidManipulate::StartDriver() {
   // Install and load driver
   // return error code
   int ErrNo = 0;

   if (1) {
      // Load driver
      ErrNo = msr.LoadDriver();
      if (ErrNo == 1060) {
         // Driver not installed. Install it
         msr.InstallDriver();
         ErrNo = msr.LoadDriver();
      }
      if (ErrNo) return ErrNo;
   }

   return 0;
}

void CCpuidManipulate::Uninstall() {
   // Uninstall driver
   msr.LoadDriver();
   msr.UnloadDriver();
   msr.UnInstallDriver();
}

// put record into multiple start queues
void CCpuidManipulate::Put1 (int num_threads,
EMSR_COMMAND msr_command, unsigned int register_number,
unsigned int value_lo, unsigned int value_hi) {
   for (int t = 0; t < num_threads; t++) {
      queue1[t].put(msr_command, register_number, value_lo, value_hi);
   }
}

// Run commands in queue
void CCpuidManipulate::RunQueue() {
   for (int thread = 0; thread < NumThreads; thread++) {
      int e = msr.AccessRegisters(queue1[thread]);
   }
}

//////////////////////////////////////////////////////////////////////////////
//
//                    CpuID class member functions
//
//   This reads current CPUID information
//
//////////////////////////////////////////////////////////////////////////////

int CpuID::IsItVIA() {
   // Is it a VIA processor? Check without reading the vendor string
   int abcd[4];
   __cpuid (abcd, 0xC0000000);              // Check VIA features
   if ((unsigned int)abcd[0] < 0xC0000001) return 0;     // No VIA feature flags
   __cpuid (abcd, 0xC0000001);              // Get VIA feature flags
   if (abcd[3] & 0x544) return 1;           // VIA features detected
   return 0;
}

void CpuID::PrintVendor() {
   // Print vendor string
   int abcd[4];
   union {
      char s[16];
      int  i[4];
   } name;
   __cpuid (abcd, 0);     // get vendor string
   name.i[0] = abcd[1];   // reorder dwords
   name.i[1] = abcd[3];
   name.i[2] = abcd[2];
   name.i[3] = 0;         // terminate string
   printf("%s", name.s);  // print string
}

void CpuID::PrintFamilyAndModel() {
   // Print family and model number
   int abcd[4];
   union {
      int i;
      CPUID0EAX bits;
   } eax;
   __cpuid (abcd, 1);     // get family and model int eax
   eax.i = abcd[0];
   // print family. Family and extended family numbers are added together
   printf("Family: 0x%X, ext: 0x%X, total: 0x%X", 
      eax.bits.family, eax.bits.Efamily, eax.bits.family + eax.bits.Efamily);
   // print model. Model and extended model numbers are concatenated as bits
   printf("\nModel:  0x%X, ext: 0x%X, total: 0x%X", 
      eax.bits.model, eax.bits.Emodel, eax.bits.model | (eax.bits.Emodel << 4));
}

void CpuID::PrintFullName() {
   // Print name string
   unsigned int j, k;
   int abcd[4];
   union {
      char s[52];
      unsigned int i[13];
   } name;
   __cpuid (abcd, 0x80000000);     // get vendor string
   if ((unsigned int)abcd[0] < 0x80000004) {
      // No extended name string
      PrintVendor();
   }
   else {
      for (j = 0x80000002, k = 0; j <= 0x80000004; j++, k += 4) {
         __cpuid (abcd, j);     // get 16 characters
         name.i[k]   = abcd[0];
         name.i[k+1] = abcd[1];
         name.i[k+2] = abcd[2];
         name.i[k+3] = abcd[3];
      }
      name.i[k] = 0;            // terminate string
      printf("%s", name.s);     // print string
   }
}

void CpuID::PrintAll() {
   // Print vendor, name, family and model
   printf("Vendor: ");
   CpuID::PrintVendor();
   printf("\nName: ");
   CpuID::PrintFullName();
   printf("\n");
   CpuID::PrintFamilyAndModel();
   printf("\n");
}

//////////////////////////////////////////////////////////////////////////////
//
//                    CMenu class member functions
//
//   This is the user interface. Makes console based menu and prompts.
//   Implemented as a state machine.
//
//////////////////////////////////////////////////////////////////////////////

CMenu::CMenu(CCpuidManipulate & m) : manip(m) {
   // constructor
   state = 0;
}
   
void CMenu::ShowIntro() {
   // show introductory text
   printf("\n                    *** CPUID Fake program ***\n\n"
      "This program can change the CPUID vendor string, family and model number\n"
      "on a VIA Nano processor. Does not work on any other processor.");

   // Write CPUID information before change
   printf("\n\nCurrent CPUID:\n");
   CpuID::PrintAll();
   printf("\n");
}
 
void CMenu::ShowPrompt() {
   // show prompt according to state
   switch (state) {
   case 0: case 1:   // main menu
      printf("\nChange CPUID to:\n"
         "0: No change\n"
         "1: VIA Nano, family. 6, model 0x0F\n"
         "2: AMD Opteron, family 0x10, model 4\n"
         "3: Intel Core 2, family 6, model 0x17\n"
         "4: Intel Pentium 4, family 15, model 4\n"
         "5: Intel Atom, family 6, model 0x1C\n"
         "6: Intel (nonexisting), family 7, model 1\n"
         "V: Other vendor string\n"
         "F: Other family and model number (decimal)\n"
         "H: Other family and model number (hexadecimal)\n"
         "U: Uninstall driver for CpuidFake\n\n"
         "Enter choice: ");
      break;
   case 100:   // V submenu
      printf("\nEnter new vendor string, 12 characters: ");
      break;
   case 200:   // F submenu
      printf("\nEnter family (0-15): ");
      break;
   case 201:   // F submenu
      printf("\nEnter extended family (0-255): ");
      break;
   case 202:   // F submenu
      printf("\nEnter model (0-15): ");
      break;
   case 203:   // F submenu
      printf("\nEnter extended model (0-15): ");
      break;
   case 300:   // H submenu
      printf("\nEnter family, model, etc. as hexadecimal digits in the form 0EEX0FMS, where\n"
         "EE = extended family, X = extended model, F = family, M = model, S = stepping:\n");
      break;
   default:
      printf("\nInternal error, unknown state %i", state);
      break;
   }
}
 
void CMenu::ReadInput() {
   // read user input after prompt
   fgets(InputText, sizeof(InputText), stdin);
}

void CMenu::ParseInput() {
   // interpret input
   int i; char c;
   if ((unsigned char)InputText[0] < 0x20) {
      // no input
      InputValue = -1; InputText[0] = 0;
      return;
   }
   switch (state) {
   case 100:  // V menu: vendor string
      InputText[12] = 0;  //Make sure string is no longer than 12 characters
      break;
   case 300:  // H menu: hexadecimal input
      InputValue = 0;
      for (i = 0; i < 16; i++) {  // interpret hexadecimal input
         c = InputText[i];  // read one character
         if ((unsigned char)c < 0x20) break;  // end of string
         c |= 0x20;  // make lower case
         if (c >= '0' && c <= '9') {
            InputValue = (InputValue << 4) | (c - '0');       // 0 - 9
         }
         else if (c >= 'a' && c <= 'f') {
            InputValue = (InputValue << 4) | (c - 'a' + 10);  // a - f
         }
         else if (c == 'x' || c == 'h' || c == ' ') {
            // ignore leading space, 0x prefix or h suffix
         }
         else { // other character
            printf("\nUnknown character %c\n", InputText[i]);
            break;
         }
      }
      break;
   default:  // Other menus: read number or single letter
      i = 0;
      while (InputText[i] == ' ' && i < 16) i++;  // skip leading space
      if (InputText[i] >= '0' && InputText[i] <= '9') {
         InputValue = atoi(InputText + i);        // numeric input
      }
      else if ((unsigned char)InputText[i+1] < 0x20 && (InputText[i] | 0x20) >= 'a' && (InputText[i] | 0x20) <= 'z') {
         InputValue = InputText[i] | 0x20;        // single letter input, convert to lower case
      }
      else { // other
         printf("\nUnknown command %s\n", InputText);
      }
   }
}

void CMenu::DispatchInput() {
   // do command according to input
   if (InputValue == -1) {
      InputValue = 0;
      return;  // invalid input, try again
   }
   switch (state) {
   case 1:  // main menu
      switch (InputValue) {
      case 0: // no change
         state = 900;  break;
      case 1: // VIA Nano
         manip.SetFamilyModel(6, 0, 0x0F, 0);
         manip.SetVendorString("CentaurHauls");
         manip.OriginalVendorString();
         state = 800;  break;
      case 2: // AMD Opteron
         manip.SetFamilyModel(0x0F, 1, 4, 0);
         manip.SetVendorString("AuthenticAMD");
         state = 800;  break;
      case 3: // Intel Core2
         manip.SetFamilyModel(6, 0, 7, 1);
         manip.SetVendorString("GenuineIntel");
         state = 800;  break;
      case 4: // Intel Pentium 4
         manip.SetFamilyModel(15, 0, 4, 0);
         manip.SetVendorString("GenuineIntel");
         state = 800;  break;
      case 5: // Intel Atom
         manip.SetFamilyModel(6, 0, 0xC, 1);
         manip.SetVendorString("GenuineIntel");
         state = 800;  break;
      case 6: // Intel nonexisting, family 7
         manip.SetFamilyModel(7, 0, 1, 0);
         manip.SetVendorString("GenuineIntel");
         state = 800;  break;
      case 'v': // Other vendor string
         state = 100;  return;
      case 'f': // Other family (dec)
         state = 200;  return;
      case 'h': // Other family (hex)
         state = 300;  return;
      case 'u': // uninstall
         state = 801;  break;
      default:  // unknown
         printf("\nUnknown command %i\n", state);
         break;
      }
      break;
   case 100: // response to V menu
      manip.SetVendorString(InputText);
      state = 800;
      break;
   case 200: // response to F menu 0
      manip.SetFamily(InputValue);
      state++;  break;
   case 201: // response to F menu 1
      manip.SetEFamily(InputValue);
      state++;  break;
   case 202: // response to F menu 2
      manip.SetModel(InputValue);
      state++;  break;
   case 203: // response to F menu 3
      manip.SetEModel(InputValue);
      state = 800;  break;
   case 300: // response to H menu
      manip.SetFamilyModel(InputValue);
      state = 800;  break;
   default:  // unknown
      printf("\nunknown state %i\n", state);
      state = 0;
      break;
   }
   if (state == 800) {
      // do changes
      if (CpuID::IsItVIA()) {
         // Install and load driver
         int e = manip.StartDriver();
         if (e) {
            switch (e) {
            case 6:   // access denied. please run as administartor. error message already written
            case 577: // driver not signed. error message already written
               break;
            default:
               printf("\nError %i starting driver\n", e);
            }
            state = 900;  return;
         }
         // Make driver use all processor numbers. (is this necessary ??)
         //manip.LockProcessor();
         manip.DoChanges();
      }
      state = 901;
   }
   if (state == 801) {
      // uninstall driver
      manip.Uninstall();
      printf("\nYou need to restart the computer before the CPUID is normal again\n");
      state = 900;
   }
}
 
void CMenu::Run() {
   // run prompt-input loop
   state = 1;
   ShowIntro();
   while (state < 900) {                  // implemented as a state machine
      ShowPrompt();                       // show prompt according to state
      ReadInput();                        // read user input after prompt
      ParseInput();                       // interpret input
      DispatchInput();                    // do command according to input
   }
   if (state == 901) {
      // show changed CPUID
      printf("\nVerifying new CPUID:\n");
      CpuID::PrintAll();
   }
   state = 900;
}
