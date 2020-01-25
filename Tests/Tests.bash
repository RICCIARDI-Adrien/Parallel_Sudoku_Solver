#!/bin/bash

Processors_Count=$(cat /proc/cpuinfo | grep processor | wc -l)
Program="../Parallel_Sudoku_Solver ${Processors_Count}"

# Start result file with useful system information
if [ -n "$1" ]
then
	printf "######################################################################\n" > "$1"
	printf "# Test details                                                       #\n" >> "$1"
	printf "######################################################################\n" >> "$1"
	printf "Parallel Sudoku Solver version : $(git log --format=%H -1)\n" >> "$1"
	printf "Starting date : $(date)\n\n" >> "$1"

	printf "######################################################################\n" >> "$1"
	printf "# Processor details                                                  #\n" >> "$1"
	printf "######################################################################\n" >> "$1"
	printf "Cores count : ${Processors_Count}\n\n" >> "$1"
	printf "$(cat /proc/cpuinfo)\n\n" >> "$1"

	printf "######################################################################\n" >> "$1"
	printf "# System details                                                     #\n" >> "$1"
	printf "######################################################################\n" >> "$1"
	printf "$(lsb_release -a)\n" >> "$1"
fi

function PrintFailure
{
	echo -e "\033[31m!!!!!!!!!!!!!"
	echo -e "!! FAILURE !!"
	echo -e "!!!!!!!!!!!!!\033[0m"
}

function PrintSuccess
{
	echo -e "\033[32m#########################################"
	echo -e "## SUCCESS : all tests were successful ##"
	echo -e "#########################################\033[0m"
}

function SolveList
{
	for File in $Files_List
	do
		$Program $File
		if [ $? != 0 ]
		then
			PrintFailure
			exit
		fi
	done
}

# Solve all 6x6 grids
Files_List=`find 6x6_*.txt`
SolveList

# Solve all 9x9 grids
Files_List=`find 9x9_*.txt`
SolveList

# Solve all 12x12 grids
Files_List=`find 12x12_*.txt`
SolveList

# Solve all 16x16 grids
Files_List=`find 16x16_*.txt`
SolveList

PrintSuccess
