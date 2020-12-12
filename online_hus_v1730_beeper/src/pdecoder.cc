
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include <TSystem.h>
#include <TROOT.h>
#include "pdecoder.h"
#include <bitset>
//#include <DataStruct_dict.C>



#define V1730_EVENT_TYPE 1

#define V1730_HDR 6
#define V1730_N_CH 8
#define V1730_MAX_N_CH 8
#define V1730_PACKET 52
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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define N_REFESH_WF 1000
#define N_MAX_WF_LENGTH 500

using namespace std;

#include <stdint.h>

#define N_EVENT_BEEP 500

int init_done = 0;
int n_v1730;
v1730event v1730evt;
int event_counter_prev;

char myfifo[500];
int fifobuf;

void addfifo(int fifobufin)
{
    fifobuf=fifobufin;
    int fd = open(myfifo, O_WRONLY|O_NONBLOCK);
    write(fd, &fifobuf, sizeof(fifobuf));
    //printf("Send = %d\n",fifobuf);
}

int pinit()
{
  if (init_done) return 1;
  init_done = 1;
  gROOT->ProcessLine(".L libDataStruct.so");
  n_v1730=0;

  //! fifo for interprocess communication
  sprintf(myfifo,"/tmp/myfifo");
  mkfifo(myfifo,0666);
  fifobuf=0;

  event_counter_prev=0;
  return 0;
}

int process_event (Event * e)
{


    //! v1740 packet
    Packet *p1730=e->getPacket(V1730_PACKET);
    if (p1730)
    {
//        usleep(1000000);
        n_v1730++;
        int* tmp;
        int* words;
        words=(int*) p1730->getIntArray(tmp);
        int nevt=p1730->getPadding();

//        cout<<"\nProcessing events block"<<n_v1730<<" Number of Events="<<nevt<<endl;
        int ipos=0;
        for (int evt=0;evt<nevt;evt++){
            v1730evt.Clear();
            v1730evt.decode(words,ipos);
        }//end of event loop

        for (int evt=0;evt<nevt;evt++){
            v1730evt.Clear();
            v1730evt.decode(words,ipos);
//            v1730evt.Print();
            int ElapsedCounts=v1730evt.event_counter-event_counter_prev;
            if (ElapsedCounts>N_EVENT_BEEP){
                cout<<"send beep"<<v1730evt.event_counter<<endl;
                addfifo(v1730evt.event_counter);
                event_counter_prev=v1730evt.event_counter;
            }
            ipos+=4;
            //calculate sample length first
            int nch=0;
            for (int ch=0;ch<V1730_MAX_N_CH;ch++){
                if (!(v1730evt.channel_mask & (1 << ch))) continue;
                nch++;
            }
            int nsamples_per_ch = (v1730evt.event_size-4)*2/nch;
//            cout<<"nsamples_per_ch = "<<nsamples_per_ch<<endl;
            //loop all channel with given sample length
            for (int ch=0;ch<V1730_MAX_N_CH;ch++){
                if (!(v1730evt.channel_mask & (1 << ch))) continue;
                for (int i=0;i<nsamples_per_ch/2;i++){
                    ipos++;
                }
            }//end of channel loop
        }//end of event loop
        delete p1730;
    }//end of packet loop
    return 0;
}


int pclose(){
    cout<<"Finish!"<<endl;
    return 0;
}

