#!/bin/bash -e
#---------------------------------------------------------------
#   Script: launch.sh
#  Mission: alpha_heron
#   Author: Mike Benjamin
#   LastEd: 2021-Jun-08
#---------------------------------------------------------------
#  Part 1: Set global var defaults
#---------------------------------------------------------------
ME=`basename "$0"`
TIME_WARP=1
JUST_MAKE=""
VERBOSE=""
VNAME1="henry"
VLAUNCH_ARGS=" --auto --sim --vname=$VNAME1 --index=1 --start=0,-10,180 "

#---------------------------------------------------------------
#  Part 2: Check for and handle command-line arguments
#---------------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
	echo "$ME [SWITCHES] [time_warp]                                 "
	echo "  --help, -h         Show this help message                "
	echo "  --just_make, -j    Just make targ files, no launch       "
	echo "  --verbose, -v      Verbose output, confirm before launch "
	echo "                                                           "
	echo "  --east, -e         Eastern survey zone                   "
	echo "  --west, -w         Wester survey zone                    "
	echo "                                                           "
	exit 0
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_make" -o "${ARGI}" = "-j" ]; then
	JUST_MAKE="-j"
    elif [ "${ARGI}" = "--verbose" -o "${ARGI}" = "-v" ]; then
	VERBOSE="--verbose"
    elif [ "${ARGI}" = "--east" -o "${ARGI}" = "-e" ]; then
	VLAUNCH_ARGS+=" $ARGI"
    elif [ "${ARGI}" = "--west" -o "${ARGI}" = "-w" ]; then
	VLAUNCH_ARGS+=" $ARGI"
    else
        echo "$ME: Bad arg:" $ARGI "Exit Code 1."
        exit 1
    fi
done

#---------------------------------------------------------------
#  Part 3: Initialize and Launch the vehicles
#---------------------------------------------------------------
echo "$ME: Launching $VNAME1 ..."
./launch_vehicle.sh $VLAUNCH_ARGS $VERBOSE $JUST_MAKE $TIME_WARP

#---------------------------------------------------------------
#  Part 4: Launch the shoreside
#---------------------------------------------------------------
echo "$ME: Launching Shoreside ..."
./launch_shoreside.sh --auto $VERBOSE $JUST_MAKE $TIME_WARP

#---------------------------------------------------------------
# Part 5: Launch uMAC until the mission is quit
#---------------------------------------------------------------
uMAC targ_shoreside.moos
kill -- -$$

