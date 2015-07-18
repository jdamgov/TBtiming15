#include "interface/H4treeReco.h"
#include "interface/WaveformUtils.hpp"
#include "interface/WaveformFit.hpp"
#include "interface/Waveform.hpp"

#include <iostream>

//
H4treeReco::H4treeReco(TChain *tree,TString outUrl) : 
  H4tree(tree),
  wcXl_(4),   //TDC Xleft
  wcXr_(5),   //TDC Xright
  wcYd_(6),   //TDC Ydown
  wcYu_(7),   //TDC Yup
  MaxTdcChannels_(16),
  MaxTdcReadings_(20),
  nActiveDigitizerChannels_(8)
{
  fOut_  = TFile::Open(outUrl,"RECREATE");
  recoT_ = new TTree("H4treeReco","H4treeReco");
  recoT_->SetDirectory(fOut_);

  //event header
  recoT_->Branch("runNumber",    &runNumber,    "runNumber/i");
  recoT_->Branch("spillNumber",  &spillNumber,  "spillNumber/i");
  recoT_->Branch("evtNumber",    &evtNumber,    "evtNumber/i");
  recoT_->Branch("evtTimeDist",  &evtTimeDist,  "evtTimeDist/i");
  recoT_->Branch("evtTimeStart", &evtTimeStart, "evtTimeStart/i");
  recoT_->Branch("nEvtTimes",    &nEvtTimes,    "nEvtTimes/i");

  //TDC 
  tdc_readings_.resize(MaxTdcChannels_);
  recoT_->Branch("tdc_recox", &tdc_recox_, "tdc_recox/F");
  recoT_->Branch("tdc_recoy", &tdc_recoy_, "tdc_recoy/F");
  
  //add more variables relevant for the study
  recoT_->Branch("maxch", &maxch_, "maxch/i");
  recoT_->Branch("group", group_,  "group[maxch]/F");
  recoT_->Branch("ch",    ch_,     "ch[maxch]/F");
  recoT_->Branch("isample",    isample_,     "isample[1024]/I");
  recoT_->Branch("wf0",    wf0_,     "wf0[1024]/F");
  recoT_->Branch("wf3",    wf3_,     "wf3[1024]/F");
  recoT_->Branch("wf4",    wf4_,     "wf4[1024]/F");
  recoT_->Branch("pedestal",    pedestal_,     "pedestal[maxch]/F");
  recoT_->Branch("wave_max",    wave_max_,     "wave_max[maxch]/F");
  recoT_->Branch("charge_integration",    charge_integration_,     "charge_integration[maxch]/F");
  recoT_->Branch("t_max",           t_max_,     	"t_max[maxch]/F");
  recoT_->Branch("t_max_frac30",    t_max_frac30_,     	"t_max_frac30[maxch]/F");
  recoT_->Branch("t_max_frac50",    t_max_frac50_,     	"t_max_frac50[maxch]/F");

  InitDigi();
}

//
void H4treeReco::InitDigi()
{
  //init channels of interest
  groupsAndChannels_.insert( std::pair<Int_t,Int_t>(0,0) ); //MPC
  groupsAndChannels_.insert( std::pair<Int_t,Int_t>(0,3) ); //Si Pad #1
  groupsAndChannels_.insert( std::pair<Int_t,Int_t>(0,4) ); //Si Pad #2
  groupsAndChannels_.insert( std::pair<Int_t,Int_t>(1,8) ); //Trigger
  
// static index for interactive ploting
  for(int i=0;i<1024;i++) isample_[i]=i;

  for(std::set< std::pair<Int_t,Int_t> >::iterator key=groupsAndChannels_.begin();
      key!=groupsAndChannels_.end();
      ++key)
    {
      Int_t iGroup(key->first),iChannel(key->second);
      if(iChannel>=nActiveDigitizerChannels_) continue;
	  
      char name[100];
      int iThisEntry=0;
      int iHistEntry=0;
      sprintf(name,"digi_ch%02d",iGroup*8+iChannel);
      varplots_[name] = new VarPlot(&iThisEntry,&iHistEntry,kPlot2D);
      varplots_[name]->waveform = new Waveform();
      varplots_[name]->SetName(name);
      varplots_[name]->doPlot   =false;
      varplots_[name]->SetGM(iGroup,iChannel);
//      std::cout<<"iGroup,iChannel   "<<iGroup<<"   "<<iChannel<<std::endl;

    }
}

//
void H4treeReco::FillWaveforms()
{
	for (std::map<TString,VarPlot*>::iterator it=varplots_.begin();it!=varplots_.end();++it)
	  if (it->second->waveform) it->second->waveform->clear();
	
	//fill waveforms
	char name[100];
	for (uint iSample = 0 ; iSample < nDigiSamples ; ++iSample)
	  {
	  	std::pair<Int_t,Int_t> key(digiGroup[iSample],digiChannel[iSample]);
	  	if(groupsAndChannels_.find(key)==groupsAndChannels_.end()) continue;
	        if(digiChannel[iSample]>=nActiveDigitizerChannels_) continue;
      		sprintf(name,"digi_ch%02d",key.first*8 +key.second);
                if(key.first*8 +key.second==0) wf0_[digiSampleIndex[iSample]]=digiSampleValue[iSample];
                if(key.first*8 +key.second==3) wf3_[digiSampleIndex[iSample]]=digiSampleValue[iSample];
                if(key.first*8 +key.second==4) wf4_[digiSampleIndex[iSample]]=digiSampleValue[iSample];
		//varplots_[name]->Fill2D(digiSampleIndex[iSample], digiSampleValue[iSample],1.);
	    	varplots_[name]->waveform->addTimeAndSample(digiSampleIndex[iSample]*0.2,digiSampleValue[iSample]);
	  }

	Waveform * waveform;
	//reconstruct waveforms
	maxch_=0;
	for (std::map<TString,VarPlot*>::iterator it=varplots_.begin();it!=varplots_.end();++it,++maxch_)
	{
		// Extract waveform information:
		waveform = it->second->waveform ;
       		Waveform::baseline_informations wave_pedestal;
	       	Waveform::max_amplitude_informations wave_max;
       
		wave_pedestal= waveform->baseline(5,44); //use 40 samples between 5-44 to get pedestal and RMS
		pedestal_[maxch_]=wave_pedestal.pedestal;

		//substract a fixed value from the samples
		waveform->offset(wave_pedestal.pedestal);

		//rescale all the samples by a given rescale factor, i.e. invert the signal
		waveform->rescale(-1); 
		wave_max=waveform->max_amplitude(50,900,5); //find max amplitude between 50 and 900 samples
		if(wave_max.max_amplitude<20)
		{
			waveform->rescale(-1);	
			Waveform::max_amplitude_informations wave_max_inv = waveform->max_amplitude(50,900,5);
			if(wave_max_inv.max_amplitude > wave_max.max_amplitude)
				wave_max = wave_max_inv;
			else   // stay with negative signals
				waveform->rescale(-1);
		}
		wave_max_[maxch_]=wave_max.max_amplitude;
//                std::cout<<wave_max.max_amplitude<<std::endl;
		

//		charge_integration_[maxch_] = waveform->charge_integrated(0,900);// pedestal already subtracted 
		charge_integration_[maxch_] = waveform->charge_integrated(150,350);// pedestal already subtracted 
		t_max_[maxch_]              = wave_max.time_at_max;
		t_max_frac30_[maxch_]       = waveform->time_at_frac(wave_max.time_at_max-1.3e-8,wave_max.time_at_max,0.3,wave_max,7);
		t_max_frac50_[maxch_]       = waveform->time_at_frac(wave_max.time_at_max-1.3e-8,wave_max.time_at_max,0.5,wave_max,7);
		ch_[maxch_]=0;
                if(it->first.Contains("digi_ch03"))ch_[maxch_]=3;
                if(it->first.Contains("digi_ch04"))ch_[maxch_]=4;
	}
}


//
void H4treeReco::Loop()
{
  if (fChain == 0) return;
  
  Long64_t nentries = fChain->GetEntries();
//  nentries = 2000;
  for (Long64_t jentry=0; jentry<nentries;jentry++) 
    {
      
      //progress bar
      if(jentry%10==0) 
	{
	  printf("\r[H4treeReco] status [ %3d/100 ]",int(100*float(jentry)/float(nentries)));
	  std::cout << std::flush;
	}
      
      //readout the event
      fChain->GetEntry(jentry); 

      //save x/y coordinates from the wire chambers
      FillTDC();      
      
      //loop over the relevant channels and reconstruct the waveforms
      //https://github.com/cmsromadaq/H4DQM/blob/master/src/plotterTools.cpp#L1785
      FillWaveforms();
	
      //optional:
      //save pulse, pedestal subtracted and aligned using trigger time?
      
      recoT_->Fill();
    }
}

//
void H4treeReco::FillTDC()
{
  tdc_recox_=-999;
  tdc_recoy_=-999;

  for (uint j=0; j<MaxTdcChannels_; j++){
    tdc_readings_[j].clear();
  }
  
  for (uint i=0; i<nTdcChannels; i++){
    if (tdcChannel[i]<MaxTdcChannels_){
      tdc_readings_[tdcChannel[i]].push_back((float)tdcData[i]);
    }
  }
  
  if (tdc_readings_[wcXl_].size()!=0 && tdc_readings_[wcXr_].size()!=0){
    float TXl = *std::min_element(tdc_readings_[wcXl_].begin(),tdc_readings_[wcXl_].begin()+tdc_readings_[wcXl_].size());
    float TXr = *std::min_element(tdc_readings_[wcXr_].begin(),tdc_readings_[wcXr_].begin()+tdc_readings_[wcXr_].size());
    tdc_recox_ = (TXr-TXl)*0.005; // = /40./5./10. //position in cm 0.2mm/ns with 25ps LSB TDC
  }
  if (tdc_readings_[wcYd_].size()!=0 && tdc_readings_[wcYu_].size()!=0){
    float TYd = *std::min_element(tdc_readings_[wcYd_].begin(),tdc_readings_[wcYd_].begin()+tdc_readings_[wcYd_].size());
    float TYu = *std::min_element(tdc_readings_[wcYu_].begin(),tdc_readings_[wcYu_].begin()+tdc_readings_[wcYu_].size());
    tdc_recoy_ = (TYu-TYd)*0.005; // = /40./5./10. //position in cm 0.2mm/ns with 25ps LSB TDC
  }
}


H4treeReco::~H4treeReco()
{
  fOut_->cd();
  recoT_->Write();
  fOut_->Close();
}
