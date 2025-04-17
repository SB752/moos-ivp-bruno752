#!/bin/bash 
#-------------------------------------------------------------- 
#   Script: launch.sh    
#   Author: Michael Benjamin   
#   LastEd: April 2024
#-------------------------------------------------------------- 
#  Part 1: Define a convenience function for producing terminal
#          debugging/status output depending on the verbosity.
#-------------------------------------------------------------- 
vecho() { if [ "$VERBOSE" != "" ]; then echo "$ME: $1"; fi }

#-------------------------------------------------------------- 
#  Part 2: Set Global variables
#-------------------------------------------------------------- 
ME=`basename "$0"`
TIME_WARP=1
JUST_MAKE=""
VERBOSE=""
CMD_ARGS=""
RANDSTART="false"
VLAUNCH_ARGS="--auto --sim "
SLAUNCH_ARGS="--auto "

XLAUNCHED="no"
SWIM_REGION="60,10:-30.3602,-32.8374:-4.6578,-87.0535:85.7024,-44.2161"
SWIMMERS=15
UNREGERS=0
#VROLES=("rescue" "rescue")
VROLES=("fixed")
VROLES_LIST="fixed"

#-------------------------------------------------------
#  Part 3: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    CMD_ARGS+="${ARGI} "
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
	echo "$ME: [OPTIONS] [time_warp]                          "
	echo "                                                    "
	echo "Options:                                            "
        echo "  --help, -h                                        "
        echo "    Show this help message                          "
        echo "  --verbose, -v                                     "
        echo "    Increase verbosity, confirm before launch.      "
	echo "  --just_make, -j                                   " 
	echo "    Just make the targ files, but do not launch.    " 
	echo "  --rand                                            " 
	echo "    Randomly generate vpositions.txt.               "
	echo "    Randomly generate swim_file if not provided     "
	echo "                                                    "
	echo "  --rescue, -r                                      " 
	echo "    Only one rescue vehicle                         "
	echo "  --rescue-scout, -rs                               " 
	echo "    Launch one rescue vehicle, one scout vehicle    "
	echo "    as teammates                                    "
	echo "  --rescue-scout2, -rs2                             " 
	echo "    Launch two rescue vehicles, two scout vehicles  "
	echo "                                                    "
	echo "  --pav60                                           " 
	echo "    Use mit small region for random gen of swimmers "
	echo "  --pav90                                           "
	echo "    Use mit big region for random gen of swimmers   "
	echo "  --swim_file=<mit_00.txt>                          " 
	echo "    Set the swim file.                              "
	echo "  --swimmers=<15>                                   " 
	echo "    Num registered swimmers randomly placed         "
	echo "  --unreg=<0>                                       " 
	echo "    Num UNregistered swimmers randomly placed       "
	echo "                                                    "
	echo "  -1 :  Short for --swim_file=mit_01.txt            "
	echo "  -2 :  Short for --swim_file=mit_02.txt            "
	echo "  -3 :  Short for --swim_file=mit_03.txt            "
	echo "  -4 :  Short for --swim_file=mit_04.txt            "
	echo "  -5 :  Short for --swim_file=mit_05.txt            "
	echo "  -6 :  Short for --swim_file=mit_06.txt            "
	exit 0;
    elif [ "${ARGI}" = "--verbose" -o "${ARGI}" = "-v" ]; then
	VERBOSE="--verbose"
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_make" -o "${ARGI}" = "-j" ]; then
	JUST_MAKE="-j"
    elif [ "${ARGI}" = "--rand" ]; then
	RANDSTART="true"
    elif [ "${ARGI}" = "--nogui" -o "${ARGI}" = "-n" ]; then
        SLAUNCH_ARGS+=" --nogui "
    elif [ "${ARGI}" = "--auto" ]; then
        XLAUNCHED="yes"

    elif [ "${ARGI:0:11}" = "--max_time=" ]; then
        SLAUNCH_ARGS+=" ${ARGI}"

    elif [ "${ARGI}" = "--rescue" -o "${ARGI}" = "-r" ]; then
	VROLES=("rescue")
	VROLES_LIST="rescue"
    elif [ "${ARGI}" = "--rescue-rescue" -o "${ARGI}" = "-rr" ]; then
	VROLES=("rescue" "rescue")
	VROLES_LIST="rescue,rescue"
    elif [ "${ARGI}" = "--rescue-scout" -o "${ARGI}" = "-rs" ]; then
	VROLES=("rescue" "scout")
	VROLES_LIST="rescue,scout"
	TMATES=("abe" "abe")
    elif [ "${ARGI}" = "--rescue-scout2" -o "${ARGI}" = "-rs2" ]; then
	VROLES=("rescue" "rescue" "scout" "scout")
	VROLES_LIST="rescue,rescue,scout,scout"
	TMATES=("abe" "ben" "abe" "ben")
    elif [ "${ARGI}" = "--mit_small" -o "${ARGI}" = "--pav60" ]; then
	SWIM_REGION="60,10:-30.3602,-32.8374:-4.6578,-87.0535:85.7024,-44.2161"
	RANDSTART="true"
    elif [ "${ARGI}" = "--mit_big" -o "${ARGI}" = "--pav90" ]; then
	SWIM_REGION="60,10:-75.5402,-54.2561:-36.9866,-135.58:98.5536,-71.3241"
	RANDSTART="true"
    elif [ "${ARGI:0:11}" = "--swimmers=" ]; then
        SWIMMERS="${ARGI#--swimmers=*}"
    elif [ "${ARGI:0:8}" = "--unreg=" ]; then
        UNREGERS="${ARGI#--unreg=*}"

    elif [ "${ARGI:0:12}" = "--swim_file=" ]; then
        SLAUNCH_ARGS+=" ${ARGI}"
    elif [ "${ARGI}" = "-1" -o "${ARGI}" = "-2" ]; then
        SLAUNCH_ARGS+=" ${ARGI}"
    elif [ "${ARGI}" = "-3" -o "${ARGI}" = "-4" ]; then
        SLAUNCH_ARGS+=" ${ARGI}"
    elif [ "${ARGI}" = "-5" -o "${ARGI}" = "-6" ]; then
        SLAUNCH_ARGS+=" ${ARGI}"
    else 
	echo "$ME: Bad Arg: $ARGI. Exit Code 1."
	exit 1
    fi
done

AMT=${#VROLES[@]}

#---------------------------------------------------------------
#  Part 4: If verbose, show vars and confirm before launching
#---------------------------------------------------------------
if [ "${VERBOSE}" != "" ]; then 
    echo "=================================="
    echo "  launch.sh SUMMARY               "
    echo "=================================="
    echo "$ME"
    echo "CMD_ARGS =      [${CMD_ARGS}]     "
    echo "TIME_WARP =     [${TIME_WARP}]    "
    echo "JUST_MAKE =     [${JUST_MAKE}]    "
    echo "----------------------------------"
    echo "SWIM_REGION =   [${SWIM_REGION}]  "
    echo "SWIMMERS =      [${SWIMMERS}]     "
    echo "UNREGERS =      [${UNREGERS}]     "
    echo "VROLES_LIST =   [${VROLES_LIST}]  "
    echo "RAND_START =    [${RANDSTART}]    "
    echo -n "Hit any key to continue launch "
    read ANSWER
fi

#-------------------------------------------------------------
# Part 5: Generate random starting positions
#-------------------------------------------------------------
vecho "Picking starting position and colors"

if [ "${RANDSTART}" = "true" -o  ! -f "vpositions.txt" ]; then
    pickpos --poly="-2,-8 : 4,-13 : 60,13 : 57,18" --buffer=20  \
	    --amt=$AMT  > vpositions.txt  
fi

# generate randomly placed swimmers
if [ "${RANDSTART}" = "true" -o  ! -f "mit_00.txt" ]; then
    gen_swimmers --poly=$SWIM_REGION --swimmers=$SWIMMERS   \
		 --unreg=$UNREGERS --sep=7 > mit_00.txt
fi

VEHPOS=(`cat vpositions.txt`)
VCOLORS=("yellow" "red" "blue" "green")
VNAMES=("abe" "ben" "cal" "deb")

#-------------------------------------------------------------
# Part 6: Launch the vehicles
#-------------------------------------------------------------
ALL_VNAMES=""
for INDEX in `seq 1 $AMT`;
do
    sleep 0.2
    ARRAY_INDEX=`expr $INDEX - 1`
    START=${VEHPOS[$ARRAY_INDEX]}
    VNAME=${VNAMES[$ARRAY_INDEX]}
    COLOR=${VCOLORS[$ARRAY_INDEX]}
    VROLE=${VROLES[$ARRAY_INDEX]}
    TMATE=${TMATES[$ARRAY_INDEX]}
    
    if [ "${ALL_VNAMES}" != "" ]; then
        ALL_VNAMES+=":"
    fi
    ALL_VNAMES+=$VNAME

    MOOS_PORT=`expr $INDEX + 9000`
    PSHARE_PORT=`expr $INDEX + 9200`
    
    IX_VLAUNCH_ARGS="$VLAUNCH_ARGS $TIME_WARP $VERBOSE $JUST_MAKE "
    if [ "${VROLE}" = "scout" -a "${VNAME}" != "${TMATE}" ]; then
        IX_VLAUNCH_ARGS+=" --tmate=${TMATE} "
    fi
    IX_VLAUNCH_ARGS+=" --start=$START --color=$COLOR "
    IX_VLAUNCH_ARGS+=" --mport=$MOOS_PORT --pshare=$PSHARE_PORT "
    IX_VLAUNCH_ARGS+=" --vname=$VNAME --vrole=$VROLE"

    vecho "Launching: $VNAME"
    vecho "IX_VLAUNCH_ARGS: [$IX_VLAUNCH_ARGS]"

    ./launch_vehicle.sh $IX_VLAUNCH_ARGS 
done

#-------------------------------------------------------------
# Part 7: Launch the Shoreside mission file
#-------------------------------------------------------------
SLAUNCH_ARGS+=" $JUST_MAKE $VERBOSE $TIME_WARP "
vecho "Launching the shoreside. Args: $SLAUNCH_ARGS $TIME_WARP"

./launch_shoreside.sh $SLAUNCH_ARGS 

if [ "${JUST_MAKE}" != "" ]; then
    echo "$ME: Files assembled; nothing launched; exiting per request."
    exit 0
fi

#-------------------------------------------------------------- 
#  Part 8: Unless auto-launched, launch uMAC until mission quit          
#-------------------------------------------------------------- 
if [ "${XLAUNCHED}" = "no" ]; then
    uMAC targ_shoreside.moos 
    kill -- -$$
fi

exit 0
