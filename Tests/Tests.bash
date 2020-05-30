#!/bin/bash

# Check parameters
if [ "$1" = "--slow" ]
then
	Is_Slow_Grids_Enabled=1
elif [ -n "$1" ]
then
	Result_File_Name="$1"
fi

Processors_Count=$(cat /proc/cpuinfo | grep processor | wc -l)
Program="../Parallel_Sudoku_Solver ${Processors_Count}"

# Start result file with useful system information
if [ -n "$Result_File_Name" ]
then
	printf "######################################################################\n" > "$Result_File_Name"
	printf "# Test details                                                       #\n" >> "$Result_File_Name"
	printf "######################################################################\n" >> "$Result_File_Name"
	printf "Parallel Sudoku Solver version : $(git log --format=%H -1)\n" >> "$Result_File_Name"
	printf "Starting date : $(date)\n\n" >> "$Result_File_Name"

	printf "######################################################################\n" >> "$Result_File_Name"
	printf "# Processor details                                                  #\n" >> "$Result_File_Name"
	printf "######################################################################\n" >> "$Result_File_Name"
	printf "Cores count : ${Processors_Count}\n\n" >> "$Result_File_Name"
	printf "$(cat /proc/cpuinfo)\n\n" >> "$Result_File_Name"

	printf "######################################################################\n" >> "$Result_File_Name"
	printf "# System details                                                     #\n" >> "$Result_File_Name"
	printf "######################################################################\n" >> "$Result_File_Name"
	printf "$(lsb_release -a)\n\n" >> "$Result_File_Name"

	printf "######################################################################\n" >> "$Result_File_Name"
	printf "# Starting tests                                                     #\n" >> "$Result_File_Name"
	printf "######################################################################\n" >> "$Result_File_Name"
fi

function SolveList
{
	for File in $Files_List
	do
		# Append test result to result file if present
		if [ -n "$Result_File_Name" ]
		then
			$Program $File >> "$Result_File_Name"
			if [ $? != 0 ]
			then
				printf "!!!!!!!!!!!!!\n" >> "$Result_File_Name"
				printf "!! FAILURE !!\n" >> "$Result_File_Name"
				printf "!!!!!!!!!!!!!\n" >> "$Result_File_Name"
				exit
			fi
		else
			$Program $File
			if [ $? != 0 ]
			then
				printf "\033[31m!!!!!!!!!!!!!\n"
				printf "!! FAILURE !!\n"
				printf "!!!!!!!!!!!!!\033[0m\n"
				exit
			fi
		fi
	done
}

# Solve all 6x6 grids
Files_List=$(find 6x6_*.txt)
SolveList

# Solve all 9x9 grids
Files_List=$(find 9x9_*.txt)
SolveList

# Solve all 12x12 grids
Files_List=$(find 12x12_*.txt)
SolveList

# Solve all 16x16 grids
Files_List=$(find 16x16_*.txt)
SolveList

# Solve slow grids if enabled
if [ "$Is_Slow_Grids_Enabled" = "1" ]
then
	Files_List=$(find Slow_*.txt)
	SolveList
fi

if [ -n "$Result_File_Name" ]
then
	printf "#########################################\n" >> "$Result_File_Name"
	printf "## SUCCESS : all tests were successful ##\n" >> "$Result_File_Name"
	printf "#########################################\n" >> "$Result_File_Name"
else
	printf "\033[32m#########################################\n"
	printf "## SUCCESS : all tests were successful ##\n"
	printf "#########################################\033[0m\n"
fi
