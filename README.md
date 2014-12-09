tenvolt-chaser
==============

This project is a simple low cost programmable light chaser intended to be used 
as a component in a ten volt stage lighting system. It can control up to four
channels in a system by bumping the voltage up to ten volts in any pattern set
by the user. It has an onboard ten volt regulator and can be used as the sole 
controller in a system. 

Building and programming
------------------------

The code for the MSP430G2221 is written in C and intended to be compiled using
the msp430gcc compiler and tool set. The code uses the standard TI header files
so it may be compilable in Code Composer Studio, but this was not tested.

in addition, an rf2500 compatable MSP430 programming tool and mspdebug must be
used to program the chip. An MSP430G2 Launchpad was used in development.

To build, simply type: 
	
	make
	
To program, simply type

	make prog
	
Operation
---------

The chase pattern can be programmed using the bump switches on the board.
Press the program button to set the unit into program mode, then use the
bump switches to toggle the desired fixtures on or off. Pressing STARTSTOP
will save the pattern and advance to the next step. Repeat this process until
the desired pattern is programmed and then push the program button again.

The system can store up to 64 program steps and if this is reached, it will
save the pattern up to that many, then go back into execute mode.

In execute mode, the heartbeat led will pulse at the current tempo. This can
be modified by tapping the TAP button at the proper tempo. New settings take
effect immediately and can be changed on the fly.

In execute mode, the STARTSTOP button pauses and unpauses the chase routine,
flashing lights according to the program set in program mode.

Details
-------

The system requires at last 12vdc power to operate, however it is not 
recommended to provide much more than this, as heat may become an issue in 
the ten volt regulator as well as the 3.3 volt digital system voltage 
regulator.

There is a ten volt signal input and output on the board. Signals coming in
to the board from the input connector are ORed with the bump signals from the
microcontroller through a diode. There is about .2 to .7 volts loss depending
on the diode used which may affect the maximum light intensity avaliable to 
controllers before the chaser board, however this is most likely unnoticable.

There is an onboard ten volt regualtor which makes it possible to use this 
board as a standalone controller with full on or full off capability, 
however there is no provision for a blackout mode. 

The ten volt supply can be exported from the board should a user care to
implement other control methods such as varying control votlages with minimal
extra complexity.

Contact
-------

Contact Cirvinfox via github regarding any questions/issues/features. I would
love to hear from anyone wanting to make one of these themselves.
