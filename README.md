# Parallel Sudoku Solver

## Description

A really simple solver using a backtrack algorithm to recursively solve the grid.  
Complex grids solving is faster than my previous [sequential sudoku solver](https://github.com/RICCIARDI-Adrien/Sudoku_Solver) because several grids can be searched in the same time on multicore processors.

## Building

Type `make` to build the program.

## Testing

Go to `Tests` directory and type `./Tests.bash`.

## Importing sudokus from [Sudoku Puzzles Online](https://www.sudoku-puzzles-online.com) website

* Choose a grid from the 16x16 sudoku ones.
* Click `Save the grid` button. This will download a file called `hexa.txt`.
* Run the following command to convert the file to Parallel Sudoku Solver format :
  ```
  cat hexa.txt | sed -e 's/ //g' | sed -e 's/*/./g' | sed -e '/^$/d' | head -n 16 > 16x16_x.txt
  ```
