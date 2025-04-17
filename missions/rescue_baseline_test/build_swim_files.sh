#!/bin/bash 
#-------------------------------------------------------------- 
#   Script: build_swim_files.sh  
#   Author: Michael Benjamin   
#   LastEd: April 2023
#-------------------------------------------------------------- 
#  Part 1: Define a convenience function for producing terminal
#          debugging/status output depending on the verbosity.
#-------------------------------------------------------------- 
vecho() { if [ "$VERBOSE" != "" ]; then echo "$ME: $1"; fi }


#-------------------------------------------------------------- 
#  Part 2: Set Global variables
#-------------------------------------------------------------- 
PAV_60="60,10:-30.3602,-32.8374:-4.6578,-87.0535:85.7024,-44.2161"
PAV_90="60,10:-75.5402,-54.2561:-36.9866,-135.58:98.5536,-71.3241"

VALL="yes"
V60G="no"
V60X="no"
V90G="no"
V90X="no"

#-------------------------------------------------------
#  Part 3: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ]; then
	echo "$ME: [OPTIONS] [time_warp]                          "
	echo "                                                    "
	echo "Options:                                            "
        echo "  --help, -h                                        "
        echo "    Display this help message                       "
	echo "                                                    "
	echo "  --60g                                             " 
	echo "    variant: pav60 region, no unreg swimmers        "
	echo "  --60x                                             " 
	echo "    variant: pav60 region, with unreg swimmers      "
	echo "  --90g                                             " 
	echo "    variant: pav90 region, no unreg swimmers        "
	echo "  --90x                                             " 
	echo "    variant: pav90 region, with unreg swimmers      "
	echo "                                                    "
	exit 0;
    elif [ "${ARGI}" = "--verbose" -o "${ARGI}" = "-v" ]; then
	VERBOSE="--verbose"
    elif [ "${ARGI}" = "--clean" -o "${ARGI}" = "-c" ]; then
	rm -f mit_60* mit_90*
	exit 0
    elif [ "${ARGI}" = "--60g" ]; then
	V60G="yes"; VALL="no"
    elif [ "${ARGI}" = "--60x" ]; then
	V60X="yes"; VALL="no"
    elif [ "${ARGI}" = "--90g" ]; then
	V90G="yes"; VALL="no"
    elif [ "${ARGI}" = "--90x" ]; then
	V90X="yes"; VALL="no"
    else 
	echo "$ME: Bad Arg: $ARGI. Exit Code 1."
	exit 1
    fi
done


#-------------------------------------------------------
#  Part 4: Build the swim files
#-------------------------------------------------------
#
# Pav60 Missions
# ==============
# Reg only:   good for testing 1-1 missions
# Reg/UnReg:  good for testing 2-0 missions

# Pav90 Missions
# ==============
# Reg only:   marginal: 1-1 mission
# Reg/Unreg:  2-2 missions
# 
# mit_60_01.txt            7, 15(3), 21
# mit_60_unreg_01.txt      7/7, 5/11(3), 17/5
# mit_90_01.txt            11, 21(3), 29
# mit_90_unreg_01.txt      9/9, 11/15(3), 21/7

if [ "${VALL}" = "yes" -o "${V60G}" = "yes" ]; then 
    gen_swimmers --poly=$PAV_60 --swimmers=7  --sep=7  > mit_60g_00.txt
    gen_swimmers --poly=$PAV_60 --swimmers=15 --sep=7  > mit_60g_01.txt
    gen_swimmers --poly=$PAV_60 --swimmers=15 --sep=15 > mit_60g_02.txt
    gen_swimmers --poly=$PAV_60 --swimmers=15 --sep=7  > mit_60g_03.txt
    gen_swimmers --poly=$PAV_60 --swimmers=21 --sep=7  > mit_60g_04.txt
fi
 
if [ "${VALL}" = "yes" -o "${V60X}" = "yes" ]; then 
    gen_swimmers --poly=$PAV_60 --swimmers=7  --unreg=7  --sep=7  > mit_60x_00.txt
    gen_swimmers --poly=$PAV_60 --swimmers=5  --unreg=11 --sep=7  > mit_60x_01.txt
    gen_swimmers --poly=$PAV_60 --swimmers=5  --unreg=11 --sep=15 > mit_60x_02.txt
    gen_swimmers --poly=$PAV_60 --swimmers=5  --unreg=11 --sep=7  > mit_60x_03.txt
    gen_swimmers --poly=$PAV_60 --swimmers=17 --unreg=15 --sep=7  > mit_60x_04.txt
fi

if [ "${VALL}" = "yes" -o "${V90G}" = "yes" ]; then 
    gen_swimmers --poly=$PAV_90 --swimmers=11 --sep=10 > mit_90g_10.txt
    gen_swimmers --poly=$PAV_90 --swimmers=21 --sep=10 > mit_90g_11.txt
    gen_swimmers --poly=$PAV_90 --swimmers=21 --sep=15 > mit_90g_12.txt
    gen_swimmers --poly=$PAV_90 --swimmers=21 --sep=10 > mit_90g_13.txt
    gen_swimmers --poly=$PAV_90 --swimmers=29 --sep=8  > mit_90g_14.txt
fi

if [ "${VALL}" = "yes" -o "${V90G}" = "yes" ]; then 
    gen_swimmers --poly=$PAV_90 --swimmers=9  --unreg=9  --sep=10 > mit_90x_10.txt
    gen_swimmers --poly=$PAV_90 --swimmers=11 --unreg=15 --sep=10 > mit_90x_11.txt
    gen_swimmers --poly=$PAV_90 --swimmers=11 --unreg=15 --sep=15 > mit_90x_12.txt
    gen_swimmers --poly=$PAV_90 --swimmers=11 --unreg=15 --sep=10 > mit_90x_13.txt
    gen_swimmers --poly=$PAV_90 --swimmers=21 --unreg=21  --sep=8 > mit_90x_14.txt
fi
