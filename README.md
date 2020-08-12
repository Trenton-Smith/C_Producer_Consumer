# CSC 415 - Project 3 - Producer Consumer

## Student Name : Trenton Smith

## Build Instructions

1. Download the repo

- Open the terminal
- Navigate to where you would like to save the repo by typing "cd" followed by a space and the directory, i.e. "cd ~/repos" 
- Clone the repo to that directory by typing "git clone https://github.com/csc415-01-SU2020/csc415-p3-Trenton-Smith.git"

2. Build the project through command line

- Navigate to the cloned repo, i.e. "cd ~/csc415-p3-Trenton-Smith"
- Type "make" which will run the file to build the executable file "pandc"

## Run Instructions

1. Run the program through command line

- If not already, open the terminal and navigate to the repo through the command line, i.e. "cd ~/csc415-p3-Trenton-Smith"
- Type "./pandc" followed by the following arguments:<br>
	N: Number of Buffers<br>
	P: Number of Producers<br>
	C: Number of Consumers<br>
	X: Number of items each Producer will create<br>
	Ptime: Time (seconds) that each Producer will sleep<br>
	Ctime: Time (seconds) that each Consumer will sleep
	
- Full example: "./pandc 7 5 3 16 1 1"
