use_hls_lib "lab7_interface"
use_hls_lib "lab7_memory"
#*******************************************************************************
# Author:      111033238
# Date:        05/12/2026
# Description: Median + Scharr pipeline using grey_line_buffer
#*******************************************************************************

set LIB_PATH "[get_install_path]/share/stratus/techlibs/GPDK045/gsclib045_svt_v4.4/gsclib045/timing"
set LIB_LEAF "slow_vdd1v2_basicCells.lib"
use_tech_lib "$LIB_PATH/$LIB_LEAF"

set_attr clock_period   10.0
set_attr message_detail 3
set_attr flatten_arrays all

set CLOCK_PERIOD 10.0

# Default image size (edit to switch)
set WIDTH  256
set HEIGHT 256

set_attr cc_options "-DCLOCK_PERIOD=$CLOCK_PERIOD -DIMAGE_WIDTH=$WIDTH -DIMAGE_HEIGHT=$HEIGHT -g"
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
define_hls_module MedianFilter ../MedianFilter.cpp
define_hls_module ScharrFilter ../ScharrFilter.cpp

############################################################
# HLS configurations
############################################################
foreach mod { MedianFilter ScharrFilter } {
  define_hls_config $mod BASIC
  define_hls_config $mod DPA -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod UNROLL -DUNROLL -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE1 -DPIPELINE -DII=1 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE2 -DPIPELINE -DII=2 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE4 -DPIPELINE -DII=4 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
  define_hls_config $mod PIPE_UNROLL -DUNROLL -DPIPELINE -DII=2 -DDPOPT_ALL -DLAT=HLS_ACHIEVABLE
}

############################################################
# Simulation configurations
############################################################
set IMAGE_DIR "../img"
set IN_LENA   "${IMAGE_DIR}/lena_color_256.bmp"
set OUT_LENA  "out_lena_color_256.bmp"
set IN_CAM    "${IMAGE_DIR}/cameraman.bmp"
set OUT_CAM   "out_cameraman.bmp"

define_sim_config B \
  -argv "$IN_LENA $OUT_LENA"

define_sim_config B_cam \
  -argv "$IN_CAM $OUT_CAM"

foreach cfg { BASIC DPA UNROLL PIPE1 PIPE2 PIPE4 PIPE_UNROLL} {
    define_sim_config V_${cfg} \
        "MedianFilter RTL_V ${cfg}" \
        "ScharrFilter RTL_V ${cfg}" \
        -argv "$IN_LENA $OUT_LENA"

    define_sim_config V_${cfg}_cam \
        "MedianFilter RTL_V ${cfg}" \
        "ScharrFilter RTL_V ${cfg}" \
        -argv "$IN_CAM $OUT_CAM"
}
