#include "emu/odmbdev/Buttons.h"
#include "emu/odmbdev/utils.h"
#include "emu/odmbdev/Manager.h"

#include "emu/pc/Crate.h"
#include "emu/pc/VMEController.h"
#include "emu/pc/CFEB.h"
#include "emu/pc/DAQMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/DDU.h"
#include "emu/pc/TMB.h"
#include "emu/pc/ALCTController.h"
#include "emu/pc/TMB_constants.h"

#include "emu/utils/String.h"
#include "cgicc/HTMLClasses.h"

#include "eth_lib.cc"

#include <stdio.h>
#include <cstdio>
#include <iomanip>
#include <ctime>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>

extern char *wbuf;
extern char rbuf[];
extern int nrbuf;
extern int fd_schar;
extern unsigned char hw_source_addr[];
extern unsigned char hw_dest_addr[];

using namespace std;
using namespace emu::pc;


/******************************************************************************
 * Some classes are declared in the header file because they are short and
 * sweet. Check there first!
 *
 * Also note, at the end of this file is a template for new Action
 * sub-classes.
 * 
 *****************************************************************************/
bool myfunction (pair<float, int> i, pair<float, int> j) { return (i.first < j.first); }
namespace emu {
  namespace odmbdev {
    
    int Manager::slot_number = 15;

    void HardReset::respond(xgi::Input * in, ostringstream & out) { cout<<"==>HardReset"<<endl; 
      if(ccb_->GetCCBmode() != CCB::VMEFPGA) ccb_->setCCBMode(CCB::VMEFPGA); // we want the CCB in this mode for out test stand
      ccb_->HardReset_crate(); // send a simple hard reset without any sleeps
      usleep(150000); // need at least 150 ms after hard reset 
    }
    
    void L1Reset::respond(xgi::Input * in, ostringstream & out) { cout<<"==>L1Reset"<<endl; ccb_->l1aReset(); }

    void BC0::respond(xgi::Input * in, ostringstream & out) { cout<<"==>BC0"<<endl; ccb_->bc0(); }


    /**************************************************************************
     * Reads back the user codes
     *
     *************************************************************************/

    ReadBackUserCodes::ReadBackUserCodes(Crate * crate)
      : Action(crate) {}


    void ReadBackUserCodes::display(xgi::Output * out)
    {
      addButton(out, "Read back usercodes","width: 250px; ");
    }

    void ReadBackUserCodes::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>ReadBackUserCodes"<<endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  vector <CFEB> cfebs = (*dmb)->cfebs();
      
	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb)      
	    {
	      int cfeb_index = (*cfeb).number();

	      out << " ********************* " << endl
		  << " FEB" << cfeb_index << " : "
		  << " Usercode: " << hex << (*dmb)->febfpgauser(*cfeb) << endl
		  << " Virtex 6 Status: " << (*dmb)->virtex6_readreg(7);
	    }

	  (*dmb)->RedirectOutput(&out);
	  (*dmb)->daqmb_adc_dump();
	  (*dmb)->RedirectOutput(&cout);
	}
    }

    /**************************************************************************
     * SetComparatorMode
     *
     *************************************************************************/

    SetComparatorMode::SetComparatorMode(Crate * crate)
      : Action(crate),
        ActionValue<string>("0xa") {}

    void SetComparatorMode::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set Comparator Mode Bits (hex):",
                           "ComparatorMode",
                           numberToString(value()));
    }

    void SetComparatorMode::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetComparatorMode"<<endl; 
      int ComparatorMode = getFormValueIntHex("ComparatorMode", in);
      //value(ComparatorMode); // save the value


      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->set_comp_mode(ComparatorMode);
	  cout << "calling set comparator mode bits with " << ComparatorMode << endl;
	}
    }

    /**************************************************************************
     * SetComparatorThresholds
     *
     *************************************************************************/

    SetComparatorThresholds::SetComparatorThresholds(Crate * crate)
      : Action(crate),
        ActionValue<float>(0.03f) {}

    void SetComparatorThresholds::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set Comparator Thresholds (volts):",
                           "ComparatorThresholds",
                           numberToString(value()));
    }

    void SetComparatorThresholds::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetComparatorThresholds"<<endl; 
      float ComparatorThresholds = getFormValueFloat("ComparatorThresholds", in);
      value(ComparatorThresholds); // save the value

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->set_comp_thresh(ComparatorThresholds);
	}
    }











    /**************************************************************************
     * SetComparatorThresholdsBroadcast
     *
     *************************************************************************/

    SetComparatorThresholdsBroadcast::SetComparatorThresholdsBroadcast(Crate * crate)
      : Action(crate),
        ActionValue<float>(0.03f) {}

    void SetComparatorThresholdsBroadcast::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set Comparator Thresholds-broadcast (volts):",
                           "ComparatorThresholds",
                           numberToString(value()));
    }

    void SetComparatorThresholdsBroadcast::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetComparatorThresholdsBroadcast"<<endl; 
      float ComparatorThresholds = getFormValueFloat("ComparatorThresholds", in);
      value(ComparatorThresholds); // save the value

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->dcfeb_set_comp_thresh_bc(ComparatorThresholds);
	}
    }

    /**************************************************************************
     * SetUpComparatorPulse
     *
     *************************************************************************/

    SetUpComparatorPulse::SetUpComparatorPulse(Crate * crate)
      : Action(crate),
        Action2Values<int,int>(16,-1) {}

    void SetUpComparatorPulse::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
				"Set up comparator pulse: halfstrip(0-31), DCFEB(0-4):",
				"halfstrip",
				numberToString(value1()),
				"dcfeb_number",
				numberToString(value2()));
    }

    void SetUpComparatorPulse::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetUpComparatorPulse"<<endl; 
      int halfstrip = getFormValueInt("halfstrip", in);
      int dcfeb_number = getFormValueInt("dcfeb_number", in);
      value1(halfstrip); // save the value
      value2(dcfeb_number); // save the value
     
      tmb_->SetClctPatternTrigEnable(1); // enable CLCT pretriggers
      tmb_->WriteRegister(emu::pc::seq_trig_en_adr);

      // enable L1A and clct_pretrig from any of dmb_cfeb_calib
      // signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      usleep(100);
      
      int hp[6] = {halfstrip+1, halfstrip, halfstrip+1, halfstrip, halfstrip+1, halfstrip};
      // Note: +1 for layers 0,2,4 is because ME1/1 doesn't have
      // staggered strips, but DAQMB codes assumes staggering.
      int CFEB_mask = 0xff;

      switch(dcfeb_number){
      case 0: CFEB_mask = 0x01; break;
      case 1: CFEB_mask = 0x02; break;
      case 2: CFEB_mask = 0x04; break;
      case 3: CFEB_mask = 0x08; break;
      case 4: CFEB_mask = 0x10; break;
      default: CFEB_mask = 0x1f; break;
      }

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->trigsetx(hp, CFEB_mask); // this calls chan2shift, which does the shift
	  usleep(100);
	}

      ccb_->syncReset();//check
      usleep(100);
      ccb_->bx0();   //check
    }

    /**************************************************************************
     * SetUpPrecisionCapacitors
     *
     *************************************************************************/

    SetUpPrecisionCapacitors::SetUpPrecisionCapacitors(Crate * crate)
      : Action(crate),
        Action2Values<int,int>(8,-1) {}

    void SetUpPrecisionCapacitors::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
				"Set up precision pulse: strip(0-15), DCFEB(0-4)",
				"StripToPulse",
				numberToString(value1()),
				"dcfeb_number",
				numberToString(value2()));
    }

    void SetUpPrecisionCapacitors::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetUpPrecisionCapacitors"<<endl; 
      int strip_to_pulse = getFormValueInt("StripToPulse", in);
      int dcfeb_number = getFormValueInt("dcfeb_number", in);
      value1(strip_to_pulse); // save the value
      value2(dcfeb_number); // save the value
      
      //tmb_->SetClctPatternTrigEnable(1); // set flag to enable CLCT pretriggers
      //usleep(1000);
      //tmb_->WriteRegister(emu::pc::seq_trig_en_adr); // make it so
      //usleep(1000);
      //tmb_->SetTmbAllowClct(1); // set flag to enable CLCT only triggers
      //usleep(1000);
      //tmb_->SetTmbAllowMatch(1); // set flag to enable CLCT+ALCT match triggers
      //usleep(1000);
      //tmb_->WriteRegister(emu::pc::tmb_trig_adr); /// make it so
      //usleep(1000);

      // enable L1A and clct_pretrig from any of dmb_cfeb_calib
      // signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      usleep(100);
      
      
      //crate_->vmeController()->SetPrintVMECommands(1); // turn on debug printouts of VME commands
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->set_ext_chanx(strip_to_pulse, dcfeb_number); // this only sets the array in software
	  (*dmb)->buck_shift(); // this shifts the array into the buckeyes
	  usleep(100);
	}
      //crate_->vmeController()->SetPrintVMECommands(0); // turn off debug printouts of VME commands

      ccb_->l1aReset();
      usleep(1000);
      ccb_->bx0();
      usleep(1000);
    }


    SetTMBdavDelay::SetTMBdavDelay(Crate * crate)
      : Action(crate),
        ActionValue<int>(22) {}

    void SetTMBdavDelay::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set tmb_dav_delay",
			   "idelay",
                           numberToString(value()));
    }

    void SetTMBdavDelay::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetTMBdavDelay"<<endl; 
      int idelay = getFormValueInt("idelay", in);
      value(idelay); // save the value
      
      for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
        {
          (*dmb)->varytmbdavdelay(idelay);
        }   

    }

    /**************************************************************************
     * PulseInternalCapacitorsDMB
     *
     *************************************************************************/

    PulseInternalCapacitorsDMB::PulseInternalCapacitorsDMB(Crate * crate)
      : Action(crate)
    { /* ... nothing to see here ... */ }

    void PulseInternalCapacitorsDMB::display(xgi::Output * out) {
      addButton(out, "Pulse internal capacitors via DMB");
    }

    void PulseInternalCapacitorsDMB::respond(xgi::Input * in, ostringstream & out) {
      cout<<"==>PulseInternalCapacitorsDMB"<<endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->inject(1,0);
	}
    }

    /**************************************************************************
     * PulseInternalCapacitorsCCB
     *
     *************************************************************************/

    PulseInternalCapacitorsCCB::PulseInternalCapacitorsCCB(Crate * crate)
      : Action(crate)
    { /* ... nothing to see here ... */ }

    void PulseInternalCapacitorsCCB::display(xgi::Output * out) {
      addButton(out, "Pulse internal capacitors via CCB");
    }

    void PulseInternalCapacitorsCCB::respond(xgi::Input * in, ostringstream & out) {
      cout<<"==>PulseInternalCapacitorsCCB"<<endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  ccb_->inject(1,0);
	}
    }

    /**************************************************************************
     * PulsePrecisionCapacitorsDMB
     *
     *************************************************************************/

    PulsePrecisionCapacitorsDMB::PulsePrecisionCapacitorsDMB(Crate * crate)
      : Action(crate)
    { /* ... nothing to see here ... */ }

    void PulsePrecisionCapacitorsDMB::display(xgi::Output * out) {
      addButton(out, "Pulse precision capacitors via DMB");
    }

    void PulsePrecisionCapacitorsDMB::respond(xgi::Input * in, ostringstream & out) {
      cout<<"==>PulsePrecisionCapacitorsDMB"<<endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->pulse(1,0);
	}
    }

    /**************************************************************************
     * PulsePrecisionCapacitorsCCB
     *
     *************************************************************************/

    PulsePrecisionCapacitorsCCB::PulsePrecisionCapacitorsCCB(Crate * crate)
      : Action(crate)
    { /* ... nothing to see here ... */ }

    void PulsePrecisionCapacitorsCCB::display(xgi::Output * out) {
      addButton(out, "Pulse precision capacitors via CCB");
    }

    void PulsePrecisionCapacitorsCCB::respond(xgi::Input * in, ostringstream & out) {
      cout<<"==>PulsePrecisionCapacitorsCCB"<<endl; 
      //ccb_->pulse(1,0); // send the pulses 
      ccb_->GenerateDmbCfebCalib0(); // send a pulse
    }

    /**************************************************************************
     * SetDMBDACs
     *
     *************************************************************************/

    SetDMBDACs::SetDMBDACs(Crate * crate)
      : Action(crate),
        ActionValue<float>(1.0f) {}

    void SetDMBDACs::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set DMB DACs 0 and 1 to (volts):",
                           "DAC",
                           numberToString(value()));
    }

    void SetDMBDACs::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetDMBDACs"<<endl; 
      float DAC = getFormValueFloat("DAC", in);
      value(DAC); // save the value

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->set_dac(DAC,DAC); // this was tested and appeared to work correctly
	}
    }

    /**************************************************************************
     * ShiftBuckeyesNormRun
     *
     *************************************************************************/

    ShiftBuckeyesNormRun::ShiftBuckeyesNormRun(Crate * crate)
      : Action(crate) {}

    void ShiftBuckeyesNormRun::display(xgi::Output * out)
    {
      addButton(out, "Shift Buckeyes into normal mode");
    }

    void ShiftBuckeyesNormRun::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>ShiftBuckeyesNormRun"<<endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->shift_all(NORM_RUN);
	(*dmb)->buck_shift();
      }
    }
    
    /**************************************************************************
     * SetPipelineDepthAllDCFEBs
     *
     *************************************************************************/

    SetPipelineDepthAllDCFEBs::SetPipelineDepthAllDCFEBs(Crate * crate)
      : Action(crate),
        ActionValue<int>(66) {}

    void SetPipelineDepthAllDCFEBs::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Set pipeline depth on all DCFEBs:",
                           "depth",
                           numberToString(value()));
    }

    void SetPipelineDepthAllDCFEBs::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetPipelineDepthAllDCFEBs"<<endl; 
      int depth = getFormValueInt("depth", in);
      value(depth); // save the value

      //crate_->vmeController()->SetPrintVMECommands(1); // turn on debug printouts of VME commands
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
      	vector <CFEB> cfebs = (*dmb)->cfebs();
	
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  (*dmb)->dcfeb_set_PipelineDepth(*cfeb, depth);
	  usleep(100);
	  (*dmb)->Pipeline_Restart( *cfeb );
	}
      }
      //crate_->vmeController()->SetPrintVMECommands(0); // turn off debug printouts of VME commands
    }


    Stans_SetPipelineDepthAllDCFEBs::Stans_SetPipelineDepthAllDCFEBs(Crate * crate)
      : Action(crate),
        ActionValue<int>(66) {}

    void Stans_SetPipelineDepthAllDCFEBs::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Stan's Set pipeline depth on all DCFEBs:",
                           "depth",
                           numberToString(value()));
    }

    void Stans_SetPipelineDepthAllDCFEBs::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>Stans_SetPipelineDepthAllDCFEBs"<<endl; 
      int depth = getFormValueInt("depth", in);
      value(depth); // save the value

      //crate_->vmeController()->SetPrintVMECommands(1); // turn on debug printouts of VME commands
      //// HACK to see if Stan's functions work better -Joe
      //      DAQMB* dmb = dmbs_[0];
      //// This is the same orders and the oringinal

      //// For SVN, this is commented out to do nothing because the methods called are just experimental and not in SVN.
      out << "THIS DOES NOTHING UNLESS YOU UNCOMMENT THE EXPERIMENTAL CODE." << endl;

      //// Experimental code not in SVN:
      //       dmb->Set_PipelineDepth_Stan(F1DCFEBM, depth);
      //       dmb->Pipeline_Restart_Stan( F1DCFEBM );
      //       usleep(100);
      //       dmb->Set_PipelineDepth_Stan(F2DCFEBM, depth);
      //       dmb->Pipeline_Restart_Stan( F2DCFEBM );
      //       usleep(100);
      //       dmb->Set_PipelineDepth_Stan(F3DCFEBM, depth);
      //       dmb->Pipeline_Restart_Stan( F3DCFEBM );
      //       usleep(100);
      //       dmb->Set_PipelineDepth_Stan(F4DCFEBM, depth);
      //       dmb->Pipeline_Restart_Stan( F4DCFEBM );
      //       usleep(100);
      //       dmb->Set_PipelineDepth_Stan(F5DCFEBM, depth);
      //       dmb->Pipeline_Restart_Stan( F5DCFEBM );
      //       usleep(100);

      //crate_->vmeController()->SetPrintVMECommands(0); // turn off debug printouts of VME commands
    }


    /**************************************************************************
     * ReadPipelineDepthAllDCFEBs
     *
     *Added by J. Pilot 5 Mar 2013
     *************************************************************************/

    ReadPipelineDepthAllDCFEBs::ReadPipelineDepthAllDCFEBs(Crate * crate)
      : Action(crate){}

    void ReadPipelineDepthAllDCFEBs::display(xgi::Output * out)
    {
      addButton(out, "Read Pipeline Depth");
    }

    void ReadPipelineDepthAllDCFEBs::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>ReadPipelineDepthAllDCFEBs"<<endl; 
      //int depth = getFormValueInt("depth", in);
      //value(depth); // save the value
      
      out << "Reading pipeline depth....." << endl;
      
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector <CFEB> cfebs = (*dmb)->cfebs();
	//int currentPD = -1;
	
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  out << "ReadPipelineDepthAllDCFEBs is still under constuction." << endl;
	  // currentPD = (*cfeb).GetPipelineDepth();
	  // out << "Pipeline Depth is: " << currentPD << endl;
	  // (*dmb)->dcfeb_set_PipelineDepth(*cfeb, currentPD);
	  // usleep(100);
	  // (*dmb)->Pipeline_Restart( *cfeb );
	}
      }
    }



    /**************************************************************************
     * SetFineDelayForADCFEB
     *
     *************************************************************************/

    SetFineDelayForADCFEB::SetFineDelayForADCFEB(Crate * crate)
      : Action(crate),
        Action2Values<int, int>(1, 0) {}

    void SetFineDelayForADCFEB::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Set Fine Delay: delay(0-15), DCFEB(0-4):",
                                "FineDelay",
                                numberToString(value1()),
                                "DcfebNumber",
                                numberToString(value2()));
    }
    
    void SetFineDelayForADCFEB::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>SetFineDelayForADCFEB"<<endl; 
      int delay = getFormValueInt("FineDelay", in);
      int cfeb_number = getFormValueInt("DcfebNumber", in);
      value1(delay); // save the value
      value2(cfeb_number); // save the value
      
      for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector<CFEB> cfebs = (*dmb)->cfebs();
	(*dmb)->dcfeb_fine_delay(cfebs.at(cfeb_number), delay); // careful, I this may depend on the order in the xml
	usleep(100);
	(*dmb)->Pipeline_Restart(cfebs[cfeb_number]); // careful, I this may depend on the order in the xml
      }
    }

    /**************************************************************************
     * PipelineDepthScan with Pulses
     *
     *************************************************************************/

    PipelineDepthScan_Pulses::PipelineDepthScan_Pulses(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
        Action2Values<int, int>(50, 70) {}

    void PipelineDepthScan_Pulses::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Scan pipeline depth with pulses",
                                "From",
                                numberToString(value1()),
                                "To",
                                numberToString(value2()));
    }

    void PipelineDepthScan_Pulses::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>PipelineDepthScan_Pulses"<<endl; 
      int fromDepth = getFormValueInt("From", in);
      value1( fromDepth ); // save the value

      int toDepth = getFormValueInt("To", in);
      value2( toDepth ); // save the value

      const int strip_to_pulse = 8; // TODO: make configurable
      int n_pulses = 5; // TODO: make configurable

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
	(*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
      }
      
      // set register 0 appropriately for communication over the VME backplane,
      // this is necessary for the CCB to issue hard resets and to respond to L1
      // requests from the TMB.
      ccb_->setCCBMode(CCB::VMEFPGA);
      usleep(100);

      //// Hard Reset ////
      // ccb_->HardReset_crate(); // send a simple hard reset without
      // any sleeps usleep(150000); // need at least 150 ms after hard reset
      ccb_->hardReset();
      
      //// Set DAC (pulse height) ////
      float DAC = 1.0;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_dac(DAC,DAC); // I'm not sure this is working. -Joe
      }
      usleep(100);
      
      //// Set comparator thresholds ////
      float ComparatorThresholds = 0.05;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_comp_thresh(ComparatorThresholds);
      }
      usleep(100);
      
      
      //// Resync ////
      //ccb_->l1aReset();
      //usleep(10000);
      
      //// SetUpPrecisionCapacitors
      
      //// This should enable CLCT pretriggers and trigger with a CLCT
      //tmb_->SetClctPatternTrigEnable(1); // set flag to enable CLCT pretriggers
      //usleep(1000);
      //tmb_->WriteRegister(emu::pc::seq_trig_en_adr); // make it so
      //usleep(1000);
      //tmb_->SetTmbAllowClct(1); // set flag to enable CLCT only triggers
      //usleep(1000);
      //tmb_->SetTmbAllowMatch(1); // set flag to enable CLCT+ALCT match triggers
      //usleep(1000);
      //tmb_->WriteRegister(emu::pc::tmb_trig_adr); /// make it so
      //usleep(1000);

      // enable L1A and clct_pretrig from any of dmb_cfeb_calib
      // signals and disable all other trigger sources
      if(n_pulses>0){
	ccb_->EnableL1aFromDmbCfebCalibX();
	usleep(100);
	
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->set_ext_chanx(strip_to_pulse);
	  (*dmb)->buck_shift();
	  usleep(10);
	}
      }else{
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->shift_all(NORM_RUN);
	  (*dmb)->buck_shift();
	  usleep(10);
	}
      }
      usleep(100);
      ccb_->l1aReset(); // needed after shifting buckeyes
      usleep(100);
      
      string subdir = "PDScan_Pulses_" + emu::utils::getDateTime( true );
      manager_->setDAQOutSubdir( subdir );
      //
      // Loop over the requested range of pipeline depth
      //
      for ( int iDepth = fromDepth; iDepth <= toDepth; ++iDepth ){
	
	
	//// Set the pipeline depth on all DCFEBs ////
	for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  vector<CFEB> cfebs = (*dmb)->cfebs();
	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	    
	    (*dmb)->dcfeb_set_PipelineDepth( *cfeb, iDepth );
	    usleep(100);
	    (*dmb)->Pipeline_Restart( *cfeb );
	    usleep(100);
	  }
	}
	ccb_->l1aReset(); // needed after setting/restarting pipeline
	usleep(100);

	//// Start triggering
	ccb_->bc0();
	usleep(100);

	// start DAQ
	cout<<"starting DAQ..."<<endl;
	manager_->startDAQ( string("Pipeline")+emu::utils::stringFrom<int>( iDepth ) );

	// send pulses
	for(int i=0; i<n_pulses; ++i){
	  ccb_->pulse(1,0);
	  //ccb_->GenerateDmbCfebCalib0();
	  usleep(10000);
	}
	
	// stop DAQ
	cout<<"stopping DAQ..."<<endl;
	manager_->stopDAQ();
	
      } // loop over next pipeline depth
      manager_->setDAQOutSubdir( "" );
    }

    /**************************************************************************
     * PipelineDepthScan with Cosmics
     *
     *************************************************************************/

    PipelineDepthScan_Cosmics::PipelineDepthScan_Cosmics(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
        Action2Values<int, int>(50, 70) {}

    void PipelineDepthScan_Cosmics::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Scan pipeline depth with cosmics",
                                "From",
                                numberToString(value1()),
                                "To",
                                numberToString(value2()));
    }

    void PipelineDepthScan_Cosmics::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>PipelineDepthScan_Cosmics"<<endl; 
      int fromDepth = getFormValueInt("From", in);
      value1( fromDepth ); // save the value

      int toDepth = getFormValueInt("To", in);
      value2( toDepth ); // save the value

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
	(*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
      }
      
      // set register 0 appropriately for communication over the VME backplane,
      // this is necessary for the CCB to issue hard resets and to respond to L1
      // requests from the TMB.
      ccb_->setCCBMode(CCB::VMEFPGA);
      usleep(10000);

      //// Hard Reset ////
      // ccb_->HardReset_crate(); // send a simple hard reset without
      // any sleeps usleep(150000); // need at least 150 ms after hard reset
      ccb_->hardReset();

      //// Set comparator thresholds ////
      float ComparatorThresholds = 0.05;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_comp_thresh(ComparatorThresholds);
      }
      usleep(10000);
      
      //// Shift into normal mode
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->shift_all(NORM_RUN);
	(*dmb)->buck_shift();
	usleep(10);
      }
      usleep(10000);
      ccb_->l1aReset(); // needed after buckeye shift
      usleep(10000);

      

      string subdir = "PDScan_Cosmics_" + emu::utils::getDateTime( true );
      manager_->setDAQOutSubdir( subdir );

      //
      // Loop over the requested range of pipeline depth
      //
      for ( int iDepth = fromDepth; iDepth <= toDepth; ++iDepth ){
	
	//// Set the pipeline depth on all DCFEBs ////
	for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  vector<CFEB> cfebs = (*dmb)->cfebs();
	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	    
	    (*dmb)->dcfeb_set_PipelineDepth( *cfeb, iDepth );
	    usleep(100);
	    (*dmb)->Pipeline_Restart( *cfeb );
	    usleep(100);
	  }
	}
	ccb_->l1aReset(); // needed after setting/restarting pipeline
	usleep(100);

	//// Start triggering
	ccb_->bc0();
	usleep(100);

	// start DAQ
	cout<<"starting DAQ..."<<endl;
	manager_->startDAQ( string("Pipeline")+emu::utils::stringFrom<int>( iDepth ) );

	//// Take cosmics
	::sleep(1);
	
	// stop DAQ
	cout<<"stopping DAQ..."<<endl;
	manager_->stopDAQ();
	
      } // loop over next pipeline depth
      manager_->setDAQOutSubdir( "" );
    }
    
    /**************************************************************************
     * TmbDavDelayScan
     *
     *************************************************************************/

    TmbDavDelayScan::TmbDavDelayScan(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
        Action2Values<int, int>(0, 31) {}

    void TmbDavDelayScan::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Scan TMB DAV delay with pulses",
                                "From",
                                numberToString(value1()),
                                "To",
                                numberToString(value2()));
    }

    void TmbDavDelayScan::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>TmbDavDelayScan"<<endl; 
      int fromdelay = getFormValueInt("From", in);
      value1( fromdelay ); // save the value

      int todelay = getFormValueInt("To", in);
      value2( todelay ); // save the value

      const int strip_to_pulse = 8; // TODO: make configurable

      // set register 0 appropriately for communication over the VME
      // backplane, this is necessary for the CCB to issue hard resets
      // and to respond to L1 requests from the TMB.
      ccb_->setCCBMode(CCB::VMEFPGA);

      //
      // SetUpPrecisionCapacitors
      //
      ccb_->hardReset();


      // tmb_->SetClctPatternTrigEnable(1); // enable CLCT pretriggers
      // tmb_->WriteRegister(emu::pc::seq_trig_en_adr);

      // enable L1A and clct_pretrig from any of dmb_cfeb_calib
      // signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      usleep(100);


      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  (*dmb)->set_ext_chanx(strip_to_pulse);
	  (*dmb)->buck_shift();
	  usleep(100);
	}


      //
      // Loop over the requested range of dav delays
      //
      for ( int idelay = fromdelay; idelay <= todelay; ++idelay ){
	
	ccb_->l1aReset();
	usleep(100);
	
	for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->varytmbdavdelay(idelay);
	}
	
	ccb_->l1aReset();
	usleep(100);
	ccb_->bc0();
	
	manager_->startDAQ( string("TmbDavDelay")+emu::utils::stringFrom<int>( idelay ) );
	
	ccb_->pulse(1,0);
	//ccb_->GenerateDmbCfebCalib0();
	
	manager_->stopDAQ();
      }
    }


    /**************************************************************************
     * L1aDelayScan
     *
     *************************************************************************/

    L1aDelayScan::L1aDelayScan(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
	Action2Values<int, int>(100, 140) {}

    void L1aDelayScan::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Scan CCB L1A delay with pulses",
                                "From",
                                numberToString(value1()),
                                "To",
                                numberToString(value2()));
    }

    void L1aDelayScan::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>L1aDelayScan"<<endl; 
      int fromdelay = getFormValueInt("From", in);
      value1( fromdelay ); // save the value

      int todelay = getFormValueInt("To", in);
      value2( todelay ); // save the value

      const int strip_to_pulse = 8; // TODO: make configurable

      // set register 0 appropriately for communication over the VME
      // backplane, this is necessary for the CCB to issue hard resets
      // and to respond to L1 requests from the TMB.
      ccb_->setCCBMode(CCB::VMEFPGA);

      //
      // SetUpPrecisionCapacitors
      //
      ccb_->hardReset();

      // enable L1A and clct_pretrig from any of dmb_cfeb_calib
      // signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      usleep(100);
      //ccb_->SetExtTrigDelay(19);
      //usleep(100);

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_ext_chanx(strip_to_pulse);
	(*dmb)->buck_shift();
	usleep(100);
      }


      //
      // Loop over the requested range of l1a delays
      //
      for ( int idelay = fromdelay; idelay <= todelay; ++idelay ){

	ccb_->l1aReset();
	usleep(100);
	
	ccb_->SetL1aDelay(idelay);

	ccb_->l1aReset();
	usleep(100);
	ccb_->bc0();
	
	manager_->startDAQ( string("L1aDelay")+emu::utils::stringFrom<int>( idelay ) );
    
	ccb_->pulse(1,0);
	//ccb_->GenerateDmbCfebCalib0();
	usleep(10);

	manager_->stopDAQ();
      }
    }

    /**************************************************************************
     * TMBHardResetTest
     *
     *************************************************************************/

    TMBHardResetTest::TMBHardResetTest(Crate * crate)
      : Action(crate),
        ActionValue<int>(100) {}

    void TMBHardResetTest::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "TMB Hard Reset Test, number of resets:",
                           "NumberOfHardResets",
                           numberToString(value()));
    }

    void TMBHardResetTest::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>TMBHardResetTest"<<endl; 
      int NumberOfHardResets = getFormValueInt("NumberOfHardResets", in);
      value(NumberOfHardResets); // save the value

      out << "=== TMB Hard Reset Test ===\n";

      int expected_day = tmb_->GetExpectedTmbFirmwareDay();
      int expected_month = tmb_->GetExpectedTmbFirmwareMonth();
      int expected_year = tmb_->GetExpectedTmbFirmwareYear();
      int expected_type = tmb_->GetExpectedTmbFirmwareType();
      int expected_version = tmb_->GetExpectedTmbFirmwareVersion();
      int hiccup_number = 0;

      time_t now = time(0);
      struct tm* tm = localtime(&now);
      out << "Beginning time: " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec << endl;

      int i; // we'll want to get the value of this after the loop is complete
      // in order to print how many succesful hard resets we ran
      bool firmware_lost = false;

      // the CCB writes to stdout every time it issues a hard rest, but we
      // don't care we turn this back on after the loop
      ostringstream waste;
      ccb_->RedirectOutput(&waste);

      for (i = 0;
           i < NumberOfHardResets && !firmware_lost;
           ++i)
	{
	  if (i % 500 == 0) {
	    out << "Hard Reset Number " << i << endl;
	  }

	  //ccb_->hardReset(); // slow
	  ccb_->HardReset_crate(); // no sleeps
	  usleep(800000); // need at least 150 ms for hard resets

	  const int maximum_firmware_readback_attempts = 2;
	  int firmware_readback_attempts = 0;
	  do {
	    firmware_lost = false;
	    tmb_->FirmwareDate(); // reads the month and day off of the tmb
	    int actual_day = tmb_->GetReadTmbFirmwareDay();
	    int actual_month = tmb_->GetReadTmbFirmwareMonth();
	    tmb_->FirmwareYear(); // reads the year off of the tmb
	    int actual_year = tmb_->GetReadTmbFirmwareYear();
	    tmb_->FirmwareVersion(); // reads the version off of the tmb
	    int actual_type = tmb_->GetReadTmbFirmwareType();
	    int actual_version = tmb_->GetReadTmbFirmwareVersion();

	    if ((actual_day != expected_day) ||
		(actual_month != expected_month) ||
		(actual_year != expected_year) ||
		(actual_type != expected_type) ||
		(actual_version != expected_version))
	      {
		firmware_lost = true;
		hiccup_number++;
		usleep(1000); // sometimes the readback fails, so wait and try again
	      }
	    // if we haven't gone over our maximum number of readback attempts and
	    // the firmware was "lost" (i.e. the readback didn't match the expected
	    // values), then try again.

	  } while (++firmware_readback_attempts < maximum_firmware_readback_attempts &&
		   firmware_lost);
	}

      ccb_->RedirectOutput(&cout);

      now = time(0);
      tm =  localtime(&now);
      out << "End time: " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec << endl;
      out << "Number of hiccups: " << hiccup_number << endl;

      if(firmware_lost) {
        out << "The frimware was lost after " << i << " hard resets." << endl;
      } else {
        out << "The firmware was *never* lost after " << i << " hard resets." << endl;
      }
    }

    TMBRegisters::TMBRegisters(Crate * crate)
      : Action(crate) { }

    void TMBRegisters::display(xgi::Output * out)
    {
      addButton(out, "Read TMB Registers");
    }

    void TMBRegisters::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>TMBRegisters"<<endl;      
      //Print out current registers
      out<< "GTX Registers for fibers 0-6 "<<endl;
      out<<" 0x14a \t"<<std::hex<<tmb_->ReadRegister(0x14a)<<endl; 
      out<<" 0x14c \t"<<std::hex<<tmb_->ReadRegister(0x14c)<<endl;
      out<<" 0x14e \t"<<std::hex<<tmb_->ReadRegister(0x14e)<<endl;
      out<<" 0x150 \t"<<std::hex<<tmb_->ReadRegister(0x150)<<endl;
      out<<" 0x152 \t"<<std::hex<<tmb_->ReadRegister(0x152)<<endl;
      out<<" 0x154 \t"<<std::hex<<tmb_->ReadRegister(0x154)<<endl;
      out<<" 0x156 \t"<<std::hex<<tmb_->ReadRegister(0x156)<<endl;
      out<<" 0x158 \t"<<std::hex<<tmb_->ReadRegister(0x158)<<endl;
      out<<"Hot Channel Masks"<<endl;
      out<<" 0x42 \t"<<std::hex<<tmb_->ReadRegister(0x42)<<endl;
      out<<" 0x4a \t"<<std::hex<<tmb_->ReadRegister(0x4a)<<endl;
      out<<" 0x4c \t"<<std::hex<<tmb_->ReadRegister(0x4c)<<endl;
      out<<" 0x4e \t"<<std::hex<<tmb_->ReadRegister(0x4e)<<endl;
      out<<" 0x50 \t"<<std::hex<<tmb_->ReadRegister(0x50)<<endl;
      out<<" 0x52 \t"<<std::hex<<tmb_->ReadRegister(0x52)<<endl;
      out<<" 0x54 \t"<<std::hex<<tmb_->ReadRegister(0x54)<<endl;
      out<<" 0x56 \t"<<std::hex<<tmb_->ReadRegister(0x56)<<endl;
      out<<" 0x58 \t"<<std::hex<<tmb_->ReadRegister(0x58)<<endl;
      out<<" 0x5a \t"<<std::hex<<tmb_->ReadRegister(0x5a)<<endl;
      out<<" 0x5c \t"<<std::hex<<tmb_->ReadRegister(0x5c)<<endl;
      out<<" 0x5e \t"<<std::hex<<tmb_->ReadRegister(0x5e)<<endl;
      out<<" 0x60 \t"<<std::hex<<tmb_->ReadRegister(0x60)<<endl;
      out<<" 0x62 \t"<<std::hex<<tmb_->ReadRegister(0x62)<<endl;
      out<<" 0x64 \t"<<std::hex<<tmb_->ReadRegister(0x64)<<endl;
      out<<" 0x66 \t"<<std::hex<<tmb_->ReadRegister(0x66)<<endl;
      out<<" 0x16e \t"<<std::hex<<tmb_->ReadRegister(0x16e)<<endl;
      out<<" 0x170 \t"<<std::hex<<tmb_->ReadRegister(0x170)<<endl;
      out<<" 0x172 \t"<<std::hex<<tmb_->ReadRegister(0x172)<<endl;
      out<<" 0x174 \t"<<std::hex<<tmb_->ReadRegister(0x174)<<endl;
      out<<" 0x176 \t"<<std::hex<<tmb_->ReadRegister(0x176)<<endl;
      out<<" 0x178 \t"<<std::hex<<tmb_->ReadRegister(0x178)<<endl;
      out<<"bad bit registers"<<endl;
      out<<" 0x122 \t"<<std::hex<<tmb_->ReadRegister(0x122)<<endl;
      out<<" 0x15c \t"<<std::hex<<tmb_->ReadRegister(0x15c)<<endl; 
      
    }
    
    TMBSetRegisters::TMBSetRegisters(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
        Action2Values<string, string>("0x14a", "0x0001") {}

    void TMBSetRegisters::display(xgi::Output * out)
    {
      addButtonWithTwoTextBoxes(out,
                                "Set TMB Registers (hex)",
                                "Register",
                                value1(),
                                "Value",
                                value2());
    }

    void TMBSetRegisters::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>TMBSetRegisters"<<endl; 
      int RegisterValue = getFormValueIntHex("Register", in);
      int setting = getFormValueIntHex("Value", in);

      std::stringstream hexstr_RegisterValue;
      hexstr_RegisterValue << std::hex << RegisterValue;
      value1( hexstr_RegisterValue.str() ); // save value in hex
      
      std::stringstream hexstr_setting;
      hexstr_setting << std::hex << setting;
      value2( hexstr_setting.str() ); // save value in hex
      
      tmb_->WriteRegister(RegisterValue,setting);										      
      usleep(100000);
      
      out<<"Set TMB Register: "<<std::hex<<RegisterValue<<" to "<<std::hex<<tmb_->ReadRegister(RegisterValue)<<endl;
    }
    TMBEnableCLCTInput::TMBEnableCLCTInput(Crate * crate)
      : Action(crate),
        ActionValue<int>(1) {}

    void TMBEnableCLCTInput::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Enable DCFEB:",
                           "(1-7)",
                           numberToString(value()));
    }

    void TMBEnableCLCTInput::respond(xgi::Input * in, ostringstream & out)
    {
      int DCFEBtoEnable = getFormValueInt("(1-7)", in);
      value(DCFEBtoEnable); // save the value
      if(DCFEBtoEnable ==1){
        tmb_->WriteRegister(0x4a,0xffff);
        tmb_->WriteRegister(0x4c,0xffff);
        tmb_->WriteRegister(0x4e,0xffff);
        if(tmb_->ReadRegister(0x4a)==0xffff && tmb_->ReadRegister(0x4c)==0xffff &&tmb_->ReadRegister(0x4e)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB1"<<endl;
      }
      if(DCFEBtoEnable ==2){
        tmb_->WriteRegister(0x50,0xffff);
        tmb_->WriteRegister(0x52,0xffff);
        tmb_->WriteRegister(0x54,0xffff);
        if(tmb_->ReadRegister(0x50)==0xffff && tmb_->ReadRegister(0x52)==0xffff &&tmb_->ReadRegister(0x54)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB2"<<endl;
      }
      if(DCFEBtoEnable ==3){
        tmb_->WriteRegister(0x56,0xffff);
        tmb_->WriteRegister(0x58,0xffff);
        tmb_->WriteRegister(0x5a,0xffff);
        if(tmb_->ReadRegister(0x56)==0xffff && tmb_->ReadRegister(0x58)==0xffff &&tmb_->ReadRegister(0x5a)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB3"<<endl;
      }
      if(DCFEBtoEnable ==4){
        tmb_->WriteRegister(0x5c,0xffff);
        tmb_->WriteRegister(0x5e,0xffff);
        tmb_->WriteRegister(0x60,0xffff);
        if(tmb_->ReadRegister(0x5c)==0xffff && tmb_->ReadRegister(0x5e)==0xffff &&tmb_->ReadRegister(0x60)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB4"<<endl;
      }
      if(DCFEBtoEnable ==5){
        tmb_->WriteRegister(0x62,0xffff);
        tmb_->WriteRegister(0x64,0xffff);
        tmb_->WriteRegister(0x66,0xffff);
        if(tmb_->ReadRegister(0x62)==0xffff && tmb_->ReadRegister(0x64)==0xffff &&tmb_->ReadRegister(0x66)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB5"<<endl;
      }
      if(DCFEBtoEnable ==6){
        tmb_->WriteRegister(0x16e,0xffff);
        tmb_->WriteRegister(0x170,0xffff);
        tmb_->WriteRegister(0x172,0xffff);
        if(tmb_->ReadRegister(0x16e)==0xffff && tmb_->ReadRegister(0x170)==0xffff &&tmb_->ReadRegister(0x172)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB6"<<endl;
      }
      if(DCFEBtoEnable ==7){
        tmb_->WriteRegister(0x174,0xffff);
        tmb_->WriteRegister(0x176,0xffff);
        tmb_->WriteRegister(0x178,0xffff);
        if(tmb_->ReadRegister(0x174)==0xffff && tmb_->ReadRegister(0x176)==0xffff &&tmb_->ReadRegister(0x178)==0xffff) out<<"FELICIDADES! Has prendido el DCFEB7"<<endl;
      }
    }

    TMBDisableCLCTInput::TMBDisableCLCTInput(Crate * crate)
      : Action(crate),
        ActionValue<int>(1) {}

    void TMBDisableCLCTInput::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
                           "Disable DCFEB:",
                           "(1-7)", 
                           numberToString(value()));
    }

    void TMBDisableCLCTInput::respond(xgi::Input * in, ostringstream & out)
    {
      int DCFEBtoDisable = getFormValueInt("(1-7)", in);
      value(DCFEBtoDisable); // save the value
      if(DCFEBtoDisable ==1){ 
        tmb_->WriteRegister(0x4a,0);
        tmb_->WriteRegister(0x4c,0);
        tmb_->WriteRegister(0x4e,0);
        if(tmb_->ReadRegister(0x4a)==0 && tmb_->ReadRegister(0x4c)==0 &&tmb_->ReadRegister(0x4e)==0) out<<"FELICIDADES! Has apagado el DCFEB1"<<endl;
      }
      if(DCFEBtoDisable ==2){  
        tmb_->WriteRegister(0x50,0);
        tmb_->WriteRegister(0x52,0);
        tmb_->WriteRegister(0x54,0);
        if(tmb_->ReadRegister(0x50)==0 && tmb_->ReadRegister(0x52)==0 &&tmb_->ReadRegister(0x54)==0) out<<"FELICIDADES! Has apagado el DCFEB2"<<endl;
      }
      if(DCFEBtoDisable ==3){
        tmb_->WriteRegister(0x56,0);
        tmb_->WriteRegister(0x58,0);
        tmb_->WriteRegister(0x5a,0);
        if(tmb_->ReadRegister(0x56)==0 && tmb_->ReadRegister(0x58)==0 &&tmb_->ReadRegister(0x5a)==0) out<<"FELICIDADES! Has apagado el DCFEB3"<<endl;
      }
      if(DCFEBtoDisable ==4){
        tmb_->WriteRegister(0x5c,0);
        tmb_->WriteRegister(0x5e,0);
        tmb_->WriteRegister(0x60,0);
        if(tmb_->ReadRegister(0x5c)==0 && tmb_->ReadRegister(0x5e)==0 &&tmb_->ReadRegister(0x60)==0) out<<"FELICIDADES! Has apagado el DCFEB4"<<endl;
      }
      if(DCFEBtoDisable ==5){
        tmb_->WriteRegister(0x62,0);
        tmb_->WriteRegister(0x64,0);
        tmb_->WriteRegister(0x66,0);
        if(tmb_->ReadRegister(0x62)==0 && tmb_->ReadRegister(0x64)==0 &&tmb_->ReadRegister(0x66)==0) out<<"FELICIDADES! Has apagado el DCFEB5"<<endl;
      }
      if(DCFEBtoDisable ==6){
        tmb_->WriteRegister(0x16e,0);
        tmb_->WriteRegister(0x170,0);
        tmb_->WriteRegister(0x172,0);
        if(tmb_->ReadRegister(0x16e)==0 && tmb_->ReadRegister(0x170)==0 &&tmb_->ReadRegister(0x172)==0) out<<"FELICIDADES! Has apagado el DCFEB6"<<endl;
      }
      if(DCFEBtoDisable ==7){
        tmb_->WriteRegister(0x174,0);
        tmb_->WriteRegister(0x176,0);
        tmb_->WriteRegister(0x178,0);
        if(tmb_->ReadRegister(0x174)==0 && tmb_->ReadRegister(0x176)==0 &&tmb_->ReadRegister(0x178)==0) out<<"FELICIDADES! Has apagado el DCFEB7)"<<endl;
      }
    }

    
    TMBDisableCopper::TMBDisableCopper(Crate * crate)
      : Action(crate) { }

    void TMBDisableCopper::display(xgi::Output * out)
    {
      addButton(out, "TMB Disable Copper");
    }

    void TMBDisableCopper::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>TMBDisableCopper"<<endl; 
      
      out<<" Initial TMB Register 42 = "<<std::hex<<tmb_->ReadRegister(42)<<endl;
      
      tmb_->SetDistripHotChannelMask(0,0x00000000ff);                       
      tmb_->SetDistripHotChannelMask(1,0x00000000ff);                        
      tmb_->SetDistripHotChannelMask(2,0x00000000ff);                        
      tmb_->SetDistripHotChannelMask(3,0x00000000ff);                        
      tmb_->SetDistripHotChannelMask(4,0x00000000ff);  
      tmb_->SetDistripHotChannelMask(5,0x00000000ff);                      
      tmb_->WriteDistripHotChannelMasks(); 
      
      
      
      out<<" TMB Register 42 = "<<std::hex<<tmb_->ReadRegister(42)<<endl;
      
    }
 

    /**************************************************************************
     * PulseWires (based on STEP test 16)
     *
     *************************************************************************/

    PulseWires::PulseWires(Crate * crate)
      : Action(crate), 
	ActionValue<int>(20) {}

    void PulseWires::display(xgi::Output * out)
    {

      addButtonWithTextBox(out,
                           "Pulse Wires with External Trigger Delay =",
                           "ExTrigDelay",
                           numberToString(value()));

    }

    void PulseWires::respond(xgi::Input * in, ostringstream & out)
    {



      ////////////////// -- set xml parameters specific to test 1

      int x_CCBmode = 1;
      int x_l1aDelay = 155;
      int x_alct_send_empty = 1;
      int x_alct_trig_mode = 0;
      int x_all_cfeb_active = 1;
      int x_clct_pretrig_enable = 0;
      int x_clct_readout_without_trig = 0;
      int x_clct_trig_enable = 1;
      int x_match_trig_enable = 0;
      int x_request_l1a = 1;
      int x_alct_l1a_delay = 146;

      ccb_->SetCCBmode(x_CCBmode);
      ccb_->Setl1adelay(x_l1aDelay);
      alct_->SetSendEmpty(x_alct_send_empty);
      alct_->SetTriggerMode(x_alct_trig_mode);
      tmb_->SetEnableAllCfebsActive(x_all_cfeb_active);
      tmb_->SetClctPatternTrigEnable(x_clct_pretrig_enable); // actually named pretrig in xml
      tmb_->SetAllowClctNontrigReadout(x_clct_readout_without_trig);
      tmb_->SetTmbAllowClct(x_clct_trig_enable);
      tmb_->SetTmbAllowMatch(x_match_trig_enable);
      tmb_->SetRequestL1a(x_request_l1a);
      alct_->SetL1aDelay(x_alct_l1a_delay);
      // tmb_->SetL1aDelay(x_alct_l1a_delay);
      alct_->configure();

      for(unsigned long int numReg = 0; numReg<  tmb_->TMBConfigurationRegister.size(); numReg++){
	unsigned long int x_address = tmb_->TMBConfigurationRegister.at(numReg);
	tmb_->WriteRegister(x_address);
      }
      
      //////////////////


      unsigned int ExTrigDelay = getFormValueInt("ExTrigDelay", in);
      value(ExTrigDelay); // save the value

      
      for(int AFEB_STANDBY=0; AFEB_STANDBY<=1; AFEB_STANDBY++){


	ostream noBuffer( NULL );
	const uint64_t nLayerPairs = 3; // Pairs of layers to scan, never changes. (Scans 2 layers at a time.)
	uint64_t events_per_layer    = 1; //parameters_["events_per_layer"]; normally 1000
	uint64_t alct_test_pulse_amp = 255; //parameters_["alct_test_pulse_amp"];


	// /home/cscme11/TriDAS/emu/emuDCS/PeripheralCore/include/emu/pc
	// CCB.h 
	ccb_->EnableL1aFromSyncAdb();
      
	// fixed to unsigned int
	ccb_->SetExtTrigDelay(ExTrigDelay); 

	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;

	std::cout<<" ExTrigDelay = "<<ExTrigDelay<<" AFEB_STANDBY = "<<AFEB_STANDBY<<std::endl;

	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;
	std::cout<<" ************************** "<<std::endl;

	uint64_t afebGroupMask = 0x7f; // AFEB mask - pulse all of them from test 16
	// uint64_t afebGroupMask = 0x3fff; // all afebs from test 14

	
	alct_->SetUpPulsing( alct_test_pulse_amp, PULSE_AFEBS, afebGroupMask, ADB_SYNC );

	tmb_->EnableClctExtTrig();
	alct_->SetInvertPulse_(ON);  
	std::cout<<" invert pulse has been set to "<<alct_->Get_InvertPulse()<<std::endl;

	alct_->FillTriggerRegister_();
	alct_->WriteTriggerRegister_();

	// added a call to the print out 
	alct_->PrintTriggerRegister_();



	// added a call to the config printout 
	alct_->PrintALCTConfiguration();



	for ( uint64_t iLayerPair = 0; iLayerPair < nLayerPairs; ++iLayerPair ){

	  // reprogram standby register to enable 2 layers at a time
	  //const int standby_fmask[nLayerPairs] = {066, 055, 033};



	  if(AFEB_STANDBY==1)
	    {

	      for (int lct_chip = 0; lct_chip < alct_->MaximumUserIndex() / 6; lct_chip++)
		{
		  //int astandby = standby_fmask[iLayerPair];
		  for (int afeb = 0; afeb < 6; afeb++)
		    {
		      //	alct_->SetStandbyRegister_(lct_chip*6 + afeb, (astandby >> afeb) & 1);
		
		    }
		}
	      //		alct_->WriteStandbyRegister_();
	      ::sleep(10);
	    }
	  //ccb_->RedirectOutput( &noBuffer ); // ccb prints a line on each test pulse - waste it
	  ccb_->RedirectOutput( &cout ); // ccb prints a line on each test pulse - waste it

		      
	  for ( uint64_t iPulse = 1; iPulse <= events_per_layer; ++iPulse )
	    {

	      // from test 14 also throw in this call
	      //    ccb_->GenerateAlctAdbASync();
	      //    usleep(10000);	 



	      // from test 16
	      ccb_->GenerateAlctAdbSync();
	      usleep(10);	  

		 
	    } 
      
	  ccb_->RedirectOutput (&cout); // get back ccb output

	  /////////
	     
	}
      }
    }
    
    

    /**************************************************************************
     * DDU_KillFiber
     *
     *************************************************************************/

    DDU_KillFiber::DDU_KillFiber(Crate * crate)
      : Action(crate),
	ActionValue<string>("read") {}

    void DDU_KillFiber::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "Read(read)/Write(15bit hex#) DDU Kill Fiber",
			   "KillFiber",
			   value());
    }

    void DDU_KillFiber::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>DDU_KillFiber"<<endl; 
      int KillFiber = getFormValueIntHex("KillFiber", in);
      string KillFiberString = getFormValueString("KillFiber", in);
      //value( KillFiberString ); // save value in hex
      value("read"); // always default to "read"

      if( KillFiberString == "read" ){ // READ
	out << "DDU Read Kill Fiber:" << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu){
	  out << "  DDU in slot " << (*ddu)->slot() << ": " << endl;
	  out << "  DDU with ctrl fpga user code: " << (*ddu)->CtrlFpgaUserCode()
	      << hex << setfill('0') // set up for next two hex values
	      << " and vme prom user code: "
	      << setw(8) << (*ddu)->VmePromUserCode()
	      << " has Kill Fiber is set to: "
	      << setw(4) << (*ddu)->readFlashKillFiber() << endl;
	}
      }else{  // WRITE
	out << "DDU Write Kill Fiber:" << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu){
	  out << "  DDU in slot " << (*ddu)->slot() << "..." << endl;
	  (*ddu)->writeFlashKillFiber(KillFiber);
	}
      }
    }

    /**************************************************************************
     * DDU_EthPrescale
     *
     *************************************************************************/

    DDU_EthPrescale::DDU_EthPrescale(Crate * crate)
      : Action(crate),
	ActionValue<string>("read") {}
    
    void DDU_EthPrescale::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "Read(read)/Write(hex#) DDU Gb Eth Prescale",
			   "prescale",
			   value());
    }
    
    void DDU_EthPrescale::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>DDU_EthPrescale"<<endl; 
      int prescale = getFormValueIntHex("prescale", in);
      string prescaleString = getFormValueString("prescale", in);
      //value( prescaleString );
      value("read"); // always default to "read"      

      if(prescaleString == "read" ){ // READ
	out << "DDU Read Gb Eth Prescale: " << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu) {
	  out << "  DDU in slot " << (*ddu)->slot() << hex << setfill('0') << ": " << setw(4) << (*ddu)->readGbEPrescale() << dec << endl;
	}
	
      }else{ // WRITE
	out << "DDU Write Gb Eth Prescale: " << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu) {
	  out << "  DDU in slot " << (*ddu)->slot() << "..." << endl;
	  (*ddu)->writeGbEPrescale(prescale);
	}
      }
    }

    /**************************************************************************
     * DDU_FakeL1
     *
     *************************************************************************/

    DDU_FakeL1::DDU_FakeL1(Crate * crate)
      : Action(crate),
	ActionValue<string>("read") {}
    
    void DDU_FakeL1::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "Read(read)/Write(hex#) DDU Fake L1 (passthrough)",
			   "mode",
			   value());
    }
    
    void DDU_FakeL1::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>DDU_FakeL1"<<endl; 
      int mode = getFormValueIntHex("mode", in);
      string modeString = getFormValueString("mode", in);
      //value( modeString );
      value("read"); // always default to "read"
      
      if(modeString == "read" ){ // READ
	out << "DDU Read Fake L1 (passthrough): " << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu) {
	  out << "  DDU in slot " << (*ddu)->slot() << hex << setfill('0') << ": " << setw(4) << (*ddu)->readFakeL1() << dec << endl;
	}
	
      }else{ // WRITE
	out << "DDU Write Fake L1 (passthrough): " << endl;
	for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end(); ++ddu) {
	  out << "  DDU in slot " << (*ddu)->slot() << "..." << endl;
	  (*ddu)->writeFakeL1(mode);
	}
      }
    }




    std::string ToUpper(std::string s){
      for(unsigned int i(0); i<s.length(); ++i){
	s.at(i)=toupper(s.at(i));
      }
      return s;
    }

    string FixLength(unsigned int Number, unsigned int Length, bool isHex){
      std::stringstream Stream;
      if(isHex) Stream << std::hex << Number;
      else Stream << std::dec << Number;
      string sNumber = Stream.str();
      for(unsigned int cha=0; cha<sNumber.size(); cha++) sNumber[cha] = toupper(sNumber[cha]);
      while(sNumber.size() < Length) sNumber = " " + sNumber;
      return sNumber;
    }

    unsigned short CountSetBits(unsigned short x){
      unsigned short count(0);
      for(count =0; x; count++){
	x &= x-1;
      }
      return count;
    }

    int write_eth_raw(std::string tag){
      time_t rawtime;
      struct tm *timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      char year[10], month[10], day[10], hour[10], minute[10], second[10];
      strftime(year, 10, "%g", timeinfo ); strftime(month, 10, "%m", timeinfo ); strftime(day, 10, "%d", timeinfo ); 
      strftime(hour, 10, "%H", timeinfo ); strftime(minute, 10, "%M", timeinfo ); strftime(second, 10, "%S", timeinfo ); 
      std::ostringstream outnamestream;

      outnamestream << "/local/data/odmb_ucsb/raw/odmb_" << tag <<"_" <<year<<month<<day<<"_"
		    <<hour<<minute<<second<<".raw";
      std::string outname =outnamestream.str();
      FILE *outfile;
      outfile = fopen(outname.c_str(),"wb");

      unsigned short dduheader[] = {0xF860, 0x0602, 0x0200, 0x5000, 0x0000, 0x8000, 
				    0x0001, 0x8000, 0x2FC1, 0x0001, 0x3030, 0x0001};
      unsigned short ddutrailer[] = {0x8000, 0x8000, 0xFFFF, 0x8000, 0x0001, 0x0005, 
				     0xC2DB, 0x8040, 0xC2C0, 0x4918, 0x000E, 0xA000};
      unsigned short odmbewords[] = {0xE001, 0xE002, 0xE003, 0xE004};
      EthBuf myeth;
      eth_open("schar3",myeth);
      usleep(1);
      eth_register_mac();

      EthStr ethstr=eth_readmm(myeth);
      int n = ethstr.n_evt; int nevents = 0;
      vector<char> rbuf = ethstr.rbuf;

      while(n!=0){// && nevents < 10){
	fwrite(dduheader, sizeof(dduheader[0]), 12, outfile);
	//if((rbuf[n-7]|0x0F) == -1 && (rbuf[n-9]|0x0F) == -1 && (rbuf[n-11]|0x0F) == -1 && (rbuf[n-13]|0x0F) == -1)
	//  cout <<" matches"<<endl;
	//	printf("%02x%02x %02x%02x %02x%02x", rbuf[n-7], rbuf[n-8], rbuf[n-9], rbuf[n-10], rbuf[n-11], rbuf[n-12]);

    	printf(" Event %d has %d words\n",nevents+1,n/2);
       	for(int j=0; j<n; j = j+2){
	  printf(" %02x%02x",rbuf[j+1]&0xff,rbuf[j]&0xff);
	  if(j%16 == 14){   //print words in 24 columns, in three sets of 8 
	    printf("   ");} 
	  if(j%48 == 46){   //if header for odmb go to next event  
	    printf("\n");}
	  //if((rbuf[j]&0xF000)==0xF001) printf("X");
      	}
      	printf("\n\n");

	for(int j=0; j<n-6; j++){
	  fwrite(&rbuf[j], sizeof(rbuf[j]), 1, outfile);
	}
	fwrite(odmbewords, sizeof(odmbewords[0]), 4, outfile);
	fwrite(ddutrailer, sizeof(ddutrailer[0]), 12, outfile);
	nevents++;

	EthStr j_ethstr=eth_readmm(myeth);
        n = j_ethstr.n_evt;
      }

      if(outfile) fclose(outfile);
      eth_close(myeth);

      cout<<"Written "<<outname<<endl;
      return nevents;

    }

    /**************************************************************************
     * ChangeSlotNumber 
     *
     * A domain-specific-lanaguage for issuing vme commands. 
     *************************************************************************/

    ChangeSlotNumber::ChangeSlotNumber(Crate * crate, emu::odmbdev::Manager* manager)
      : OneTextBoxAction(crate, manager, "update")
    {
      //cout << "Updating the slot number box" << endl; 
    }

    int parse(string str_temp){
      int str = 0;
      //If it ends in h:
      if (str_temp.size() > 2 && str_temp.substr(str_temp.size()-2,2) == "_h"){ 
        stringstream hex_string;
        hex_string << hex << str_temp.substr(0,str_temp.size()-2).c_str();
        hex_string >> str; 
      } else if (str_temp.size() > 2 && str_temp.substr(str_temp.size()-2,2) == "_d"){ //If it ends in d: 
        stringstream dec_string; 
        dec_string << dec << str_temp.substr(0,str_temp.size()-2).c_str();
        dec_string >> str;
      } else { //If it ends in nothing at all
        stringstream hex_string;
        hex_string << hex <<  str_temp.c_str();
        hex_string >> str;
      }
      return str;
    }

    int parse_default_d(string str_temp){
      int str = 0;
      //If it ends in h:
      if (str_temp.size() > 2 && str_temp.substr(str_temp.size()-2,2) == "_h"){ 
        stringstream hex_string;
        hex_string << hex << str_temp.substr(0,str_temp.size()-2).c_str();
        hex_string >> str; 
      }

      //If it ends in d: 
      else if (str_temp.size() > 2 && str_temp.substr(str_temp.size()-2,2) == "_d"){ 
        stringstream dec_string; 
        dec_string << dec << str_temp.substr(0,str_temp.size()-2).c_str();
        dec_string >> str;
      }
 
      //If it ends in nothing at all
      else { 
        stringstream dec_string;
        dec_string << dec <<  str_temp.c_str();
        dec_string >> str;
      }
      return str;
    }

    void ChangeSlotNumber::respond(xgi::Input * in, ostringstream & out) {
      OneTextBoxAction::respond(in, out);
      out<<"SET_SLOT     "<< Manager::getSlotNumber() <<"           Changed default slot to " 
	 << Manager::getSlotNumber()<<endl<<endl;
    } // End ChangeSlotNumber::respond


    /**************************************************************************
     * ExecuteVMEDSL
     *
     * A domain-specific-lanaguage for issuing vme commands. 
     *************************************************************************/

    ExecuteVMEDSL::ExecuteVMEDSL(Crate * crate, emu::odmbdev::Manager* manager)
      : FourTextBoxAction(crate, manager, "Run VME commands")
    {
      cout << " Initializing the DAQMBs, which are VMEModules" << endl; 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb) 
        (*dmb)->start();
      nCommand = 0;
      cout<<"Creating DAQ folder"<<endl;
      manager_->setDAQOutSubdir( "" );
      cout<<"Created DAQ folder"<<endl;
    }
    
    void ExecuteVMEDSL::respond(xgi::Input * in, ostringstream & out) {
      FourTextBoxAction::respond(in, out);
      
      nCommand++;
      out<<"****************   VME command "<<nCommand<<"   ***************"<<endl;
      int slot = Manager::getSlotNumber(); //the slot number to use
      char rcv[2];
      unsigned int sleepTimer;
      unsigned int testReps(0);
      unsigned int addr;
      unsigned short int data;
      unsigned int pipeDepth;
      int irdwr, TypeCommand=0; 
      // irdwr:   
      // 0 bufread
      // 1 bufwrite 
      // 2 bufread snd  
      // 3 bufwrite snd 
      // 4 flush to VME (disabled)
      // 5 loop back (disabled)
      // 6 delay

      //------------------------------------------SETUP NAME/LOCATION OF OUTPUT LOG FILE------------------------------------------------
      //Variables needed for output log file
      char outputfilename[80]; //the name of the output file
      string log_subdir = "-1"; //the box number read from text box

      time_t rawtime;
      struct tm *timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);

      //Read box number from test box
      if (this->textBoxContent3 != "") log_subdir = (this->textBoxContent3).c_str();
      
      //If log subdir is empty, just put the log file directly in the logfiles directory
      if (log_subdir == "-1"){ strftime( outputfilename, 80, "logfiles/%y_%m_%d_ODMB_testing.log", timeinfo );
	cout << "output file name: " << outputfilename << endl; }
      
      //Otherwise, put the log files into a new subdirectory specified by the box number
      else {
	//Put format with time information into temporary variable
	char temp_outputfilename[80]; //temporary, used to hold time information, but without box number information
	strftime( temp_outputfilename, 80, "%y_%m_%d_ODMB_testing.log", timeinfo );
	//Add the box number to this, and change type to string stream
	stringstream outputfilename3;
	outputfilename3 << "logfiles/" << log_subdir << "/" << temp_outputfilename;
	//Read this filename into "outputfilename", which has the proper type
	outputfilename3 >> outputfilename;
	//cout << "output file name: " << outputfilename << endl; 

	//Now we need to create the directory.  Start with a stringstream with the directory name
	stringstream temp_outputdirectory;
	temp_outputdirectory << "logfiles/" << log_subdir;
	//Convert this to a string
	string temp2_outputdirectory;
	temp2_outputdirectory = temp_outputdirectory.str();
	//Convert this to const char*.  
	const char* outputdirectory;
	outputdirectory = temp2_outputdirectory.c_str(); 
	//Use this to make the directory
	mkdir(outputdirectory, S_IRWXU);
      }

      //Needed to put time stamps in logs
      char timestamp[80];
      strftime( timestamp, 80, "%T", timeinfo );

      //Needed to write header in logs
      struct stat statbuffer;
      int writeheader = stat( outputfilename, &statbuffer );
           
      //Open logs and write header
      ofstream logfile;
      logfile.open( outputfilename, ios::app );
      if( writeheader!=0 ) logfile << "# R/W\tCommand\tData\tSlot\tTime\t\tComments" << endl;

      //------------------------------------------READ COMMANDS FROM BOX OR TEXT FILE------------------------------------------------
      istringstream countertext(this->textBoxContent1); //read the number of times to repeat commands 
      istringstream filetext(this->textBoxContent2); //input a file path to the wimp page, read commands from the file
      bool textFileMode(false); // Option to input a text file from the second box on WIMP
      string filePath, fileContents, file_line;
 
      //Read file containing commands
      if (filetext.str().size()) { 
      	textFileMode = true;
      	filePath = filetext.str();
      	ifstream fileFromWIMP(filePath.c_str());
      	while ( fileFromWIMP.good() ) {
          getline (fileFromWIMP,file_line);
      	  fileContents+=file_line;
      	  fileContents+="\n";
    	}
    	fileFromWIMP.close();
      }

      istringstream alltext; 
      if (textFileMode) alltext.str(fileContents); 
      else alltext.str(this->textBoxContent4); // get the text from the box                                                                                                      
      
      //Now alltext has the commands no matter whether you used the box or the file mode.
      string line, buffer;
      getline(countertext,line,'\n');
      const unsigned long repeatNumber=strtoul(line.c_str(),NULL,0);
      std::vector<std::string> allLines(0);
      while (getline(alltext,line,'\n')){
	allLines.push_back(line);
      }
      bool writeHex = true;

      //------------------------------------------EXECUTE COMMANDS (REST OF FUNCTION)------------------------------------------------
      for(unsigned int repNum=0; repNum<repeatNumber; ++repNum){
        unsigned int loopLevel(0);
        std::vector<unsigned int> loopCounter(0), loopMax(0), loopStart(0);
        for(unsigned int lineNum=0; lineNum<allLines.size(); ++lineNum){
          slot = Manager::getSlotNumber(); //Load the default slot number for each iteration
          line=allLines.at(lineNum);
          while(line.length()>0 && (line.at(0)==' ' || line.at(0)=='\t')) line.erase(0,1);
          istringstream iss(line);

          // 0 :       End of run, stop execution
          // 1 :       Binary (simulation file style)
          // 2,r,R :   Reads in hex
          // 3,w,W :   Writes in hex
          // 4,rs,RS : Reads in hex
          // 5,ws,WS : Writes in hex
          // 6,bl,BL : Begin loop
          // 7,el,EL : End loop
          // 
          // >3 : Produces a sleep for that number of microseconds
          // 
          // <0 : Send an error message to the output and abort further execution.
          if(line.size()>1) iss >> buffer;
          else buffer = "#";  // This avoids problems when the line is too short
          buffer=ToUpper(buffer);
          if(buffer=="0")  {
            out<<"Found EOR, exiting."<<endl;
            break;// EOR instruction
          } 
          else if(buffer=="1") { // Expect 32 bits for address and 32 bits for data
            string addr_str, data_str, tmp_str;

            while(addr_str.size()<32 && iss.good()){ // read in 32 bits for address
              iss >> tmp_str;
              addr_str += tmp_str;
            }
            while(data_str.size()<32 && iss.good()){ // read in 32 bits for data
              iss >> tmp_str;
              data_str += tmp_str;
            }
            if(addr_str.size()!=32 || data_str.size()!=32){
              out<<"ERROR: address("<<addr_str<<") or data("<<data_str
        	 <<") is not 32 bits on line: "<<line<<endl;
              return;
            }
            // 26th and 25th "bits" from right tell read (10) or write (01)
            irdwr = (addr_str.at(addr_str.size()-26)=='1')? 2 : 3; 
            addr = binaryStringToUInt(addr_str);
            data = binaryStringToUInt(data_str);
            TypeCommand = irdwr;
          } 
          else if(buffer=="2" || buffer=="R") { // Read in hex
            string addr_temp;
            iss >> addr_temp;	
            addr = parse(addr_temp); 
            string data_temp;
            iss >> data_temp;
            data = parse(data_temp);
            irdwr = 2; TypeCommand = 2;
            //cout << "Read command.  Addr in hex: " << hex << addr << " data in hex: " << hex << data << endl;
          } 
          else if(buffer=="3" || buffer=="W") {
            string addr_temp;
            iss >> addr_temp;	
            addr = parse(addr_temp); 
            string data_temp;
            iss >> data_temp;
            data = parse(data_temp);
            irdwr = 3; TypeCommand = 3;
            //cout << "Write command.  Addr in hex: " << hex << addr << " data in hex: " << hex << data << endl;
          } 
          else if(buffer=="4" || buffer=="RS") { // Read in hex with slot
            string addr_temp, data_temp, slot_temp;
            iss >> addr_temp >> data_temp >> slot;	
            addr = parse(addr_temp);
            data = parse(data_temp);
            irdwr = 2; TypeCommand = 4;
            //cout << "Read command.  Addr in hex: " << hex << addr << " data in hex: " << hex << data << " slot: " << dec << slot << endl;
          } 
          else if(buffer=="5" || buffer=="WS") { // Write in hex with slot
            string addr_temp;
            iss >> addr_temp;	
            addr = parse(addr_temp);
            string data_temp;
            iss >> data_temp;
            data = parse(data_temp);
            if(addr >= 0x4000 && addr <= 0x4018) {iss >> dec >> data >> dec >> slot; writeHex = false;}	
            string slot_temp;
            iss >> slot;
            irdwr = 3; TypeCommand = 5;
            //cout << "Write command.  Addr in hex: " << hex << addr << " data in hex: " << hex << data << " slot: " << dec << slot << endl;
          } 
          else if(buffer=="6" || buffer=="BL") {
            ++loopLevel;
            loopCounter.push_back(1);
            int thisLoopMax(0);
            iss >> dec >> thisLoopMax;
            loopMax.push_back(thisLoopMax);
            loopStart.push_back(lineNum);
            irdwr = -1; TypeCommand= -1; //Negative; do not send to VME - AD
          } 
          else if(buffer=="7" || buffer=="EL") {
            if(loopCounter.at(loopLevel-1)<loopMax.at(loopLevel-1)){
              lineNum=loopStart.at(loopLevel-1);
              ++loopCounter.at(loopLevel-1);
            }else{
              loopCounter.pop_back();
              loopMax.pop_back();
              loopStart.pop_back();
              --loopLevel;
            }
            irdwr = -1; TypeCommand = -1; //Negative; do not send to VME - AD
          } 
          else if(buffer=="8" || buffer=="SLEEP" || buffer=="WAIT") {
            iss >> sleepTimer;
            usleep(sleepTimer);
            continue; // Nothing else to do from this line.
          } 
          else if(buffer=="RL1A") {
            TypeCommand = 9;
            iss >> sleepTimer;
          } 
          else if(buffer=="RL1AM") {
            TypeCommand = 10;
            iss >> sleepTimer;
          } 
          else if(buffer=="COMP_DDU") {
            TypeCommand = 11;
            iss >> testReps;
          } 
          else if(buffer=="COMP_PC"){
            TypeCommand = 12;
            iss >> testReps;
          } else if(buffer=="BEGIN_DAQ"){
            manager_->startDAQ( string("DDU_ODMB_UCSB") );
            out << "BEGIN_DAQ"<<endl;
            logfile << "BEGIN_DAQ"<< "                "  << "    "<< timestamp << "\t" <<" Starting DAQ"<<endl;
            continue; // Next line, please.
          } else if(buffer=="END_DAQ"){
            manager_->stopDAQ();
            out << "END_DAQ"<<endl;
            logfile << "END_DAQ"<< "                "  << "    "<< timestamp << "\t" <<" Stopping DAQ"<<endl;
            continue; // Next line, please.
          } else if(buffer=="RESET_ETH"){
            EthBuf myeth;
            eth_open("schar3",myeth);
            eth_reset(myeth);
            eth_reset_close(myeth);
            out<<"RESET_ETH"<<endl;
            logfile<<"RESET_ETH"<<endl;
            continue; // Next line, please.
          } else if(buffer=="WRITE_ETH"){
            std::string tag;
            iss >> tag;
            int nevents = write_eth_raw(tag);
            out << "WRITE_ETH               "<<tag << " - Written "<<nevents<<" events"<<endl;
            logfile << "WRITE_ETH               "<<timestamp<<"\t"<<tag << " - Written "<<nevents<<" events"<<endl;
            continue; // Next line, please.

          } else if(buffer=="SET_SLOT"){
            int old_slot = slot; 
            string slot_temp;
            iss >> dec >> slot;
            Manager::setSlotNumber(slot);
            out << "SET_SLOT     "<<slot<<"           Changed default slot from "<<dec<<old_slot << " to " << slot << endl;
	    continue;
          } else if(buffer=="SET_PIPE"){
            TypeCommand = 13;
            iss >> pipeDepth;
            out << "SET_PIPE   " << pipeDepth << endl;
          } else { // Anything else is treated as a comment.
            out  << line << endl;
            logfile << "# " << line << endl;
            continue; // Next line, please.
          }

          //// We are at the end of official commands, add the rest to comments - KF
          string comments;
          getline( iss, comments );
          while(comments.size()>0 && (comments.at(0)==' ' || comments.at(0)=='\t')) comments.erase(0,1);
          //// Output type of command is always W or R - KF
          //// Added command types L for loops, T for tests - AD
          string commandtype;
          switch(irdwr){
          case 2:
            commandtype="R";
            break;
          case 3:
            commandtype="W";
            break;
          case -1:
            commandtype="L";
            break;
          case 11:
          case 12:
            commandtype="T";
            break;
          default:
            commandtype="Error with command type!";
            break;
          }

          //// If we make it here, we have a VME command to run! ////
          //Only if TypeCommand>0 due to addition of loop! - AD
          //printf("TypeCommand: %d\n", TypeCommand);
          //out << TypeCommand << endl;

          // Set the top bits of address to the slot number
          unsigned int shiftedSlot = slot << 19;
          int nDigits = 5;
          if(TypeCommand>=2 && TypeCommand<=5){
            addr = (addr & 0x07ffff) | shiftedSlot;	    
            printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x})  ", irdwr, 
        	   (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
            crate_->vmeController()->vme_controller(irdwr, addr, &data, rcv); // Send the VME command!
            usleep(1);
            
            // If it was a read, then show the result
            if(irdwr==2) printf("  ==> rcv[1,0] = %02x %02x", (rcv[1] & 0xff), (rcv[0] & 0xff));
            printf("\n");
            fflush(stdout);
            
            
            // Output to website
            addr = (addr & 0x07ffff);  // Masks out the slot number
            unsigned int VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
	    stringstream slot_stream; 
	    slot_stream << slot;
	    string slot_s = slot_stream.str(), label = "  ", command;
            bool readHex = true;
            if((addr >= 0x321C && addr <= 0x337C) || (addr >= 0x33FC && addr <= 0x3FFF) 
               || addr == 0x500C || addr == 0x510C || addr == 0x520C || addr == 0x530C || addr == 0x540C   || addr == 0x550C || addr == 0x560C 
               || addr == 0x8004 ||  (addr == 0x5000 && VMEresult < 0x1000)) {
	      readHex = false;
	      label = "_d";
	    }
            switch (irdwr) {
            case 2:
	      command = "R  ";
	      if(slot != Manager::getSlotNumber()) {
		command = "RS ";
		comments = slot_s + "\t" + comments;
	      }
	      out << command << FixLength(addr) <<  "        "  << FixLength(VMEresult, nDigits, readHex) << label  << "   "<<comments<<endl;
	      logfile << command << FixLength(addr) <<  "        "  << FixLength(VMEresult, nDigits, readHex) << label << "   "
		      << timestamp << "\t" << comments<<endl;
	      break;
            case 3:
	      command = "W  ";
	      if(slot != Manager::getSlotNumber()) {
		command = "WS ";
		comments = slot_s + "\t" + comments;
	      }
              out << command << FixLength(addr) << " " << FixLength(data, nDigits, true) << label <<  "          "<<comments<<endl;
              logfile << command << FixLength(addr) << "  " << FixLength(data, nDigits, true) <<  label << "          "
        	      << timestamp << "\t" << comments<<endl;
              break;
            }
          } else if (TypeCommand==9 || TypeCommand==10){
            unsigned int read_fifo = 0x005000;
            unsigned int reset_fifo = 0x005020;
            // Set the top bits of address to the slot number
            read_fifo = (read_fifo & 0x07ffff) | shiftedSlot;
            reset_fifo = (reset_fifo & 0x07ffff) | shiftedSlot;
            crate_->vmeController()->vme_controller(2, read_fifo, &data, rcv); // Read the 1st word for L1A_MATCH
            if(TypeCommand == 9) crate_->vmeController()->vme_controller(2, read_fifo, &data, rcv); //2nd word for L1A
            unsigned int VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
            usleep(1);
            
            data = 0xff;
            crate_->vmeController()->vme_controller(3, reset_fifo, &data, rcv); // Send the VME command!
            usleep(1);
            long l1a_cnt = VMEresult;
            string command = "RL1A ", l1a_comment = "L1A";
            if(TypeCommand == 10) {command = "RL1AM";l1a_comment = "L1A_MATCH";}
            if(l1a_cnt < 0x1000){
              out << command<<"           "  << FixLength(l1a_cnt, nDigits, false)  << "    "<<comments<<endl;
              logfile << command<< "           "  << FixLength(l1a_cnt, nDigits, false)  << "    "
        	      << timestamp << "\t" << comments<<endl;
            } else {
              out << command<< "            XXXX"  << "    "<<"No good DCFEB "<<l1a_comment<<"_CNT read"<<endl;
              logfile << command<< "            XXXX"  << "    "<< timestamp << "\t" <<"No good DCFEB "<<l1a_comment<<"_CNT read"<<endl;
            }
          }else if (TypeCommand==11 || TypeCommand==12){
            std::string testType = TypeCommand==11 ? "DDU" : "PC ";
            char rcv_tx[2], rcv_rx[2];
            unsigned short rcv_tx_i(0), rcv_rx_i(0), rcv_diff_i(0);
            unsigned long bitChanges(0), bitMatches(0);
            unsigned short arg(0);
            const unsigned int addr_tx = TypeCommand==11 ? (0x005300 & 0x07ffff) | shiftedSlot : (0x005100 & 0x07ffff) | shiftedSlot;
            const unsigned int addr_rx = TypeCommand==11 ? (0x005400 & 0x07ffff) | shiftedSlot : (0x005200 & 0x07ffff) | shiftedSlot;
            for(unsigned int j(0); j<testReps; ++j){
              crate_->vmeController()->vme_controller(2, addr_tx, &arg, rcv_tx); //Read TX FIFO
              usleep(1);
              crate_->vmeController()->vme_controller(2, addr_rx, &arg, rcv_rx); //Read RX FIFO
              usleep(1);
              
              //Combine into int
              rcv_tx_i=(rcv_tx[1]*0x100) + rcv_tx[0];
              rcv_rx_i=(rcv_rx[1]*0x100) + rcv_rx[0];
              
              //Compare FIFOs
              rcv_diff_i=rcv_tx_i^rcv_rx_i;
              
              //Count bits set to 1 (where differences occur)
              unsigned int setBits(CountSetBits(rcv_diff_i));
              bitChanges+=setBits;
              bitMatches+=16-setBits;
            }
            if(bitChanges){
              out << "COMP_" << testType << FixLength(testReps, nDigits+2, false) << FixLength(bitChanges, nDigits+1, false) << "    Bit changes = " << bitChanges << ", bit matches = " << bitMatches << std::endl;
              logfile << "COMP_" << testType << FixLength(testReps, nDigits+2, false) << FixLength(bitChanges, nDigits+1, false) << "    " << timestamp << "\tBit changes = " << bitChanges << ", bit matches = " << bitMatches << std::endl;
            }else{
              out << "COMP_" << testType << FixLength(testReps, nDigits+2, false) << FixLength(bitChanges, nDigits+1, false) << "    All " << bitMatches << " bits match" << std::endl;
              logfile << "COMP_" << testType << FixLength(testReps, nDigits+2, false) << FixLength(bitChanges, nDigits+1, false) << "    " << timestamp << "\tAll " << bitMatches << " bits match" << std::endl;
            }
          } else if (TypeCommand == 13){

	    cout << "Setting PD" << endl;

	    for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
      	
	      vector <CFEB> cfebs = (*dmb)->cfebs();
        
	      for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
		(*dmb)->dcfeb_set_PipelineDepth(*cfeb, pipeDepth);
		usleep(100);
		cout << "Set PD CFEB" << endl;
		(*dmb)->Pipeline_Restart( *cfeb );
	      }
	    }
	  }	

        } // while parsing lines
        out<<endl;
      }
      
      //// close the logfile -KF
      logfile.close();
      
    } // End ExecuteVMEDSL::respond
    
    /**************************************************************************
     * SYSMON
     *
     * A small class to print the SYSMON
     **************************************************************************/
    SYSMON::SYSMON(Crate * crate) 
      : ButtonAction(crate,"Check ODMB Volt & Temp") 
    { /* The choices here are really a blank constructor vs duplicating the ExecuteVMEDSL constructor.
	 I've tried the former -- TD
      */
    }
    
    void SYSMON::respond(xgi::Input * in, ostringstream & out) { // TD
      out << "********** System Monitoring **********" << endl;
      int slot = Manager::getSlotNumber();
      unsigned int shiftedSlot = slot << 19;
      char rcv[2];
      unsigned int read_addr_vec[9] = {0x7150, 0x7120, 0x7000, 0x7160, 0x7140, 0x7100, 0x7130, 0x7110, 0x7170};
      string description[9] = {" C\t -  Thermistor 1 temperature", " C\t -  Thermistor 2 temperature", " C\t -  FPGA temperature", 
			       " V\t -  P1V0: Voltage for FPGA", " V\t -  P2V5: Voltage for FPGA", " V\t -  LV_P3V3: Voltage for FPGA", 
			       " V\t -  P3V3_PP: Voltage for PPIB", " V\t -  P5V: General voltage", " V\t -  P5V_LVMB: Voltage for LVMB"};
      //int precision[9] = {1, 1, 1, 2, 2, 2, 2, 2, 2};
      float voltmax[9] = {1.0, 1.0, 1.0, 1.0, 2.5, 3.3, 3.3, 5.0, 5.0};
      float result2[9];
      for (int i = 0; i < 9; i++){
        int read_addr = (read_addr_vec[i] & 0x07ffff) | shiftedSlot;
        unsigned short int data;
        crate_->vmeController()->vme_controller(2,read_addr,&data,rcv);
        unsigned short int VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
        crate_->vmeController()->vme_controller(2,read_addr,&data,rcv);
        VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
        if (i == 0 && VMEresult > 0xfff){ cout << "ERROR: bad readout from system monitoring." << endl; out << "ERROR: bad readout from system monitoring." << endl; break; }
        if (i == 2){ result2[i] = 503.975*VMEresult/4096.0 - 273.15; }
        else if (i > 1) result2[i] = VMEresult*2.0*voltmax[i]/4096.0;
        else if (i == 0 || i == 1) result2[i] = 7.865766417e-10*pow(VMEresult,3) - 7.327237418e-6*pow(VMEresult,2) + 3.38189673e-2*VMEresult - 9.678340882;

        //out << "R  " << FixLength(read_addr & 0xffff) << "        " << description[i] << ": " << setprecision(precision[i]) << result2[i] << endl;      
        out << setprecision(3) << result2[i] << description[i]<< endl;      
      }
      out<<endl;
    }

    LVMBtest::LVMBtest(Crate * crate, emu::odmbdev::Manager* manager) 
      : ThreeTextBoxAction(crate, manager, "LVMB test") 
    { 
      //This constructor intentionally left blank.
    }
    
    void LVMBtest::respond(xgi::Input * in, ostringstream & out) { // TD
      ThreeTextBoxAction::respond(in, out);
      out << "********** Low Voltage Monitoring **********" << endl;
      string textBoxContent = this->textBoxContent;
      string textBoxContent2 = this->textBoxContent2;
      string textBoxContent3 = this->textBoxContent3;
      istringstream textBoxContent1(textBoxContent);
      string volt1_string, volt2_string;
      textBoxContent1 >> volt1_string;
      textBoxContent1 >> volt2_string;
      float volt1 = atof(volt1_string.c_str());
      float volt2 = atof(volt2_string.c_str());
      if (!(volt1 > 0 && volt1 < 10)) volt1 = 0;
      if (!(volt2 > 0 && volt2 < 10)) volt2 = 0;
      float expectedV[8] = {volt1, volt1, 0, volt1, volt2, volt2, volt2, 0};
      float tol = atof(textBoxContent2.c_str());
      int slot = Manager::getSlotNumber();
      unsigned int shiftedSlot = slot << 19;
      char rcv[2];
      unsigned short int data;
      //addresses
      unsigned short int VMEresult = 0;
      int addr_sel_adc = (0x008020 & 0x07ffff) | shiftedSlot;
      int addr_cntl_byte = (0x008000 & 0x07ffff) | shiftedSlot;
      int addr_read_adc = (0x008004 & 0x07ffff) | shiftedSlot;
      int addr_turn_on = (0x008010 & 0x07ffff) | shiftedSlot;
      int addr_verify_on = (0x008014 & 0x07ffff) | shiftedSlot;
      int notConnected = 0;
      int nReps = atoi(textBoxContent3.c_str());
      int nTests = 2;
      int fail_turn_off = 0;
      int fail_turn_on = 0;
      unsigned short int ctrl_byte_vec[7] =  {0x89, 0xA9, 0xB9, 0xC9, 0x99, 0xE9, 0xD9};
      unsigned short int ctrl_byte_vec2[7] = {0x81, 0xA1, 0xB1, 0xC1, 0x91, 0xE1, 0xD9};
      vector <pair<float, int> > voltages[7];
      unsigned short int ADC_number_vec[7] = {0x0, 0x4, 0x1, 0x05, 0x3, 0x7, 0x2};
      int pass = 0;
      int fail = 0;
      //1) Test on-off 
      //Send control byte to ADC
      unsigned short int ctrl_byte = 0x00;
      crate_->vmeController()->vme_controller(3, addr_turn_on, &ctrl_byte, rcv);
      usleep(10);
      //Read from ADC
      crate_->vmeController()->vme_controller(2, addr_verify_on, &data, rcv);
      usleep(10);
      //Format result
      VMEresult = 100;
      VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
      if (VMEresult != 0x00){ out << "Failed turn off test!" << endl; fail_turn_off++;}
      //Send control byte to ADC
      ctrl_byte = 0xFF;
      crate_->vmeController()->vme_controller(3, addr_turn_on, &ctrl_byte, rcv);
      usleep(10);
      //Read from ADC
      crate_->vmeController()->vme_controller(2, addr_verify_on, &data, rcv);
      usleep(10);
      //Format result
      VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
      if (VMEresult == 0xBAAD){ out << "No device connected." << endl; return; }
      else if (VMEresult != 0xFF){ out << "Failed turn on test!" << endl; fail_turn_on++; return; }
      for (int k = 0; k < nTests; k++){
        //2) Test ADC
        for (int j = 0; j < nReps; j++){
          for (int i = 0; i < 7; i++){
            if (i == 2) continue;
            //Write ADC to be read 
            unsigned short int ADC_number = ADC_number_vec[i];
            crate_->vmeController()->vme_controller(3, addr_sel_adc, &ADC_number, rcv);
            //Send control byte to ADC
            ctrl_byte = ctrl_byte_vec[i];
            crate_->vmeController()->vme_controller(3, addr_cntl_byte, &ctrl_byte, rcv);
            usleep(30);
            //Read from ADC
            crate_->vmeController()->vme_controller(2, addr_read_adc, &data, rcv); //often returns -100, maybe fixed by introducing sleeps?
            usleep(10);
            //Format result
            VMEresult = 0;
            VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
            float voltage_result_1 = float(VMEresult*10.0)/float(0xfff);
            //Send control byte to ADC -- method 2
            ctrl_byte = ctrl_byte_vec2[i];
            crate_->vmeController()->vme_controller(3, addr_cntl_byte, &ctrl_byte, rcv);
            usleep(10);
            //Read from ADC
            crate_->vmeController()->vme_controller(2, addr_read_adc, &data, rcv);
            usleep(10);
            //Format result
            VMEresult = 0;
            VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
            if (VMEresult == 65535 && i == 0){ 
              VMEresult = 0;
              notConnected++;
              if (notConnected == 1) out << "Failed test: LVMB not connected" << endl;
              if (notConnected == 3) return; 
              break;
            }
            else{
              float voltage_result_2 = float(VMEresult)*5.0/float(0xfff);
              //Error checking
              if (fabs(voltage_result_2 - voltage_result_1) > .05 && voltage_result_1 < 4.9){ out << "ERROR!  TWO METHODS DISAGREE!! Results are: " << voltage_result_1 << " and " << voltage_result_2 << endl; }
              float voltage = (voltage_result_1 > 4.9 ? voltage_result_1 : 0.5*(voltage_result_1 + voltage_result_2));
              bool done = false;
              if ( fabs(voltage - expectedV[i]) > tol ){ fail++; cout << "fail! " << VMEresult << " " << i << " " << j << " volt " << voltage << " " << voltage_result_1 << " " << voltage_result_2 << " exp  " << expectedV[i] << endl;   }
              else{
                pass++;
              
              if (done == false){
                voltages[i].push_back(make_pair(voltage, 1));
              }
             
                for (unsigned int l = 0; l < voltages[i].size(); l++){
                  if (fabs(voltages[i][l].first - voltage) < .00001){
                    voltages[i][l].second++;
                    done = true;
                    break;
                  } 
                }
                
                if (done == false){
                  voltages[i].push_back(make_pair(voltage, 1));
                }
             }
 
              //output result
              //out << "LVMB test " << i << ": " << std::fixed << setprecision(2) << voltage_result_1 << " V "<< endl;
            }
          }//i-loop
        }//j-loop
      }//k-loop
      out << "Failed turn on test " << fail_turn_on << " times out of " << nTests << endl;
      out << "Failed turn off test " << fail_turn_off << " times out of " << nTests << endl;
      out << "Voltage reading failure rate: " << fail << " out of " << nTests*nReps*6  << ". " << endl; 
      out << "Voltage reading pass rate: " << pass << " out of " << nTests*nReps*6 << ". " << endl; 
      for (int i = 0; i < 8; i++){
         if (i == 2 || i == 7) continue;
        std::sort( voltages[i].begin(), voltages[i].end(), myfunction );
        cout << "Printing outliers for box " << i << ":" << endl;
        for (unsigned int l = 0; l < voltages[i].size(); l++){
          if (voltages[i][l].first - expectedV[i] > tol) cout << voltages[i][l].first << " " << voltages[i][l].second << endl;
          //cout << voltages[i][l].first << " " << voltages[i][l].second << endl;
        }
      }
      out << " " << endl;
      if (fail < 10 && fail_turn_on == 0 && fail_turn_off == 0) out << "LVMB Test Passed!" << endl;
      else out << "LVMB Test Failed!" << endl;
    }
    
    DCFEBJTAGcontrol::DCFEBJTAGcontrol(Crate * crate, emu::odmbdev::Manager* manager) 
      : RepeatTextBoxAction(crate, manager, "DCFEB JTAG Control"/*,"1"*/) 
    { 
      // Has a "repeat n times" textbox.
    }
    
    void DCFEBJTAGcontrol::respond(xgi::Input * in, ostringstream & out) { // JB-F
        out << "********** Read DCFEB UserCode **********" << endl;
        RepeatTextBoxAction::respond(in, out);
        istringstream countertext(this->textBoxContent);
        string line;
      	getline(countertext,line,'\n');
      	const unsigned long repeatNumber=strtoul(line.c_str(),NULL,0);
      	
      	vector<unsigned int> v_nUCReads(7,0); // number of times we successfully read UserCode of each DCFEB
      	vector<unsigned int> v_nShiftReads(7,0); // number of times we successfully read shifted bits on ADC mask
      	vector<string> v_firmwareVersion(7,""); // store the firmware version
      	vector<unsigned int> v_nJTAGshifts(7,0); // number of JTAG shifts commands issued
      	for(unsigned int repNum=0; repNum<repeatNumber; ++repNum){ // repeat the test repNum times
			int slot = Manager::getSlotNumber();
			unsigned int shiftedSlot = slot << 19;
			char rcv[2];
			unsigned short int data;
			unsigned short int reg_user_code = 0x3C8;
			unsigned short int reg_dev_sel = 0x3C2;
			unsigned short int reg_dev_val = 0xC;
			unsigned short int reg_val_sel = 0x3C3;
			unsigned short int VMEresult;
			//addresses
			int addr_sel_dcfeb = (0x001020 & 0x07ffff) | shiftedSlot;
			int addr_read_dcfeb = (0x001024 & 0x07ffff) | shiftedSlot;
			int addr_set_int_reg = (0x00191C & 0x07ffff) | shiftedSlot;
			int addr_read_hdr = (0x001F04 & 0x07ffff) | shiftedSlot;
			int addr_read_tlr = (0x001F08 & 0x07ffff) | shiftedSlot;
			int addr_shift_ht = (0x00170C & 0x07ffff) | shiftedSlot;
			int addr_shift_dr = (0x001B0C & 0x07ffff) | shiftedSlot;
			int addr_read_tdo = (0x001F14 & 0x07ffff) | shiftedSlot;
			unsigned short int DCFEB_number[7] = {0x1, 0x2, 0x4, 0x08, 0x10, 0x20,0x40};
			unsigned int nConnected(0);
			for (int d = 0; d < 7; d++){ // Loop over all DCFEBs
				// Select DCFEB (one bit per DCFEB)
				crate_->vmeController()->vme_controller(3, addr_sel_dcfeb, &DCFEB_number[d], rcv);
				// Read selected DCFEB
				crate_->vmeController()->vme_controller(2, addr_read_dcfeb, &DCFEB_number[d], rcv);
				VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
				// Set instruction register to *Read UserCode*
				crate_->vmeController()->vme_controller(3, addr_set_int_reg, &reg_user_code, rcv);
				// Shift 16 lower bits
				crate_->vmeController()->vme_controller(3, addr_read_hdr, &data, rcv);
				// Read first half of UserCode
				crate_->vmeController()->vme_controller(2, addr_read_tdo, &data, rcv);
				// check firmware version
				VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
				string s_result = FixLength(VMEresult, 4, true);
				string firmwareVersion = s_result.substr(1,1)+"."+s_result.substr(2,2);
				// Shift 16 upper bits
				crate_->vmeController()->vme_controller(3, addr_read_tlr, &data, rcv);
				// Read second half of UserCode
				crate_->vmeController()->vme_controller(2, addr_read_tdo, &data, rcv);	
				crate_->vmeController()->vme_controller(2, addr_read_tdo, &data, rcv);
				// check to see if DCFEB is connected
				VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
				if (FixLength(VMEresult, 4, true)!="DCFE") continue;
				else {
					if (v_firmwareVersion[d].empty()) v_firmwareVersion[d] = firmwareVersion;
					v_nUCReads[d]++;
					nConnected++;
				}
				// Set instruction register to *Device select*
				crate_->vmeController()->vme_controller(3, addr_set_int_reg, &reg_dev_sel, rcv);
				// Set device register to "ADC mask"
				crate_->vmeController()->vme_controller(3, addr_shift_ht, &reg_dev_val, rcv);
				// Set IR to *Value select*
				crate_->vmeController()->vme_controller(3, addr_set_int_reg, &reg_val_sel, rcv);
				vector<string> tdi;
				vector<string> tdo;
				//unsigned int nMatches(0), n_misMatches(0);
				unsigned short int start(0x111), end(0xFFF);
				for (unsigned short int reg_val_shft = start; reg_val_shft<=end; reg_val_shft++) {
					if (reg_val_shft>start) v_nJTAGshifts[d]++;
					// Set DR, shift 12 bits
					crate_->vmeController()->vme_controller(3, addr_shift_dr, &reg_val_shft, rcv);
					usleep(10);
					tdi.push_back(FixLength(reg_val_shft, 3, true));
					// Read TDO
					crate_->vmeController()->vme_controller(2, addr_read_tdo, &data, rcv);
					usleep(10);
					VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
					tdo.push_back(FixLength(VMEresult, 4, true));
					if (reg_val_shft == 0x111) continue;
					if (tdo[reg_val_shft-0x111].substr(0,3) == tdi[reg_val_shft-1-0x111]) v_nShiftReads[d]++;
				}
				//out << "  Shifted register C " << end-start << " times, ";
				//out << "read expected tdo " << nMatches << " times." << endl;
			} // Loop over all DCFEBs
			if (nConnected==0) out << "Error: could not find DCFEBs. Please check connections." << endl;
      	} // repeat the test repNum times
      	// Now loop over DCFEBs again to display summary
      	for (int d = 0; d < 7; d++){ 
      		out << "DCFEB " << d+1 << ": ";
      		if (v_nUCReads[d]==0) out << "not connected." << endl;
      		else {
      			out << "read UserCode " << v_nUCReads[d] << "/" << repeatNumber << " times.";
      			out << " Firmware version " << v_firmwareVersion[d] << ".";
      			out << " Successful reads/shifts = " << v_nShiftReads[d] << "/" 
      				<< v_nJTAGshifts[d] << "." << endl;
      			//out << "  Shifted register C " << v_nJTAGshifts[d] << " times, ";
				//out << "read expected tdo " << v_nShiftReads[d] << " times." << endl;
			}
      	}
    }

    /**************************************************************************
     * ResetRegisters
     *
     * A small class to implement a reset from the ODMB_CTRL bits --TD
     **************************************************************************/
    ResetRegisters::ResetRegisters(Crate * crate) 
      : ButtonAction(crate,"Reset ODMB registers/FIFOs") 
    { /* The choices here are really a blank constructor vs duplicating the ExecuteVMEDSL constructor.
	 I've tried the former -- TD
      */
    }
    
    void ResetRegisters::respond(xgi::Input * in, ostringstream & out) { // TD
      // ButtonAction::respond(in, out);
      out << "********** VME REGISTER RESET **********" << endl;
      bool debug = false;
      int slot = Manager::getSlotNumber();
      unsigned int shiftedSlot = slot << 19;
      char rcv[2];
      // These are the appropriate R/W addresses for register reset
      unsigned int read_addr = 0x003000;
      unsigned int write_addr = 0x003000;
      // Set the top bits of address to the slot number
      read_addr = (read_addr & 0x07ffff) | shiftedSlot;
      write_addr = (write_addr & 0x07ffff) | shiftedSlot;
      unsigned short int reset_command = 0x100;
      unsigned short int data;
      if (debug) out << "data initialized to " << hex << data << endl;
      
      // Read = 2
      // Write = 3
      
      if (debug) printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x})  \n", 2, 
			(read_addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));      
      crate_->vmeController()->vme_controller(2,read_addr,&data,rcv);
      unsigned short int VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
      if (debug) {
	out << "read: " << FixLength(VMEresult) << endl;
	printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x})  \n", 3, 
	       (write_addr & 0xffffff), (reset_command & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      }
      crate_->vmeController()->vme_controller(3,write_addr,&reset_command,rcv);
      usleep(100);      
      if (debug) printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x})  \n", 3, 
			(write_addr & 0xffffff), (VMEresult & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      crate_->vmeController()->vme_controller(3,write_addr,&VMEresult,rcv);
      usleep(100);
      if (debug) printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x})  \n", 2, 
			(read_addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));            
      crate_->vmeController()->vme_controller(2,read_addr,&data,rcv);
      VMEresult = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);

      out << "R  " << FixLength(read_addr & 0xffff) << "        " << FixLength(VMEresult)  << endl<< endl;      
      
    }

    /**************************************************************************
     * ResetRegisters
     *
     * A small class to implement a reset from the ODMB_CTRL bits --TD
     **************************************************************************/
    ReprogramDCFEB::ReprogramDCFEB(Crate * crate) 
      : ButtonAction(crate,"Reprogram DCFEBs") 
    { /* The choices here are really a blank constructor vs duplicating the ExecuteVMEDSL constructor.
	 I've tried the former -- TD
      */
    }
    
    void ReprogramDCFEB::respond(xgi::Input * in, ostringstream & out) { // TD
      out << "********** VME REGISTER RESET **********" << endl;
      int slot = Manager::getSlotNumber();
      unsigned int shiftedSlot = slot << 19;
      char rcv[2];
      // These are the appropriate R/W addresses for register reset
      unsigned int write_addr = 0x003010;
      // Set the top bits of address to the slot number
      write_addr = (write_addr & 0x07ffff) | shiftedSlot;
      unsigned short int reset_command = 0x1;
      
      crate_->vmeController()->vme_controller(3,write_addr,&reset_command,rcv);
      usleep(100);      

      out << "W   3010      1          Reprogram all DCFEBs" << endl;      
      
    }
    


    /**************************************************************************
     * Buck Shift Test
     *
     *************************************************************************/

    BuckShiftTest::BuckShiftTest(Crate * crate)
      : Action(crate) {}

    void BuckShiftTest::display(xgi::Output * out)
    {
      addButton(out, "Buck Shift Test");
    }

    void BuckShiftTest::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>BuckShiftTest"<<endl; 
      out << "=== Buck Shift Test ===" << endl;

      //crate_->vmeController()->SetPrintVMECommands(1); // turn on debug printouts of VME commands

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	int val = (*dmb)->buck_shift_test();
	cout<<"Buck Shift Test returns: "<<val<<endl;
      }

      //crate_->vmeController()->SetPrintVMECommands(0); // turn off debug printouts of VME commands
    }

    /**************************************************************************
     * Buck Shift Test Debug
     *
     *************************************************************************/

    BuckShiftTestDebug::BuckShiftTestDebug(Crate * crate, emu::odmbdev::Manager* manager)
      : Action(crate,manager),
        ActionValue<int>(2) {}

    void BuckShiftTestDebug::display(xgi::Output * out)
    {

      addButtonWithTextBox(out,
                           "Buck Shift Test Debug",
                           "extrabits",
                           numberToString(value()));   


    }




    void BuckShiftTestDebug::respond(xgi::Input * in, ostringstream & out)
    {
      /*need to define
	#define VTX_USR1  0x02
	#define VTX_USR2  0x03
	#define CHIP_MASK 10
	#define CHIP_SHFT 11
	#define VTX_BYPASS 0x1F
	#define VTX6_ISC_NOOP 0x3D4
      */      
      unsigned int vtx_usr1 = 0x02;
      unsigned int vtx_usr2 =  0x03;
      unsigned int chip_mask = 10;
      unsigned int chip_shft = 11;
      unsigned int vtx_bypass = 0x1F;
      unsigned int vtx6_isc_noop = 0x3D4;
      //char rcvbuf[];

      cout<<"==>BuckShiftTestDebug"<<endl;
      out << "=== Buck Shift Test Debug===" << endl;

      //crate_->vmeController()->SetPrintVMECommands(1); // turn on debug printouts of VME commands

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
        int i,j,nmtch;
        int nchips2;
        int xtrabits = getFormValueInt("extrabits", in);

        int swtchbits = -1;
        unsigned int pat[42],chk[42] = {0xBA,0xDF,0xEE,0xD5,0xDE,0xAD};
        
        char rcvbuf[8192];
        char sndbuf[8192];
        char cmd[8192];
 
        //int pass=0;
        int boffset=xtrabits+swtchbits;
        vector<CFEB> cfebs = (*dmb)->cfebs();
        for(CFEBItr cfebItr = cfebs.begin(); cfebItr != cfebs.end(); ++cfebItr) {

          DEVTYPE dv = cfebItr->scamDevice();
          //int brdn=cfebItr->number();

          cmd[0]=vtx_usr1;
          sndbuf[0]=chip_mask;
	  crate_->vmeController()->devdo(dv,5,cmd,8,sndbuf,rcvbuf,0);
          cmd[0]=vtx_usr2;
          char chip_mask = cfebItr->chipMask();
          crate_->vmeController()->devdo(dv,5,cmd,6,&chip_mask,rcvbuf,0);
          nchips2=cfebItr->buckeyeChips().size();
          printf(" nchips2 %d chip_mask %04x \n",nchips2,chip_mask);
          for(i=0;i<6*nchips2;i++)pat[i]=0;
          cmd[0]=vtx_usr1;
          sndbuf[0]=chip_shft;
          crate_->vmeController()->devdo(dv,5,cmd,8,sndbuf,rcvbuf,2);
          cmd[0]=vtx_usr2;
          sndbuf[0]=0xAD;
          sndbuf[1]=0xDE;
          sndbuf[2]=0xD5;
          sndbuf[3]=0xEE;
          sndbuf[4]=0xDF;
          sndbuf[5]=0xBA;
          for(i=6;i<6*(nchips2+1)+1;i++)sndbuf[i]=0x00;
          crate_->vmeController()->devdo(dv,5,cmd,6*(nchips2+1)*8+xtrabits,sndbuf,rcvbuf,1);
          j=0;
	  //   for(i=6*(nchips2+1);i>=6*nchips2;i--){
          for(i=6*(nchips2+1);i>=0;i--){
            printf("shfttst %d %02X \n",i,rcvbuf[i]&0xFF);
            pat[j]=pat[j]|(rcvbuf[i]<<(8-boffset));
            pat[j]=pat[j]|((rcvbuf[i-1]>>boffset)&~(~0<<(8-boffset)));
            j++;
          }
          nmtch=0;
          for(i=0;i<6;i++){
	    // (*MyOutput_)<< hex <<pat[i] << dec;
            printf("%02x",pat[i]&0xff);
            if(pat[i]==chk[i])nmtch++;
          }
          printf("\n");
          // (*MyOutput_)<< endl;
	  /* if(nmtch==6){
	     pass=1;
	     (*MyOutput_)<<"Pattern returned is OK\n";
	     } else {
	     pass=0;
	     (*MyOutput_)<<"Pattern returned is Wrong\n";
	     }*/
          cmd[0]=vtx_usr1;
          sndbuf[0]=vtx6_isc_noop;//NOOP;
          crate_->vmeController()->devdo(dv,5,cmd,8,sndbuf,rcvbuf,0);
          cmd[0]=vtx_bypass;
          crate_->vmeController()->devdo(dv,5,cmd,0,sndbuf,rcvbuf,2);
	}

  
      }

      //crate_->vmeController()->SetPrintVMECommands(0); // turn off debug printouts of VME commands
    }
    



    /**************************************************************************
     * Enable VME debug printouts
     *
     *************************************************************************/

    enableVmeDebugPrintout::enableVmeDebugPrintout(Crate * crate)
      : Action(crate), ActionValue<int>(0) {}

    void enableVmeDebugPrintout::display(xgi::Output * out)
    {
      addButtonWithTextBox(out, 
			   "Enable VME Debug Printouts",
			   "enable",
			   numberToString(value()));
    }
    
    void enableVmeDebugPrintout::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>enableVmeDebugPrintout"<<endl; 
      bool enable = getFormValueInt("enable", in);
      value(int(enable)); // save the value
      crate_->vmeController()->SetPrintVMECommands(enable); // turn on/off debug printouts of VME commands
    }


    /**************************************************************************
     * DCFEB debug dump
     *
     *************************************************************************/

    dcfebDebugDump::dcfebDebugDump(Crate * crate)
      : Action(crate) {}

    void dcfebDebugDump::display(xgi::Output * out)
    {
      addButton(out, "DCFEB Debug Dump");
    }

    void dcfebDebugDump::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>dcfebDebugDump"<<endl; 
      out << "DCFEB Debug Dump send to stdout" << endl;

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb)
	{
	  vector <CFEB> cfebs = (*dmb)->cfebs();
	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb)	
	    {
	      int cfeb_index = (*cfeb).number();
	      
	      cout << " ********************* " << endl
		   << " FEB" << cfeb_index << " : " << endl;
	      (*dmb)->dcfeb_readreg_statusvirtex6( *cfeb );
	      (*dmb)->dcfeb_readreg_cor0virtex6( *cfeb );
	      (*dmb)->dcfeb_readreg_cor1virtex6( *cfeb ); 
	    }
	}
    }


    /**************************************************************************
     * ODMB_OTMB_LCT_Testing
     * -- S.Z. Shalhout Jul 17, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    ODMB_OTMB_LCT_Testing::ODMB_OTMB_LCT_Testing(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
        ActionValue<int>(1000) {}
     
    void ODMB_OTMB_LCT_Testing::display(xgi::Output * out)  
    {
      addButtonWithTextBox(out,
                           "Generate_CCB_L1As_Custom",
                           "requested",
                           numberToString(value()));
    }


    void ODMB_OTMB_LCT_Testing::respond(xgi::Input * in, ostringstream & out)
    {

      int _requested = getFormValueInt("requested", in);      





      ccb_->setCCBMode(CCB::VMEFPGA);
      ccb_->EnableL1aFromVme();
      ccb_->l1aReset();
      usleep(1000);
      ccb_->bc0();
      usleep(1000);

      for ( uint64_t indx = 1; indx <= (uint64_t)_requested; ++indx )
        {
        
      
          //////////

	  ccb_->GenerateL1A();// generate L1A and pretriggers
	  usleep(1000);
	  //////////



	}


     


    }

    /**************************************************************************
     * ODMB_L1A_Testing
     * -- W. Wulsin (w.wulsin@cern.ch) & S.Z. Shalhout Jul 16, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    ODMB_L1A_Testing::ODMB_L1A_Testing(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
	ActionValue<int>(1000) {}
    
    void ODMB_L1A_Testing::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "ODMB generated L1A+MATCH",
			   "requested",
                           numberToString(value()));
    }


    void ODMB_L1A_Testing::respond(xgi::Input * in, ostringstream & out)
    {

      int _requested = getFormValueInt("requested", in);      


      // the arguments for vme_controller // 
      char rcv[2]; 
      unsigned int addr; 
      unsigned short int data; 
      int irdwr; 


      // Reading from file: /home/cscme11/TriDAS/emu/odmbdev/commands/test_realdata_all7.txt
      // Calling this line:  W  3010 1       Reprogram DCFEB FIFO
      // Calling:   vme_controller(3,783010,&0001,{b8,1f})  
      irdwr = 3; addr = 0x783010; data = 0x0001; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", 
	     irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      usleep(2000000);  
      // Calling this line:  W  3000 100     Reset ODMB Registers
      // Calling:   vme_controller(3,783000,&0100,{b8,1f})  
      irdwr = 3; addr = 0x783000; data = 0x0100; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      usleep(10000);  
      // Calling this line:  W  3000  300                Reset
      // Calling:   vme_controller(3,783000,&0300,{b8,1f})  
      irdwr = 3; addr = 0x783000; data = 0x0300; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      usleep(1000);  
      // Calling this line:  W  20   18       Select FIFO 4 [DDU data]
      // Calling:   vme_controller(3,780020,&0018,{b8,1f})  
      irdwr = 3; addr = 0x780020; data = 0x0018; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      // Calling this line:  W  3000  200                Set real data and internal triggers
      // Calling:   vme_controller(3,783000,&0200,{b8,1f})  
      irdwr = 3; addr = 0x783000; data = 0x0200; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      // Calling this line:  R  441C  0                          Read KILL
      // Calling:   vme_controller(2,78441c,&0000,{b8,1f})    ==> rcv[1,0] = 01 ff
      irdwr = 2; addr = 0x78441C; data = 0x0000; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
      // Calling this line:  W  401C  0                            Set KILL
      // Calling:   vme_controller(3,78401c,&0000,{ff,01})  
      irdwr = 3; addr = 0x78401C; data = 0x0000; 
      printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff),(rcv[1] & 0xff)); 
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 

      /// start set up DCFEBS + DDU
      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu)
        {

          (*ddu)->writeFlashKillFiber(0x7fff);
          usleep(20);
          ccb_->HardReset_crate();
          usleep(250000);
          (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
          usleep(10);
          (*ddu)->writeFakeL1( 0x8787 ); // 0x8787: passthrough // 0x0000: normal
          usleep(10);
          ccb_->l1aReset();
          usleep(50000);
          usleep(50000);
          ccb_->bc0();

        }

      // set comparator threshold
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
        (*dmb)->set_comp_thresh(0.03);
      }
      usleep(1000);


      //  set the pipeline depth
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
        vector <CFEB> cfebs = (*dmb)->cfebs();
        for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
          int depth = cfeb->GetPipelineDepth(); // get value that was read in from the crate config xml (unless it was changed later)
          //int delay = cfeb->GetFineDelay();  // get value that was read in from the crate config xml (unless it was changed later)
          (*dmb)->dcfeb_set_PipelineDepth(*cfeb, depth);
          usleep(100);
	  //          (*dmb)->dcfeb_fine_delay(*cfeb, delay); // careful, I this may depend on the order in the xml
	  //          usleep(100);
          (*dmb)->Pipeline_Restart( *cfeb );
          usleep(100);
        }
      }

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector <CFEB> cfebs = (*dmb)->cfebs();
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){

	  //                                (*dmb)->dcfeb_Set_ReadAnyL1a(*cfeb);
	  usleep(100);
	}
      }

      ccb_->l1aReset();
      usleep(1000);
      ccb_->bc0();
      usleep(1000);


      // shift buckeyes into normal mode
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){

        (*dmb)->set_comp_mode(0xa); // added by J. Pilot, 5/23, for testing.
        //usleep(1);
        //(*dmb)->restoreCFEBIdle();
        //ccb_->l1aReset();
        usleep(1);
        (*dmb)->shift_all(NORM_RUN);
        (*dmb)->buck_shift();
        usleep(100);
      }




      /// end set up DCFEBS+DDU


      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ODMB_generated_L1A_L1A_MATCH") );
      sleep(5);




      for ( uint64_t indx = 1; indx <= (uint64_t)_requested; ++indx )
	{
	  
            
	  //////////
	  // VME COMMANDS HERE



	  // Calling this line:  W  3010  10                         Send test L1A(_MATCH) to all DCFEBs
	  // Calling:   vme_controller(3,783010,&0010,{ff,01})  
	  irdwr = 3; addr = 0x783010; data = 0x0010; 
	  printf("Calling l1a/match:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), 
		 (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  usleep(1000);  



	  // Calling this line:  R  33FC                      0   Read L1A_COUNTER
	  // Calling:   vme_controller(2,7833fc,&0000,{ff,01})    ==> rcv[1,0] = 00 01
	  //	   irdwr = 2; addr = 0x7833FC; data = 0x0000; 
	  //	   printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), 
	  //(rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  //	   crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 

	  // // Calling this line:  R  327C  0                            Read L1A_MATCH_CNT(7)
	  // // Calling:   vme_controller(2,78327c,&0000,{01,00})    ==> rcv[1,0] = 00 01
	  //	   irdwr = 2; addr = 0x78327C; data = 0x0000; 
	  //	   printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  //	   crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 

	  // // Calling this line:  R  337C  0                            Read LCT_L1A_GAP(7)
	  // // Calling:   vme_controller(2,78337c,&0000,{01,00})    ==> rcv[1,0] = 00 63
	  // irdwr = 2; addr = 0x78337C; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  347C  0                            Number of received packets [DCFEB 7]
	  // // Calling:   vme_controller(2,78347c,&0000,{63,00})    ==> rcv[1,0] = 00 01
	  // irdwr = 2; addr = 0x78347C; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  W  5010 7                             Select DCFEB FIFO 7
	  // // Calling:   vme_controller(3,785010,&0007,{01,00})  
	  // irdwr = 3; addr = 0x785010; data = 0x0007; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5014 0                           Read which DCFEB FIFO is selected
	  // // Calling:   vme_controller(2,785014,&0000,{01,00})    ==> rcv[1,0] = 00 07
	  // irdwr = 2; addr = 0x785014; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  500C 0                             Read word count of DCFEB FIFO
	  // // Calling:   vme_controller(2,78500c,&0000,{07,00})    ==> rcv[1,0] = 03 22
	  // irdwr = 2; addr = 0x78500C; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5000 0                             Read DCFEB FIFO
	  // // Calling:   vme_controller(2,785000,&0000,{22,03})    ==> rcv[1,0] = 00 01
	  // irdwr = 2; addr = 0x785000; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5000 0                             Read DCFEB FIFO
	  // // Calling:   vme_controller(2,785000,&0000,{01,00})    ==> rcv[1,0] = 00 01
	  // irdwr = 2; addr = 0x785000; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5000 0                             Read DCFEB FIFO
	  // // Calling:   vme_controller(2,785000,&0000,{01,00})    ==> rcv[1,0] = 44 03
	  // irdwr = 2; addr = 0x785000; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5000 0                             Read DCFEB FIFO
	  // // Calling:   vme_controller(2,785000,&0000,{03,44})    ==> rcv[1,0] = 44 33
	  // irdwr = 2; addr = 0x785000; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  R  5000 0                             Read DCFEB FIFO
	  // // Calling:   vme_controller(2,785000,&0000,{33,44})    ==> rcv[1,0] = 44 13
	  // irdwr = 2; addr = 0x785000; data = 0x0000; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 
	  // // Calling this line:  W  5020 7f      Reset DCFEB FIFOs
	  // // Calling:   vme_controller(3,785020,&007f,{13,44})  
	  // irdwr = 3; addr = 0x785020; data = 0x007F; 
	  // printf("Calling:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff)); 
	  // crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv); 

	  
	  //////////

	  usleep(100);

	}

      // stop DAQ 
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();
      cout<<" Done "<<endl;


    }
    



   
    /**************************************************************************
     * Investigate_MissingEvents
     * -- S.Z. Shalhout May 22, 2013 (sshalhou@cern.ch)
     *************************************************************************/


    Investigate_MissingEvents::Investigate_MissingEvents(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
	ActionValue<int>(1000) {}
    
    void Investigate_MissingEvents::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "Investigate Missing Events",
			   "requested",
                           numberToString(value()));
    }


    void Investigate_MissingEvents::respond(xgi::Input * in, ostringstream & out)
    {

      char rcv[2];
      unsigned int addr;
      unsigned short int data;
      int irdwr;


      /////////////////
      // investigation of missing event counts
    
      int _requested = getFormValueInt("requested", in);      
      int _counted = 0;
      int _l1delay = 137;
      int _alct_send_empty = 1;
      int _alct_trig_mode = 0;
      int _all_cfeb_active  = 1;
      int _clct_pretrig_enable = 0;
      int _clct_readout_without_trig = 0;
      int _clct_trig_enable = 1;
      int _match_trig_enable = 0;
      int _request_l1a = 1;


      ///////////////////
      // crate config parameters
      // pulled in from xml files
      // for testing only

  
      ccb_->setCCBMode(CCB::VMEFPGA);
      ccb_->SetL1aDelay(_l1delay);
      alct_->SetSendEmpty(_alct_send_empty);
      alct_->SetTriggerMode(_alct_trig_mode);
      tmb_->SetEnableAllCfebsActive(_all_cfeb_active);
      tmb_->SetClctPatternTrigEnable(_clct_pretrig_enable); // actually named pretrig in xml
      tmb_->SetAllowClctNontrigReadout(_clct_readout_without_trig);
      tmb_->SetTmbAllowClct(_clct_trig_enable);
      tmb_->SetTmbAllowMatch(_match_trig_enable);
      tmb_->SetRequestL1a(_request_l1a);


      ccb_->EnableL1aFromVme();
      //      ccb_->EnableL1aFromTmbL1aReq();

      ccb_->SetExtTrigDelay( 0 );
      tmb_->EnableClctExtTrig();
      ccb_->RedirectOutput( &cout );


      for(unsigned long int numReg = 0; numReg<  tmb_->TMBConfigurationRegister.size(); numReg++)
	{
	  unsigned long int x_address = tmb_->TMBConfigurationRegister.at(numReg);
	  tmb_->WriteRegister(x_address);
	}

 
      //    alct_->SetL1aOffset(0); // set Write values...
      alct_->configure();


      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu)
        {

          (*ddu)->writeFlashKillFiber(0x7fff);
          usleep(20);
          ccb_->HardReset_crate();
          usleep(250000);
          (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
          usleep(10);
          (*ddu)->writeFakeL1( 0x8787 ); // 0x8787: passthrough // 0x0000: normal
          usleep(10);
          ccb_->l1aReset();
          usleep(50000);
          usleep(50000); 
          ccb_->bc0();
       
        } 



      /// start set up DCFEBS 

      // set comparator threshold 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_comp_thresh(0.03);
      }
      usleep(1000);
      
      
      //  set the pipeline depth
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector <CFEB> cfebs = (*dmb)->cfebs();
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  int depth = cfeb->GetPipelineDepth(); // get value that was read in from the crate config xml (unless it was changed later)
          //int delay = cfeb->GetFineDelay();  // get value that was read in from the crate config xml (unless it was changed later)
	  (*dmb)->dcfeb_set_PipelineDepth(*cfeb, depth);
	  usleep(100);
	  //          (*dmb)->dcfeb_fine_delay(*cfeb, delay); // careful, I this may depend on the order in the xml
	  //          usleep(100);
	  (*dmb)->Pipeline_Restart( *cfeb );
	  usleep(100);
	}
      }


      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector <CFEB> cfebs = (*dmb)->cfebs();
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){

	  (*dmb)->dcfeb_Set_ReadAnyL1a(*cfeb);
	  usleep(100);
	}
      }



      ccb_->l1aReset();
      usleep(1000);
      ccb_->bc0();
      usleep(1000);


     
      // shift buckeyes into normal mode
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	
	(*dmb)->set_comp_mode(0xa); // added by J. Pilot, 5/23, for testing.
	//usleep(1);
	//(*dmb)->restoreCFEBIdle();
	//ccb_->l1aReset();
	usleep(1);
        (*dmb)->shift_all(NORM_RUN);
	(*dmb)->buck_shift();
	usleep(100);
      }




      /// end set up DCFEBS







	
      ccb_->WriteRegister( emu::pc::CCB::enableL1aCounter, 0 );
      ccb_->WriteRegister( emu::pc::CCB::resetL1aCounter , 0 ); // zero L1A counter

	

      ///////////////////
   


      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ME11Test_InvestigateMissingEvents") );
      sleep(10);






      int NWORDS = 802;



      for ( uint64_t indx = 1; indx <= (uint64_t)_requested; ++indx )
	{





	  // reset odmb fifos   W  3000  100    Reset     (3,783000,&0100,{18,fe})
	  // use 300 for internal triggers, 100 for CCB triggers

	  irdwr = 3;  // 2 = read; 3 = write
	  addr = 0x783000;    
	  data = 0x0300;  // irrelevant for reads
	  rcv[0] = '0';  
	  rcv[1] = '0';  

	  printf("Calling reset odmb fifos:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
		 (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);

	  usleep(10000);

	  /*
	  // Set real data and CCB triggers (W 3000 0)
	  irdwr = 3;  // 2 = read; 3 = write
	  addr = 0x783000;
	  data = 0x0000;
	  rcv[0] = '0';
	  rcv[1] = '0';

	  printf("Calling set real data & internal trig:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr,
	  (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
	  */

	  // Set Kill TMB, ALCT, keep all 7 DCFEBs on
	  // W  401C  180 
	  irdwr = 3;  // 2 = read; 3 = write
	  addr = 0x78401C;
	  data = 0x0180;
	  rcv[0] = '0';
	  rcv[1] = '0';

	  printf("Calling kill tmb+alct:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr,
		 (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);



	  // R   4424            3    Firmware Version Read
	  // vme_controller(2,784424,&0000,{18,08})
	  irdwr = 2;  // 2 = read; 3 = write
	  addr = 0x784424;
	  data = 0x0000;  // irrelevant for reads
	  rcv[0] = '0';
	  rcv[1] = '0';
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
	  printf("Reading Firmware Version:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr,
		 (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));


	  std::cout<<" kill done A"<<std::endl;
	  std::cout<<" kill done A"<<std::endl;
	  std::cout<<" kill done A"<<std::endl;


	  usleep(100);
	  ccb_->GenerateL1A();// generate L1A and pretriggers



	  // Sent an internally generated L1A/L1A_MATCH to all DCFEBs
	  // W  3010  10 	
	  irdwr = 3;
	  addr = 0x783010;  
	  data=0x0010; 
	  //           crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
	  printf("Internally generate L1A/L1A_MATCH:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
		 (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));

	  usleep(10000);

	  std::cout<<" kill done B"<<std::endl;
	  std::cout<<" kill done B"<<std::endl;
	  std::cout<<" kill done B"<<std::endl;


	  //////////

	  // R   33FC            0    Read L1A_COUNTER   (2,7833fc,&0000,{18,fe}) 
	  irdwr = 2;
	  addr = 0x7833fc;  
	  crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
	  printf("Reading L1A counter:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
		 (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));



	  std::cout<<" kill done C"<<std::endl;
	  std::cout<<" kill done C"<<std::endl;
	  std::cout<<" kill done C"<<std::endl;


	  for (unsigned short int idcfeb = 1; idcfeb <= 5; ++idcfeb ) {
            
            //W  5010 1 Select DCFEB FIFO   (3,785010,&0001,{00,00})
            data = idcfeb; 
            addr = 0x785010;  
            irdwr = 3;
            crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
            printf("Called select dcfeb fifo:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
                   (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));

	    //R  5014 0 Read which DCFEB FIFO is selected (2,785014,&0000,{00,00})
            data = 0;  // irrelevant for reads
            irdwr = 2; 
            addr = 0x785014;    
            crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
            printf("Called read which dcfeb fifo selected:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
                   (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));

            //R  500C 0 Read word count of DCFEB FIFO     (2,78500c,&0000,{07,00})
            addr = 0x78500c;    
            crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
            printf("Called read word count of dcfeb fifo:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
                   (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
   
            for ( int iword = 1; iword <= NWORDS; ++iword ) {
              

              //   R  5000 0    Read DCFEB FIFO (2,785000,&0000,{00,00})
              addr = 0x785000;  
              crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);


	      if(iword<10){
		printf("Called read dcfeb fifo:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ", irdwr, 
		       (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));

	      } // iword < 10


            } //           for ( int iword = 1; iword <= NWORDS; ++iword ) {
            
	  } //       for ( int idcfeb = 1; idcfeb <= 5; ++idcfeb ) {


	  //////////

	  usleep(100);
	  _counted++;

	}
      ///////////////////
      std::cout<<" Requested "<< _requested<<" L1A's issued "<<_counted<<std::endl;
      ////////////////////       


      // stop DAQ 
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();
      cout<<" Done "<<endl;

      uint32_t l1a_counter_LSB = ccb_->ReadRegister( emu::pc::CCB::readL1aCounterLSB ) & 0xffff; // read lower 16 bits
      uint32_t l1a_counter_MSB = ccb_->ReadRegister( emu::pc::CCB::readL1aCounterMSB ) & 0xffff; // read higher 16 bits
      uint32_t l1a_counter     = l1a_counter_LSB | (l1a_counter_MSB << 16); // merge into counter

      std::cout<<" ccb counter "<<l1a_counter<<std::endl;

    }
    



    /**************************************************************************
     * CommonUtilities_setupDDU_passThrough
     * -- S.Z. Shalhout June 26, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    CommonUtilities_setupDDU_passThrough::CommonUtilities_setupDDU_passThrough(Crate * crate)
      : Action(crate) {}
 
    void CommonUtilities_setupDDU_passThrough::display(xgi::Output * out)
    {
      addButton(out, "SetUp DDU PassThrough","width: 250px; ");
    } 

    void CommonUtilities_setupDDU_passThrough::respond(xgi::Input * in, ostringstream & out)
    {

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu)
	{
	
	  (*ddu)->writeFlashKillFiber(0x7fff); 
	  usleep(20);
	  //ccb_->HardReset_crate();
	  //usleep(250000);
	  (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
	  usleep(10);
	  (*ddu)->writeFakeL1( 0x8787 ); // 0x8787: passthrough // 0x0000: normal
	  usleep(10);
	  //ccb_->l1aReset();
	  //usleep(50000);
	  //usleep(50000);
	  //ccb_->bc0();


	} 

    }



    /**************************************************************************
     * CommonUtilities_setupDDU
     * -- S.Z. Shalhout April 26, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    CommonUtilities_setupDDU::CommonUtilities_setupDDU(Crate * crate)
      : Action(crate) {}
 
    void CommonUtilities_setupDDU::display(xgi::Output * out)
    {
      addButton(out, "SetUp DDU ","width: 250px; ");
    } 

    void CommonUtilities_setupDDU::respond(xgi::Input * in, ostringstream & out)
    {

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu)
	{
	
	  (*ddu)->writeFlashKillFiber(0x7fff); 
	  usleep(20);
	  ccb_->HardReset_crate();
	  usleep(250000);
	  (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
	  usleep(10);
	  (*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
	  usleep(10);
	  ccb_->l1aReset();
	  usleep(50000);
	  usleep(50000);
	  ccb_->bc0();


	} 

    }


  
    /**************************************************************************
     * CommonUtilities_restoreCFEBIdle
     * -- S.Z. Shalhout April 26, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    CommonUtilities_restoreCFEBIdle::CommonUtilities_restoreCFEBIdle(Crate * crate)
      : Action(crate) {}
 
    void CommonUtilities_restoreCFEBIdle::display(xgi::Output * out)
    {
      addButton(out, "Restore CFEBS to IDLE","width: 100%; ");
    } 

    void CommonUtilities_restoreCFEBIdle::respond(xgi::Input * in, ostringstream & out)
    {

      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }

    }


    /**************************************************************************
     * RoutineTest_ShortCosmicsRun
     * -- S.Z. Shalhout April 4, 2013 (sshalhou@cern.ch)
     *************************************************************************/

    RoutineTest_ShortCosmicsRun::RoutineTest_ShortCosmicsRun(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ),
	ActionValue<int>(10) {}
    
    void RoutineTest_ShortCosmicsRun::display(xgi::Output * out)
    {
      addButtonWithTextBox(out,
			   "Routine Test - Short Cosmics Run (HV should be ON)",
			   "time",
                           numberToString(value()));
    }
    
    void RoutineTest_ShortCosmicsRun::respond(xgi::Input * in, ostringstream & out)
    {
      
      // Assuming that you have HV on, and have already written FLASH to crate with
      // the needed xml parameters ...
      
      // equivalent to EmuPeripheralCrateConfig::PrepareForTriggering
      
      int _time = getFormValueInt("time", in);      
      
      ccb_->setCCBMode(CCB::VMEFPGA);
      ccb_->hardReset();
      ccb_->EnableL1aFromTmbL1aReq();
      usleep(1000);

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
	(*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
      	usleep(10);
	(*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
      	usleep(10);
      }




      // set comparator threshold 
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_comp_thresh(0.03);
      }
      usleep(1000);
      
      
      //  set the pipeline depth
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector <CFEB> cfebs = (*dmb)->cfebs();
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  int depth = cfeb->GetPipelineDepth(); // get value that was read in from the crate config xml (unless it was changed later)
          int delay = cfeb->GetFineDelay();  // get value that was read in from the crate config xml (unless it was changed later)
	  (*dmb)->dcfeb_set_PipelineDepth(*cfeb, depth);
	  usleep(100);
          (*dmb)->dcfeb_fine_delay(*cfeb, delay); // careful, I this may depend on the order in the xml
          usleep(100);
	  (*dmb)->Pipeline_Restart( *cfeb );
	  usleep(100);

          (*dmb)->dcfeb_Set_ReadAnyL1a(*cfeb);
          usleep(100);

	}
      }





      ccb_->l1aReset();
      usleep(1000);
     
      // shift buckeyes into normal mode
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	
	(*dmb)->set_comp_mode(0xa); // added by J. Pilot, 5/23, for testing.
	//usleep(1);
	//(*dmb)->restoreCFEBIdle();
	//ccb_->l1aReset();
	usleep(1);
        (*dmb)->shift_all(NORM_RUN);
	(*dmb)->buck_shift();
	usleep(100);
      }
      //Indara Needs for ALCT-only cosmic run//
      /*   tmb_->WriteRegister(0x4a,0);
	   tmb_->WriteRegister(0x4c,0);
	   tmb_->WriteRegister(0x4e,0);
	   tmb_->WriteRegister(0x50,0);
	   tmb_->WriteRegister(0x52,0);
	   tmb_->WriteRegister(0x54,0);
	   tmb_->WriteRegister(0x56,0);
	   tmb_->WriteRegister(0x58,0);
	   tmb_->WriteRegister(0x5a,0);
	   tmb_->WriteRegister(0x5c,0);
	   tmb_->WriteRegister(0x5e,0);
	   tmb_->WriteRegister(0x60,0);
	   tmb_->WriteRegister(0x62,0);
	   tmb_->WriteRegister(0x64,0);
	   tmb_->WriteRegister(0x66,0);
	   tmb_->WriteRegister(0x16e,0);
	   tmb_->WriteRegister(0x170,0);
	   tmb_->WriteRegister(0x172,0);
	   tmb_->WriteRegister(0x174,0);
	   tmb_->WriteRegister(0x176,0);
	   tmb_->WriteRegister(0x178,0);      */
  
      ccb_->l1aReset();
      usleep(1000);
      ccb_->bc0();
      usleep(1000);
      



      /*

      ////////////////////////////// TEST LA1_LCT_DELAY OF 26
      // this VME command is:   vme_controller(3,784000,&001a,{a8,18}) 

      // the arguments for vme_controller // 
      char rcv[2]; 
      unsigned int addr; 
      unsigned short int data; 
      int irdwr; 



      irdwr = 3; addr = 0x784000; data = 0x001a;
      printf("Calling Set L1A_LCT_DELAY To 26:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ",
      irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
      usleep(100000);


      irdwr = 3; addr = 0x783000; data = 0x0000;
      printf("Calling Real Data + CCB Triggers:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ",
      irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
      usleep(100000);



      irdwr = 2; addr = 0x78441C; data = 0x0000;
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);
      printf("Calling Read KILL:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ",
      irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      
  
      usleep(100000);


      irdwr = 3; addr = 0x78401c; data = 0x0180; //data = 0x01FE;
      printf("Calling Set DCFEBS ALCT TMB ON:  vme_controller(%d,%06x,&%04x,{%02x,%02x}) \n ",
      irdwr, (addr & 0xffffff), (data & 0xffff), (rcv[0] & 0xff), (rcv[1] & 0xff));
      crate_->vmeController()->vme_controller(irdwr,addr,&data,rcv);

      usleep(100000);

      */


      /////////////////////////////// END TEST



      //////// TEST FIFO A



      //////// TEST FIFO A END


      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ME11Test_ShortCosmicsRun")+emu::utils::stringFrom<int>( _time )+string("s") );
      
      ::sleep(_time); // take data
      
      // stop DAQ 
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();
      cout<<" Done "<<endl;
    }
    
    
    /**************************************************************************
     * RoutineTest_PrecisionPulses
     *
     *************************************************************************/
  
    RoutineTest_PrecisionPulses::RoutineTest_PrecisionPulses(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ) {}
    void RoutineTest_PrecisionPulses::display(xgi::Output * out)
    {
      addButton(out,"Routine Test - Precision Pulses");
    }
    
    void RoutineTest_PrecisionPulses::respond(xgi::Input * in, ostringstream & out)
    {
      cout<<"==>RoutineTest_PrecisionPulses"<<endl; 

      const int N_DCFEBS = 5; // number of DCFEBs per DMB
      const int N_STRIPS = 16; // number of strips per DCFEB
      int n_pulses = 2; // this must be >1 because there is a "feature" that the DAQ doesn't record the first pulse after an l1aReset (except for the very first event)

      // set register 0 appropriately for communication over the VME backplane.
      ccb_->setCCBMode(CCB::VMEFPGA);
      ccb_->hardReset();
      usleep(250000);
      // Enable L1A and clct_pretrig from any of dmb_cfeb_calib signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      ccb_->SetExtTrigDelay( 19 ); // Delay of ALCT and CLCT external triggers before distribution to backplane


      usleep(1000);

      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
	(*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
      	usleep(10);
	(*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
      	usleep(100);
      }
      

      //// Set pulse height ////
      float PulseHeight = 0.5;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_dac(PulseHeight, PulseHeight);
      }
      usleep(1000);
      
      //// Set comparator thresholds ////
      float ComparatorThresholds = 0.01;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	(*dmb)->set_comp_thresh(ComparatorThresholds);
      }
      usleep(1000);
  
      //// Set the pipeline depth on all DCFEBs ////
      for(vector<DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	vector<CFEB> cfebs = (*dmb)->cfebs();
        out<<cfebs.size()<<endl; 
	for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  
	  int depth = cfeb->GetPipelineDepth(); // get value that was read in from the crate config xml (unless it was changed later)
	  //int depth = 44; // get value that was read in from the crate config xml (unless it was changed later)
	  (*dmb)->dcfeb_set_PipelineDepth( *cfeb, depth ); // set it on the hardware
	  usleep(1000);
	  (*dmb)->Pipeline_Restart( *cfeb ); // must restart pipeline after setting it
	  usleep(1000);
	}
      }
      //szs ccb_->l1aReset(); // needed after setting/restarting pipeline
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }
      usleep(10000);
      
      //// Still need settings to time in TMB data with L1a from pulse
      // I think this means tuning CCB l1aDelay and CCB external trigger delay, but this can come later
      

      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ME11Test_PrecisionPulses") );
      
      //// Pulse individual DCFEBs ////
      int strip_to_pulse = 0;
      int feb_to_pulse = -1;


      for(int dcfeb_to_pulse=0; dcfeb_to_pulse < N_DCFEBS; ++dcfeb_to_pulse){
	strip_to_pulse = 1 + 13*dcfeb_to_pulse/(N_DCFEBS-1); // just a cute way to move the pulse along with the DCFEB
	
	//szs	ccb_->l1aReset(); // stop triggering
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }
	//ccb_->stopTrigger();
      	usleep(10000);
	
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->set_ext_chanx(strip_to_pulse, dcfeb_to_pulse); // this only sets the array in software
	  (*dmb)->buck_shift(); // this shifts the array into the buckeyes
	  usleep(10000);
	}
	//// We are now configured to send pulses
	
	//szs	ccb_->l1aReset();
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }
	//ccb_->stopTrigger();
        usleep(10000);
	ccb_->bc0(); // start triggering
        usleep(10000);
	cout<<"npulses = "<<n_pulses<<endl;
	for(int p=0; p<n_pulses; ++p){
	  cout<<"pulsing dcfeb "<<dcfeb_to_pulse<<", strip "<<strip_to_pulse<<endl;
	  //ccb_->pulse(1,0); // send the pulses
	  ccb_->GenerateDmbCfebCalib0(); // send the pulses 
	  usleep(10000);
	}
      }	
      

      //// Loop over strips, pulsing all DCFEBs ("walking one")////
      feb_to_pulse = -1; // -1 means all DCFEBs
      for(strip_to_pulse=0; strip_to_pulse < N_STRIPS; ++strip_to_pulse){

	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }
	//	ccb_->l1aReset(); // stop triggering
      	usleep(10000);
	
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->set_ext_chanx(strip_to_pulse, feb_to_pulse); // this only sets the array in software
	  (*dmb)->buck_shift(); // this shifts the array into the buckeyes
	  usleep(1000);
	}
	//// We are now configured to send pulses

	//	ccb_->l1aReset();
	for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){ (*dmb)->restoreCFEBIdle(); }
        usleep(10000);
	ccb_->bc0(); // start triggering
        usleep(10000);

	for(int p=0; p<n_pulses; ++p){
	  cout<<"pulsing dcfeb "<<feb_to_pulse<<", strip "<<strip_to_pulse<<endl;
	  //ccb_->pulse(1,0); // send the pulses
	  ccb_->GenerateDmbCfebCalib0(); // send the pulses 
	  usleep(10000);
	}
      }	
      
      // stop DAQ
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();
    }

    /**************************************************************************
     * STEP9b:Fiber Tests Debug
     * I. Suarez
     *************************************************************************/
    STEP9bFibers::STEP9bFibers(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ) {}
    void STEP9bFibers::display(xgi::Output * out)
    {
      addButton(out,"STEP 9b Fiber Test");
    }
    void STEP9bFibers::respond(xgi::Input * in, ostringstream & out)
    {
      /*  // set register 0 appropriately for communication over the VME backplane.
	  ccb_->setCCBMode(CCB::VMEFPGA);
	  ccb_->hardReset();

	  //tmb_->SetRequestL1a(0);  //do not request l1a cause the pulse comes with one

	  for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
	  (*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
	  usleep(10);
	  (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
	  usleep(10);
	  }

	  // Enable L1A and clct_pretrig from any of dmb_cfeb_calib signals and disable all other trigger sources
	  // ccb_->EnableL1aFromDmbCfebCalibX();
	  // ccb_->SetExtTrigDelay( 16 ); // Delay of ALCT and CLCT external triggers before distribution to backplane
	  // usleep(100);

	  //// Set pulse height ////
	  float PulseHeight = 0.5;
	  float ComparatorThresholds = 0.1;
	  int NumberofPulses = 2;
	  int timebetweenpulse = 4000;
	  ccb_->l1aReset(); // needed after setting/restarting pipeline
	  usleep(100);
	  int DMBposition=0;

	  int dcfeb_pulsed = 0;
	  unsigned int dcfebtrig[5] = {0x01,0x02,0x04,0x08,0x10};
	  unsigned int tmb_CounterRegValue[7] = {0x1ce2,0x1ee2,0x20e2,0x22e2,0x24e2,0x26e2,0x28e2};
	  unsigned int tmb_CountersReadReg = 0xd0;
	  unsigned int tmb_CountersWriteReg = 0xd2;
	  int counter_DCFEB[7];
	  int sum_DCFEBCounters=0;

	  //TMB FW 240T/12042013 halfstripmaps; recall me11a is flipped(walking back)
	  int min_hp[7]= {0,32,64,96,223,191,159};
	  int max_hp[7]= {31,63,95,127,192,160,128};

	  int last_keyhalfstrip_pulsed = 0;
     
	  //  set the pipeline depth
	  for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  vector <CFEB> cfebs = (*dmb)->cfebs();
	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
	  int depth = cfeb->GetPipelineDepth(); // get value that was read in from the crate config xml (unless it was changed later)
	  int delay = cfeb->GetFineDelay();  // get value that was read in from the crate config xml (unless it was changed later)
	  (*dmb)->dcfeb_set_PipelineDepth(*cfeb, depth);
	  usleep(100);
	  (*dmb)->dcfeb_fine_delay(*cfeb, delay); // careful, I this may depend on the order in the xml
	  usleep(100);
	  (*dmb)->Pipeline_Restart( *cfeb );
	  usleep(100);
	  (*dmb)->dcfeb_Set_ReadAnyL1a(*cfeb);
	  usleep(100);
	  }
	  } 

	  for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
	  (*dmb)->set_dac(PulseHeight, PulseHeight);
	  (*dmb)->set_comp_thresh(ComparatorThresholds);
	  (*dmb)->settrgsrc(0);
	  DMBposition = int(dmb - dmbs_.begin());
	  if(DMBposition==0) dcfeb_pulsed=5;
	  if(DMBposition==1) dcfeb_pulsed=0;
	  vector<CFEB> cfebs = (*dmb)->cfebs();

	  for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); cfeb++){
	  for(int halfstrip = 2; halfstrip<30; ++halfstrip){
          int  hp[6] = {halfstrip+1, halfstrip, halfstrip+1, halfstrip, halfstrip+1, halfstrip};
          (*dmb)->trigsetx(hp, dcfebtrig[cfeb->number()]);

          ccb_->l1aReset(); // needed after setting/restarting pipeline
          usleep(1000);
          (*dmb)->restoreCFEBIdle(); // need to restore DCFEB JTAG after a buckshift
          usleep(1000);
          ccb_->bc0(); // start triggering
          usleep(1000);


          tmb_->ResetCounters();
          usleep(1000);
          //if(halfstrip==0)
          out<<dcfeb_pulsed+1<<"\"t<<NumberofPulses<<"\t"<<halfstrip<<"\t";
          ccb_->inject(NumberofPulses,timebetweenpulse); // send the pulses
          usleep(100000);

          last_keyhalfstrip_pulsed = int(((tmb_->ReadRegister(0x8a78) & 0xff00)>>8 ));
          out<<last_keyhalfstrip_pulsed<<"\t";
          //check key halfstrip decision, exclude  the edge halfstrips
	  if(dcfeb_pulsed<5 && last_keyhalfstrip_pulsed != min_hp[dcfeb_pulsed]+halfstrip) cout<<"WARNING: INCORRECT TMB Fiber Mapping for DCFEB "<<dcfeb_pulsed<<endl<<"Meant to pulse halfstrip "<<min_hp[dcfeb_pulsed]+halfstrip<<" but TMB key-halfstrip reads "<<last_keyhalfstrip_pulsed<<endl;
	  if(dcfeb_pulsed>4 && last_keyhalfstrip_pulsed != min_hp[dcfeb_pulsed]-halfstrip) cout<<"WARNING: INCORRECT TMB Fiber Mapping for DCFEB "<<dcfeb_pulsed<<endl<<"Meant to pulse halfstrip "<<min_hp[dcfeb_pulsed]-halfstrip<<" but TMB key-halfstrip reads "<<last_keyhalfstrip_pulsed;
         
          //checking pulse efficiency per halfstrip
          for(int ndcfeb=0;ndcfeb<7;ndcfeb++){
	  tmb_->WriteRegister(tmb_CountersReadReg,tmb_CounterRegValue[ndcfeb]);
	  counter_DCFEB[ndcfeb] = int(tmb_->ReadRegister(tmb_CountersWriteReg));
	  //out<<counter_DCFEB[0]<<"\t";
          }

          //This has to do with counter efficiency; when STEP are integrated with 7DCFEBs, this will become a plot 
          if(NumberofPulses != counter_DCFEB[dcfeb_pulsed]) out<<"Houston, we got problems.  DCFEB"<<dcfeb_pulsed+1<<" got "<<counter_DCFEB[dcfeb_pulsed]<<" and expected "<<NumberofPulses<<endl;
          sum_DCFEBCounters = counter_DCFEB[0]+counter_DCFEB[1]+counter_DCFEB[2]+counter_DCFEB[3]+counter_DCFEB[4]+counter_DCFEB[5]+counter_DCFEB[6];
          if((sum_DCFEBCounters - counter_DCFEB[dcfeb_pulsed]) > 0) out<<"More than 1 DCFEB was pulsed"<<endl;


	  //shifting everything back to normal
	  usleep(1000);
	  cout<<"Shifting Buckeyes to normal mode"<<endl;
	  (*dmb)->shift_all(NORM_RUN);
	  (*dmb)->buck_shift();
          usleep(1000);
          (*dmb)->restoreCFEBIdle(); // need to restore DCFEB JTAG after a buckshift
          usleep(1000);

          }
          dcfeb_pulsed++;
	  }
	  }
      */
    }

    /**************************************************************************
     * RoutineTest_PatternPulses:Counters
     * I. Suarez
     *************************************************************************/
    RoutineTest_PatternPulses_TMBCounters::RoutineTest_PatternPulses_TMBCounters(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ) {}
    void RoutineTest_PatternPulses_TMBCounters::display(xgi::Output * out)
    {
      addButton(out,"Routine Test - Pattern Pulses");
    }

    void RoutineTest_PatternPulses_TMBCounters::respond(xgi::Input * in, ostringstream & out)
    {

      // set register 0 appropriately for communication over the VME backplane.
      ccb_->setCCBMode(CCB::VMEFPGA);
      ccb_->hardReset();

      //tmb_->SetRequestL1a(0);  //do not request l1a cause the pulse comes with one            
      
      for(vector <DDU*>::iterator ddu = ddus_.begin(); ddu != ddus_.end();++ddu){
        (*ddu)->writeFakeL1( 0x0000 ); // 0x8787: passthrough // 0x0000: normal
        usleep(10);
        (*ddu)->writeGbEPrescale( 0x7878 ); // 0x7878: test-stand without TCC
        usleep(10);
      }

      // Enable L1A and clct_pretrig from any of dmb_cfeb_calib signals and disable all other trigger sources
      ccb_->EnableL1aFromDmbCfebCalibX();
      ccb_->SetExtTrigDelay( 16 ); // Delay of ALCT and CLCT external triggers before distribution to backplane
      usleep(100);
      //new from Joe's 21
      // tmb_->EnableClctExtTrig(); // TODO: via XML

      //// Set pulse height ////
      float PulseHeight = 1.0;
      float ComparatorThresholds = 0.1;
      int NumberofPulses = 2;
      int timebetweenpulse = 4000;
      ccb_->l1aReset(); // needed after setting/restarting pipeline
      usleep(100);
      int DMBposition=0;

      // int halfstrip2 = 10;
      // int hp[6] = {halfstrip2+1, halfstrip2, halfstrip2+1, halfstrip2, halfstrip2+1, halfstrip2};
      int dcfeb_pulsed = 0;
      unsigned int dcfebtrig[5] = {0x01,0x02,0x04,0x08,0x10};
      // unsigned int tmb_CounterRegValue[7] = {0x1ce2,0x1ee2,0x20e2,0x22e2,0x24e2,0x26e2,0x28e2};
      // unsigned int tmb_CountersReadReg = 0xd0;
      // unsigned int tmb_CountersWriteReg = 0xd2;
      //int position;
      int counter_DCFEB[7]; 
      //int sum_DCFEBCounters=0;
     
      int last_keyhalfstrip_pulsed = 0;
      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
        vector<CFEB> cfebs = (*dmb)->cfebs();
        for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); ++cfeb){
          int depth = 64; // get value that was read in from the crate config xml (unless it was changed later)
          (*dmb)->dcfeb_set_PipelineDepth( *cfeb, depth ); // set it on the hardware
          usleep(100);
          (*dmb)->Pipeline_Restart( *cfeb ); // must restart pipeline after setting it
          usleep(100);
          ccb_->l1aReset(); // needed after setting/restarting pipeline
          usleep(100);
        }
      }
     
      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ME11Test_PatternPulses") );


      for(vector <DAQMB*>::iterator dmb = dmbs_.begin(); dmb != dmbs_.end(); ++dmb){
        (*dmb)->set_dac(PulseHeight, PulseHeight);
        (*dmb)->set_comp_thresh(ComparatorThresholds);
        (*dmb)->settrgsrc(0);
        DMBposition = int(dmb - dmbs_.begin());
        if(DMBposition==0) dcfeb_pulsed=5;
        if(DMBposition==1) dcfeb_pulsed=0;
        vector<CFEB> cfebs = (*dmb)->cfebs();

        for(CFEBItr cfeb = cfebs.begin(); cfeb != cfebs.end(); cfeb++){
          for(int halfstrip = 0; halfstrip<32; ++halfstrip){
	    int  hp[6] = {halfstrip+1, halfstrip, halfstrip+1, halfstrip, halfstrip+1, halfstrip};
	    (*dmb)->trigsetx(hp, dcfebtrig[cfeb->number()]);

	    //ccb_->l1aReset(); // needed after setting/restarting pipeline
	    usleep(1000);
	    (*dmb)->restoreCFEBIdle(); // need to restore DCFEB JTAG after a buckshift
	    usleep(1000);
	    ccb_->bc0(); // start triggering
	    usleep(1000);
         
         
	    tmb_->ResetCounters();
	    usleep(1000);
	    //if(halfstrip==0) 
	    out<<dcfeb_pulsed+1<<"\t"<<NumberofPulses<<"\t"<<halfstrip<<"\t";
	    ccb_->inject(NumberofPulses,timebetweenpulse); // send the pulses
	    usleep(100000);
	  
	    last_keyhalfstrip_pulsed = int(((tmb_->ReadRegister(0x8a78) & 0xff00)>>8 ));
	    out<<last_keyhalfstrip_pulsed<<"\t";

	    tmb_->WriteRegister(0xd0,0x1ce2);
	    counter_DCFEB[0] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[0]<<"\t";

	    tmb_->WriteRegister(0xd0,0x1ee2);
	    counter_DCFEB[1] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[1]<<"\t";

	    tmb_->WriteRegister(0xd0,0x20e2);
	    counter_DCFEB[2] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[2]<<"\t"; 

	    tmb_->WriteRegister(0xd0,0x22e2);
	    counter_DCFEB[3] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[3]<<"\t";

	    tmb_->WriteRegister(0xd0,0x24e2);
	    counter_DCFEB[4] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[4]<<"\t";

	    tmb_->WriteRegister(0xd0,0x26e2);
	    counter_DCFEB[5] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[5]<<"\t";

	    tmb_->WriteRegister(0xd0,0x28e2);
	    counter_DCFEB[6] = int(tmb_->ReadRegister(0xd2));
	    out<<counter_DCFEB[6]<<endl;  

	    //shifting everything back to normal
	    usleep(1000);
	    cout<<"Shifting Buckeyes to normal mode"<<endl;
	    (*dmb)->shift_all(NORM_RUN);
	    (*dmb)->buck_shift();
	    usleep(1000);
	    (*dmb)->restoreCFEBIdle(); // need to restore DCFEB JTAG after a buckshift
	    usleep(1000);

          }
          dcfeb_pulsed++;
        }
      }  
      // stop DAQ
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();

    }

    /***********************************************
     *  A place to run other buttons in some sequence
     ***********************************************/

    ButtonTests::ButtonTests(Crate * crate, emu::odmbdev::Manager* manager)
      : Action( crate, manager ){}
    
    void ButtonTests::display(xgi::Output * out)
    {
      addButton(out, "Button Tests");

      *out << cgicc::input().set("type","hidden").set("name","depth").set("value","67")<< endl;
      *out << cgicc::input().set("type","hidden").set("name","ComparatorThresholds").set("value","0.03")<< endl;
      
    }
    
    void ButtonTests::respond(xgi::Input * in, ostringstream & out)
    {

      int sleeptime=300000;

      // Hard Reset
      HardReset hr(crate_);
      hr.respond(in,out);
      
      usleep(sleeptime);

      // Setup DDU
      CommonUtilities_setupDDU setupDDU(crate_);
      setupDDU.respond(in,out);
      
      usleep(sleeptime);

      //Set pipeine depth: 68
      SetPipelineDepthAllDCFEBs pd(crate_);
      pd.respond(in,out);

      usleep(sleeptime);

      //Restore CFEBs
      CommonUtilities_restoreCFEBIdle restore(crate_);
      //restore.respond(in,out);
      ccb_->l1aReset();
      
      usleep(sleeptime);

      //Set comp thresh: 0.03
      SetComparatorThresholds comp(crate_);
      comp.respond(in,out);
      
      usleep(sleeptime);

      //Shift to normal mode
      ShiftBuckeyesNormRun norm(crate_);
      norm.respond(in,out);

      usleep(sleeptime);

      //Restore CFEBs
      restore.respond(in,out);
      //ccb_->l1aReset();

      usleep(sleeptime);

      //BC0
      ccb_->bc0();

      usleep(sleeptime);

      // start DAQ
      cout<<"starting DAQ..."<<endl;
      manager_->startDAQ( string("ME11Test_ButtonTests") );

      ::sleep(3);
      
      // stop DAQ 
      cout<<"stopping DAQ..."<<endl;
      manager_->stopDAQ();
      cout<<" Done "<<endl;
    }

  } // namespace odmbdev
} // namespace emu
