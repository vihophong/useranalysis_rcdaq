export ONLINE_MAIN=/home/dnp/Public/phong/install-rcdaq
export OFFLINE_MAIN=/home/dnp/Public/phong/install-rcdaq
export PATH=$PATH:/home/dnp/Public/phong/install-rcdaq/bin
export LD_LIBRARY_PATH=$ONLINE_MAIN/lib
source $ONLINE_MAIN/bin/aliases.sh
export ROOT_INCLUDE_PATH=$ONLINE_MAIN/include:$ONLINE_MAIN/include/Event:$ONLINE_MAIN/include/pmonitor
