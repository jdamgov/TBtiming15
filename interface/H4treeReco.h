#ifndef H4treeReco_h
#define H4treeReco_h

#include "interface/H4tree.h"
#include "interface/ChannelReco.h"
#include "interface/JSONWrapper.h"

#include "TFile.h"
#include "TString.h"

#include <set>

class H4treeReco : public H4tree
{

 public:
  H4treeReco(TChain *,JSONWrapper::Object *cfg,TString outUrl="H4treeRecoOut.root");
  void Loop(); 
  void FillTDC();
  void FillWaveforms();
  ~H4treeReco();
  
 private:

  JSONWrapper::Object *cfg_;
  void InitDigi();
  
  inline float timeSampleUnit(int drs4Freq)
  {
    if (drs4Freq == 0)
      return 0.2E-9;
    else if (drs4Freq == 1)
      return 0.4E-9;
    else if (drs4Freq == 2)
      return 1.E-9;    
    return -999.;
  }

  typedef std::pair<UInt_t,UInt_t>         GroupChannelKey_t;
  std::map<GroupChannelKey_t,ChannelReco*> chPlots_;

  //TDC readings
  UInt_t MaxTdcChannels_,MaxTdcReadings_;
  std::vector< std::vector<Float_t> > tdc_readings_;
  Float_t wc_recox_[16], wc_recoy_[16];
  UInt_t wc_xl_hits_[16], wc_xr_hits_[16], wc_yu_hits_[16], wc_yd_hits_[16]; 
  UInt_t nwc_,wcXl_[4],wcXr_[4],wcYu_[4],wcYd_[4];



  //Channels to RECO
  UInt_t nActiveDigitizerChannels_;
  UInt_t maxch_;
  Int_t isample_[1024];
  Float_t wf0_[1024],wf2_[1024],wf3_[1024],wf4_[1024],wf8_[1024];
  Float_t group_[100],ch_[100];
  Float_t pedestal_[100],         pedestalRMS_[100];
  Float_t wave_max_[100], wave_max_aft_[100], wave_aroundmax_[100][50], time_aroundmax_[100][50];
  Float_t charge_integ_max_[100], charge_integ_max30_[100], charge_integ_max50_[100], charge_integ_[100];
  Float_t t_max_[100],            t_max_frac30_[100],       t_max_frac50_[100],       t_at_threshold_[100], t_over_threshold_[100];

  TTree *recoT_;
  TFile *fOut_;
};

#endif
