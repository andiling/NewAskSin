## -- definitions -------------------------------------------------------------------------------------------
use strict; package usrRegs; my %regList;

## -- Sub Type ID information -------------------------------------------------------------------------------
##  "0x01" => "AlarmControl"      "0x41" => "sensor"          "0x70" => "THSensor"
##  "0x10" => "switch"            "0x42" => "swi"             "0x80" => "threeStateSensor"
##  "0x12" => "outputUnit"        "0x43" => "pushButton"      "0x81" => "motionDetector"
##  "0x20" => "dimmer"            "0x44" => "singleButton"    "0xC0" => "keyMatic"
##  "0x30" => "blindActuator"     "0x51" => "powerMeter"      "0xC1" => "winMatic"
##  "0x39" => "ClimateControl"    "0x58" => "thermostat"      "0xC3" => "tipTronic"
##  "0x40" => "remote"            "0x60" => "KFM100"          "0xCD" => "smokeDetector"


## -- device config -----------------------------------------------------------------------------------------
my %confType = (
	## some basic information to create a new device, serial number and hm id has to be unique in your network
	## on modelID you could choose between an already existing configuration or you can create a new device
	
	serial      => 'XMS1234567', 												# 0 to get it automatically generated - otherwise 10 byte ASCII format
	hmID        => '0', 														# 0 to get it automatically generated - otherwise 6 HEX digits
	
	modelID     => 0x0f43,														# if model id is known, details will taken from HM config xml files, 4 HEX digits
	firmwareVer => 0x10, 														# firmware version, 2 HEX digits - important if you took a model id where more then one device exists


	## no input needed if model id is valid and device already exists in HM config software,
	## otherwise fill accordingly to create a proper register.h and xml config file 

	name        => 'Test127',													# name of the device, ascii [A-Z, a-z, 0-9, '-'], no blanks
	description => 'das ist ein test',											# short description of the device

	subtypeID   => 0x70,														# depending on type of device
	deviceInfo  => 0x030100,													# not complete clear yes, but 3 bytes HEX needed
	
	burstRx     => 1,               		                              		# device needs a burst signal to wakeup
	localResDis => 1,															# local reset disable 
	intKeysVis  => 1,  															# internal keys visible

	battValue   => 30, 															# one byte default value in milli volt, 0 if not a battery device 
	battVisib   => 0,															# battery flag visible in registers of channel 0
	battChkDura => 3600000,														# the time between two measurements, value in milli seconds

);

## -- channel config ----------------------------------------------------------------------------------------
## predefined type in linkset.xml choose xmlDimmer, xmlSwitch, xmlKey, xmlWeather - more to come
## peers reflects the amount of possible peers, hidden makes a channel hidden for the config software, but will still work
## with linked you can link channels together, e.g. key to dimmer
## todo: linked

$regList{1}     = {type => "xmlDimmer", peers => 6, hidden => 0, linked => 0     };
$regList{2}     = {type => "xmlSwitch", peers => 6, hidden => 1, linked => {3,4} };
$regList{3}     = {type => "xmlKey",    peers => 6, hidden => 0, linked => 2     };
$regList{4}     = {type => "xmlKey",    peers => 6, hidden => 0, linked => 2     };
	





## -- helpers -----------------------------------------------------------------------------------------------
sub usr_getHash($){
  my $hn = shift;
  return %regList    if($hn eq "regList"      );
  return %confType   if($hn eq "configType"       );
}
