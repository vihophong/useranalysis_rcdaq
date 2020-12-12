#ifndef __MYANALYSIS_H__
#define __MYANALYSIS_H__

#include <Event/Event.h>
#include <iostream>
#include <bitset>
using namespace std;

int process_event (Event *e); //++CINT 
int ppclose();


class v1730event : public TObject {
public:
  //! default constructor
  v1730event(){

  }
  virtual ~v1730event(){}
  void Clear(Option_t * = ""){
      event_size = -9999;
      board_id = -9999;
      board_fail_flag = -9999;
      event_format_reserved = -9999;
      pattern = -9999;
      channel_mask = -9999;
      event_counter = 0;
      trigger_time_tag = 0;
      trigger_time_tag_extended = 0;
  }
  void Print(Option_t *option = "") const {
      cout<<"************************"<<endl;
      cout<<"event_size = "<<event_size<<endl;
      cout<<"board_id = "<<board_id<<endl;
      cout<<"board_fail_flag = "<<board_fail_flag<<endl;
      cout<<"event_format_reserved = "<<event_format_reserved<<endl;
      cout<<"pattern = "<<pattern<<endl;
      cout<<"channel_mask = "<<std::bitset<16>(channel_mask)<<endl;
      cout<<"event_counter = "<<event_counter<<endl;
      cout<<"trigger_time_tag = "<<trigger_time_tag<<endl;
      cout<<"trigger_time_tag_extended = "<<trigger_time_tag_extended<<endl;
  }
  void decode(int* words,int ipos){
      event_size = words[ipos+0]&0xFFFFFFF;
      board_id = (words[ipos+1]&0xF8000000)>>27;
      board_fail_flag = (words[ipos+1]&0x4000000)>>26;
      event_format_reserved = (words[ipos+1]&0x1000000)>>24;
      pattern = (words[ipos+1]&0xFFFF00)>>8;
      int channel_mask_lsb = words[ipos+1]&0xFF;
      int channel_mask_msb = (words[ipos+2]&0xFF000000)>>24;
      channel_mask = (channel_mask_msb<<8)|channel_mask_lsb;
      event_counter = words[ipos+2]&0xFFFFFF;
      trigger_time_tag = words[ipos+3]&0xFFFFFFFF;
      trigger_time_tag_extended = ((unsigned long long)pattern)<<32|(unsigned long long)trigger_time_tag;
  }
  int event_size;
  int board_id;
  int board_fail_flag;
  int event_format_reserved;
  int pattern;
  int channel_mask;
  unsigned int event_counter;
  unsigned int trigger_time_tag;
  unsigned long long trigger_time_tag_extended;
};

#endif /* __MYANALYSIS_H__ */
