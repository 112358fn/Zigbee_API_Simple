TESTS TOOLS
===========

*"run_test": Script to test Zigbee.c"
Dependencies:
 socat - Multipurpose relay (SOcket CAT)
 konsole - terminal emulator for KDE
Parameters: start - stop - restart

*"makefile": MAKEFILE to compile Zigbee.c
Parameters: clean - all

*"Zigbee.c": C Program to Test the library components

To Test your modifications:
===========================
1.Make the modification you wish to the library or to the test code.
2.See if it compiles with "make all".
3.If it does then launch the test script "run_test start".
4.You will see two terminals that simulates both ends of a serial communication.
5.Choose one the frames proposed and paste it on one terminal.
6.Hit ENTER to send it to the other terminal.
7.If everything went fine you should see the in the second terminal the procesed frame.