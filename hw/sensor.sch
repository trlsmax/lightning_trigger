EESchema Schematic File Version 2
LIBS:root-rescue
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:lightning-cache
LIBS:sensor-cache
LIBS:root-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L OPT101 U1
U 1 1 5562C8C5
P 5800 3850
F 0 "U1" H 5500 4400 60  0000 C BNN
F 1 "OPT101" H 5600 4350 60  0000 C TNN
F 2 "Housings_DIP:DIP-8__300_ELL" H 5800 3700 60  0001 C CNN
F 3 "" H 5800 3700 60  0000 C CNN
	1    5800 3850
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 5562C91E
P 4800 3700
F 0 "C1" H 4825 3800 50  0000 L CNN
F 1 "100n" H 4825 3600 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 4838 3550 30  0001 C CNN
F 3 "" H 4800 3700 60  0000 C CNN
	1    4800 3700
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X03 P1
U 1 1 5562C973
P 7000 4050
F 0 "P1" H 7000 4250 50  0000 C CNN
F 1 "CONN_SENSOR" V 7100 4050 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03" H 7000 4050 60  0001 C CNN
F 3 "" H 7000 4050 60  0000 C CNN
	1    7000 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	6300 4050 6800 4050
Wire Wire Line
	5250 4050 5150 4050
Wire Wire Line
	5150 4050 5150 4300
Wire Wire Line
	5150 4300 6450 4300
Wire Wire Line
	6450 4300 6450 4050
Connection ~ 6450 4050
Wire Wire Line
	6300 3600 6650 3600
Wire Wire Line
	6650 3600 6650 4450
Wire Wire Line
	6650 4150 6800 4150
Wire Wire Line
	6650 4450 4800 4450
Wire Wire Line
	4800 4450 4800 3850
Connection ~ 6650 4150
Wire Wire Line
	5250 3900 4800 3900
Connection ~ 4800 3900
Wire Wire Line
	4800 3550 4800 3100
Wire Wire Line
	4800 3100 6800 3100
Wire Wire Line
	6800 3100 6800 3950
Wire Wire Line
	5250 3600 5150 3600
Wire Wire Line
	5150 3600 5150 3100
Connection ~ 5150 3100
NoConn ~ 6300 3750
NoConn ~ 6300 3900
NoConn ~ 5250 3750
Text Notes 4875 4675 0    60   ~ 0
转危为安
$EndSCHEMATC
