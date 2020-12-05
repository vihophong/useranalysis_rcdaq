#include "MyMainFrame.h"
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
#include <TGLabel.h>

#include <TDirectory.h>
#include <iostream>


#include <iostream>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string execget(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    auto pipe = popen(cmd, "r");

    if (!pipe) throw std::runtime_error("popen() failed!");

    while (!feof(pipe))
    {
        if (fgets(buffer.data(), 128, pipe) != nullptr)
            result += buffer.data();
    }

    auto rc = pclose(pipe);

//    if (rc == EXIT_SUCCESS)
//    {
//        std::cout << "SUCCESS\n";
//    }
//    else
//    {
//        std::cout << "FAILED\n";
//    }

    return result;
}


MyMainFrame::MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {
    fupdate_time=1000;
    fRateCal_time=10000;
    c1=new TCanvas("c1","c1",1200,1200);
    c2=new TCanvas("c2","c2",900,600);
    // Create a main frame
    fMain = new TGMainFrame(p,w,h);

    // Create a horizontal frame widget with buttons
    TGGroupFrame* hframe = new TGGroupFrame(fMain, "DAQ controls", kHorizontalFrame);
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
    TGGroupFrame* hframe2 = new TGGroupFrame(fMain, "Online Controls", kHorizontalFrame);
    TGTextButton *stoponlinebtn = new TGTextButton(hframe2,"&Pause Online");
    stoponlinebtn->Connect("Clicked()","MyMainFrame",this,"StopOnline()");
    hframe2->AddFrame(stoponlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *resumeonlinebtn = new TGTextButton(hframe2,"&Resume Online");
    resumeonlinebtn->Connect("Clicked()","MyMainFrame",this,"ResumeOnline()");
    hframe2->AddFrame(resumeonlinebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *resetbtn = new TGTextButton(hframe2,"&Clear Histograms");
    resetbtn->Connect("Clicked()","MyMainFrame",this,"DoReset()");
    hframe2->AddFrame(resetbtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));




    fMain->AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Create bottom frame for reset button
    TGGroupFrame* hframe3 = new TGGroupFrame(fMain, "Timer Settings", kHorizontalFrame);
    autoupdateentry = new TGTextEntry(hframe3,"1000");
    hframe3->AddFrame(autoupdateentry, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    TGTextButton *autoupdatebtn = new TGTextButton(hframe3,"&Set AutoUpdate Timer");
    autoupdatebtn->Connect("Clicked()","MyMainFrame",this,"SetTimer()");
    hframe3->AddFrame(autoupdatebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));

    ratecalentry = new TGTextEntry(hframe3,"10000");
    hframe3->AddFrame(ratecalentry, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    TGTextButton *setratecaltimebtn = new TGTextButton(hframe3,"&Set Rate Cal. Timer");
    setratecaltimebtn->Connect("Clicked()","MyMainFrame",this,"SetRateCalTime()");
    hframe3->AddFrame(setratecaltimebtn, new TGLayoutHints(kLHintsCenterX,
                                             5,5,3,4));
    fMain->AddFrame(hframe3, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    TGGroupFrame* grframe = new TGGroupFrame(fMain, "Options", kHorizontalFrame);
    grframe->SetTitlePos(TGGroupFrame::kLeft); // right aligned
    // 2 column, n rows
    grframe->SetLayoutManager(new TGMatrixLayout(grframe, 0, 2, 10));

    grframe->AddFrame(new TGLabel(grframe, new TGHotString("Pad to be assigned: ")));

    TGTextBuffer *tbuf = new TGTextBuffer(10);
    tbuf->AddText(0, "1");
    PadIdTent = new TGTextEntry(grframe, tbuf);
    PadIdTent->Resize(50, PadIdTent->GetDefaultHeight());
    grframe->AddFrame(PadIdTent);

    grframe->AddFrame(new TGLabel(grframe, new TGHotString("Channel to be assigned")));

    TGTextBuffer *tbuf2 = new TGTextBuffer(10);
    tbuf2->AddText(0, "0");
    ChIDTent = new TGTextEntry(grframe, tbuf2);
    ChIDTent->Resize(50, ChIDTent->GetDefaultHeight());
    grframe->AddFrame(ChIDTent);

    fMain->AddFrame(grframe, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));


    TGHorizontalFrame *hframe4 = new TGHorizontalFrame(fMain,900,20);
    TGTextButton *assignchbutton = new TGTextButton(hframe4,"&Assign Pad to Channel");
    assignchbutton->Connect("Clicked()","MyMainFrame",this,"AssignChannel()");
    hframe4->AddFrame(assignchbutton);
    fMain->AddFrame(hframe4, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    TGGroupFrame* grframestatus = new TGGroupFrame(fMain, "DAQ Status", kHorizontalFrame);
    DAQstatusTextDialogs = new TGTextView(grframestatus,800,200, kSunkenFrame | kDoubleBorder);
    DAQstatusTextDialogs->LoadBuffer("checking...");
    grframestatus->AddFrame(DAQstatusTextDialogs);
    fMain->AddFrame(grframestatus, new TGLayoutHints(kLHintsCenterX,
                                              2,2,2,2));

    // Create status frame containing a button and a text entry widget
    statusbar = new TGStatusBar(fMain, 900, 20, kVerticalFrame);
    Int_t parts[] = {45, 15, 10, 30};
    statusbar->SetParts(parts, 4);
    statusbar->Draw3DCorner(kFALSE);
    fMain->AddFrame(statusbar, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

    // Set a name to the main frame
    fMain->SetWindowName("DAQ");

    // Map all subwindows of main frame
    fMain->MapSubwindows();

    // Initialize the layout algorithm
    fMain->Resize(fMain->GetDefaultSize());

    // Map main frame
    fMain->MapWindow();

    timer = new TTimer();
    timer->Connect("Timeout()", "MyMainFrame", this, "DoUpdate()");
    timer->Start(fupdate_time, kFALSE);   // 2 seconds single-shot

    timer2 = new TTimer();
    timer2->Connect("Timeout()", "MyMainFrame", this, "DoCalculateRate()");
    timer2->Start(fRateCal_time, kFALSE);   // 2 seconds single-shot
}
MyMainFrame::~MyMainFrame() {
    // Clean up used widgets: frames, buttons, layout hints
    fMain->Cleanup();
    delete fMain;
}

void MyMainFrame::DoCalculateRate() {
    TH1F* h1tmp=(TH1F*) gDirectory->Get("h1tmp");
    TH1F* h1=(TH1F*) gDirectory->Get("h1");
    for (int i=0;i<h1tmp->GetNbinsX();i++){
        h1->SetBinContent(i+1,(double)h1tmp->GetBinContent(i+1)/(double)fRateCal_time*1000.);
    }
    h1tmp->Reset();
}

void MyMainFrame::DoUpdate() {
    for (int ch=0;ch<N_CH_PLOT;ch++){
        c1->cd(ch+1)->Modified();
        c1->cd(ch+1)->Update();
    }
    c2->cd(1)->Modified();
    c2->cd(1)->Update();
    c2->cd(2)->Modified();
    c2->cd(2)->Update();

    std::string res;
    res=execget("rcdaq_client daq_status -ll");
    DAQstatusTextDialogs->LoadBuffer(res.data());
}


void MyMainFrame::AssignChannel() {
    int padid=atoi(PadIdTent->GetText());
    int chid=atoi(ChIDTent->GetText());
    if (padid<=0||padid>N_CH_PLOT) return;
    if (chid>=MAX_N_CH) return;
    TH1F* h=(TH1F*) gDirectory->Get(Form("hadc%d",chid));
    c1->cd(padid);
    h->Draw();
    sprintf(buf,"Assigned channel spectrum %d to Pad %d",chid,padid);
    statusbar->SetText(buf,0);
}

void MyMainFrame::DoReset() {
    TH1F* hadc[MAX_N_CH];
    for (Int_t i=0;i<MAX_N_CH;i++){
        hadc[i]=(TH1F*) gDirectory->Get(Form("hadc%d",i));
        hadc[i]->Reset();
    }
    TH1F* h1=(TH1F*) gDirectory->Get("h1");
    h1->Reset();
    TH1F* h2=(TH1F*) gDirectory->Get("h2");
    h2->Reset();
    sprintf(buf,"Histograms reset");
    statusbar->SetText(buf,0);
}

void MyMainFrame::StopOnline() {
    gROOT->ProcessLine("pstop()");
    sprintf(buf,"Stopped ONLINE");
    statusbar->SetText(buf,0);
}
void MyMainFrame::ResumeOnline(){
    gROOT->ProcessLine("pstart()");
    sprintf(buf,"Resumed ONLINE");
    statusbar->SetText(buf,0);
}

void MyMainFrame::DoBeginDAQ() {
    std::string res=execget("rcdaq_client daq_begin");
    statusbar->SetText(res.data(),0);
}
void MyMainFrame::DoEndDAQ() {
    std::string res=execget("rcdaq_client daq_end");
    statusbar->SetText(res.data(),0);
}
void MyMainFrame::DoOpenDAQ(){
    std::string res=execget("rcdaq_client daq_open");
    sprintf(buf,"DAQ configured");
    statusbar->SetText(buf,0);
}
void MyMainFrame::DoDAQConfig() {
    std::string res=execget("./daqconfig1.sh");
    sprintf(buf,"DAQ configured");
    statusbar->SetText(buf,0);
    statusbar->SetText(res.data(),0);
}

void MyMainFrame::SetTimer() {
    fupdate_time=atoi(autoupdateentry->GetText());        
    timer->SetTime(fupdate_time);
    sprintf(buf,"Set auto update every %d ms",(int)fupdate_time);
    statusbar->SetText(buf,0);
}

void MyMainFrame::SetRateCalTime() {
    fRateCal_time=atoi(ratecalentry->GetText());
    timer2->SetTime(fRateCal_time);
    sprintf(buf,"Set Time for Rate calculator of %d ms",(int)fRateCal_time);
    statusbar->SetText(buf,0);
}
