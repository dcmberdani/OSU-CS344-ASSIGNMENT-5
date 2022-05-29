Assignment: One-Time Pads (4)
Class: CS 344
Name: Daniel Mendes (mendesd@oregonstate.edu)
ID: 933827213
Date: 5/28/22

COMPILING:
	The program archive contains a makefile
	Simply type 'make' into the terminal after extracting the archive to compile the program
	The 5 generated executables are titled: 
		'keygen'
		'enc_client'
		'dec_client'
		'enc_server'
		'dec_server'

EXECUTING/CLEANUP:
	Keygen is executed by typing:
		'./keygen [keylength]'
		
	The clients are executed by typing:
       		'./[exe] ./[plain/ciphertext] ./key [port #]'
		The correct server must be running on the given port for the client to connect

	The servers are executed by typing:
       		'./[exe] [port #] &'
		They should be run in the background to allow for the clients to actually connect
		They can't close on their own, so use signals to terminate them when finished testing

	A script is provided to execute the program as well. To use it, type:
		'bash ./p5testscript [randport1] [randport2]'

	In order to remove the executable/object files from the directory, type 'make clean'
