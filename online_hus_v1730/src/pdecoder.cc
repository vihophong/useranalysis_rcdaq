
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include <TSystem.h>
#include <TROOT.h>
#include "pdecoder.h"
#include <dpp.h>
#include <libDataStruct.h>
//#include <DataStruct_dict.C>

#define DGTZ_CLK_RES 8
#define V1730_EVENT_TYPE 1
#define V1740_EVENT_TYPE 2
#define TDC_EVENT_TYPE 3

#define V1730_N_MAX_BOARD 5
#define V1730_N_MAX_CH 16
#define N_MAX_INFO 9

#define TDC_BOARD_N 50
#define TDC_PACKET 10
#define TDC_N_CH 64

#define V1740_BOARD_N 1
#define V1740_HDR 6
#define V1740_N_CH 8
#define V1740_PACKET 11
#define V1740_N_MAX_CH 8
#define NSBL 8
// 5000000 - 15 % memory = 1.2 Gb

/* ********************************************************************** */

#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <vector>
#include <map>
#include <TCanvas.h>

#define N_REFESH_WF 1000
#define N_MAX_WF_LENGTH 500

#define COUNTS_TO_DISPLAY 10000
#define COUNTS_TO_CAL 5000
#define MAX_N_SAMPLE 500

#define MAX_MAP_LENGTH 2000000

using namespace std;

//counter
//Int_t dgtz_clong[V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH];
int trgcnt_prev;
int its;
long long ts_prev;

int dgtzcnt_prev[V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH];
long long dgtzts_prev[V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH];
int tdccnt_prev[TDC_N_CH];
long long tdcts_prev[TDC_N_CH];

//! histograms and trees
TH2F *h2;
TH1F *h1wf[V1730_N_MAX_BOARD][V1730_N_MAX_CH];
TH2F *h2wf[V1730_N_MAX_BOARD][V1730_N_MAX_CH];
TH1F *htdc;

TH1F *h3wf[V1740_N_MAX_CH];
TH2F *h4wf[V1740_N_MAX_CH];
TH2F *h5;

TH2F *h6;

TH1F *hratedgtz;

TH1F *htrgrateintegrate;

TH1F* probe1;
TH1F* probe2;

TH1F* bc1l;
TH1F* bc1r;
TH1F* bc2l;
TH1F* bc2r;


void Init(){
    htrgrateintegrate = new TH1F("rate","rate",1000,0,1000);
    //h2 = new TH2F("e2d","Energy Spectra 2D",V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH+TDC_N_CH,0,V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH+TDC_N_CH,400,0,100000);
    h2 = new TH2F("e2d","Energy Spectra 2D",V1740_N_CH,0,V1740_N_CH,500,0,500);
    h5 = new TH2F("e2d740","Energy Spectra 2D",V1740_N_CH,0,V1740_N_CH,2000,0,2000);
    for (Int_t i=0;i<V1730_N_MAX_BOARD;i++){
        for (Int_t j=0;j<V1730_N_MAX_CH;j++){
            h1wf[i][j] = new TH1F (Form("wf1d_%d_%d",i,j),Form("Waveform 1d %d_%d",i,j), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH );
            h2wf[i][j] = new TH2F (Form("wf2d_%d_%d",i,j),Form("Waveform 2d %d_%d",i,j), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH, 1700, 0,17000 );
        }
    }
    for (int i=0;i<V1740_N_MAX_CH;i++){
    h3wf[i] = new TH1F (Form("wf740_1d_%d",i),Form("Waveform 1d v1740 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH );
    h4wf[i] = new TH2F (Form("wf740_2d_%d",i),Form("Waveform 2d v1740 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH, 1700, 0,17000 );
    }
    htdc = new TH1F("htdc","htdc",TDC_N_CH,0,TDC_N_CH);
    for (int i=0;i<TDC_N_CH;i++){
      tdccnt_prev[i]=0;
      tdcts_prev[i]=0;
    }
    for (int i=0;i<V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH;i++){
    dgtzcnt_prev[i] = 0;
    dgtzts_prev[i] = 0;

    }

    trgcnt_prev=0;
    ts_prev = 0;
    its = 0;

    hratedgtz=new TH1F("ratedgtz","ratedgtz",V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH,0,V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH);

    probe1=new TH1F("cp","cp",200,0,10000);
    probe2=new TH1F("kyto","kyoto",200,0,10000);
    bc1l=new TH1F("bc1l","bc1l",200,0,50000);
    bc1r=new TH1F("bc1r","bc1r",200,0,50000);
    bc2l=new TH1F("bc2l","bc2l",200,0,50000);
    bc2r=new TH1F("bc2r","bc2r",200,0,50000);
}

void ProcessEvent(NIGIRIHit* data){
    if (data->evt_type == V1730_EVENT_TYPE){
        int b = data->b;
        int ch  = data->ch;
        int itcnt=0;
    if (data->clong>0) h2->Fill(b*V1730_N_MAX_CH+ch,data->clong);
    if (b==2&&ch==11) probe1->Fill(data->clong);
    if (b==2&&ch==12) probe2->Fill(data->clong);
    if (data->clong>0&&b==2&&ch==4) bc1l->Fill(data->clong);
    if (data->clong>0&&b==2&&ch==5) bc1r->Fill(data->clong);
    if (data->clong>0&&b==2&&ch==6) bc2l->Fill(data->clong);
    if (data->clong>0&&b==2&&ch==7) bc2r->Fill(data->clong);
        for (std::vector<UShort_t>::iterator it = data->pulse.begin() ; it != data->pulse.end(); ++it){
            if (itcnt<N_MAX_WF_LENGTH){
          if (data->clong>0) h1wf[b][ch]->SetBinContent(itcnt+1,*it);
                h2wf[b][ch]->Fill(itcnt,*it);
            }
            itcnt++;
        }
    }else if (data->evt_type == V1740_EVENT_TYPE){
        int b = data->b;
        int ch  = (b-V1740_BOARD_N)*V1730_N_MAX_CH + data->ch;
//cout<<(b-V1740_BOARD_N)*V1730_N_MAX_CH + data->ch<<endl;
        int itcnt=0;
    if (data->clong>0) {h5->Fill(ch,data->clong);h2->Fill(ch,data->finets);}
    for (std::vector<UShort_t>::iterator it = data->pulse.begin() ; it != data->pulse.end(); ++it){
            if (itcnt<N_MAX_WF_LENGTH){
                h4wf[ch]->Fill(itcnt,*it);
                h3wf[ch]->SetBinContent(itcnt+1,*it);
            }
            itcnt++;
        }
    }else if (data->evt_type == TDC_EVENT_TYPE){
      for (std::vector<UShort_t>::iterator it = data->tdc_ch.begin() ; it != data->tdc_ch.end(); ++it){
    htdc->Fill(*it);
    h2->Fill(*it+V1730_N_MAX_BOARD*V1730_N_MAX_CH+V1740_N_CH,25000);
    //! tdc counter, rate metter
      }
      if (((data->evt-trgcnt_prev)>COUNTS_TO_CAL==0)){
    double tdiff= (double)((long long)data->ts - (long long)ts_prev)/1e10;
    double rate = (double)(data->evt-trgcnt_prev)/tdiff;//cps
    if (ts_prev>0){
       htrgrateintegrate->SetBinContent(its+1,rate);
       its++;
       if (its>1000) {
        htrgrateintegrate->Reset();
        its = 0;
       }
    }
        ts_prev = (long long) data->ts;
    trgcnt_prev = (int)data->evt;
      }


    }else{
        cout<<"ERROR! @.@"<<endl;
    }
    //! digitizer counter rate meter
    if (data->evt_type != TDC_EVENT_TYPE){
        int ich =data->b*V1730_N_MAX_CH+data->ch;
        if (dgtzcnt_prev[ich]%COUNTS_TO_CAL==0){
        if (dgtzcnt_prev[ich]>0){
           double rate=((double)(data->evt-dgtzcnt_prev[ich]))/((double)(data->ts-dgtzts_prev[ich]))/DGTZ_CLK_RES*1e9;
           hratedgtz->SetBinContent(ich+1,rate);
        }
        dgtzts_prev[ich] = data->ts;
        dgtzcnt_prev[ich] = data->evt;
        }
    }


}

void CloseMe(){

}


//!------------**********************
//! //!------------**********************
//! //!------------**********************
//! //!------------**********************
//! //!------------**********************
//! //!------------**********************
//! //!------------**********************

#include <stdint.h>

int CFD_delay=1;//in Tclk unit, 1 Tclk=2ns
double CFD_fraction=0.5;
double LED_threshold=100;
double LED_threshold_LED=100;
double LED_threshold_740=40;
double LED_threshold_LED_740=40;
int gateOffset=10;
int shortGate=20;
int longGate=150;
int nBaseline=16;
int minVarBaseline=100; //criteria for baseline determination
int mode_selection=0;


//! dont care about the following code
uint16_t seperateint(int inint,bool islsb){
  if (islsb) return (uint16_t) (inint&0xFFFF);
  else return (uint16_t) (inint>>16);
}

NIGIRIHit* data;

int init_done = 0;
int pinit()
{
  if (init_done) return 1;
  init_done = 1;
  gROOT->ProcessLine(".L libDataStruct.so");
  data = new NIGIRIHit;
  Init();
  return 0;
}

int process_event (Event * e)
{

  //! v1740 packet
  Packet *p1740=e->getPacket(V1740_PACKET);
  if (p1740)
    {
      int* temp;
      int* gg;
      gg=(int*) p1740->getIntArray(temp);
      int size=p1740->getPadding();
      //! content
      int k=V1740_HDR+V1740_N_MAX_CH;
      for (int i=0;i<V1740_N_CH;i++){
        //! header
        data->Clear();
        data->evt_type = V1740_EVENT_TYPE;
        data->b = V1740_BOARD_N+i/V1730_N_MAX_CH;//for sorter
        data->evt = gg[2]+1;//this event start from 0
        data->overrange = (Char_t) gg[1];//intepret as channel(group) mask
        UInt_t tslsb = (UInt_t)gg[5];
        UInt_t tsmsb = (UInt_t)gg[4];
        data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;//resolution is 16 ns!
        data->ch = i%V1730_N_MAX_CH;//for sorter

        int nsample = gg[i+V1740_HDR];
        data->nsample = nsample;
        UShort_t WaveLine[nsample];

        int ispl = 0;
        for (int j=0;j<nsample/2+nsample%2;j++){
          if (ispl<nsample) {
              WaveLine[ispl]=gg[k]&0xFFFF;
              data->pulse.push_back(gg[k]&0xFFFF);

          }
          ispl++;
          if (ispl<nsample) {
              WaveLine[ispl]=(gg[k]>>16)&0xFFFF;
              data->pulse.push_back((gg[k]>>16)&0xFFFF);
          }
          ispl++;
          k++;
        }

        if (nsample>NSBL){
            dpp *oj=new dpp(nsample,WaveLine);
            oj->baselineMean(nBaseline,minVarBaseline);
            double timeData = 0;
            int cShort,cLong;

            if (mode_selection==0) {
                timeData=oj->led(LED_threshold_LED_740);
            }else if (mode_selection==1) {
                timeData=oj->ledWithCorr(LED_threshold_740);
            }else if (mode_selection==2){
                oj->makeCFD(LED_threshold_740,CFD_delay,CFD_fraction);
                timeData=oj->cfdFast();
            }else if (mode_selection==3){
                oj->makeCFD(LED_threshold_740,CFD_delay,CFD_fraction);
                timeData=oj->cfdSlow();
            }

            //oj->chargeInter(cShort,cLong,timeData,gateOffset,shortGate,longGate);
            data->cshort = 0;
            data->clong = oj->maxAdcPos(N_MAX_WF_LENGTH)-oj->bL;
            //cout<<cLong<<endl;
            data->baseline = oj->bL;
            data->finets = timeData;
            delete oj;
        }
    ProcessEvent(data);

    /*
      if (gg[2]==1000) {
        for (int ii=0;ii<2;ii++) cout<<i<<" "<<nsample<<" "<<ii<<" "<<WaveLine[ii]<<endl;
        for (int ii=nsample-3;ii<nsample;ii++) cout<<i<<" "<<nsample<<" "<<ii<<" "<<WaveLine[ii]<<endl;
      }
    */
      }
       delete p1740;
  }
  return 0;
}

int pclose(){
    CloseMe();
    cout<<"Finish!"<<endl;
    return 0;
}

