#ifndef __emu_odmbdev_Buttons_h__
#define __emu_odmbdev_Buttons_h__

#include "emu/utils/Cgi.h"
#include "emu/utils/System.h"
#include "emu/pc/VMEController.h"
//#include "/home/cscme11/XDAQ/11/TriDAS/emu/emuDCS/PeripheralCore/src/common/VMEController.cc"

#include <vector>
#include <sstream>

#include "emu/odmbdev/Action.h"
#include "emu/odmbdev/LogAction.h"
#include "emu/odmbdev/ActionValue.h"
#include "emu/odmbdev/ButtonAction.h"
#include "emu/odmbdev/FourTextBoxAction.h"
#include "emu/odmbdev/OneTextBoxAction.h"
#include "emu/odmbdev/ThreeTextBoxAction.h"
#include "emu/odmbdev/RepeatTextBoxAction.h"
#include "emu/odmbdev/ParameterTextBoxAction.h"

/******************************************************************************
 * The Buttons
 *
 * This header file and the associated cc file contains all the actions in the
 * application. Short and simple ones are often defined in the header file so
 * as to avoid duplicating code (declaration and definition).
 *
 * If this file starts to take a very long time to compile, you ought to
 * consider breaking it into pieces (perhaps grouped by type of test [e.g. TMB,
 * DMB]). Just remember to include all the pieces in the Manager.h file.
 *
 * NB The Makefile will not check if this file has been modified, if you modify
 * it without modifying the Buttons.cc file, use the command `touch Buttons.cc'
 * to ensure that it is recompiled
 *****************************************************************************/


namespace emu { namespace odmbdev {

    /**************************************************************************
     * Simple Buttons
     *
     * For tests/actions that are one or two lines and just need a button and
     * no text boxes 
     *************************************************************************/

    class Manager;
    int write_eth_raw(std::string tag);
    string FixLength(unsigned int Number, unsigned int Length = 5, bool isHex = true);
    unsigned short CountSetBits(const unsigned short);
    std::string ToUpper(std::string);

    class HardReset : public Action {
    public:
      HardReset(emu::pc::Crate * crate) : Action(crate) {}
      void display(xgi::Output * out) { addButton(out, "Hard Reset", "width: 250px "); }
      void respond(xgi::Input * in, std::ostringstream & out);
    };
    
    class L1Reset : public Action {
    public:
      L1Reset(emu::pc::Crate * crate) : Action(crate) {}
      void display(xgi::Output * out) { addButton(out, "L1 Reset (Resync)", "width: 100%; "); }
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class BC0 : public Action {
    public:
      BC0(emu::pc::Crate * crate) : Action(crate) {}
      void display(xgi::Output * out) { addButton(out, "BC0", "width: 250px; "); }
      void respond(xgi::Input * in, std::ostringstream & out);
    };


    /**************************************************************************
     * Other Buttons
     *
     *************************************************************************/

    // class Header_GeneralFunctions : public Header {
    // public:
    //   Header_GeneralFunctions(emu::pc::Crate * crate);
    //   void display(xgi::Output * out);
    //  };


    class ReadBackUserCodes : public Action {
    public:
      ReadBackUserCodes(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetTMBdavDelay : public Action, public ActionValue<int> {
    public:
      SetTMBdavDelay(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetComparatorMode : public Action, public ActionValue<std::string> {
    public:
      SetComparatorMode(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
    
    class SetComparatorThresholds : public Action, public ActionValue<float> {
    public:
      SetComparatorThresholds(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetComparatorThresholdsBroadcast : public Action, public ActionValue<float> {
    public:
      SetComparatorThresholdsBroadcast(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetUpComparatorPulse : public Action, public Action2Values<int,int> {
    public:
      SetUpComparatorPulse(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetUpPrecisionCapacitors : public Action, public Action2Values<int,int> {
    public:
      SetUpPrecisionCapacitors(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class PulseInternalCapacitorsDMB : public Action {
    public:
      PulseInternalCapacitorsDMB(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class PulseInternalCapacitorsCCB : public Action {
    public:
      PulseInternalCapacitorsCCB(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class PulsePrecisionCapacitorsDMB : public Action {
    public:
      PulsePrecisionCapacitorsDMB(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class PulsePrecisionCapacitorsCCB : public Action {
    public:
      PulsePrecisionCapacitorsCCB(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class SetDMBDACs : public Action, public ActionValue<float> {
    public:
      SetDMBDACs(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class ShiftBuckeyesNormRun : public Action {
    public:
      ShiftBuckeyesNormRun(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


    class PulseWires : public Action, public ActionValue<int> {
    public:
      PulseWires(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


    class SetPipelineDepthAllDCFEBs : public Action, public ActionValue<int> {
    public:
      SetPipelineDepthAllDCFEBs(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class Stans_SetPipelineDepthAllDCFEBs : public Action, public ActionValue<int> {
    public:
      Stans_SetPipelineDepthAllDCFEBs(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class ReadPipelineDepthAllDCFEBs : public Action {
    public:
      ReadPipelineDepthAllDCFEBs(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
  

    class SetFineDelayForADCFEB : public Action, public Action2Values<int, int> {
    public:
      SetFineDelayForADCFEB(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class TMBHardResetTest : public Action, public ActionValue<int> {
    public:
      TMBHardResetTest(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class DDU_KillFiber : public Action, public ActionValue<std::string> {
    public:
      DDU_KillFiber(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class DDU_EthPrescale : public Action, public ActionValue<std::string> {
    public:
      DDU_EthPrescale(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class DDU_FakeL1 : public Action, public ActionValue<std::string> {
    public:
      DDU_FakeL1(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class BuckShiftTest : public Action {
    public:
      BuckShiftTest(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


  class BuckShiftTestDebug : public Action, public ActionValue<int> {
    public:
     BuckShiftTestDebug(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


    
    class TMBRegisters : public Action {
    public:
      TMBRegisters(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class TMBSetRegisters : public Action, public Action2Values<std::string, std::string> {
    public:
      TMBSetRegisters(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
   
    class TMBDisableCopper : public Action {
    public:
      TMBDisableCopper(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

   class TMBEnableCLCTInput : public Action, public ActionValue<int> {
    public:
      TMBEnableCLCTInput(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
   };
 
   class TMBDisableCLCTInput : public Action, public ActionValue<int> {
    public:
      TMBDisableCLCTInput(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
   };  

    class PipelineDepthScan_Pulses : public Action, public Action2Values<int, int> {
    public:
      PipelineDepthScan_Pulses(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     

    class PipelineDepthScan_Cosmics : public Action, public Action2Values<int, int> {
    public:
      PipelineDepthScan_Cosmics(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     

    class TmbDavDelayScan : public Action, public Action2Values<int, int> {
    public:
      TmbDavDelayScan(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     

    class L1aDelayScan : public Action, public Action2Values<int, int> {
    public:
      L1aDelayScan(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     

    /**************************************************************************
     * Change Slot Number
     *
     * A domain-specific-lanaguage for issuing vme commands.
     *************************************************************************/
    class ChangeSlotNumber : public OneTextBoxAction {
    public:
      ChangeSlotNumber(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    };

    /**************************************************************************
     * ExecuteVMEDSL
     *
     * A domain-specific-lanaguage for issuing vme commands.
     *************************************************************************/
    class ExecuteVMEDSL : public FourTextBoxAction {
    public:
      ExecuteVMEDSL(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
      int nCommand;
    };

    /**************************************************************************
     * ResetRegisters
     *
     * A small class to implement a reset from the ODMB_CTRL bits
     **************************************************************************/
    class ResetRegisters : public ButtonAction {
    public:
      ResetRegisters(Crate * crate);
      void respond(xgi::Input * in, ostringstream & out);
    };      
    /**************************************************************************
     * ResetRegisters
     *
     * A small class to reprogram the DCFEB
     **************************************************************************/
    class ReprogramDCFEB : public ButtonAction {
    public:
      ReprogramDCFEB(Crate * crate);
      void respond(xgi::Input * in, ostringstream & out);
    };      

    /**************************************************************************
     * SYSMON
     *
     * A small class to print the System monitoring informaiton
     **************************************************************************/
    class SYSMON : public ButtonAction {
    public:
      SYSMON(Crate * crate);
      void respond(xgi::Input * in, ostringstream & out);
    };      

    /**************************************************************************
     * LVMBtest
     *
     * A small class to print the results of the low voltage monitoring test
     **************************************************************************/
    class LVMBtest : public ThreeTextBoxAction {
    public:
      LVMBtest(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    };   
    
    /**************************************************************************
     * DCFEBJTAGcontrol
     *
     * A small class to read DCFEB UserCode and print the results
	 **************************************************************************/
    class DCFEBJTAGcontrol : public RepeatTextBoxAction {
    public:
      DCFEBJTAGcontrol(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    };     
    
    /**************************************************************************
     * DCFEBFiber
     *
     * A small class to perform high statistics test of DCFEB fibers (trigger, send/receive packets)
	 **************************************************************************/
    class DCFEBFiber : public ParameterTextBoxAction {
    public:
      DCFEBFiber(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    }; 
    
    /**************************************************************************
     * CCBReg
     *
     * A small class to perform high statistics test of CCB firmware and communication
	 **************************************************************************/
    class CCBReg : public RepeatTextBoxAction {
    public:
      CCBReg(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    }; 
       
    /**************************************************************************
     * DDUFIFOTest
     *
     * A small class to perform high statistics test of DDU FIFO
	 **************************************************************************/
    class DDUFIFOTest : public RepeatTextBoxAction {
    public:
      DDUFIFOTest(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    }; 
       
    /**************************************************************************
     * PCFIFOTest
     *
     * A small class to perform high statistics test of DDU FIFO
	 **************************************************************************/
    class PCFIFOTest : public RepeatTextBoxAction {
    public:
      PCFIFOTest(Crate * crate, emu::odmbdev::Manager* manager);
      void respond(xgi::Input * in, ostringstream & out);
    }; 
       
    /**************************************************************************
     * Log Buttons
     *
     * These are special actions that modify the log
     *************************************************************************/

    class ClearLog : public LogAction {
    public:
      ClearLog(emu::pc::Crate * crate) : LogAction(crate) { }
      void display(xgi::Output * out) { addButton(out, "Clear Log"); }
      void respond(xgi::Input * in, xgi::Output * out, std::ostringstream & ssout, std::ostringstream & log) {
	log.clear(); // remove any error flags
	log.str(""); // empty the log
      }
    };

    class SaveLogAsFile : public LogAction {
    public:
      SaveLogAsFile(emu::pc::Crate * crate) : LogAction(crate) { }
      void display(xgi::Output * out) { addButton(out, "Save Log as..."); }
      void respond(xgi::Input * in, xgi::Output * out, std::ostringstream & ssout, std::ostringstream & log) {
        std::string file_name = "odmbdev_" + emu::utils::getDateTime(true) + ".log";
        emu::utils::saveAsFileDialog(out, log.str(), file_name);
        ssout.str("*** Contents above was saved to a log file ***");
      }
    };

    class DumpLog : public LogAction {
    public:
      DumpLog(emu::pc::Crate * crate) : LogAction(crate) { }
      void display(xgi::Output * out) { addButton(out, "Dump Log to stdout"); }
      void respond(xgi::Input * in, xgi::Output * out, std::ostringstream & ssout, std::ostringstream & log) {
        std::cout<<"----- "<<emu::utils::getDateTime(false)<<": Log Dump -----"<<std::endl<<std::endl;
        std::cout<<log.str()<<std::endl;
        std::cout<<"----- End of Log Dump -----"<<std::endl;
        ssout.str("*** Contents above was dumped to stdout ***");
      }
    };

    class dcfebDebugDump : public Action {
    public:
      dcfebDebugDump(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


    class enableVmeDebugPrintout : public Action, ActionValue<int> {
    public:
      enableVmeDebugPrintout(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


   class CommonUtilities_setupDDU_passThrough : public Action {
   public:
      CommonUtilities_setupDDU_passThrough(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
   };


   class CommonUtilities_setupDDU : public Action {
   public:
      CommonUtilities_setupDDU(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
   };


   class CommonUtilities_restoreCFEBIdle : public Action {
   public:
      CommonUtilities_restoreCFEBIdle(emu::pc::Crate * crate);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
   };




 class ODMB_OTMB_LCT_Testing : public Action, public ActionValue<int> {
    public:
     ODMB_OTMB_LCT_Testing(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
  


  class Investigate_MissingEvents : public Action, public ActionValue<int> {
    public:
     Investigate_MissingEvents(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };


  class ODMB_L1A_Testing : public Action, public ActionValue<int> {
    public:
     ODMB_L1A_Testing(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };



  class RoutineTest_ShortCosmicsRun : public Action, public ActionValue<int> {
    public:
      RoutineTest_ShortCosmicsRun(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };

    class RoutineTest_PrecisionPulses : public Action {
    public:
      RoutineTest_PrecisionPulses(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
    class RoutineTest_PatternPulses_TMBCounters : public Action {
    public:
      RoutineTest_PatternPulses_TMBCounters(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };
    class STEP9bFibers : public Action {
    public:
      STEP9bFibers(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     

    class ButtonTests : public Action {
    public:
      ButtonTests(emu::pc::Crate * crate, emu::odmbdev::Manager* manager);
      void display(xgi::Output * out);
      void respond(xgi::Input * in, std::ostringstream & out);
    };     
    
  }
}

#endif //__emu_odmbdev_Buttons_h__


