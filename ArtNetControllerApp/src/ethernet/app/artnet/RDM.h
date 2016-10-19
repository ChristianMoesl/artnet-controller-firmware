/////////////////////////////////////////////////////////////////////
//
//	Copyright Artistic Licence Holdings Ltd	2002 - 2013
//	Author:	Wayne Howell
//	Email:	Support@ArtisticLicence.com
//      Please use discussion forum for tech support.
//       www.ArtisticLicence.com
//
//	This file contains all key defines and structures for RDM
//
//      Created 7/4/02 WDH
//
//	Notes on updates to RDM Standard V1.0
//
//	1) Structure change to allow 16 bit sub devices - there
//	    are now two structure definitions: S_RdmStd, S_RdmDraft
//	2) Numerous PIDs have changed and unfortunately draft and
//	    standard PIDs intersect without same meaning.
//	    When this happens, the PID now has a _STD suffix for
//	    RDM Standard V1.0 and _DRAFT suffix for RDM Draft.
//
//
////////////////////////////////////////////////////////////////////

#ifndef _RDM_H_
#define _RDM_H_
//#pragma pack(1) //set to byte packing in data structures // TODO

#include <stdbool.h>
#include <stdint.h>


#define uchar unsigned char
#define ushort unsigned short int
#define ulong unsigned int
// 8, 16, 32 bit fields


#define RDM_SC_RDM_DRAFT                0xf0    // Slot 0 Start Code for development
#define RDM_SC_RDM_STD                  0xcc    // Slot 0 Start Code for development

#define RDM_SC_SUB_MESSAGE              0x01    // Slot 1 RDM Protocol Data Structure ID

// Global Device UID's  RDM Standard

#define RDM_GLOBAL_ALL_DEVICES_ID       0xffffffffffff    // Global all manufacturers
#define RDM_ALL_DEVICES_ID              0x0000ffffffff    // Specific manufacturer ID
#define RDM_GLOBAL_ALL_DEVICES_ID_HI    0xffff
#define RDM_ALL_DEVICES_ID_LO           0xffffffff

// RDM Response Type  RDM Standard

#define RDM_RESPONSE_TYPE_ACK               0x00
#define RDM_RESPONSE_TYPE_ACK_TIMER         0x01
#define RDM_RESPONSE_TYPE_NACK_REASON       0x02
#define RDM_RESPONSE_TYPE_ACK_BULK_DRAFT    0x03	// NB AL retain this duplicate use of code 0x03 for file upload only
#define RDM_RESPONSE_TYPE_ACK_OVERFLOW	    0x03

// RDM Parameter commands  RDM Standard

#define RDM_DISCOVERY_COMMAND           0x10
#define RDM_DISCOVERY_COMMAND_RESPONSE  0x11
#define RDM_GET_COMMAND                 0x20
#define RDM_GET_COMMAND_RESPONSE        0x21
#define RDM_SET_COMMAND                 0x30
#define RDM_SET_COMMAND_RESPONSE        0x31

// RDM Parameter ID's  PIDs

#define RDM_DISCOVERY_UNIQUE_BRANCH             0x0001
#define RDM_DISCOVERY_MUTE                      0x0002
#define RDM_DISCOVERY_UN_MUTE                   0x0003

#define RDM_PROXIED_DEVICES                     0x0010
#define RDM_PROXIED_DEVICE_COUNT                0x0011
#define RDM_COMMS_STATUS			0x0015

#define RDM_GET_POLL_STD                        0x0020  // Queued messages

#define RDM_STATUS_MESSAGES                     0x0030

#define RDM_STATUS_ID_DESCRIPTION               0x0031
#define RDM_CLEAR_STATUS_ID                     0x0032
#define RDM_SUB_DEVICE_STATUS_REPORT_THRESHOLD  0x0033

#define RDM_SUPPORTED_PARAMETERS                0x0050
#define RDM_PARAMETER_DESCRIPTION               0x0051

#define RDM_DEVICE_INFO		                0x0060          //NB This PID was MODEL_ID in draft.
#define RDM_PRODUCT_DETAIL_ID_LIST              0x0070
#define RDM_DEVICE_MODEL_DESC_STD        	0x0080
#define RDM_MANUFACTURER_LABEL_STD              0x0081
#define RDM_DEVICE_LABEL_STD                    0x0082

#define RDM_FACTORY_DEFAULTS                    0x0090

#define RDM_LANGUAGE_CAPABILITIES               0x00a0
#define RDM_LANGUAGE                            0x00b0
#define RDM_SOFTWARE_VERSION_LABEL              0x00c0		//NB Reply format changed between draft and std
#define RDM_BOOT_SOFTWARE_VERSION_ID 		0x00C1
#define RDM_BOOT_SOFTWARE_VERSION_LABEL  	0x00c2

#define RDM_DMX_PERSONALITY                     0x00e0
#define RDM_DMX_PERSONALITY_DESCRIPTION         0x00e1

#define RDM_DMX_START_ADDRESS                   0x00f0

#define RDM_SLOT_ID                             0x0120
#define RDM_SLOT_DESCRIPTION                    0x0121
#define RDM_SLOT_DEFAULT_VALUE                  0x0122


#define RDM_SENSOR_DEFINITION_STD               0x0200
#define RDM_SENSOR_STD                          0x0201
#define RDM_SENSOR_RECORD_ALL_STD               0x0202

#define RDM_DIMMER_TYPE                         0x0301   	//These are draft pids awaiting resolution of RDM standard V1.1
#define RDM_DIMMER_CURVE_CLASS                  0x0302          //These are draft pids awaiting resolution of RDM standard V1.1

#define RDM_DEVICE_HOURS_STD                    0x0400
#define RDM_LAMP_HOURS_STD                      0x0401
#define RDM_LAMP_STRIKES_STD                    0x0402
#define RDM_LAMP_STATE_STD                      0x0403
#define RDM_LAMP_ON_MODE_STD                    0x0404

#define RDM_DISPLAY_INVERT_STD                  0x0500
#define RDM_DISPLAY_LEVEL_STD                   0x0501

#define RDM_PAN_INVERT                          0x0600
#define RDM_TILT_INVERT                         0x0601
#define RDM_PAN_TILT_SWAP                       0x0602



#define RDM_IDENTIFY_DEVICE                     0x1000

#define RDM_EXIT_COMMAND                        0x1002      //Note - this was deleted in Std RDM - need to check alternate functionality
#define RDM_RESET_DEVICE_STD                    0x1001

#define RDM_PERFORM_SELFTEST                    0x1020
#define RDM_SELF_TEST_DESCRIPTION               0x1021
#define RDM_CAPTURE_PRESET                      0x1030
#define RDM_PRESET_PLAYBACK                     0x1031


#define RDM_PID_DESCRIPTION                     0x1040      //Note - this was deleted in Std RDM - need to check alternate functionality

//The following PIDs were deleted in the standard. Any API call that requests transmission of these PIDs will be rerouted as a DEVICE_INFO PID.
// This is purely for backwards compatibility.

#define RDM_DMX_FOOTPRINT_DRAFT                 0x00d0
#define RDM_SUB_DEVICE_COUNT_DRAFT              0x0100
#define RDM_PROTOCOL_VERSION_DRAFT              0x0040
#define RDM_SENSOR_QUANTITY_STD                 0x0203  // NB The _STD suffix is not an error - see below for _DRAFT

//The following PIDs were renumbered from Draft to Std RDM

#define RDM_GET_POLL_DRAFT                      0x0012  // Queued messages
#define RDM_DEVICE_MODEL_DESC_DRAFT      	0x0062
#define RDM_MANUFACTURER_LABEL_DRAFT            0x0070
#define RDM_DEVICE_LABEL_DRAFT                  0x0080
#define RDM_SENSOR_DRAFT                        0x0200
#define RDM_SENSOR_QUANTITY_DRAFT               0x0201
#define RDM_SENSOR_DEFINITION_DRAFT             0x0202
#define RDM_SENSOR_RECORD_ALL_DRAFT             0x0203
#define RDM_DEVICE_HOURS_DRAFT                  0x0401
#define RDM_LAMP_HOURS_DRAFT                    0x0402
#define RDM_LAMP_STRIKES_DRAFT                  0x0403
#define RDM_LAMP_STATE_DRAFT                    0x0404
#define RDM_LAMP_ON_MODE_DRAFT                  0x0405
#define RDM_DISPLAY_INVERT_DRAFT                0x0501
#define RDM_DISPLAY_LEVEL_DRAFT                 0x0502
#define RDM_RESET_DEVICE_DRAFT                  0x1003

//The following PIDs exist in Draft but not Standard RDM. However these PIDs are used for AL firmware upload (which is Draft V1.0 BDT) for all uploads.

#define RDM_BULK_DATA_REQUEST                   0x2060
#define RDM_BULK_DATA_OFFER                     0x2070
#define RDM_BULK_DATA_QUERY                     0x2080


#define RDM_UNSUPPORTED_ID                      0x7fff

// Artistic Licence published PID's

#define RDM_ART_PROGRAM_UID			0x8000
#define RDM_ART_LS_SPECIAL			0x8001	//used by Light-Switch for product sync
#define RDM_ART_SC_SPECIAL			0x8002	//used by Sign-Control & Light-Switch for product sync (V3.08 firmware onwards)
#define RDM_ART_DATA_LOSS_MODE			0x8003	//used by Artistic products to define action on loss of data.
#define RDM_ART_FORCE_ROM_BOOT			0x8004	//used by Artistic products to force rom boot / factory restart


#define MaxPid          64      		// Number of PID's defined in PidLookup
#define MaxSupportedPid	128                     // Max number of PIDs from SupportedPids that we log.

// Max Data field lengths - these numbers are based on Draft, may need to change

#define COUNT_DEVICE_MODEL_DESCRIPTION 		32
#define COUNT_MANUFACTURER_LABEL 		32
#define COUNT_DEVICE_LABEL 			32
#define COUNT_SOFTWARE_LABEL 			32
#define COUNT_PERSONALITY_DESCRIPTION 		32
#define COUNT_PERSONALITY_LONG_DESCRIPTION 	64
#define COUNT_SELF_TEST_DESCRIPTION 		32
#define COUNT_SLOT_DESCRIPTION 			8
#define COUNT_SENSOR_DESCRIPTION 		32
#define COUNT_RDM_MAX_FILE_BLOCK		216




typedef struct {
	uchar Uid[6];	// UID Hi Byte first
} T_Uid;

typedef struct S_RdmStd {
	uchar   SubStartCode;           // defines protocol as RDM
        uchar   SlotCount;              // defines slot containing Chksum hi byte - range 23-255
        T_Uid   Dud;                    // Destination UID
        T_Uid   Sud;                    // Source UID
	uchar   SequenceNumber;         // Increments with each packet
	uchar   ResponseType;           // The Ack code / Port ID
        uchar   MessageCount;           // Number of messages queued at device
	ushort  SubDevice;              // Index of sub device, 0 is root device
	uchar   CommandClass;           // Class of command - get / set
        ushort  ParameterId;            // The command
        uchar   ParameterSlotCount;     // Index of following array, also SlotCount+23=ParameterSlotCount
        uchar   Data[256];
} T_RdmStd;


typedef struct S_RdmDraft {
	uchar   SubStartCode;           // defines protocol as RDM
        uchar   SlotCount;              // defines slot containing Chksum hi byte - range 23-255
        T_Uid   Dud;                    // Destination UID
        T_Uid   Sud;                    // Source UID
        uchar   SequenceNumber;         // Increments with each packet
        uchar   ResponseType;           // The Ack code
        uchar   MessageCount;           // Number of messages queued at device
        uchar   SubDevice;              // Index of sub device, 0 is root device
	uchar   CommandClass;           // Class of command - get / set
        ushort  ParameterId;            // The command
	uchar   ParameterSlotCount;     // Index of following array, also SlotCount+23=ParameterSlotCount
        uchar   Data[256];
} T_RdmDraft;


// RDM SoftwareVersion

typedef struct S_Software {
	uchar   Major;
	uchar   Minor;
        uchar   Build[2];
        uchar   Status;
        uchar   Year[2];
        uchar   Month;
        uchar   Day;
        uchar   Hour;
        uchar   Minute;
} T_Software;



// RDM SensorDescription

typedef struct S_SensorDescription {
	uchar   SensorNumber;
	uchar   Type;
	uchar   Unit;
	uchar   Prefix;
	short  RangeMinimum;
	short  RangeMaximum;
	short  NormalMinimum;
	short  NormalMaximum;
	uchar  RecMode;
	char    Name[COUNT_SENSOR_DESCRIPTION];
} T_SensorDescription;

// Defines used in calls to RdmDeviceGetRootSensorValueString  - 
#define SENSOR_VALUE		0
#define SENSOR_MIN_RANGE	1
#define SENSOR_MAX_RANGE	2
#define SENSOR_MIN_NORMAL	3
#define SENSOR_MAX_NORMAL	4
#define SENSOR_MIN_RECORDED	5
#define SENSOR_MAX_RECORDED	6
#define SENSOR_RECORDED		7

#define SENSOR_VALUE_TAB	8
#define SENSOR_MIN_RANGE_TAB	9
#define SENSOR_MAX_RANGE_TAB	10
#define SENSOR_MIN_NORMAL_TAB	11
#define SENSOR_MAX_NORMAL_TAB	12
#define SENSOR_MIN_RECORDED_TAB	13
#define SENSOR_MAX_RECORDED_TAB	14
#define SENSOR_RECORDED_TAB	15

//Status Message data  RDM Standard

#define RDM_STATUS_NONE        		0x00
#define RDM_STATUS_GET_LAST_MESSAGE	0x01
#define RDM_STATUS_ADVISORY             0x02
#define RDM_STATUS_WARNING        	0x03
#define RDM_STATUS_ERROR                0x04


// RDM SensorValueResponse

typedef struct S_SensorValue {
	uchar   SensorNumber;
	short  Value;
	short  LowestDetectedValue;
	short  HighestDetectedValue;
	short  RecValue;
} T_SensorValue;




// This structure is used to hold the Parameter ID name to PID cross reference table.


typedef struct S_Psc {
	bool    Valid;          // true if this PID id valid in combination with this Command Class
        uchar   Count;          // Expected PSC for this PID, 255 == variable
} T_Psc;


typedef struct S_Pid {
	char    Name[48];        // Name of parameter ID
	ushort  Pid;             // Parameter ID code
        T_Psc   PscDisc;         // Expected PSC for Discovery command, 255 == variable
        T_Psc   PscDiscResp;
        T_Psc   PscGet;
        T_Psc   PscGetResp;
        T_Psc   PscSet;
	T_Psc   PscSetResp;
} T_Pid;


#define MaxLang 11


//-----------------------------

#define RDM_VERBOSE_SC_ALL                              0x00
#define RDM_VERBOSE_SC_ERROR                            0x01
#define RDM_VERBOSE_SC_WARNING                          0x02
#define RDM_VERBOSE_SC_ADVISORY                         0x03
#define RDM_VERBOSE_SC_NONE                             0x04

#define MaxVerbose 5



// defines for slot values are temporary as not yet in standard.


#define RDM_SLOT_ID_DIMMER              0
#define RDM_SLOT_ID_PAN                 1
#define RDM_SLOT_ID_TILT                2
#define RDM_SLOT_ID_COLOUR_1            3
#define RDM_SLOT_ID_COLOUR_2            4
#define RDM_SLOT_ID_GOBO_1              5
#define RDM_SLOT_ID_GOBO_2              6
#define RDM_SLOT_ID_COLOUR_RED          7
#define RDM_SLOT_ID_COLOUR_GREEN        8
#define RDM_SLOT_ID_COLOUR_BLUE         9
#define RDM_SLOT_ID_CONTROL             10


#define MaxSid 11

// Dimmer Curves

#define RDM_DIMMER_CURVE_LINEAR                         0x00
#define RDM_DIMMER_CURVE_SWITCHED                       0x01
#define RDM_DIMMER_CURVE_SQUARE                         0x02
#define RDM_DIMMER_CURVE_S                              0x03
#define RDM_DIMMER_CURVE_FLUORESCENT_MAGNETIC           0x04
#define RDM_DIMMER_CURVE_FLUORESCENT_ELECTRONIC         0x05
#define RDM_DIMMER_CURVE_CUSTOM_1                       0x06
#define RDM_DIMMER_CURVE_CUSTOM_2                       0x07
#define RDM_DIMMER_CURVE_CUSTOM_3                       0x08
#define RDM_DIMMER_CURVE_OTHER                          0xff


#define RdmMaxCurve 10


// Define Dimmer Sub Device Types

#define RDM_DIMMER_TYPE_EMPTY                           0x00
#define RDM_DIMMER_TYPE_DIM                             0x01
#define RDM_DIMMER_TYPE_NON_DIM                         0x02
#define RDM_DIMMER_TYPE_FLUORESCENT                     0x03
#define RDM_DIMMER_TYPE_SINE                            0x04
#define RDM_DIMMER_TYPE_DC                              0x05
#define RDM_DIMMER_TYPE_COLD_CATHODE                    0x06
#define RDM_DIMMER_TYPE_LOW_VOLTAGE                     0x07
#define RDM_DIMMER_TYPE_LED                             0x08
#define RDM_DIMMER_TYPE_HMI                             0x09
#define RDM_DIMMER_TYPE_CONSTANT                        0x0a
#define RDM_DIMMER_TYPE_RELAY_ELECTRONIC                0x0b
#define RDM_DIMMER_TYPE_RELAY_MECHANICAL                0x0c
#define RDM_DIMMER_TYPE_CONTACTOR                       0x0d
#define RDM_DIMMER_TYPE_FREQUENCY_MODULATED             0x0e
#define RDM_DIMMER_TYPE_PULSE_WIDTH                     0x0f
#define RDM_DIMMER_TYPE_BIT_ANGLE                       0x10
#define RDM_DIMMER_TYPE_OTHER                           0xff



#define MaxSubType 18


//-------------------------

#define RDM_PRODUCT_TYPE_DIMMER                         0x01
#define RDM_PRODUCT_TYPE_MOVING_LIGHT                   0x02
#define RDM_PRODUCT_TYPE_SCROLLER                       0x03
#define RDM_PRODUCT_TYPE_SPLITTER                       0x04
#define RDM_PRODUCT_TYPE_ACCESSORY                      0x05
#define RDM_PRODUCT_TYPE_STROBE                         0x06
#define RDM_PRODUCT_TYPE_ATMOSPHERIC                    0x07
#define RDM_PRODUCT_TYPE_DEMUX                          0x08
#define RDM_PRODUCT_TYPE_PROTOCOL_CONV                  0x09
#define RDM_PRODUCT_TYPE_ETHERNET                       0x0a
#define RDM_PRODUCT_TYPE_OTHER                          0xff

#define MaxProductType 11

//-----------  These defines only relate to the way data was returned in the Draft version of this PID

#define RDM_SOFTWARE_PROTOTYPE                          0x00
#define RDM_SOFTWARE_DEVELOPMENT                        0x01
#define RDM_SOFTWARE_BETA                               0x02
#define RDM_SOFTWARE_CUSTOM                             0x03
#define RDM_SOFTWARE_RELEASE                            0xff

#define MaxVersionStatus 5



//-----------  Sensor Types

#define RDM_SENS_TEMPERATURE		0x00
#define RDM_SENS_VOLTAGE		0x01
#define RDM_SENS_CURRENT		0x02
#define RDM_SENS_FREQUENCY		0x03
#define RDM_SENS_RESISTANCE		0x04
#define RDM_SENS_POWER			0x05
#define RDM_SENS_MASS			0x06
#define RDM_SENS_LENGTH			0x07
#define RDM_SENS_AREA			0x08
#define RDM_SENS_VOLUME			0x09
#define RDM_SENS_DENSITY		0x0A
#define RDM_SENS_VELOCITY		0x0B
#define RDM_SENS_ACCELERATION		0x0C
#define RDM_SENS_FORCE			0x0D
#define RDM_SENS_ENERGY			0x0E
#define RDM_SENS_PRESSURE		0x0F
#define RDM_SENS_TIME			0x10
#define RDM_SENS_ANGLE			0x11
#define RDM_SENS_POSITION_X		0x12
#define RDM_SENS_POSITION_Y		0x13
#define RDM_SENS_POSITION_Z		0x14
#define RDM_SENS_ANGULAR_VELOCITY	0x15
#define RDM_SENS_LUMINOUS_INTENSITY	0x16
#define RDM_SENS_LUMINOUS_FLUX		0x17
#define RDM_SENS_ILLUMINANCE		0x18
#define RDM_SENS_CHROMINANCE_RED	0x19
#define RDM_SENS_CHROMINANCE_GREEN	0x1a
#define RDM_SENS_CHROMINANCE_BLUE	0x1b
#define RDM_SENS_CONTACTS		0x1c
#define RDM_SENS_MEMORY			0x1d
#define RDM_SENS_ITEMS			0x1e
#define RDM_SENS_HUMIDITY		0x1f
#define RDM_SENS_COUNTER_16BIT		0x20
#define RDM_SENS_OTHER			0x7f

#define MaxSensorTypes 			0x80

//-----------  Sensor Units

#define RDM_UNIT_NONE			0x00
#define RDM_UNIT_CENTIGRADE		0x01
#define RDM_UNIT_VOLTS_DC		0x02
#define RDM_UNIT_VOLTS_AC_PEAK		0x03
#define RDM_UNIT_VOLTS_AC_RMS		0x04
#define RDM_UNIT_AMPERE_DC		0x05
#define RDM_UNIT_AMPERE_AC_PEAK		0x06
#define RDM_UNIT_AMPERE_AC_RMS		0x07
#define RDM_UNIT_HERTZ			0x08
#define RDM_UNIT_OHMS			0x09
#define RDM_UNIT_WATT			0x0a
#define RDM_UNIT_KILOGRAM		0x0b
#define RDM_UNIT_METRES			0x0c
#define RDM_UNIT_METRES2		0x0d
#define RDM_UNIT_METRES3		0x0e
#define RDM_UNIT_KG_PER_M3		0x0f
#define RDM_UNIT_METRES_PER_SECOND	0x10
#define RDM_UNIT_METRES_PER_SECOND2	0x11
#define RDM_UNIT_NEWTON			0x12
#define RDM_UNIT_JOULE			0x13
#define RDM_UNIT_PASCAL			0x14
#define RDM_UNIT_SECOND			0x15
#define RDM_UNIT_DEGREE			0x16
#define RDM_UNIT_STERADIAN		0x17
#define RDM_UNIT_CANDELA		0x18
#define RDM_UNIT_LUMEN			0x19
#define RDM_UNIT_LUX			0x1a
#define RDM_UNIT_IRE			0x1b
#define RDM_UNIT_BYTE			0x1c

#define MaxSensorUnits 			0x1d

//-----------  Sensor Prefix  RDM Standard

#define RDM_PREFIX_NONE			0x00
#define RDM_PREFIX_DECI			0x01
#define RDM_PREFIX_CENTI 		0x02
#define RDM_PREFIX_MILLI 		0x03
#define RDM_PREFIX_MICRO 		0x04
#define RDM_PREFIX_NANO			0x05
#define RDM_PREFIX_PICO			0x06
#define RDM_PREFIX_FEMPTO		0x07
#define RDM_PREFIX_ATTO			0x08
#define RDM_PREFIX_ZEPTO		0x09
#define RDM_PREFIX_YOCTO		0x0A

#define RDM_PREFIX_BAD1			0x0B
#define RDM_PREFIX_BAD2			0x0C
#define RDM_PREFIX_BAD3			0x0D
#define RDM_PREFIX_BAD4			0x0E
#define RDM_PREFIX_BAD5			0x0F
#define RDM_PREFIX_BAD6			0x10

#define RDM_PREFIX_DECA			0x11
#define RDM_PREFIX_HECTO		0x12
#define RDM_PREFIX_KILO			0x13
#define RDM_PREFIX_MEGA			0x14
#define RDM_PREFIX_GIGA			0x15
#define RDM_PREFIX_TERRA		0x16
#define RDM_PREFIX_PETA			0x17
#define RDM_PREFIX_EXA			0x18
#define RDM_PREFIX_ZETTA		0x19
#define RDM_PREFIX_YOTTA		0x1A

#define MaxSensorPrefix			0x1B

//-----------  Bulk Block Defines  RDM Draft plus all AL product uploads

#define RDM_BB_FIRST	0x01
#define RDM_BB_CONTINUE	0x02
#define RDM_BB_RETRY	0x03
#define RDM_BB_FINAL	0x04

#define MaxBulkBlock 5


//-----------  Bulk Request Defines  RDM Draft plus all AL product uploads

#define RDM_BR_FIRST		0x01
#define RDM_BR_MORE		0x02
#define RDM_BR_RETRY		0x03

#define RDM_BR_NEGOTIATED	0x05	// not rdm defined - used internally in driver to show block size negotiation completed
#define RDM_BR_ACK_BULK 	0x06	// not rdm defined - used internally in driver to show final packet AckBulk has been received

#define MaxBulkRequest 4

//-----------  Bulk Data Defines  RDM Draft plus all AL product uploads

#define RDM_BD_DDL		0x01
#define RDM_BD_FIRMWARE		0x02
#define RDM_BD_CURVE		0x03




//-----------  Nak Reason Codes - RDM Standard

#define RDM_NR_UNKNOWN_PID			0x0000
#define RDM_NR_FORMAT_ERROR			0x0001
#define RDM_NR_HARDWARE_FAULT			0x0002
#define RDM_NR_PROXY_REJECT			0x0003
#define RDM_NR_PROTECT				0x0004
#define RDM_NR_UNSUPPORTED_COMMAND_CLASS	0x0005
#define RDM_NR_DATA_OUT_OF_RANGE		0x0006
#define RDM_NR_BUFFER_FULL			0x0007
#define RDM_NR_UNSUPPORTED_PACKET_SIZE		0x0008
#define RDM_NR_SUB_DEVICE_OUT_OF_RANGE		0x0009


#define MaxNakReason 0x0a


#ifndef _RDM_DATAH_
#define _RDM_DATAH_


T_Pid PidLookup[]=
	{
       // Text name                     Define                          Discovery      Disc Resp      Get            Get Resp       Set            Set Resp
       //                                                               Valid  Count   Valid  Count   Valid  Count   Valid  Count   Valid  Count   Valid  Count
	{"ERROR: RDM_DISCOVERY_UNIQUE_BRANCH", RDM_DISCOVERY_UNIQUE_BRANCH,
									{true , 6   }, {true , 255  }, {false , 0   }, {false , 0   }, {false , 0   }, {false , 0   }     },
	{"ERROR: RDM_DISCOVERY_MUTE", RDM_DISCOVERY_MUTE,               {true , 0   }, {true , 255  }, {false , 0   }, {false , 0   }, {false , 0   }, {false , 0   }     },
	{"ERROR: RDM_DISCOVERY_UN_MUTE", RDM_DISCOVERY_UN_MUTE,         {true , 0   }, {true , 255  }, {false, 0  }, {false , 0   }, {false , 0   }, {false , 0   }      },

	{"RDM_PROXIED_DEVICES", RDM_PROXIED_DEVICES,                    {false, 0   }, {false, 0    }, {true , 0  }, {true , 255  }, {false , 0   }, {false , 0   }     },
	{"RDM_PROXIED_DEVICE_COUNT", RDM_PROXIED_DEVICE_COUNT,          {false, 0   }, {false, 0    }, {true , 0  }, {true , 3    }, {true  , 1   }, {true , 0   }     },
	{"RDM_COMMS_STATUS", RDM_COMMS_STATUS,		                {false, 0   }, {false, 0    }, {true , 0  }, {true , 6    }, {true  , 0   }, {true , 0   }     },

        {"RDM_QUEUED_MESSAGES", RDM_GET_POLL_STD,                       {false, 0   }, {false, 0    }, {true , 1   }, {false , 0   }, {false , 0   }, {false , 0   }     },

        {"RDM_DEVICE_INFO", RDM_DEVICE_INFO,                            {false, 0   }, {false, 0    }, {true , 0   }, {true , 19   }, {false , 0   }, {false , 0   }     },



	{"RDM_SUB_DEVICE_COUNT", RDM_SUB_DEVICE_COUNT_DRAFT,            {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {false , 0   }, {false , 0   }     },
        {"RDM_STATUS_MESSAGES", RDM_STATUS_MESSAGES,                    {false, 0   }, {false, 0    }, {true , 1   }, {true , 255  }, {false , 0   }, {false , 0   }     },

        {"RDM_STATUS_ID_DESCRIPTION", RDM_STATUS_ID_DESCRIPTION,        {false, 0   }, {false, 0    }, {true , 2   }, {true , 34   }, {false , 0   }, {false , 0   }     },
        {"RDM_CLEAR_STATUS_ID", RDM_CLEAR_STATUS_ID,                    {false, 0   }, {false, 0    }, {false , 0  }, {false, 0    }, {true , 0   }, {true , 0   }     },
        {"RDM_SUB_DEVICE_STATUS_REPORT_THRESHOLD", RDM_SUB_DEVICE_STATUS_REPORT_THRESHOLD,
                                                                        {false, 0   }, {false, 0    }, {true , 0   }, {true , 1    }, {true , 1   }, {true , 0   }     },

        {"RDM_PROTOCOL_VERSION(Draft)", RDM_PROTOCOL_VERSION_DRAFT,     {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {false , 0   }, {false , 0   }     },
        {"RDM_SUPPORTED_PARAMETERS", RDM_SUPPORTED_PARAMETERS,          {false, 0   }, {false, 0    }, {true , 0   }, {true , 255 }, {false , 0   }, {false , 0   }     },
        {"RDM_PARAMETER_DESCRIPTION", RDM_PARAMETER_DESCRIPTION,        {false, 0   }, {false, 0    }, {true , 2   }, {true , 0x24 }, {false , 0   }, {false , 0   }     },

	{"RDM_DEVICE_INFO", RDM_DEVICE_INFO,                            {false, 0   }, {false, 0    }, {true , 0   }, {true , 2   }, {false , 0   }, {false , 0   }     },
	{"RDM_PRODUCT_DETAIL_ID_LIST", RDM_PRODUCT_DETAIL_ID_LIST,      {false, 0   }, {false, 0    }, {true , 0   }, {true , 255 }, {false , 0   }, {false , 0   }     },
	{"RDM_DEVICE_MODEL_DESCRIPTION", RDM_DEVICE_MODEL_DESC_STD,     {false, 0   }, {false, 0    }, {true , 0   }, {true , 32  }, {false , 0   }, {false , 0   }     },
	{"RDM_MANUFACTURER_LABEL", RDM_MANUFACTURER_LABEL_STD,          {false, 0   }, {false, 0    }, {true , 0   }, {true , 32  }, {false , 0   }, {false , 0   }     },
        {"RDM_DEVICE_LABEL", RDM_DEVICE_LABEL_STD,                      {false, 0   }, {false, 0    }, {true , 0   }, {true , 32  }, {true , 16   }, {true , 0   }     },
	{"RDM_FACTORY_DEFAULTS", RDM_FACTORY_DEFAULTS,                  {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 0    }, {true , 0   }     },
        {"RDM_LANGUAGE_CAPABILITIES", RDM_LANGUAGE_CAPABILITIES,        {false, 0   }, {false, 0    }, {true , 0   }, {true , 255 }, {false , 0   }, {false , 0   }     },
        {"RDM_LANGUAGE", RDM_LANGUAGE,                                  {false, 0   }, {false, 0    }, {true , 0   }, {true , 2   }, {true , 2    }, {true , 0   }     },
	{"RDM_SOFTWARE_VERSION_LABEL", RDM_SOFTWARE_VERSION_LABEL,      {false, 0   }, {false, 0    }, {true , 0   }, {true , 255 }, {false , 0   }, {false , 0   }     },
	{"RDM_BOOT_SOFTWARE_VERSION_ID", RDM_BOOT_SOFTWARE_VERSION_ID,  {false, 0   }, {false, 0    }, {true , 0   }, {true , 4},    {false , 0   }, {false , 0   }     },
  {"RDM_BOOT_SOFTWARE_VERSION_LABEL", RDM_BOOT_SOFTWARE_VERSION_LABEL,  {false, 0   }, {false, 0    }, {true , 0   }, {true , 255},  {false , 0   }, {false , 0   }     },

	{"RDM_DMX_FOOTPRINT(DRAFT)", RDM_DMX_FOOTPRINT_DRAFT,           {false, 0   }, {false, 0    }, {true , 0   }, {true , 2   }, {false , 0   }, {false , 0   }     },
	{"RDM_DMX_PERSONALITY", RDM_DMX_PERSONALITY,                    {false, 0   }, {false, 0    }, {true , 0   }, {true , 2  }, {true , 1   }, {true , 0   }     },
	{"RDM_DMX_PERSONALITY_DESCRIPTION",
		       RDM_DMX_PERSONALITY_DESCRIPTION, 		{false, 0   }, {false, 0    }, {true , 1   }, {true , 35  }, {false , 0   }, {false , 0   }     },
	{"RDM_DMX_START_ADDRESS", RDM_DMX_START_ADDRESS,                {false, 0   }, {false, 0    }, {true , 0   }, {true , 2  }, {true , 2   }, {true , 0   }     },
	{"RDM_SUB_DEVICE_COUNT(DRAFT)", RDM_SUB_DEVICE_COUNT_DRAFT,     {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {false , 0    }, {false , 0   }     },
	{"RDM_SLOT_ID", RDM_SLOT_ID,                                    {false, 0   }, {false, 0    }, {true , 0   }, {true , 255}, {false , 0   }, {false , 0   }     },
	{"RDM_SLOT_DESCRIPTION", RDM_SLOT_DESCRIPTION,                  {false, 0   }, {false, 0    }, {true , 2   }, {true , 9   }, {false , 0   }, {false , 0   }     },
	{"RDM_SLOT_DEFAULT_VALUE", RDM_SLOT_DEFAULT_VALUE,              {false, 0   }, {false, 0    }, {true , 0   }, {true , 255}, {false , 0   }, {false , 0   }     },

	{"RDM_SENSOR", RDM_SENSOR_STD,                                  {false, 0   }, {false, 0    }, {true , 1   }, {true , 255 }, {false , 0   }, {false , 0   }     },
	{"RDM_SENSOR_QUANTITY", RDM_SENSOR_QUANTITY_STD,                {false, 0   }, {false, 0    }, {true , 0   }, {true , 2   }, {false , 0   }, {false , 0   }     },
	{"RDM_SENSOR_DEFINITION", RDM_SENSOR_DEFINITION_STD,            {false, 0   }, {false, 0    }, {true , 1   }, {true , 255 }, {false , 0   }, {false , 0   }     },
	{"RDM_SENSOR_RECORD_ALL", RDM_SENSOR_RECORD_ALL_STD,            {false, 0   }, {false, 0    }, {true , 1   }, {true , 1   }, {false , 0   }, {false , 0   }     },

	{"RDM_DIMMER_TYPE", RDM_DIMMER_TYPE,                            {false, 0   }, {false, 0    }, {true , 0   }, {true , 2   }, {false , 0   }, {false , 0   }     },
	{"RDM_DIMMER_CURVE_CLASS", RDM_DIMMER_CURVE_CLASS,              {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_PRESET_PLAYBACK", RDM_PRESET_PLAYBACK,                    {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_DEVICE_HOURS", RDM_DEVICE_HOURS_STD,                      {false, 0   }, {false, 0    }, {true , 0   }, {true , 4   }, {true , 2    }, {true , 0   }     },
	{"RDM_LAMP_HOURS", RDM_LAMP_HOURS_STD,                          {false, 0   }, {false, 0    }, {true , 0   }, {true , 4   }, {true , 2    }, {true , 0   }     },
	{"RDM_LAMP_STRIKES", RDM_LAMP_STRIKES_STD,                      {false, 0   }, {false, 0    }, {true , 0   }, {true , 4   }, {true , 2    }, {true , 0   }     },

	{"RDM_SELF_TEST_DESCRIPTION", RDM_SELF_TEST_DESCRIPTION,        {false, 0   }, {false, 0    }, {true , 1   }, {true , 17  }, {false , 0   }, {false , 0   }     },
	{"RDM_PAN_INVERT", RDM_PAN_INVERT,                              {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_TILT_INVERT", RDM_TILT_INVERT,                            {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_PAN_TILT_SWAP", RDM_PAN_TILT_SWAP,                        {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_DISPLAY_INVERT", RDM_DISPLAY_INVERT_STD,                  {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_DISPLAY_LEVEL", RDM_DISPLAY_LEVEL_STD,                    {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_LAMP_STATE", RDM_LAMP_STATE_STD,                          {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },
	{"RDM_LAMP_ON_MODE", RDM_LAMP_ON_MODE_STD,                      {false, 0   }, {false, 0    }, {true , 0   }, {true , 1   }, {true , 1    }, {true , 0   }     },

	{"RDM_IDENTIFY_DEVICE", RDM_IDENTIFY_DEVICE,                    {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   }, {true , 1    }, {true , 0   }     },
	{"RDM_EXIT_COMMAND", RDM_EXIT_COMMAND,                          {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   }, {true , 0    }, {true , 0   }     },
	{"RDM_RESET_DEVICE", RDM_RESET_DEVICE_STD,                      {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   }, {true , 1    }, {true , 0   }     },
	{"RDM_PERFORM_SELFTEST", RDM_PERFORM_SELFTEST,                  {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   }, {true , 1    }, {true , 0   }     },
	{"RDM_CAPTURE_PRESET", RDM_CAPTURE_PRESET,                      {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   }, {true , 255  }, {true , 0   }     },

	{"RDM_PID_DESCRIPTION", RDM_PID_DESCRIPTION,                    {false, 0   }, {false, 0    }, {true , 2   }, {true , 0x1a }, {false , 0   }, {false , 0   }     },

	{"RDM_BULK_DATA_REQUEST", RDM_BULK_DATA_REQUEST,                {false, 0   }, {false, 0    }, {true , 6   }, {true , 0xe7 }, {false , 0   }, {false , 0   }     },
	{"RDM_BULK_DATA_OFFER", RDM_BULK_DATA_OFFER,                    {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   },  {true , 255  }, {true , 7   }     },
	{"RDM_BULK_DATA_QUERY", RDM_BULK_DATA_QUERY,                    {false, 0   }, {false, 0    }, {true , 9   }, {true , 1   },  {false , 0   }, {false , 0   }     },


	{"RDM_UNSUPPORTED_ID", RDM_UNSUPPORTED_ID,                      {false, 0   }, {false, 0    }, {false, 0   }, {false, 0   },  {false , 0   }, {false , 0   }     },

	{"RDM_ART_PROGRAM_UID", RDM_ART_PROGRAM_UID,                    {false, 0   }, {false, 0    }, {true, 6   },  {true, 6   },   {true , 6   },  {true , 6   }     },

	{"RDM_ART_LS_SPECIAL", RDM_ART_LS_SPECIAL,                    {false, 0   }, {false, 0    }, {true, 255   },  {true, 255   },   {true , 255   },  {true , 255   }     },
	{"RDM_ART_SC_SPECIAL", RDM_ART_SC_SPECIAL,                    {false, 0   }, {false, 0    }, {true, 255   },  {true, 255   },   {true , 255   },  {true , 255   }     },
	{"RDM_ART_DATA_LOSS_MODE", RDM_ART_DATA_LOSS_MODE,                    {false, 0   }, {false, 0    }, {true, 255   },  {true, 255   },   {true , 255   },  {true , 255   }     },
	{"RDM_ART_FORCE_ROM_BOOT", RDM_ART_FORCE_ROM_BOOT,                    {false, 0   }, {false, 0    }, {true, 255   },  {true, 255   },   {true , 255   },  {true , 255   }     },


	};





T_Pid LanguageLookup[]=          //to be updated
	{

                {"Default (00)", 0},
                {"English (44)", 44},
                {"French (33)", 33},
		{"German (49)", 49},
                {"Italian (39)", 39},
                {"Spanish (34)", 34},
                {"Japanese (81)", 81},
                {"Chinese (86)", 86},
                {"Russian (7)", 7},
		{"Israel (972)", 972},
                {"Cuban (53)", 53}

	};
T_Pid VerboseLookup[]=          //to be updated
        {

                {"All", RDM_VERBOSE_SC_ALL},
		{"Errors only", RDM_VERBOSE_SC_ERROR},
                {"Warnings and Errors", RDM_VERBOSE_SC_WARNING},
                {"Advisories, Warnings and Errors", RDM_VERBOSE_SC_ADVISORY},
		{"None", RDM_VERBOSE_SC_NONE}

        };




T_Pid SubTypeLookup[]=         //to be updated
        {

                {"Empty", RDM_DIMMER_TYPE_EMPTY},
                {"Dimmer", RDM_DIMMER_TYPE_DIM},
                {"Non-Dim", RDM_DIMMER_TYPE_NON_DIM},
                {"Fluorescent", RDM_DIMMER_TYPE_FLUORESCENT},
                {"Sine Wave", RDM_DIMMER_TYPE_SINE},
                {"DC", RDM_DIMMER_TYPE_DC},
                {"Cold Cathode", RDM_DIMMER_TYPE_COLD_CATHODE},
                {"Low Voltage", RDM_DIMMER_TYPE_LOW_VOLTAGE},
                {"LED", RDM_DIMMER_TYPE_LED},
                {"HMI", RDM_DIMMER_TYPE_HMI},
		{"CONSTANT", RDM_DIMMER_TYPE_CONSTANT},
		{"RELAY_ELECTRONIC", RDM_DIMMER_TYPE_RELAY_ELECTRONIC},
                {"RELAY_MECHANICAL", RDM_DIMMER_TYPE_RELAY_MECHANICAL},
                {"CONTACTOR", RDM_DIMMER_TYPE_CONTACTOR},
                {"FREQUENCY MODULATED", RDM_DIMMER_TYPE_FREQUENCY_MODULATED},
                {"PULSE WIDTH", RDM_DIMMER_TYPE_PULSE_WIDTH},
                {"BIT ANGLE", RDM_DIMMER_TYPE_BIT_ANGLE},
                {"Other", RDM_DIMMER_TYPE_OTHER}

        };

T_Pid ProductTypeLookup[]=      //to be updated to standard
        {

                {"Dimmer", RDM_PRODUCT_TYPE_DIMMER},
                {"Moving Light", RDM_PRODUCT_TYPE_MOVING_LIGHT},
                {"Scroller", RDM_PRODUCT_TYPE_SCROLLER},
                {"Splitter", RDM_PRODUCT_TYPE_SPLITTER},
                {"Accessory", RDM_PRODUCT_TYPE_ACCESSORY},
                {"Strobe", RDM_PRODUCT_TYPE_STROBE},
                {"Atmospheric", RDM_PRODUCT_TYPE_ATMOSPHERIC},
		{"Demultiplexor", RDM_PRODUCT_TYPE_DEMUX},
		{"Protocol Converter", RDM_PRODUCT_TYPE_PROTOCOL_CONV},
                {"Ethernet", RDM_PRODUCT_TYPE_ETHERNET},
                {"Other", RDM_PRODUCT_TYPE_OTHER},

        };

T_Pid VersionStatusLookup[]=	//Draft only data
	{

		{"Prototype", RDM_SOFTWARE_PROTOTYPE},
		{"Development", RDM_SOFTWARE_DEVELOPMENT},
		{"Beta", RDM_SOFTWARE_BETA},
		{"Custom", RDM_SOFTWARE_CUSTOM},
		{"Release", RDM_SOFTWARE_RELEASE},


	};


T_Pid SensorTypeLookup[]=    //RDM Standard
	{

{"Temperature", RDM_SENS_TEMPERATURE},
{"Voltage", RDM_SENS_VOLTAGE},
{"Current", RDM_SENS_CURRENT},
{"Frequency", RDM_SENS_FREQUENCY},
{"Resistance", RDM_SENS_RESISTANCE},
{"Power", RDM_SENS_POWER},
{"Mass", RDM_SENS_MASS},
{"Length", RDM_SENS_LENGTH},
{"Area", RDM_SENS_AREA},
{"Volume", RDM_SENS_VOLUME},
{"Density", RDM_SENS_DENSITY},
{"Velocity", RDM_SENS_VELOCITY},
{"Acceleration", RDM_SENS_ACCELERATION},
{"Force", RDM_SENS_FORCE},
{"Energy", RDM_SENS_ENERGY},
{"Pressure", RDM_SENS_PRESSURE},
{"Time", RDM_SENS_TIME},
{"Angle", RDM_SENS_ANGLE},
{"Position X", RDM_SENS_POSITION_X},
{"Position Y", RDM_SENS_POSITION_Y},
{"Position Z", RDM_SENS_POSITION_Z},
{"Angular Velocity", RDM_SENS_ANGULAR_VELOCITY},
{"Luminous Intensity", RDM_SENS_LUMINOUS_INTENSITY},
{"Luminous Flux", RDM_SENS_LUMINOUS_FLUX},
{"Illuminance", RDM_SENS_ILLUMINANCE},
{"Chrominance Red", RDM_SENS_CHROMINANCE_RED},
{"Chrominance Green", RDM_SENS_CHROMINANCE_GREEN},
{"Chrominance Blue", RDM_SENS_CHROMINANCE_BLUE},
{"Contacts", RDM_SENS_CONTACTS},
{"Memory", RDM_SENS_MEMORY},
{"Items", RDM_SENS_ITEMS},
{"Humidity", RDM_SENS_HUMIDITY},
{"Counter 16bit", RDM_SENS_COUNTER_16BIT},
{"Other", RDM_SENS_OTHER},

	};



T_Pid SensorUnitsLookup[]=       //RDM Standard
	{

{"", RDM_UNIT_NONE},
{"Degrees Centigrade", RDM_UNIT_CENTIGRADE},
{"Volts DC", RDM_UNIT_VOLTS_DC},
{"Volts AC Peak", RDM_UNIT_VOLTS_AC_PEAK},
{"Volts AC RMS", RDM_UNIT_VOLTS_AC_RMS},
{"Amps DC", RDM_UNIT_AMPERE_DC},
{"Amps AC Peak", RDM_UNIT_AMPERE_AC_PEAK},
{"Amps AC RMS", RDM_UNIT_AMPERE_AC_RMS},
{"Hertz", RDM_UNIT_HERTZ},
{"Ohms", RDM_UNIT_OHMS},
{"Watts", RDM_UNIT_WATT},
{"Kilogram", RDM_UNIT_KILOGRAM},
{"Metres", RDM_UNIT_METRES},
{"Square Metres", RDM_UNIT_METRES2},
{"Cubic Metres", RDM_UNIT_METRES3},
{"Kg per Cubic Metre", RDM_UNIT_KG_PER_M3},
{"Metres per Second", RDM_UNIT_METRES_PER_SECOND},
{"Metres per Second Squared", RDM_UNIT_METRES_PER_SECOND2},
{"Newtons", RDM_UNIT_NEWTON},
{"Joules", RDM_UNIT_JOULE},
{"Pascals", RDM_UNIT_PASCAL},
{"Seconds", RDM_UNIT_SECOND},
{"Degrees", RDM_UNIT_DEGREE},
{"Steradians", RDM_UNIT_STERADIAN},
{"Candelas", RDM_UNIT_CANDELA},
{"Lumens", RDM_UNIT_LUMEN},
{"Lux", RDM_UNIT_LUX},
{"IRE", RDM_UNIT_IRE},
{"Bytes", RDM_UNIT_BYTE}

	};


T_Pid SensorPrefixLookup[]=     //RDM Standard
	{

{"none", RDM_PREFIX_NONE},
{"deci", RDM_PREFIX_DECI},
{"centi", RDM_PREFIX_CENTI},
{"milli", RDM_PREFIX_MILLI},
{"micro", RDM_PREFIX_MICRO},
{"nano", RDM_PREFIX_NANO},
{"pico", RDM_PREFIX_PICO},
{"fempto", RDM_PREFIX_FEMPTO},
{"atto", RDM_PREFIX_ATTO},
{"zepto", RDM_PREFIX_ZEPTO},
{"yocto", RDM_PREFIX_YOCTO},

{"error", RDM_PREFIX_BAD1},
{"error", RDM_PREFIX_BAD2},
{"error", RDM_PREFIX_BAD3},
{"error", RDM_PREFIX_BAD4},
{"error", RDM_PREFIX_BAD5},
{"error", RDM_PREFIX_BAD6},

{"Deca", RDM_PREFIX_DECA},
{"Hecto", RDM_PREFIX_HECTO},
{"Kilo", RDM_PREFIX_KILO},
{"Mega", RDM_PREFIX_MEGA},
{"Giga", RDM_PREFIX_GIGA},
{"Terra", RDM_PREFIX_TERRA},
{"Peta", RDM_PREFIX_PETA},
{"Exa", RDM_PREFIX_EXA},
{"Zetta", RDM_PREFIX_ZETTA},
{"Yotta", RDM_PREFIX_YOTTA}


	};



T_Pid BulkBlockLookup[]=       	//RDM Draft use only
	{

		{"BB_ERROR", 0},
		{"BB_FIRST", RDM_BB_FIRST},
		{"BB_CONTINUE", RDM_BB_CONTINUE},
		{"BB_RETRY", RDM_BB_RETRY},
		{"BB_FINAL", RDM_BB_FINAL}

	};

T_Pid BulkRequestLookup[]=    	//RDM Draft use only
	{

		{"BB_ERROR", 0},
		{"BR_FIRST", RDM_BR_FIRST},
		{"BR_MORE", RDM_BR_MORE},
		{"BR_RETRY", RDM_BR_RETRY},

	};

T_Pid NakReasonLookup[]=       	//RDM Standard
	{

		{"RDM_NR_UNKNOWN_PID", RDM_NR_UNKNOWN_PID},
		{"RDM_NR_FORMAT_ERROR", RDM_NR_FORMAT_ERROR},
		{"RDM_NR_HARDWARE_FAULT", RDM_NR_HARDWARE_FAULT},
		{"RDM_NR_PROXY_REJECT", RDM_NR_PROXY_REJECT},
		{"RDM_NR_PROTECT", RDM_NR_PROTECT},
		{"RDM_NR_UNSUPPORTED_COMMAND_CLASS", RDM_NR_UNSUPPORTED_COMMAND_CLASS},
		{"RDM_NR_DATA_OUT_OF_RANGE", RDM_NR_DATA_OUT_OF_RANGE},
		{"RDM_NR_BUFFER_FULL", RDM_NR_BUFFER_FULL},
		{"RDM_NR_UNSUPPORTED_PACKET_SIZE", RDM_NR_UNSUPPORTED_PACKET_SIZE},
		{"RDM_NR_SUB_DEVICE_OUT_OF_RANGE", RDM_NR_SUB_DEVICE_OUT_OF_RANGE},


	};


#endif




//#pragma pack() //back to normal data structure packing

#endif


