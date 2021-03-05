EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L Device:R R2
U 1 1 601BEE46
P 4200 3700
F 0 "R2" H 4270 3746 50  0000 L CNN
F 1 "R100k" H 4270 3655 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 4130 3700 50  0001 C CNN
F 3 "~" H 4200 3700 50  0001 C CNN
	1    4200 3700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 601BF030
P 4200 4100
F 0 "R1" H 4270 4146 50  0000 L CNN
F 1 "R100k" H 4270 4055 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 4130 4100 50  0001 C CNN
F 3 "~" H 4200 4100 50  0001 C CNN
	1    4200 4100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R0
U 1 1 601C053C
P 4900 3900
F 0 "R0" H 4970 3946 50  0000 L CNN
F 1 "R330" H 4970 3855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 4830 3900 50  0001 C CNN
F 3 "~" H 4900 3900 50  0001 C CNN
	1    4900 3900
	0    1    1    0   
$EndComp
$Comp
L power:VCC #PWR0102
U 1 1 601C5FE6
P 4200 3450
F 0 "#PWR0102" H 4200 3300 50  0001 C CNN
F 1 "VCC" H 4215 3623 50  0000 C CNN
F 2 "" H 4200 3450 50  0001 C CNN
F 3 "" H 4200 3450 50  0001 C CNN
	1    4200 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3900 4200 3850
Wire Wire Line
	4200 3950 4200 3900
Connection ~ 4200 3900
Wire Wire Line
	4200 4250 4200 4300
Text Label 6600 3900 0    50   ~ 0
V_sense
$Comp
L Connector_Generic:Conn_01x03 J2
U 1 1 601D0F72
P 7850 4200
F 0 "J2" V 7722 4380 50  0000 L CNN
F 1 "Conn_01x03" V 7813 4380 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 7850 4200 50  0001 C CNN
F 3 "~" H 7850 4200 50  0001 C CNN
	1    7850 4200
	0    1    1    0   
$EndComp
Text Label 7950 3700 1    50   ~ 0
V_sense
$Comp
L power:GND #PWR0103
U 1 1 601D1D72
P 7850 4000
F 0 "#PWR0103" H 7850 3750 50  0001 C CNN
F 1 "GND" H 7855 3827 50  0000 C CNN
F 2 "" H 7850 4000 50  0001 C CNN
F 3 "" H 7850 4000 50  0001 C CNN
	1    7850 4000
	-1   0    0    1   
$EndComp
$Comp
L power:VCC #PWR0104
U 1 1 601D23D4
P 7750 4000
F 0 "#PWR0104" H 7750 3850 50  0001 C CNN
F 1 "VCC" H 7765 4173 50  0000 C CNN
F 2 "" H 7750 4000 50  0001 C CNN
F 3 "" H 7750 4000 50  0001 C CNN
	1    7750 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7950 3700 7950 4000
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 601D7D6A
P 4850 4550
F 0 "J1" V 4722 4362 50  0000 R CNN
F 1 "Screw_Terminal_01x02" V 4813 4362 50  0000 R CNN
F 2 "TerminalBlock:TerminalBlock_bornier-2_P5.08mm" H 4850 4550 50  0001 C CNN
F 3 "~" H 4850 4550 50  0001 C CNN
	1    4850 4550
	0    -1   1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 601EDDB7
P 5450 3900
F 0 "R4" H 5520 3946 50  0000 L CNN
F 1 "R10K" H 5520 3855 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5380 3900 50  0001 C CNN
F 3 "~" H 5450 3900 50  0001 C CNN
	1    5450 3900
	0    -1   -1   0   
$EndComp
$Comp
L Device:D D0
U 1 1 601EEE8C
P 6200 3750
F 0 "D0" H 6200 3967 50  0000 C CNN
F 1 "D" H 6200 3876 50  0000 C CNN
F 2 "Diode_THT:D_A-405_P10.16mm_Horizontal" H 6200 3750 50  0001 C CNN
F 3 "~" H 6200 3750 50  0001 C CNN
	1    6200 3750
	0    1    1    0   
$EndComp
$Comp
L Device:D D1
U 1 1 601EFCE1
P 6200 4050
F 0 "D1" H 6200 3833 50  0000 C CNN
F 1 "D" H 6200 3924 50  0000 C CNN
F 2 "Diode_THT:D_A-405_P10.16mm_Horizontal" H 6200 4050 50  0001 C CNN
F 3 "~" H 6200 4050 50  0001 C CNN
	1    6200 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	4200 3900 4650 3900
Wire Wire Line
	5900 3900 6200 3900
Connection ~ 6200 3900
Wire Wire Line
	5900 4400 5900 3900
Wire Wire Line
	5050 3900 5150 3900
Wire Wire Line
	5600 3900 5900 3900
Connection ~ 5900 3900
Wire Wire Line
	6200 3900 6600 3900
$Comp
L power:GND #PWR0101
U 1 1 601C596C
P 4200 4750
F 0 "#PWR0101" H 4200 4500 50  0001 C CNN
F 1 "GND" H 4205 4577 50  0000 C CNN
F 2 "" H 4200 4750 50  0001 C CNN
F 3 "" H 4200 4750 50  0001 C CNN
	1    4200 4750
	1    0    0    -1  
$EndComp
Connection ~ 4200 4300
Wire Wire Line
	4200 4300 4200 4750
Wire Wire Line
	4650 3900 4650 4200
Wire Wire Line
	4650 4200 4850 4200
Connection ~ 4650 3900
Wire Wire Line
	4650 3900 4750 3900
Wire Wire Line
	5150 4200 5150 3900
Wire Wire Line
	4950 4200 5150 4200
Connection ~ 5150 3900
Wire Wire Line
	5150 3900 5300 3900
Wire Wire Line
	4950 4200 4950 4350
Wire Wire Line
	4850 4200 4850 4350
$Comp
L power:GND #PWR0105
U 1 1 6028AA66
P 6200 4750
F 0 "#PWR0105" H 6200 4500 50  0001 C CNN
F 1 "GND" H 6205 4577 50  0000 C CNN
F 2 "" H 6200 4750 50  0001 C CNN
F 3 "" H 6200 4750 50  0001 C CNN
	1    6200 4750
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0106
U 1 1 6028B370
P 6200 3450
F 0 "#PWR0106" H 6200 3300 50  0001 C CNN
F 1 "VCC" H 6215 3623 50  0000 C CNN
F 2 "" H 6200 3450 50  0001 C CNN
F 3 "" H 6200 3450 50  0001 C CNN
	1    6200 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 3600 6200 3450
Wire Wire Line
	4200 3550 4200 3450
$Comp
L Device:C C20nF1
U 1 1 60298678
P 6050 4400
F 0 "C20nF1" V 5798 4400 50  0000 C CNN
F 1 "C" V 5889 4400 50  0000 C CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 6088 4250 50  0001 C CNN
F 3 "~" H 6050 4400 50  0001 C CNN
	1    6050 4400
	0    1    1    0   
$EndComp
$Comp
L Device:C C160uF1
U 1 1 60299B50
P 3650 4150
F 0 "C160uF1" H 3765 4196 50  0000 L CNN
F 1 "C" H 3765 4105 50  0000 L CNN
F 2 "Capacitor_THT:C_Disc_D7.5mm_W2.5mm_P5.00mm" H 3688 4000 50  0001 C CNN
F 3 "~" H 3650 4150 50  0001 C CNN
	1    3650 4150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 4200 6200 4400
Connection ~ 6200 4400
Wire Wire Line
	6200 4400 6200 4750
Wire Wire Line
	3650 4000 3650 3900
Wire Wire Line
	3650 3900 4200 3900
Wire Wire Line
	3650 4300 4200 4300
$EndSCHEMATC