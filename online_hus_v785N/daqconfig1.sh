#add to begin of run event
rcdaq_client daq_define_runtype beam  /home/dnp/Public/phong/daqconfig/beam/beamrun%05d.evt
rcdaq_client daq_define_runtype preparation  /home/dnp/Public/phong/daqconfig/preparation/prerun%05d.evt

#rcdaq_client create_device device_file 9 900 /home/NiGIRI/rcdaqnew-zeroSP/install/bin/temp_run_description.txt
#rcdaq_client create_device device_file 9 910 /home/NiGIRI/webcam/euricacam.jpg 25600
#rcdaq_client create_device device_file 9 945 /home/NiGIRI/brIKEN2017/daqconfig/m1740_0.txt
#rcdaq_client create_device device_file 9 946 /home/NiGIRI/brIKEN2017/daqconfig/m1740_1.txt
#rcdaq_client create_device device_file 9 947 /home/NiGIRI/brIKEN2017/daqconfig/m1740_2.txt
#rcdaq_client create_device device_file 9 948 /home/NiGIRI/brIKEN2017/daqconfig/m1740_3.txt

#load caen plugin
rcdaq_client load librcdaqplugin_CAENdrs.so

#loading lupo
rcdaq_client create_device device_CAENdrs 1 10 1 1

#loading caen1740
#./loadcaensingle.sh 100 0
#./loadcaensingle.sh 101 0
#./loadcaensingle.sh 102 0
#./loadcaensingle.sh 103 0

#set run type
rcdaq_client daq_set_runtype preparation

#reset time stamp
#resettimestamp_briken.sh

#webcontrol (optional)
#rcdaq_client daq_webcontrol 2222
