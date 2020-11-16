#include <iostream>
#include <pmonitor/pmonitor.h>
#include "myanalysis.h"
#include <TGClient.h>
#include "TSystem.h"
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TROOT.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <iostream>
#include <fstream>
#include<stdlib.h>
#include<string.h>

//#define DEBUG

#define TDC_PACKET 10

#define COUNTS_TO_UPDATE 1
int init_done = 0;

using namespace std;

TCanvas* c1;
TCanvas* c2;
Int_t c1updatetrgcnt;
TH1F *h1;
TH1F *hadc[8];
TH2F *h2;

Int_t ch_thr[8];
Int_t ch_ecal0[8];
Int_t ch_ecal1[8];

int pinit()
{

    if (init_done) return 1;
    init_done = 1;

    std::ifstream inpf("channel_calib.txt");
      if (inpf.fail()){
        cout<<"No Configuration table is given"<<endl;
        return 1;
      }

      cout<<"Start reading calibration table channel_calib.txt"<<endl;
      Int_t channel;
      Double_t threshold,ecal0,ecal1;
      Int_t mm=0;

      while (inpf.good()){
        inpf>>channel>>threshold>>ecal0>>ecal1;
        cout<<channel<<"-"<<threshold<<"-"<<ecal0<<"-"<<ecal1<<endl;
        ch_ecal0[channel]=ecal0;
        ch_ecal1[channel]=ecal1;
        ch_thr[channel]=threshold;
        mm++;
      }

    c1updatetrgcnt=0;

    h1 = new TH1F ( "h1","histogram of total counts", 10, 0, 10);
    for (Int_t i=0;i<8;i++)
        hadc[i]=new TH1F (Form("hadc%d",i),Form("Channel %d ADC spectrum",i), 5000, 0, 5000);
    h2 = new TH2F ( "h2","All channels: E vs ch",10,0,10,5000, 0, 5000);


    c1=new TCanvas("c1","c1",1200,1200);
    c1->Divide(2,4);
    for (Int_t i=0;i<8;i++){
        c1->cd(i+1);
        hadc[i]->Draw();
    }

    c2=new TCanvas("rate","rate",900,600);
    c2->Divide(1,2);
    c2->cd(1);
    h1->SetFillColor(2);
    h1->Draw();
    c2->cd(2);
    h2->Draw("colz");


    new MyMainFrame(gClient->GetRoot(),200,200);    
    return 0;
}

int process_event (Event * e)
{
    Packet *tdcp = e->getPacket(TDC_PACKET);
    if(tdcp){
        int* temp;
        int* gg;
        gg=(int*) tdcp->getIntArray(temp);
        int size=tdcp->getPadding();
        //data->Clear();
        //data->evt_type = TDC_EVENT_TYPE;
        //data->b = TDC_BOARD_N;
        //data->ch = 0;
        UInt_t tslsb = (UInt_t)gg[3];
        UInt_t tsmsb = (UInt_t)gg[2];
        //data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;//resolution is 16 ns!
        int ncounter= gg[1];
        //data->clong= gg[1];//daq counter
        //data->evt = gg[0];
        //! more tdc data here!
        int nwordtdc = 0;
        //cout<<"EVENT BEGIN"<<endl;

        bool flag_trailer = false;
        bool flag_endofv1190 = false;
        bool flag_v792counter = false;
        int nwordqdc = 0;
        int lastcounter=0;
        for (int i=4;i<size;i++){
            if((gg[i]&0xF8000000)>>27==0x12){
#ifdef DEBUG
                cout<<"Separation bit met"<<gg[i]<<endl;
#endif
                flag_endofv1190 = true;
                //break;
            }
            if((gg[i]&0xF8000000)>>27==0x13){
#ifdef DEBUG
                cout<<std::hex<<"Separation bit counter met 0x"<<gg[i]<<std::dec<<endl;
#endif
                flag_v792counter=true;
                //break;
            }
            if (!flag_v792counter&&flag_endofv1190&&(gg[i]&0xF8000000)>>27==0x1F){
                //! read qdc      
#ifdef DEBUG
                cout<<"qdc bit"<<std::hex<<"0x"<<((gg[i]&0x7000000)>>24)<<" - 0x"<<gg[i]<<std::dec<<" ch="<<((gg[i]&0x1C0000)>>18)<<" - range"<<((gg[i]&0x20000)>>17)<<" - adc"<<(gg[i]&0xFFF)<<endl;
#endif
                if (((gg[i]&0x7000000)>>24)==0x2){//Header met
                    //int qdcnmemorisech=(int)((gg[i]&0x3F00)>>8);
                    //cout<<qdcnmemorisech<<endl;
                }else if(((gg[i]&0x7000000)>>24)==0x0){
                    int qdcch = ((gg[i]&0x1C0000)>>18);
                    int qdcdata =(gg[i]&0xFFF);
                    int range=((gg[i]&0x20000)>>17);                    
                    if (range==0) {
                        double ecal=qdcdata*ch_ecal1[qdcch]+ch_ecal0[qdcch];
                        h2->Fill(qdcch,ecal);
                        if (qdcdata>ch_thr[qdcch]) h1->Fill(qdcch);
                        hadc[qdcch]->Fill(ecal);
                    }
                    if (range==0) {
                        if (c1updatetrgcnt>COUNTS_TO_UPDATE){
                            for (int ch=0;ch<8;ch++){
                                c1->cd(ch+1)->Modified();
                                c1->cd(ch+1)->Update();
                            }
                            c2->cd(1)->Modified();
                            c2->cd(1)->Update();
                            c2->cd(2)->Modified();
                            c2->cd(2)->Update();
                            c1updatetrgcnt=0;
                        }
                        if (qdcdata>ch_thr[qdcch]) c1updatetrgcnt++;
                    }
                    //data->AddQDCHit(qdcch,qdcdata);
                    //cout<<qdcch<<"-"<<qdcdata<<endl;
                }else if (((gg[i]&0x7000000)>>24)==0x4){//end of qdc block met
                    int qdcevtcounter = gg[i]&0x7FFFFF;
                    //cout<<ncounter<<" / "<<qdcevtcounter<<" deadtime(BLT) = "<<100-(Double_t)ncounter/(Double_t)qdcevtcounter*100<<" %"<<endl;
                    lastcounter = qdcevtcounter;
                }else if (((gg[i]&0x7000000)>>24)==0x6){
                    //cout<<"empty buffer read"<<endl;
                }else{
                    cout<<"Something wrong with QDC?"<<((gg[i]&0x7000000)>>24)<<endl;
                }
                nwordqdc++;
            }
            if (flag_v792counter&&((gg[i]&0xF8000000)>>27!=0x13)){
                //cout<<gg[i]<<" / "<<lastcounter<<" deadtime(MBLT) = "<<100-(Double_t)gg[i]/(Double_t)lastcounter*100<<" %"<<endl;
            }
        }
        //cout<<"EVENT END "<<nwordqdc<<endl;
        delete tdcp;
    }
    return 0;
}

MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {
    // Create a main frame
    fMain = new TGMainFrame(p,w,h);

    // Create a horizontal frame widget with buttons
    TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain,900,20);
    TGTextButton *daqconfigbtn = new TGTextButton(hframe,"&Config DAQ");
    daqconfigbtn->Connect("Clicked()","MyMainFrame",this,"DoDAQConfig()");
    hframe->AddFrame(daqconfigbtn, new TGLayoutHints(kLHintsCenterX,
                                                   5,5,3,4));
    TGTextButton *daqopenbtn = new TGTextButton(hframe,"&Open File");
    daqopenbtn->Connect("Clicked()","MyMainFrame",this,"DoOpenDAQ()");
    hframe->AddFrame(daqopenbtn, new TGLayoutHints(kLHintsCenterX,
                                                   5,5,3,4));
    TGTextButton *daqbeginbtn = new TGTextButton(hframe,"&Begin Run");
    daqbeginbtn->Connect("Clicked()","MyMainFrame",this,"DoBeginDAQ()");
    hframe->AddFrame(daqbeginbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *daqendbtn = new TGTextButton(hframe,"&End Run");
    daqendbtn->Connect("Clicked()","MyMainFrame",this,"DoEndDAQ()");
    hframe->AddFrame(daqendbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *exit = new TGTextButton(hframe,"&Exit",
                                 "gApplication->Terminate(0)");
    hframe->AddFrame(exit, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    fMain->AddFrame(hframe, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));
    // Create canvas widget
//    fEcanvas = new TRootEmbeddedCanvas("Ecanvas",fMain,900,700);
//    fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX |
//                    kLHintsExpandY, 10,10,10,1));

    // Create bottom frame for reset button
    TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain,900,20);
    TGTextButton *resetbtn = new TGTextButton(hframe2,"&Clear Histograms");
    resetbtn->Connect("Clicked()","MyMainFrame",this,"DoReset()");
    hframe2->AddFrame(resetbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *stoponlinebtn = new TGTextButton(hframe2,"&Pause Online");
    stoponlinebtn->Connect("Clicked()","MyMainFrame",this,"StopOnline()");
    hframe2->AddFrame(stoponlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *resumeonlinebtn = new TGTextButton(hframe2,"&Resume Online");
    resumeonlinebtn->Connect("Clicked()","MyMainFrame",this,"ResumeOnline()");
    hframe2->AddFrame(resumeonlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    fMain->AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Set a name to the main frame
    fMain->SetWindowName("DAQ");

    // Map all subwindows of main frame
    fMain->MapSubwindows();

    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());

    // Map main frame
    fMain->MapWindow();

//    TCanvas *c1 = fEcanvas->GetCanvas();
//    c1->Divide(2,4);
//    for (Int_t i=0;i<8;i++) {
//        c1->cd(i+1);
//        hadc[i]->Draw();
//    }
}
MyMainFrame::~MyMainFrame() {
    // Clean up used widgets: frames, buttons, layout hints
    fMain->Cleanup();
    delete fMain;
}
void MyMainFrame::DoReset() {
    for (Int_t i=0;i<8;i++)
        hadc[i]->Reset();
    h1->Reset();
    h2->Reset();
}

void MyMainFrame::StopOnline() {
    gROOT->ProcessLine("pstop()");
}
void MyMainFrame::ResumeOnline(){
    gROOT->ProcessLine("pstart()");
}

void MyMainFrame::DoBeginDAQ() {
    system("rcdaq_client daq_begin");
}
void MyMainFrame::DoEndDAQ() {
    system("rcdaq_client daq_end");
}
void MyMainFrame::DoOpenDAQ() {
    system("rcdaq_client daq_open");
}
void MyMainFrame::DoDAQConfig() {
    system("./daqconfig1.sh");
}

//void MyMainFrame::WriteLLDValueForV1785()
//{
//    Write_reg(0x1034,0x100);
//    for (uint32_t i=0;i<16;i++) {
//        Write_reg(0x1080 +i*4,fLLDValue/16);
//    }
//}
