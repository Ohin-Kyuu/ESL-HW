use_hls_lib "memlib"
use_hls_lib "cynw_sr_mem"

#*******************************************************************************
# Author:      111033238
# Date:        06/06/2026
# Description: Hamming Matching ACC for ORB Descriptor with SRAM 
#*******************************************************************************

set LIB_PATH "[get_install_path]/share/stratus/techlibs/GPDK045/gsclib045_svt_v4.4/gsclib045/timing"
set LIB_LEAF "slow_vdd1v2_basicCells.lib"
use_tech_lib "$LIB_PATH/$LIB_LEAF"

set_attr clock_period   10.0
set_attr message_detail 3
set_attr flatten_arrays all

set CLOCK_PERIOD 10.0

set_attr cc_options "-DCLOCK_PERIOD=$CLOCK_PERIOD -g"
set_attr end_of_sim_command "make saySimPassed"

############################################################
# Testbench / system files
############################################################
define_system_module ../main.cpp
define_system_module ../Testbench.cpp
define_system_module ../System.cpp
############################################################
# HLS modules
############################################################
define_hls_module HammingMatch ../HammingMatch.cpp

############################################################
# HLS configurations
############################################################

foreach mod { HammingMatch } {
  define_hls_config $mod BASIC
  define_hls_config $mod DPO -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod UNROLL -DUNROLL -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE1 -DPIPELINE -DII=1 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE2 -DPIPELINE -DII=2 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE4 -DPIPELINE -DII=4 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE_UNROLL -DUNROLL -DPIPELINE -DII=2 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
}

############################################################
# Simulation configurations
############################################################
set DATA_DIR "../data"
set IN_QUERY  "${DATA_DIR}/query.txt"
set IN_TRAIN  "${DATA_DIR}/train.txt"

define_sim_config B \
  -argv "$IN_TRAIN $IN_QUERY"

foreach cfg { BASIC DPO UNROLL PIPE1 PIPE2 PIPE4 PIPE_UNROLL} {
    define_sim_config V_${cfg} \
        "HammingMatch RTL_V ${cfg}" \
        -argv "$IN_TRAIN $IN_QUERY"
}
