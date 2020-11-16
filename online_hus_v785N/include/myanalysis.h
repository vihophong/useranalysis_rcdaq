#ifndef __MYANALYSIS_H__
#define __MYANALYSIS_H__

#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>

#include <TQObject.h>
#include <RQ_OBJECT.h>
#include <TH1.h>

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class MyMainFrame;

int process_event (Event *e); //++CINT
int pstop();

class MyMainFrame {
   RQ_OBJECT("MyMainFrame")
private:
   TGMainFrame         *fMain;
   TRootEmbeddedCanvas *fEcanvas;
public:
   MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
   virtual ~MyMainFrame();
   void DoReset();
   void DoOpenDAQ();
   void DoBeginDAQ();
   void DoEndDAQ();
   void DoDAQConfig();
   void StopOnline();
   void ResumeOnline();
   ClassDef (MyMainFrame,0);
};




#endif /* __MYANALYSIS_H__ */
