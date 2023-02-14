
/*
 * Generated file - DO NOT EDIT
 *
 * This is the header file for the LLRP Tool Kit (LTK)
 * C++ (aka cpp) implementation. It is generated into a .inc file
 * that is included by a platform specific .h header file.
 * That .h file takes care of prerequisites needed by this file.
 */




/*
 * Message classes - forward decls
 */

struct LLRP_SCUSTOM_MESSAGE;
typedef struct LLRP_SCUSTOM_MESSAGE
    LLRP_tSCUSTOM_MESSAGE;

struct LLRP_SGET_READER_CAPABILITIES;
typedef struct LLRP_SGET_READER_CAPABILITIES
    LLRP_tSGET_READER_CAPABILITIES;

struct LLRP_SGET_READER_CAPABILITIES_RESPONSE;
typedef struct LLRP_SGET_READER_CAPABILITIES_RESPONSE
    LLRP_tSGET_READER_CAPABILITIES_RESPONSE;

struct LLRP_SADD_ROSPEC;
typedef struct LLRP_SADD_ROSPEC
    LLRP_tSADD_ROSPEC;

struct LLRP_SADD_ROSPEC_RESPONSE;
typedef struct LLRP_SADD_ROSPEC_RESPONSE
    LLRP_tSADD_ROSPEC_RESPONSE;

struct LLRP_SDELETE_ROSPEC;
typedef struct LLRP_SDELETE_ROSPEC
    LLRP_tSDELETE_ROSPEC;

struct LLRP_SDELETE_ROSPEC_RESPONSE;
typedef struct LLRP_SDELETE_ROSPEC_RESPONSE
    LLRP_tSDELETE_ROSPEC_RESPONSE;

struct LLRP_SSTART_ROSPEC;
typedef struct LLRP_SSTART_ROSPEC
    LLRP_tSSTART_ROSPEC;

struct LLRP_SSTART_ROSPEC_RESPONSE;
typedef struct LLRP_SSTART_ROSPEC_RESPONSE
    LLRP_tSSTART_ROSPEC_RESPONSE;

struct LLRP_SSTOP_ROSPEC;
typedef struct LLRP_SSTOP_ROSPEC
    LLRP_tSSTOP_ROSPEC;

struct LLRP_SSTOP_ROSPEC_RESPONSE;
typedef struct LLRP_SSTOP_ROSPEC_RESPONSE
    LLRP_tSSTOP_ROSPEC_RESPONSE;

struct LLRP_SENABLE_ROSPEC;
typedef struct LLRP_SENABLE_ROSPEC
    LLRP_tSENABLE_ROSPEC;

struct LLRP_SENABLE_ROSPEC_RESPONSE;
typedef struct LLRP_SENABLE_ROSPEC_RESPONSE
    LLRP_tSENABLE_ROSPEC_RESPONSE;

struct LLRP_SDISABLE_ROSPEC;
typedef struct LLRP_SDISABLE_ROSPEC
    LLRP_tSDISABLE_ROSPEC;

struct LLRP_SDISABLE_ROSPEC_RESPONSE;
typedef struct LLRP_SDISABLE_ROSPEC_RESPONSE
    LLRP_tSDISABLE_ROSPEC_RESPONSE;

struct LLRP_SGET_ROSPECS;
typedef struct LLRP_SGET_ROSPECS
    LLRP_tSGET_ROSPECS;

struct LLRP_SGET_ROSPECS_RESPONSE;
typedef struct LLRP_SGET_ROSPECS_RESPONSE
    LLRP_tSGET_ROSPECS_RESPONSE;

struct LLRP_SADD_ACCESSSPEC;
typedef struct LLRP_SADD_ACCESSSPEC
    LLRP_tSADD_ACCESSSPEC;

struct LLRP_SADD_ACCESSSPEC_RESPONSE;
typedef struct LLRP_SADD_ACCESSSPEC_RESPONSE
    LLRP_tSADD_ACCESSSPEC_RESPONSE;

struct LLRP_SDELETE_ACCESSSPEC;
typedef struct LLRP_SDELETE_ACCESSSPEC
    LLRP_tSDELETE_ACCESSSPEC;

struct LLRP_SDELETE_ACCESSSPEC_RESPONSE;
typedef struct LLRP_SDELETE_ACCESSSPEC_RESPONSE
    LLRP_tSDELETE_ACCESSSPEC_RESPONSE;

struct LLRP_SENABLE_ACCESSSPEC;
typedef struct LLRP_SENABLE_ACCESSSPEC
    LLRP_tSENABLE_ACCESSSPEC;

struct LLRP_SENABLE_ACCESSSPEC_RESPONSE;
typedef struct LLRP_SENABLE_ACCESSSPEC_RESPONSE
    LLRP_tSENABLE_ACCESSSPEC_RESPONSE;

struct LLRP_SDISABLE_ACCESSSPEC;
typedef struct LLRP_SDISABLE_ACCESSSPEC
    LLRP_tSDISABLE_ACCESSSPEC;

struct LLRP_SDISABLE_ACCESSSPEC_RESPONSE;
typedef struct LLRP_SDISABLE_ACCESSSPEC_RESPONSE
    LLRP_tSDISABLE_ACCESSSPEC_RESPONSE;

struct LLRP_SGET_ACCESSSPECS;
typedef struct LLRP_SGET_ACCESSSPECS
    LLRP_tSGET_ACCESSSPECS;

struct LLRP_SGET_ACCESSSPECS_RESPONSE;
typedef struct LLRP_SGET_ACCESSSPECS_RESPONSE
    LLRP_tSGET_ACCESSSPECS_RESPONSE;

struct LLRP_SGET_READER_CONFIG;
typedef struct LLRP_SGET_READER_CONFIG
    LLRP_tSGET_READER_CONFIG;

struct LLRP_SGET_READER_CONFIG_RESPONSE;
typedef struct LLRP_SGET_READER_CONFIG_RESPONSE
    LLRP_tSGET_READER_CONFIG_RESPONSE;

struct LLRP_SSET_READER_CONFIG;
typedef struct LLRP_SSET_READER_CONFIG
    LLRP_tSSET_READER_CONFIG;

struct LLRP_SSET_READER_CONFIG_RESPONSE;
typedef struct LLRP_SSET_READER_CONFIG_RESPONSE
    LLRP_tSSET_READER_CONFIG_RESPONSE;

struct LLRP_SCLOSE_CONNECTION;
typedef struct LLRP_SCLOSE_CONNECTION
    LLRP_tSCLOSE_CONNECTION;

struct LLRP_SCLOSE_CONNECTION_RESPONSE;
typedef struct LLRP_SCLOSE_CONNECTION_RESPONSE
    LLRP_tSCLOSE_CONNECTION_RESPONSE;

struct LLRP_SGET_REPORT;
typedef struct LLRP_SGET_REPORT
    LLRP_tSGET_REPORT;

struct LLRP_SRO_ACCESS_REPORT;
typedef struct LLRP_SRO_ACCESS_REPORT
    LLRP_tSRO_ACCESS_REPORT;

struct LLRP_SKEEPALIVE;
typedef struct LLRP_SKEEPALIVE
    LLRP_tSKEEPALIVE;

struct LLRP_SKEEPALIVE_ACK;
typedef struct LLRP_SKEEPALIVE_ACK
    LLRP_tSKEEPALIVE_ACK;

struct LLRP_SREADER_EVENT_NOTIFICATION;
typedef struct LLRP_SREADER_EVENT_NOTIFICATION
    LLRP_tSREADER_EVENT_NOTIFICATION;

struct LLRP_SENABLE_EVENTS_AND_REPORTS;
typedef struct LLRP_SENABLE_EVENTS_AND_REPORTS
    LLRP_tSENABLE_EVENTS_AND_REPORTS;

struct LLRP_SERROR_MESSAGE;
typedef struct LLRP_SERROR_MESSAGE
    LLRP_tSERROR_MESSAGE;


/* Custom messages */


/*
 * Parameter classes - forward decls
 */

struct LLRP_SUTCTimestamp;
typedef struct LLRP_SUTCTimestamp
    LLRP_tSUTCTimestamp;

struct LLRP_SUptime;
typedef struct LLRP_SUptime
    LLRP_tSUptime;

struct LLRP_SCustom;
typedef struct LLRP_SCustom
    LLRP_tSCustom;

struct LLRP_SGeneralDeviceCapabilities;
typedef struct LLRP_SGeneralDeviceCapabilities
    LLRP_tSGeneralDeviceCapabilities;

struct LLRP_SReceiveSensitivityTableEntry;
typedef struct LLRP_SReceiveSensitivityTableEntry
    LLRP_tSReceiveSensitivityTableEntry;

struct LLRP_SPerAntennaReceiveSensitivityRange;
typedef struct LLRP_SPerAntennaReceiveSensitivityRange
    LLRP_tSPerAntennaReceiveSensitivityRange;

struct LLRP_SPerAntennaAirProtocol;
typedef struct LLRP_SPerAntennaAirProtocol
    LLRP_tSPerAntennaAirProtocol;

struct LLRP_SGPIOCapabilities;
typedef struct LLRP_SGPIOCapabilities
    LLRP_tSGPIOCapabilities;

struct LLRP_SLLRPCapabilities;
typedef struct LLRP_SLLRPCapabilities
    LLRP_tSLLRPCapabilities;

struct LLRP_SRegulatoryCapabilities;
typedef struct LLRP_SRegulatoryCapabilities
    LLRP_tSRegulatoryCapabilities;

struct LLRP_SUHFBandCapabilities;
typedef struct LLRP_SUHFBandCapabilities
    LLRP_tSUHFBandCapabilities;

struct LLRP_STransmitPowerLevelTableEntry;
typedef struct LLRP_STransmitPowerLevelTableEntry
    LLRP_tSTransmitPowerLevelTableEntry;

struct LLRP_SFrequencyInformation;
typedef struct LLRP_SFrequencyInformation
    LLRP_tSFrequencyInformation;

struct LLRP_SFrequencyHopTable;
typedef struct LLRP_SFrequencyHopTable
    LLRP_tSFrequencyHopTable;

struct LLRP_SFixedFrequencyTable;
typedef struct LLRP_SFixedFrequencyTable
    LLRP_tSFixedFrequencyTable;

struct LLRP_SROSpec;
typedef struct LLRP_SROSpec
    LLRP_tSROSpec;

struct LLRP_SROBoundarySpec;
typedef struct LLRP_SROBoundarySpec
    LLRP_tSROBoundarySpec;

struct LLRP_SROSpecStartTrigger;
typedef struct LLRP_SROSpecStartTrigger
    LLRP_tSROSpecStartTrigger;

struct LLRP_SPeriodicTriggerValue;
typedef struct LLRP_SPeriodicTriggerValue
    LLRP_tSPeriodicTriggerValue;

struct LLRP_SGPITriggerValue;
typedef struct LLRP_SGPITriggerValue
    LLRP_tSGPITriggerValue;

struct LLRP_SROSpecStopTrigger;
typedef struct LLRP_SROSpecStopTrigger
    LLRP_tSROSpecStopTrigger;

struct LLRP_SAISpec;
typedef struct LLRP_SAISpec
    LLRP_tSAISpec;

struct LLRP_SAISpecStopTrigger;
typedef struct LLRP_SAISpecStopTrigger
    LLRP_tSAISpecStopTrigger;

struct LLRP_STagObservationTrigger;
typedef struct LLRP_STagObservationTrigger
    LLRP_tSTagObservationTrigger;

struct LLRP_SInventoryParameterSpec;
typedef struct LLRP_SInventoryParameterSpec
    LLRP_tSInventoryParameterSpec;

struct LLRP_SRFSurveySpec;
typedef struct LLRP_SRFSurveySpec
    LLRP_tSRFSurveySpec;

struct LLRP_SRFSurveySpecStopTrigger;
typedef struct LLRP_SRFSurveySpecStopTrigger
    LLRP_tSRFSurveySpecStopTrigger;

struct LLRP_SLoopSpec;
typedef struct LLRP_SLoopSpec
    LLRP_tSLoopSpec;

struct LLRP_SAccessSpec;
typedef struct LLRP_SAccessSpec
    LLRP_tSAccessSpec;

struct LLRP_SAccessSpecStopTrigger;
typedef struct LLRP_SAccessSpecStopTrigger
    LLRP_tSAccessSpecStopTrigger;

struct LLRP_SAccessCommand;
typedef struct LLRP_SAccessCommand
    LLRP_tSAccessCommand;

struct LLRP_SLLRPConfigurationStateValue;
typedef struct LLRP_SLLRPConfigurationStateValue
    LLRP_tSLLRPConfigurationStateValue;

struct LLRP_SIdentification;
typedef struct LLRP_SIdentification
    LLRP_tSIdentification;

struct LLRP_SGPOWriteData;
typedef struct LLRP_SGPOWriteData
    LLRP_tSGPOWriteData;

struct LLRP_SKeepaliveSpec;
typedef struct LLRP_SKeepaliveSpec
    LLRP_tSKeepaliveSpec;

struct LLRP_SAntennaProperties;
typedef struct LLRP_SAntennaProperties
    LLRP_tSAntennaProperties;

struct LLRP_SAntennaConfiguration;
typedef struct LLRP_SAntennaConfiguration
    LLRP_tSAntennaConfiguration;

struct LLRP_SRFReceiver;
typedef struct LLRP_SRFReceiver
    LLRP_tSRFReceiver;

struct LLRP_SRFTransmitter;
typedef struct LLRP_SRFTransmitter
    LLRP_tSRFTransmitter;

struct LLRP_SGPIPortCurrentState;
typedef struct LLRP_SGPIPortCurrentState
    LLRP_tSGPIPortCurrentState;

struct LLRP_SEventsAndReports;
typedef struct LLRP_SEventsAndReports
    LLRP_tSEventsAndReports;

struct LLRP_SROReportSpec;
typedef struct LLRP_SROReportSpec
    LLRP_tSROReportSpec;

struct LLRP_STagReportContentSelector;
typedef struct LLRP_STagReportContentSelector
    LLRP_tSTagReportContentSelector;

struct LLRP_SAccessReportSpec;
typedef struct LLRP_SAccessReportSpec
    LLRP_tSAccessReportSpec;

struct LLRP_STagReportData;
typedef struct LLRP_STagReportData
    LLRP_tSTagReportData;

struct LLRP_SEPCData;
typedef struct LLRP_SEPCData
    LLRP_tSEPCData;

struct LLRP_SEPC_96;
typedef struct LLRP_SEPC_96
    LLRP_tSEPC_96;

struct LLRP_SROSpecID;
typedef struct LLRP_SROSpecID
    LLRP_tSROSpecID;

struct LLRP_SSpecIndex;
typedef struct LLRP_SSpecIndex
    LLRP_tSSpecIndex;

struct LLRP_SInventoryParameterSpecID;
typedef struct LLRP_SInventoryParameterSpecID
    LLRP_tSInventoryParameterSpecID;

struct LLRP_SAntennaID;
typedef struct LLRP_SAntennaID
    LLRP_tSAntennaID;

struct LLRP_SPeakRSSI;
typedef struct LLRP_SPeakRSSI
    LLRP_tSPeakRSSI;

struct LLRP_SChannelIndex;
typedef struct LLRP_SChannelIndex
    LLRP_tSChannelIndex;

struct LLRP_SFirstSeenTimestampUTC;
typedef struct LLRP_SFirstSeenTimestampUTC
    LLRP_tSFirstSeenTimestampUTC;

struct LLRP_SFirstSeenTimestampUptime;
typedef struct LLRP_SFirstSeenTimestampUptime
    LLRP_tSFirstSeenTimestampUptime;

struct LLRP_SLastSeenTimestampUTC;
typedef struct LLRP_SLastSeenTimestampUTC
    LLRP_tSLastSeenTimestampUTC;

struct LLRP_SLastSeenTimestampUptime;
typedef struct LLRP_SLastSeenTimestampUptime
    LLRP_tSLastSeenTimestampUptime;

struct LLRP_STagSeenCount;
typedef struct LLRP_STagSeenCount
    LLRP_tSTagSeenCount;

struct LLRP_SAccessSpecID;
typedef struct LLRP_SAccessSpecID
    LLRP_tSAccessSpecID;

struct LLRP_SRFSurveyReportData;
typedef struct LLRP_SRFSurveyReportData
    LLRP_tSRFSurveyReportData;

struct LLRP_SFrequencyRSSILevelEntry;
typedef struct LLRP_SFrequencyRSSILevelEntry
    LLRP_tSFrequencyRSSILevelEntry;

struct LLRP_SReaderEventNotificationSpec;
typedef struct LLRP_SReaderEventNotificationSpec
    LLRP_tSReaderEventNotificationSpec;

struct LLRP_SEventNotificationState;
typedef struct LLRP_SEventNotificationState
    LLRP_tSEventNotificationState;

struct LLRP_SReaderEventNotificationData;
typedef struct LLRP_SReaderEventNotificationData
    LLRP_tSReaderEventNotificationData;

struct LLRP_SHoppingEvent;
typedef struct LLRP_SHoppingEvent
    LLRP_tSHoppingEvent;

struct LLRP_SGPIEvent;
typedef struct LLRP_SGPIEvent
    LLRP_tSGPIEvent;

struct LLRP_SROSpecEvent;
typedef struct LLRP_SROSpecEvent
    LLRP_tSROSpecEvent;

struct LLRP_SReportBufferLevelWarningEvent;
typedef struct LLRP_SReportBufferLevelWarningEvent
    LLRP_tSReportBufferLevelWarningEvent;

struct LLRP_SReportBufferOverflowErrorEvent;
typedef struct LLRP_SReportBufferOverflowErrorEvent
    LLRP_tSReportBufferOverflowErrorEvent;

struct LLRP_SReaderExceptionEvent;
typedef struct LLRP_SReaderExceptionEvent
    LLRP_tSReaderExceptionEvent;

struct LLRP_SOpSpecID;
typedef struct LLRP_SOpSpecID
    LLRP_tSOpSpecID;

struct LLRP_SRFSurveyEvent;
typedef struct LLRP_SRFSurveyEvent
    LLRP_tSRFSurveyEvent;

struct LLRP_SAISpecEvent;
typedef struct LLRP_SAISpecEvent
    LLRP_tSAISpecEvent;

struct LLRP_SAntennaEvent;
typedef struct LLRP_SAntennaEvent
    LLRP_tSAntennaEvent;

struct LLRP_SConnectionAttemptEvent;
typedef struct LLRP_SConnectionAttemptEvent
    LLRP_tSConnectionAttemptEvent;

struct LLRP_SConnectionCloseEvent;
typedef struct LLRP_SConnectionCloseEvent
    LLRP_tSConnectionCloseEvent;

struct LLRP_SLLRPStatus;
typedef struct LLRP_SLLRPStatus
    LLRP_tSLLRPStatus;

struct LLRP_SFieldError;
typedef struct LLRP_SFieldError
    LLRP_tSFieldError;

struct LLRP_SParameterError;
typedef struct LLRP_SParameterError
    LLRP_tSParameterError;

struct LLRP_SC1G2LLRPCapabilities;
typedef struct LLRP_SC1G2LLRPCapabilities
    LLRP_tSC1G2LLRPCapabilities;

struct LLRP_SC1G2UHFRFModeTable;
typedef struct LLRP_SC1G2UHFRFModeTable
    LLRP_tSC1G2UHFRFModeTable;

struct LLRP_SC1G2UHFRFModeTableEntry;
typedef struct LLRP_SC1G2UHFRFModeTableEntry
    LLRP_tSC1G2UHFRFModeTableEntry;

struct LLRP_SC1G2InventoryCommand;
typedef struct LLRP_SC1G2InventoryCommand
    LLRP_tSC1G2InventoryCommand;

struct LLRP_SC1G2Filter;
typedef struct LLRP_SC1G2Filter
    LLRP_tSC1G2Filter;

struct LLRP_SC1G2TagInventoryMask;
typedef struct LLRP_SC1G2TagInventoryMask
    LLRP_tSC1G2TagInventoryMask;

struct LLRP_SC1G2TagInventoryStateAwareFilterAction;
typedef struct LLRP_SC1G2TagInventoryStateAwareFilterAction
    LLRP_tSC1G2TagInventoryStateAwareFilterAction;

struct LLRP_SC1G2TagInventoryStateUnawareFilterAction;
typedef struct LLRP_SC1G2TagInventoryStateUnawareFilterAction
    LLRP_tSC1G2TagInventoryStateUnawareFilterAction;

struct LLRP_SC1G2RFControl;
typedef struct LLRP_SC1G2RFControl
    LLRP_tSC1G2RFControl;

struct LLRP_SC1G2SingulationControl;
typedef struct LLRP_SC1G2SingulationControl
    LLRP_tSC1G2SingulationControl;

struct LLRP_SC1G2TagInventoryStateAwareSingulationAction;
typedef struct LLRP_SC1G2TagInventoryStateAwareSingulationAction
    LLRP_tSC1G2TagInventoryStateAwareSingulationAction;

struct LLRP_SC1G2TagSpec;
typedef struct LLRP_SC1G2TagSpec
    LLRP_tSC1G2TagSpec;

struct LLRP_SC1G2TargetTag;
typedef struct LLRP_SC1G2TargetTag
    LLRP_tSC1G2TargetTag;

struct LLRP_SC1G2Read;
typedef struct LLRP_SC1G2Read
    LLRP_tSC1G2Read;

struct LLRP_SC1G2Write;
typedef struct LLRP_SC1G2Write
    LLRP_tSC1G2Write;

struct LLRP_SC1G2Kill;
typedef struct LLRP_SC1G2Kill
    LLRP_tSC1G2Kill;

struct LLRP_SC1G2Lock;
typedef struct LLRP_SC1G2Lock
    LLRP_tSC1G2Lock;

struct LLRP_SC1G2LockPayload;
typedef struct LLRP_SC1G2LockPayload
    LLRP_tSC1G2LockPayload;

struct LLRP_SC1G2BlockErase;
typedef struct LLRP_SC1G2BlockErase
    LLRP_tSC1G2BlockErase;

struct LLRP_SC1G2BlockWrite;
typedef struct LLRP_SC1G2BlockWrite
    LLRP_tSC1G2BlockWrite;

struct LLRP_SC1G2EPCMemorySelector;
typedef struct LLRP_SC1G2EPCMemorySelector
    LLRP_tSC1G2EPCMemorySelector;

struct LLRP_SC1G2_PC;
typedef struct LLRP_SC1G2_PC
    LLRP_tSC1G2_PC;

struct LLRP_SC1G2_CRC;
typedef struct LLRP_SC1G2_CRC
    LLRP_tSC1G2_CRC;

struct LLRP_SC1G2SingulationDetails;
typedef struct LLRP_SC1G2SingulationDetails
    LLRP_tSC1G2SingulationDetails;

struct LLRP_SC1G2ReadOpSpecResult;
typedef struct LLRP_SC1G2ReadOpSpecResult
    LLRP_tSC1G2ReadOpSpecResult;

struct LLRP_SC1G2WriteOpSpecResult;
typedef struct LLRP_SC1G2WriteOpSpecResult
    LLRP_tSC1G2WriteOpSpecResult;

struct LLRP_SC1G2KillOpSpecResult;
typedef struct LLRP_SC1G2KillOpSpecResult
    LLRP_tSC1G2KillOpSpecResult;

struct LLRP_SC1G2LockOpSpecResult;
typedef struct LLRP_SC1G2LockOpSpecResult
    LLRP_tSC1G2LockOpSpecResult;

struct LLRP_SC1G2BlockEraseOpSpecResult;
typedef struct LLRP_SC1G2BlockEraseOpSpecResult
    LLRP_tSC1G2BlockEraseOpSpecResult;

struct LLRP_SC1G2BlockWriteOpSpecResult;
typedef struct LLRP_SC1G2BlockWriteOpSpecResult
    LLRP_tSC1G2BlockWriteOpSpecResult;


/* Custom parameters */


/*
 * Vendor descriptor declarations.
 */


/*
 * Namespace descriptor declarations.
 */

extern const LLRP_tSNamespaceDescriptor
LLRP_nsdescllrp;


/*
 * Enumeration definitions and declarations of
 * enumeration string tables.
 */


enum LLRP_EAirProtocols
{

    LLRP_AirProtocols_Unspecified = 0,
    LLRP_AirProtocols_EPCGlobalClass1Gen2 = 1,
};

typedef enum LLRP_EAirProtocols
    LLRP_tEAirProtocols;

extern const LLRP_tSEnumTableEntry
LLRP_estAirProtocols[];


enum LLRP_EGetReaderCapabilitiesRequestedData
{

    LLRP_GetReaderCapabilitiesRequestedData_All = 0,
    LLRP_GetReaderCapabilitiesRequestedData_General_Device_Capabilities = 1,
    LLRP_GetReaderCapabilitiesRequestedData_LLRP_Capabilities = 2,
    LLRP_GetReaderCapabilitiesRequestedData_Regulatory_Capabilities = 3,
    LLRP_GetReaderCapabilitiesRequestedData_LLRP_Air_Protocol_Capabilities = 4,
};

typedef enum LLRP_EGetReaderCapabilitiesRequestedData
    LLRP_tEGetReaderCapabilitiesRequestedData;

extern const LLRP_tSEnumTableEntry
LLRP_estGetReaderCapabilitiesRequestedData[];


enum LLRP_ECommunicationsStandard
{

    LLRP_CommunicationsStandard_Unspecified = 0,
    LLRP_CommunicationsStandard_US_FCC_Part_15 = 1,
    LLRP_CommunicationsStandard_ETSI_302_208 = 2,
    LLRP_CommunicationsStandard_ETSI_300_220 = 3,
    LLRP_CommunicationsStandard_Australia_LIPD_1W = 4,
    LLRP_CommunicationsStandard_Australia_LIPD_4W = 5,
    LLRP_CommunicationsStandard_Japan_ARIB_STD_T89 = 6,
    LLRP_CommunicationsStandard_Hong_Kong_OFTA_1049 = 7,
    LLRP_CommunicationsStandard_Taiwan_DGT_LP0002 = 8,
    LLRP_CommunicationsStandard_Korea_MIC_Article_5_2 = 9,
};

typedef enum LLRP_ECommunicationsStandard
    LLRP_tECommunicationsStandard;

extern const LLRP_tSEnumTableEntry
LLRP_estCommunicationsStandard[];


enum LLRP_EROSpecState
{

    LLRP_ROSpecState_Disabled = 0,
    LLRP_ROSpecState_Inactive = 1,
    LLRP_ROSpecState_Active = 2,
};

typedef enum LLRP_EROSpecState
    LLRP_tEROSpecState;

extern const LLRP_tSEnumTableEntry
LLRP_estROSpecState[];


enum LLRP_EROSpecStartTriggerType
{

    LLRP_ROSpecStartTriggerType_Null = 0,
    LLRP_ROSpecStartTriggerType_Immediate = 1,
    LLRP_ROSpecStartTriggerType_Periodic = 2,
    LLRP_ROSpecStartTriggerType_GPI = 3,
};

typedef enum LLRP_EROSpecStartTriggerType
    LLRP_tEROSpecStartTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estROSpecStartTriggerType[];


enum LLRP_EROSpecStopTriggerType
{

    LLRP_ROSpecStopTriggerType_Null = 0,
    LLRP_ROSpecStopTriggerType_Duration = 1,
    LLRP_ROSpecStopTriggerType_GPI_With_Timeout = 2,
};

typedef enum LLRP_EROSpecStopTriggerType
    LLRP_tEROSpecStopTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estROSpecStopTriggerType[];


enum LLRP_EAISpecStopTriggerType
{

    LLRP_AISpecStopTriggerType_Null = 0,
    LLRP_AISpecStopTriggerType_Duration = 1,
    LLRP_AISpecStopTriggerType_GPI_With_Timeout = 2,
    LLRP_AISpecStopTriggerType_Tag_Observation = 3,
};

typedef enum LLRP_EAISpecStopTriggerType
    LLRP_tEAISpecStopTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estAISpecStopTriggerType[];


enum LLRP_ETagObservationTriggerType
{

    LLRP_TagObservationTriggerType_Upon_Seeing_N_Tags_Or_Timeout = 0,
    LLRP_TagObservationTriggerType_Upon_Seeing_No_More_New_Tags_For_Tms_Or_Timeout = 1,
    LLRP_TagObservationTriggerType_N_Attempts_To_See_All_Tags_In_FOV_Or_Timeout = 2,
};

typedef enum LLRP_ETagObservationTriggerType
    LLRP_tETagObservationTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estTagObservationTriggerType[];


enum LLRP_ERFSurveySpecStopTriggerType
{

    LLRP_RFSurveySpecStopTriggerType_Null = 0,
    LLRP_RFSurveySpecStopTriggerType_Duration = 1,
    LLRP_RFSurveySpecStopTriggerType_N_Iterations_Through_Frequency_Range = 2,
};

typedef enum LLRP_ERFSurveySpecStopTriggerType
    LLRP_tERFSurveySpecStopTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estRFSurveySpecStopTriggerType[];


enum LLRP_EAccessSpecState
{

    LLRP_AccessSpecState_Disabled = 0,
    LLRP_AccessSpecState_Active = 1,
};

typedef enum LLRP_EAccessSpecState
    LLRP_tEAccessSpecState;

extern const LLRP_tSEnumTableEntry
LLRP_estAccessSpecState[];


enum LLRP_EAccessSpecStopTriggerType
{

    LLRP_AccessSpecStopTriggerType_Null = 0,
    LLRP_AccessSpecStopTriggerType_Operation_Count = 1,
};

typedef enum LLRP_EAccessSpecStopTriggerType
    LLRP_tEAccessSpecStopTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estAccessSpecStopTriggerType[];


enum LLRP_EGetReaderConfigRequestedData
{

    LLRP_GetReaderConfigRequestedData_All = 0,
    LLRP_GetReaderConfigRequestedData_Identification = 1,
    LLRP_GetReaderConfigRequestedData_AntennaProperties = 2,
    LLRP_GetReaderConfigRequestedData_AntennaConfiguration = 3,
    LLRP_GetReaderConfigRequestedData_ROReportSpec = 4,
    LLRP_GetReaderConfigRequestedData_ReaderEventNotificationSpec = 5,
    LLRP_GetReaderConfigRequestedData_AccessReportSpec = 6,
    LLRP_GetReaderConfigRequestedData_LLRPConfigurationStateValue = 7,
    LLRP_GetReaderConfigRequestedData_KeepaliveSpec = 8,
    LLRP_GetReaderConfigRequestedData_GPIPortCurrentState = 9,
    LLRP_GetReaderConfigRequestedData_GPOWriteData = 10,
    LLRP_GetReaderConfigRequestedData_EventsAndReports = 11,
};

typedef enum LLRP_EGetReaderConfigRequestedData
    LLRP_tEGetReaderConfigRequestedData;

extern const LLRP_tSEnumTableEntry
LLRP_estGetReaderConfigRequestedData[];


enum LLRP_EIdentificationType
{

    LLRP_IdentificationType_MAC_Address = 0,
    LLRP_IdentificationType_EPC = 1,
};

typedef enum LLRP_EIdentificationType
    LLRP_tEIdentificationType;

extern const LLRP_tSEnumTableEntry
LLRP_estIdentificationType[];


enum LLRP_EKeepaliveTriggerType
{

    LLRP_KeepaliveTriggerType_Null = 0,
    LLRP_KeepaliveTriggerType_Periodic = 1,
};

typedef enum LLRP_EKeepaliveTriggerType
    LLRP_tEKeepaliveTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estKeepaliveTriggerType[];


enum LLRP_EGPIPortState
{

    LLRP_GPIPortState_Low = 0,
    LLRP_GPIPortState_High = 1,
    LLRP_GPIPortState_Unknown = 2,
};

typedef enum LLRP_EGPIPortState
    LLRP_tEGPIPortState;

extern const LLRP_tSEnumTableEntry
LLRP_estGPIPortState[];


enum LLRP_EROReportTriggerType
{

    LLRP_ROReportTriggerType_None = 0,
    LLRP_ROReportTriggerType_Upon_N_Tags_Or_End_Of_AISpec = 1,
    LLRP_ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec = 2,
};

typedef enum LLRP_EROReportTriggerType
    LLRP_tEROReportTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estROReportTriggerType[];


enum LLRP_EAccessReportTriggerType
{

    LLRP_AccessReportTriggerType_Whenever_ROReport_Is_Generated = 0,
    LLRP_AccessReportTriggerType_End_Of_AccessSpec = 1,
};

typedef enum LLRP_EAccessReportTriggerType
    LLRP_tEAccessReportTriggerType;

extern const LLRP_tSEnumTableEntry
LLRP_estAccessReportTriggerType[];


enum LLRP_ENotificationEventType
{

    LLRP_NotificationEventType_Upon_Hopping_To_Next_Channel = 0,
    LLRP_NotificationEventType_GPI_Event = 1,
    LLRP_NotificationEventType_ROSpec_Event = 2,
    LLRP_NotificationEventType_Report_Buffer_Fill_Warning = 3,
    LLRP_NotificationEventType_Reader_Exception_Event = 4,
    LLRP_NotificationEventType_RFSurvey_Event = 5,
    LLRP_NotificationEventType_AISpec_Event = 6,
    LLRP_NotificationEventType_AISpec_Event_With_Details = 7,
    LLRP_NotificationEventType_Antenna_Event = 8,
};

typedef enum LLRP_ENotificationEventType
    LLRP_tENotificationEventType;

extern const LLRP_tSEnumTableEntry
LLRP_estNotificationEventType[];


enum LLRP_EROSpecEventType
{

    LLRP_ROSpecEventType_Start_Of_ROSpec = 0,
    LLRP_ROSpecEventType_End_Of_ROSpec = 1,
    LLRP_ROSpecEventType_Preemption_Of_ROSpec = 2,
};

typedef enum LLRP_EROSpecEventType
    LLRP_tEROSpecEventType;

extern const LLRP_tSEnumTableEntry
LLRP_estROSpecEventType[];


enum LLRP_ERFSurveyEventType
{

    LLRP_RFSurveyEventType_Start_Of_RFSurvey = 0,
    LLRP_RFSurveyEventType_End_Of_RFSurvey = 1,
};

typedef enum LLRP_ERFSurveyEventType
    LLRP_tERFSurveyEventType;

extern const LLRP_tSEnumTableEntry
LLRP_estRFSurveyEventType[];


enum LLRP_EAISpecEventType
{

    LLRP_AISpecEventType_End_Of_AISpec = 0,
};

typedef enum LLRP_EAISpecEventType
    LLRP_tEAISpecEventType;

extern const LLRP_tSEnumTableEntry
LLRP_estAISpecEventType[];


enum LLRP_EAntennaEventType
{

    LLRP_AntennaEventType_Antenna_Disconnected = 0,
    LLRP_AntennaEventType_Antenna_Connected = 1,
};

typedef enum LLRP_EAntennaEventType
    LLRP_tEAntennaEventType;

extern const LLRP_tSEnumTableEntry
LLRP_estAntennaEventType[];


enum LLRP_EConnectionAttemptStatusType
{

    LLRP_ConnectionAttemptStatusType_Success = 0,
    LLRP_ConnectionAttemptStatusType_Failed_A_Reader_Initiated_Connection_Already_Exists = 1,
    LLRP_ConnectionAttemptStatusType_Failed_A_Client_Initiated_Connection_Already_Exists = 2,
    LLRP_ConnectionAttemptStatusType_Failed_Reason_Other_Than_A_Connection_Already_Exists = 3,
    LLRP_ConnectionAttemptStatusType_Another_Connection_Attempted = 4,
};

typedef enum LLRP_EConnectionAttemptStatusType
    LLRP_tEConnectionAttemptStatusType;

extern const LLRP_tSEnumTableEntry
LLRP_estConnectionAttemptStatusType[];


enum LLRP_EStatusCode
{

    LLRP_StatusCode_M_Success = 0,
    LLRP_StatusCode_M_ParameterError = 100,
    LLRP_StatusCode_M_FieldError = 101,
    LLRP_StatusCode_M_UnexpectedParameter = 102,
    LLRP_StatusCode_M_MissingParameter = 103,
    LLRP_StatusCode_M_DuplicateParameter = 104,
    LLRP_StatusCode_M_OverflowParameter = 105,
    LLRP_StatusCode_M_OverflowField = 106,
    LLRP_StatusCode_M_UnknownParameter = 107,
    LLRP_StatusCode_M_UnknownField = 108,
    LLRP_StatusCode_M_UnsupportedMessage = 109,
    LLRP_StatusCode_M_UnsupportedVersion = 110,
    LLRP_StatusCode_M_UnsupportedParameter = 111,
    LLRP_StatusCode_P_ParameterError = 200,
    LLRP_StatusCode_P_FieldError = 201,
    LLRP_StatusCode_P_UnexpectedParameter = 202,
    LLRP_StatusCode_P_MissingParameter = 203,
    LLRP_StatusCode_P_DuplicateParameter = 204,
    LLRP_StatusCode_P_OverflowParameter = 205,
    LLRP_StatusCode_P_OverflowField = 206,
    LLRP_StatusCode_P_UnknownParameter = 207,
    LLRP_StatusCode_P_UnknownField = 208,
    LLRP_StatusCode_P_UnsupportedParameter = 209,
    LLRP_StatusCode_A_Invalid = 300,
    LLRP_StatusCode_A_OutOfRange = 301,
    LLRP_StatusCode_R_DeviceError = 401,
};

typedef enum LLRP_EStatusCode
    LLRP_tEStatusCode;

extern const LLRP_tSEnumTableEntry
LLRP_estStatusCode[];


enum LLRP_EC1G2DRValue
{

    LLRP_C1G2DRValue_DRV_8 = 0,
    LLRP_C1G2DRValue_DRV_64_3 = 1,
};

typedef enum LLRP_EC1G2DRValue
    LLRP_tEC1G2DRValue;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2DRValue[];


enum LLRP_EC1G2MValue
{

    LLRP_C1G2MValue_MV_FM0 = 0,
    LLRP_C1G2MValue_MV_2 = 1,
    LLRP_C1G2MValue_MV_4 = 2,
    LLRP_C1G2MValue_MV_8 = 3,
};

typedef enum LLRP_EC1G2MValue
    LLRP_tEC1G2MValue;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2MValue[];


enum LLRP_EC1G2ForwardLinkModulation
{

    LLRP_C1G2ForwardLinkModulation_PR_ASK = 0,
    LLRP_C1G2ForwardLinkModulation_SSB_ASK = 1,
    LLRP_C1G2ForwardLinkModulation_DSB_ASK = 2,
};

typedef enum LLRP_EC1G2ForwardLinkModulation
    LLRP_tEC1G2ForwardLinkModulation;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2ForwardLinkModulation[];


enum LLRP_EC1G2SpectralMaskIndicator
{

    LLRP_C1G2SpectralMaskIndicator_Unknown = 0,
    LLRP_C1G2SpectralMaskIndicator_SI = 1,
    LLRP_C1G2SpectralMaskIndicator_MI = 2,
    LLRP_C1G2SpectralMaskIndicator_DI = 3,
};

typedef enum LLRP_EC1G2SpectralMaskIndicator
    LLRP_tEC1G2SpectralMaskIndicator;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2SpectralMaskIndicator[];


enum LLRP_EC1G2TruncateAction
{

    LLRP_C1G2TruncateAction_Unspecified = 0,
    LLRP_C1G2TruncateAction_Do_Not_Truncate = 1,
    LLRP_C1G2TruncateAction_Truncate = 2,
};

typedef enum LLRP_EC1G2TruncateAction
    LLRP_tEC1G2TruncateAction;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2TruncateAction[];


enum LLRP_EC1G2StateAwareTarget
{

    LLRP_C1G2StateAwareTarget_SL = 0,
    LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S0 = 1,
    LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S1 = 2,
    LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S2 = 3,
    LLRP_C1G2StateAwareTarget_Inventoried_State_For_Session_S3 = 4,
};

typedef enum LLRP_EC1G2StateAwareTarget
    LLRP_tEC1G2StateAwareTarget;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2StateAwareTarget[];


enum LLRP_EC1G2StateAwareAction
{

    LLRP_C1G2StateAwareAction_AssertSLOrA_DeassertSLOrB = 0,
    LLRP_C1G2StateAwareAction_AssertSLOrA_Noop = 1,
    LLRP_C1G2StateAwareAction_Noop_DeassertSLOrB = 2,
    LLRP_C1G2StateAwareAction_NegateSLOrABBA_Noop = 3,
    LLRP_C1G2StateAwareAction_DeassertSLOrB_AssertSLOrA = 4,
    LLRP_C1G2StateAwareAction_DeassertSLOrB_Noop = 5,
    LLRP_C1G2StateAwareAction_Noop_AssertSLOrA = 6,
    LLRP_C1G2StateAwareAction_Noop_NegateSLOrABBA = 7,
};

typedef enum LLRP_EC1G2StateAwareAction
    LLRP_tEC1G2StateAwareAction;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2StateAwareAction[];


enum LLRP_EC1G2StateUnawareAction
{

    LLRP_C1G2StateUnawareAction_Select_Unselect = 0,
    LLRP_C1G2StateUnawareAction_Select_DoNothing = 1,
    LLRP_C1G2StateUnawareAction_DoNothing_Unselect = 2,
    LLRP_C1G2StateUnawareAction_Unselect_DoNothing = 3,
    LLRP_C1G2StateUnawareAction_Unselect_Select = 4,
    LLRP_C1G2StateUnawareAction_DoNothing_Select = 5,
};

typedef enum LLRP_EC1G2StateUnawareAction
    LLRP_tEC1G2StateUnawareAction;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2StateUnawareAction[];


enum LLRP_EC1G2TagInventoryStateAwareI
{

    LLRP_C1G2TagInventoryStateAwareI_State_A = 0,
    LLRP_C1G2TagInventoryStateAwareI_State_B = 1,
};

typedef enum LLRP_EC1G2TagInventoryStateAwareI
    LLRP_tEC1G2TagInventoryStateAwareI;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2TagInventoryStateAwareI[];


enum LLRP_EC1G2TagInventoryStateAwareS
{

    LLRP_C1G2TagInventoryStateAwareS_SL = 0,
    LLRP_C1G2TagInventoryStateAwareS_Not_SL = 1,
};

typedef enum LLRP_EC1G2TagInventoryStateAwareS
    LLRP_tEC1G2TagInventoryStateAwareS;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2TagInventoryStateAwareS[];


enum LLRP_EC1G2LockPrivilege
{

    LLRP_C1G2LockPrivilege_Read_Write = 0,
    LLRP_C1G2LockPrivilege_Perma_Lock = 1,
    LLRP_C1G2LockPrivilege_Perma_Unlock = 2,
    LLRP_C1G2LockPrivilege_Unlock = 3,
};

typedef enum LLRP_EC1G2LockPrivilege
    LLRP_tEC1G2LockPrivilege;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2LockPrivilege[];


enum LLRP_EC1G2LockDataField
{

    LLRP_C1G2LockDataField_Kill_Password = 0,
    LLRP_C1G2LockDataField_Access_Password = 1,
    LLRP_C1G2LockDataField_EPC_Memory = 2,
    LLRP_C1G2LockDataField_TID_Memory = 3,
    LLRP_C1G2LockDataField_User_Memory = 4,
};

typedef enum LLRP_EC1G2LockDataField
    LLRP_tEC1G2LockDataField;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2LockDataField[];


enum LLRP_EC1G2ReadResultType
{

    LLRP_C1G2ReadResultType_Success = 0,
    LLRP_C1G2ReadResultType_Nonspecific_Tag_Error = 1,
    LLRP_C1G2ReadResultType_No_Response_From_Tag = 2,
    LLRP_C1G2ReadResultType_Nonspecific_Reader_Error = 3,
    LLRP_C1G2ReadResultType_Memory_Overrun_Error = 4,
    LLRP_C1G2ReadResultType_Memory_Locked_Error = 5,
};

typedef enum LLRP_EC1G2ReadResultType
    LLRP_tEC1G2ReadResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2ReadResultType[];


enum LLRP_EC1G2WriteResultType
{

    LLRP_C1G2WriteResultType_Success = 0,
    LLRP_C1G2WriteResultType_Tag_Memory_Overrun_Error = 1,
    LLRP_C1G2WriteResultType_Tag_Memory_Locked_Error = 2,
    LLRP_C1G2WriteResultType_Insufficient_Power = 3,
    LLRP_C1G2WriteResultType_Nonspecific_Tag_Error = 4,
    LLRP_C1G2WriteResultType_No_Response_From_Tag = 5,
    LLRP_C1G2WriteResultType_Nonspecific_Reader_Error = 6,
};

typedef enum LLRP_EC1G2WriteResultType
    LLRP_tEC1G2WriteResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2WriteResultType[];


enum LLRP_EC1G2KillResultType
{

    LLRP_C1G2KillResultType_Success = 0,
    LLRP_C1G2KillResultType_Zero_Kill_Password_Error = 1,
    LLRP_C1G2KillResultType_Insufficient_Power = 2,
    LLRP_C1G2KillResultType_Nonspecific_Tag_Error = 3,
    LLRP_C1G2KillResultType_No_Response_From_Tag = 4,
    LLRP_C1G2KillResultType_Nonspecific_Reader_Error = 5,
};

typedef enum LLRP_EC1G2KillResultType
    LLRP_tEC1G2KillResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2KillResultType[];


enum LLRP_EC1G2LockResultType
{

    LLRP_C1G2LockResultType_Success = 0,
    LLRP_C1G2LockResultType_Insufficient_Power = 1,
    LLRP_C1G2LockResultType_Nonspecific_Tag_Error = 2,
    LLRP_C1G2LockResultType_No_Response_From_Tag = 3,
    LLRP_C1G2LockResultType_Nonspecific_Reader_Error = 4,
};

typedef enum LLRP_EC1G2LockResultType
    LLRP_tEC1G2LockResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2LockResultType[];


enum LLRP_EC1G2BlockEraseResultType
{

    LLRP_C1G2BlockEraseResultType_Success = 0,
    LLRP_C1G2BlockEraseResultType_Tag_Memory_Overrun_Error = 1,
    LLRP_C1G2BlockEraseResultType_Tag_Memory_Locked_Error = 2,
    LLRP_C1G2BlockEraseResultType_Insufficient_Power = 3,
    LLRP_C1G2BlockEraseResultType_Nonspecific_Tag_Error = 4,
    LLRP_C1G2BlockEraseResultType_No_Response_From_Tag = 5,
    LLRP_C1G2BlockEraseResultType_Nonspecific_Reader_Error = 6,
};

typedef enum LLRP_EC1G2BlockEraseResultType
    LLRP_tEC1G2BlockEraseResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2BlockEraseResultType[];


enum LLRP_EC1G2BlockWriteResultType
{

    LLRP_C1G2BlockWriteResultType_Success = 0,
    LLRP_C1G2BlockWriteResultType_Tag_Memory_Overrun_Error = 1,
    LLRP_C1G2BlockWriteResultType_Tag_Memory_Locked_Error = 2,
    LLRP_C1G2BlockWriteResultType_Insufficient_Power = 3,
    LLRP_C1G2BlockWriteResultType_Nonspecific_Tag_Error = 4,
    LLRP_C1G2BlockWriteResultType_No_Response_From_Tag = 5,
    LLRP_C1G2BlockWriteResultType_Nonspecific_Reader_Error = 6,
};

typedef enum LLRP_EC1G2BlockWriteResultType
    LLRP_tEC1G2BlockWriteResultType;

extern const LLRP_tSEnumTableEntry
LLRP_estC1G2BlockWriteResultType[];


  
struct LLRP_SCUSTOM_MESSAGE
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  VendorIdentifier;

    llrp_u8_t  MessageSubtype;

    llrp_bytesToEnd_t  Data;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdCUSTOM_MESSAGE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdCUSTOM_MESSAGE[];

extern LLRP_tSCUSTOM_MESSAGE *
LLRP_CUSTOM_MESSAGE_construct (void);

extern void
LLRP_CUSTOM_MESSAGE_destruct (
 LLRP_tSCUSTOM_MESSAGE * pThis);

extern void
LLRP_CUSTOM_MESSAGE_decodeFields (
 LLRP_tSCUSTOM_MESSAGE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_CUSTOM_MESSAGE_assimilateSubParameters (
 LLRP_tSCUSTOM_MESSAGE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_CUSTOM_MESSAGE_encode (
  const LLRP_tSCUSTOM_MESSAGE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdCUSTOM_MESSAGE_VendorIdentifier;

extern llrp_u32_t
LLRP_CUSTOM_MESSAGE_getVendorIdentifier (
  LLRP_tSCUSTOM_MESSAGE *pThis);

extern LLRP_tResultCode
LLRP_CUSTOM_MESSAGE_setVendorIdentifier (
  LLRP_tSCUSTOM_MESSAGE *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdCUSTOM_MESSAGE_MessageSubtype;

extern llrp_u8_t
LLRP_CUSTOM_MESSAGE_getMessageSubtype (
  LLRP_tSCUSTOM_MESSAGE *pThis);

extern LLRP_tResultCode
LLRP_CUSTOM_MESSAGE_setMessageSubtype (
  LLRP_tSCUSTOM_MESSAGE *pThis,
  llrp_u8_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdCUSTOM_MESSAGE_Data;

extern llrp_bytesToEnd_t
LLRP_CUSTOM_MESSAGE_getData (
  LLRP_tSCUSTOM_MESSAGE *pThis);

extern LLRP_tResultCode
LLRP_CUSTOM_MESSAGE_setData (
  LLRP_tSCUSTOM_MESSAGE *pThis,
  llrp_bytesToEnd_t Value);

  


  
struct LLRP_SGET_READER_CAPABILITIES
{
    LLRP_tSMessage hdr;
  
    LLRP_tEGetReaderCapabilitiesRequestedData  eRequestedData;

  
    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_READER_CAPABILITIES;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_READER_CAPABILITIES[];

extern LLRP_tSGET_READER_CAPABILITIES *
LLRP_GET_READER_CAPABILITIES_construct (void);

extern void
LLRP_GET_READER_CAPABILITIES_destruct (
 LLRP_tSGET_READER_CAPABILITIES * pThis);

extern void
LLRP_GET_READER_CAPABILITIES_decodeFields (
 LLRP_tSGET_READER_CAPABILITIES * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_READER_CAPABILITIES_assimilateSubParameters (
 LLRP_tSGET_READER_CAPABILITIES * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_READER_CAPABILITIES_encode (
  const LLRP_tSGET_READER_CAPABILITIES *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGET_READER_CAPABILITIES_RequestedData;

extern LLRP_tEGetReaderCapabilitiesRequestedData
LLRP_GET_READER_CAPABILITIES_getRequestedData (
  LLRP_tSGET_READER_CAPABILITIES *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_setRequestedData (
  LLRP_tSGET_READER_CAPABILITIES *pThis,
  LLRP_tEGetReaderCapabilitiesRequestedData Value);

  
extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_beginCustom (
  LLRP_tSGET_READER_CAPABILITIES *pThis);

extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_GET_READER_CAPABILITIES_clearCustom (
  LLRP_tSGET_READER_CAPABILITIES *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_addCustom (
  LLRP_tSGET_READER_CAPABILITIES *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SGET_READER_CAPABILITIES_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

    LLRP_tSGeneralDeviceCapabilities * pGeneralDeviceCapabilities;

    LLRP_tSLLRPCapabilities * pLLRPCapabilities;

    LLRP_tSRegulatoryCapabilities * pRegulatoryCapabilities;

    LLRP_tSParameter * listAirProtocolLLRPCapabilities;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_READER_CAPABILITIES_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_READER_CAPABILITIES_RESPONSE[];

extern LLRP_tSGET_READER_CAPABILITIES_RESPONSE *
LLRP_GET_READER_CAPABILITIES_RESPONSE_construct (void);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_destruct (
 LLRP_tSGET_READER_CAPABILITIES_RESPONSE * pThis);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_decodeFields (
 LLRP_tSGET_READER_CAPABILITIES_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_assimilateSubParameters (
 LLRP_tSGET_READER_CAPABILITIES_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_encode (
  const LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_GET_READER_CAPABILITIES_RESPONSE_getLLRPStatus (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_setLLRPStatus (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);

extern LLRP_tSGeneralDeviceCapabilities *
LLRP_GET_READER_CAPABILITIES_RESPONSE_getGeneralDeviceCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_setGeneralDeviceCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSGeneralDeviceCapabilities *pValue);

extern LLRP_tSLLRPCapabilities *
LLRP_GET_READER_CAPABILITIES_RESPONSE_getLLRPCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_setLLRPCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSLLRPCapabilities *pValue);

extern LLRP_tSRegulatoryCapabilities *
LLRP_GET_READER_CAPABILITIES_RESPONSE_getRegulatoryCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_setRegulatoryCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSRegulatoryCapabilities *pValue);

extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_RESPONSE_beginAirProtocolLLRPCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_RESPONSE_nextAirProtocolLLRPCapabilities (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_clearAirProtocolLLRPCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_addAirProtocolLLRPCapabilities (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSParameter *pValue);


extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_RESPONSE_beginCustom (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tSParameter *
LLRP_GET_READER_CAPABILITIES_RESPONSE_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_GET_READER_CAPABILITIES_RESPONSE_clearCustom (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CAPABILITIES_RESPONSE_addCustom (
  LLRP_tSGET_READER_CAPABILITIES_RESPONSE *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SADD_ROSPEC
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSROSpec * pROSpec;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdADD_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdADD_ROSPEC[];

extern LLRP_tSADD_ROSPEC *
LLRP_ADD_ROSPEC_construct (void);

extern void
LLRP_ADD_ROSPEC_destruct (
 LLRP_tSADD_ROSPEC * pThis);

extern void
LLRP_ADD_ROSPEC_decodeFields (
 LLRP_tSADD_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ADD_ROSPEC_assimilateSubParameters (
 LLRP_tSADD_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ADD_ROSPEC_encode (
  const LLRP_tSADD_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSROSpec *
LLRP_ADD_ROSPEC_getROSpec (
  LLRP_tSADD_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_ADD_ROSPEC_setROSpec (
  LLRP_tSADD_ROSPEC *pThis,
  LLRP_tSROSpec *pValue);



  
struct LLRP_SADD_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdADD_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdADD_ROSPEC_RESPONSE[];

extern LLRP_tSADD_ROSPEC_RESPONSE *
LLRP_ADD_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_ADD_ROSPEC_RESPONSE_destruct (
 LLRP_tSADD_ROSPEC_RESPONSE * pThis);

extern void
LLRP_ADD_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSADD_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ADD_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSADD_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ADD_ROSPEC_RESPONSE_encode (
  const LLRP_tSADD_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_ADD_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSADD_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_ADD_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSADD_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SDELETE_ROSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdDELETE_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDELETE_ROSPEC[];

extern LLRP_tSDELETE_ROSPEC *
LLRP_DELETE_ROSPEC_construct (void);

extern void
LLRP_DELETE_ROSPEC_destruct (
 LLRP_tSDELETE_ROSPEC * pThis);

extern void
LLRP_DELETE_ROSPEC_decodeFields (
 LLRP_tSDELETE_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DELETE_ROSPEC_assimilateSubParameters (
 LLRP_tSDELETE_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DELETE_ROSPEC_encode (
  const LLRP_tSDELETE_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdDELETE_ROSPEC_ROSpecID;

extern llrp_u32_t
LLRP_DELETE_ROSPEC_getROSpecID (
  LLRP_tSDELETE_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_DELETE_ROSPEC_setROSpecID (
  LLRP_tSDELETE_ROSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SDELETE_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdDELETE_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDELETE_ROSPEC_RESPONSE[];

extern LLRP_tSDELETE_ROSPEC_RESPONSE *
LLRP_DELETE_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_DELETE_ROSPEC_RESPONSE_destruct (
 LLRP_tSDELETE_ROSPEC_RESPONSE * pThis);

extern void
LLRP_DELETE_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSDELETE_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DELETE_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSDELETE_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DELETE_ROSPEC_RESPONSE_encode (
  const LLRP_tSDELETE_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_DELETE_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSDELETE_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_DELETE_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSDELETE_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SSTART_ROSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdSTART_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSTART_ROSPEC[];

extern LLRP_tSSTART_ROSPEC *
LLRP_START_ROSPEC_construct (void);

extern void
LLRP_START_ROSPEC_destruct (
 LLRP_tSSTART_ROSPEC * pThis);

extern void
LLRP_START_ROSPEC_decodeFields (
 LLRP_tSSTART_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_START_ROSPEC_assimilateSubParameters (
 LLRP_tSSTART_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_START_ROSPEC_encode (
  const LLRP_tSSTART_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdSTART_ROSPEC_ROSpecID;

extern llrp_u32_t
LLRP_START_ROSPEC_getROSpecID (
  LLRP_tSSTART_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_START_ROSPEC_setROSpecID (
  LLRP_tSSTART_ROSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SSTART_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdSTART_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSTART_ROSPEC_RESPONSE[];

extern LLRP_tSSTART_ROSPEC_RESPONSE *
LLRP_START_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_START_ROSPEC_RESPONSE_destruct (
 LLRP_tSSTART_ROSPEC_RESPONSE * pThis);

extern void
LLRP_START_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSSTART_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_START_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSSTART_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_START_ROSPEC_RESPONSE_encode (
  const LLRP_tSSTART_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_START_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSSTART_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_START_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSSTART_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SSTOP_ROSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdSTOP_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSTOP_ROSPEC[];

extern LLRP_tSSTOP_ROSPEC *
LLRP_STOP_ROSPEC_construct (void);

extern void
LLRP_STOP_ROSPEC_destruct (
 LLRP_tSSTOP_ROSPEC * pThis);

extern void
LLRP_STOP_ROSPEC_decodeFields (
 LLRP_tSSTOP_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_STOP_ROSPEC_assimilateSubParameters (
 LLRP_tSSTOP_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_STOP_ROSPEC_encode (
  const LLRP_tSSTOP_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdSTOP_ROSPEC_ROSpecID;

extern llrp_u32_t
LLRP_STOP_ROSPEC_getROSpecID (
  LLRP_tSSTOP_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_STOP_ROSPEC_setROSpecID (
  LLRP_tSSTOP_ROSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SSTOP_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdSTOP_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSTOP_ROSPEC_RESPONSE[];

extern LLRP_tSSTOP_ROSPEC_RESPONSE *
LLRP_STOP_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_STOP_ROSPEC_RESPONSE_destruct (
 LLRP_tSSTOP_ROSPEC_RESPONSE * pThis);

extern void
LLRP_STOP_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSSTOP_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_STOP_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSSTOP_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_STOP_ROSPEC_RESPONSE_encode (
  const LLRP_tSSTOP_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_STOP_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSSTOP_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_STOP_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSSTOP_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SENABLE_ROSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdENABLE_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdENABLE_ROSPEC[];

extern LLRP_tSENABLE_ROSPEC *
LLRP_ENABLE_ROSPEC_construct (void);

extern void
LLRP_ENABLE_ROSPEC_destruct (
 LLRP_tSENABLE_ROSPEC * pThis);

extern void
LLRP_ENABLE_ROSPEC_decodeFields (
 LLRP_tSENABLE_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ENABLE_ROSPEC_assimilateSubParameters (
 LLRP_tSENABLE_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ENABLE_ROSPEC_encode (
  const LLRP_tSENABLE_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdENABLE_ROSPEC_ROSpecID;

extern llrp_u32_t
LLRP_ENABLE_ROSPEC_getROSpecID (
  LLRP_tSENABLE_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_ENABLE_ROSPEC_setROSpecID (
  LLRP_tSENABLE_ROSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SENABLE_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdENABLE_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdENABLE_ROSPEC_RESPONSE[];

extern LLRP_tSENABLE_ROSPEC_RESPONSE *
LLRP_ENABLE_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_ENABLE_ROSPEC_RESPONSE_destruct (
 LLRP_tSENABLE_ROSPEC_RESPONSE * pThis);

extern void
LLRP_ENABLE_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSENABLE_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ENABLE_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSENABLE_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ENABLE_ROSPEC_RESPONSE_encode (
  const LLRP_tSENABLE_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_ENABLE_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSENABLE_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_ENABLE_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSENABLE_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SDISABLE_ROSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdDISABLE_ROSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDISABLE_ROSPEC[];

extern LLRP_tSDISABLE_ROSPEC *
LLRP_DISABLE_ROSPEC_construct (void);

extern void
LLRP_DISABLE_ROSPEC_destruct (
 LLRP_tSDISABLE_ROSPEC * pThis);

extern void
LLRP_DISABLE_ROSPEC_decodeFields (
 LLRP_tSDISABLE_ROSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DISABLE_ROSPEC_assimilateSubParameters (
 LLRP_tSDISABLE_ROSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DISABLE_ROSPEC_encode (
  const LLRP_tSDISABLE_ROSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdDISABLE_ROSPEC_ROSpecID;

extern llrp_u32_t
LLRP_DISABLE_ROSPEC_getROSpecID (
  LLRP_tSDISABLE_ROSPEC *pThis);

extern LLRP_tResultCode
LLRP_DISABLE_ROSPEC_setROSpecID (
  LLRP_tSDISABLE_ROSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SDISABLE_ROSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdDISABLE_ROSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDISABLE_ROSPEC_RESPONSE[];

extern LLRP_tSDISABLE_ROSPEC_RESPONSE *
LLRP_DISABLE_ROSPEC_RESPONSE_construct (void);

extern void
LLRP_DISABLE_ROSPEC_RESPONSE_destruct (
 LLRP_tSDISABLE_ROSPEC_RESPONSE * pThis);

extern void
LLRP_DISABLE_ROSPEC_RESPONSE_decodeFields (
 LLRP_tSDISABLE_ROSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DISABLE_ROSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSDISABLE_ROSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DISABLE_ROSPEC_RESPONSE_encode (
  const LLRP_tSDISABLE_ROSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_DISABLE_ROSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSDISABLE_ROSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_DISABLE_ROSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSDISABLE_ROSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SGET_ROSPECS
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_ROSPECS;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_ROSPECS[];

extern LLRP_tSGET_ROSPECS *
LLRP_GET_ROSPECS_construct (void);

extern void
LLRP_GET_ROSPECS_destruct (
 LLRP_tSGET_ROSPECS * pThis);

extern void
LLRP_GET_ROSPECS_decodeFields (
 LLRP_tSGET_ROSPECS * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_ROSPECS_assimilateSubParameters (
 LLRP_tSGET_ROSPECS * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_ROSPECS_encode (
  const LLRP_tSGET_ROSPECS *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SGET_ROSPECS_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

    LLRP_tSROSpec * listROSpec;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_ROSPECS_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_ROSPECS_RESPONSE[];

extern LLRP_tSGET_ROSPECS_RESPONSE *
LLRP_GET_ROSPECS_RESPONSE_construct (void);

extern void
LLRP_GET_ROSPECS_RESPONSE_destruct (
 LLRP_tSGET_ROSPECS_RESPONSE * pThis);

extern void
LLRP_GET_ROSPECS_RESPONSE_decodeFields (
 LLRP_tSGET_ROSPECS_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_ROSPECS_RESPONSE_assimilateSubParameters (
 LLRP_tSGET_ROSPECS_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_ROSPECS_RESPONSE_encode (
  const LLRP_tSGET_ROSPECS_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_GET_ROSPECS_RESPONSE_getLLRPStatus (
  LLRP_tSGET_ROSPECS_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_ROSPECS_RESPONSE_setLLRPStatus (
  LLRP_tSGET_ROSPECS_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);

extern LLRP_tSROSpec *
LLRP_GET_ROSPECS_RESPONSE_beginROSpec (
  LLRP_tSGET_ROSPECS_RESPONSE *pThis);

extern LLRP_tSROSpec *
LLRP_GET_ROSPECS_RESPONSE_nextROSpec (
  LLRP_tSROSpec *pCurrent);

extern void
LLRP_GET_ROSPECS_RESPONSE_clearROSpec (
  LLRP_tSGET_ROSPECS_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_ROSPECS_RESPONSE_addROSpec (
  LLRP_tSGET_ROSPECS_RESPONSE *pThis,
  LLRP_tSROSpec *pValue);




  
struct LLRP_SADD_ACCESSSPEC
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSAccessSpec * pAccessSpec;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdADD_ACCESSSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdADD_ACCESSSPEC[];

extern LLRP_tSADD_ACCESSSPEC *
LLRP_ADD_ACCESSSPEC_construct (void);

extern void
LLRP_ADD_ACCESSSPEC_destruct (
 LLRP_tSADD_ACCESSSPEC * pThis);

extern void
LLRP_ADD_ACCESSSPEC_decodeFields (
 LLRP_tSADD_ACCESSSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ADD_ACCESSSPEC_assimilateSubParameters (
 LLRP_tSADD_ACCESSSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ADD_ACCESSSPEC_encode (
  const LLRP_tSADD_ACCESSSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSAccessSpec *
LLRP_ADD_ACCESSSPEC_getAccessSpec (
  LLRP_tSADD_ACCESSSPEC *pThis);

extern LLRP_tResultCode
LLRP_ADD_ACCESSSPEC_setAccessSpec (
  LLRP_tSADD_ACCESSSPEC *pThis,
  LLRP_tSAccessSpec *pValue);



  
struct LLRP_SADD_ACCESSSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdADD_ACCESSSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdADD_ACCESSSPEC_RESPONSE[];

extern LLRP_tSADD_ACCESSSPEC_RESPONSE *
LLRP_ADD_ACCESSSPEC_RESPONSE_construct (void);

extern void
LLRP_ADD_ACCESSSPEC_RESPONSE_destruct (
 LLRP_tSADD_ACCESSSPEC_RESPONSE * pThis);

extern void
LLRP_ADD_ACCESSSPEC_RESPONSE_decodeFields (
 LLRP_tSADD_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ADD_ACCESSSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSADD_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ADD_ACCESSSPEC_RESPONSE_encode (
  const LLRP_tSADD_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_ADD_ACCESSSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSADD_ACCESSSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_ADD_ACCESSSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSADD_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SDELETE_ACCESSSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  AccessSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdDELETE_ACCESSSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDELETE_ACCESSSPEC[];

extern LLRP_tSDELETE_ACCESSSPEC *
LLRP_DELETE_ACCESSSPEC_construct (void);

extern void
LLRP_DELETE_ACCESSSPEC_destruct (
 LLRP_tSDELETE_ACCESSSPEC * pThis);

extern void
LLRP_DELETE_ACCESSSPEC_decodeFields (
 LLRP_tSDELETE_ACCESSSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DELETE_ACCESSSPEC_assimilateSubParameters (
 LLRP_tSDELETE_ACCESSSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DELETE_ACCESSSPEC_encode (
  const LLRP_tSDELETE_ACCESSSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdDELETE_ACCESSSPEC_AccessSpecID;

extern llrp_u32_t
LLRP_DELETE_ACCESSSPEC_getAccessSpecID (
  LLRP_tSDELETE_ACCESSSPEC *pThis);

extern LLRP_tResultCode
LLRP_DELETE_ACCESSSPEC_setAccessSpecID (
  LLRP_tSDELETE_ACCESSSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SDELETE_ACCESSSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdDELETE_ACCESSSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDELETE_ACCESSSPEC_RESPONSE[];

extern LLRP_tSDELETE_ACCESSSPEC_RESPONSE *
LLRP_DELETE_ACCESSSPEC_RESPONSE_construct (void);

extern void
LLRP_DELETE_ACCESSSPEC_RESPONSE_destruct (
 LLRP_tSDELETE_ACCESSSPEC_RESPONSE * pThis);

extern void
LLRP_DELETE_ACCESSSPEC_RESPONSE_decodeFields (
 LLRP_tSDELETE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DELETE_ACCESSSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSDELETE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DELETE_ACCESSSPEC_RESPONSE_encode (
  const LLRP_tSDELETE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_DELETE_ACCESSSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSDELETE_ACCESSSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_DELETE_ACCESSSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSDELETE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SENABLE_ACCESSSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  AccessSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdENABLE_ACCESSSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdENABLE_ACCESSSPEC[];

extern LLRP_tSENABLE_ACCESSSPEC *
LLRP_ENABLE_ACCESSSPEC_construct (void);

extern void
LLRP_ENABLE_ACCESSSPEC_destruct (
 LLRP_tSENABLE_ACCESSSPEC * pThis);

extern void
LLRP_ENABLE_ACCESSSPEC_decodeFields (
 LLRP_tSENABLE_ACCESSSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ENABLE_ACCESSSPEC_assimilateSubParameters (
 LLRP_tSENABLE_ACCESSSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ENABLE_ACCESSSPEC_encode (
  const LLRP_tSENABLE_ACCESSSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdENABLE_ACCESSSPEC_AccessSpecID;

extern llrp_u32_t
LLRP_ENABLE_ACCESSSPEC_getAccessSpecID (
  LLRP_tSENABLE_ACCESSSPEC *pThis);

extern LLRP_tResultCode
LLRP_ENABLE_ACCESSSPEC_setAccessSpecID (
  LLRP_tSENABLE_ACCESSSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SENABLE_ACCESSSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdENABLE_ACCESSSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdENABLE_ACCESSSPEC_RESPONSE[];

extern LLRP_tSENABLE_ACCESSSPEC_RESPONSE *
LLRP_ENABLE_ACCESSSPEC_RESPONSE_construct (void);

extern void
LLRP_ENABLE_ACCESSSPEC_RESPONSE_destruct (
 LLRP_tSENABLE_ACCESSSPEC_RESPONSE * pThis);

extern void
LLRP_ENABLE_ACCESSSPEC_RESPONSE_decodeFields (
 LLRP_tSENABLE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ENABLE_ACCESSSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSENABLE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ENABLE_ACCESSSPEC_RESPONSE_encode (
  const LLRP_tSENABLE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_ENABLE_ACCESSSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSENABLE_ACCESSSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_ENABLE_ACCESSSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSENABLE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SDISABLE_ACCESSSPEC
{
    LLRP_tSMessage hdr;
  
    llrp_u32_t  AccessSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdDISABLE_ACCESSSPEC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDISABLE_ACCESSSPEC[];

extern LLRP_tSDISABLE_ACCESSSPEC *
LLRP_DISABLE_ACCESSSPEC_construct (void);

extern void
LLRP_DISABLE_ACCESSSPEC_destruct (
 LLRP_tSDISABLE_ACCESSSPEC * pThis);

extern void
LLRP_DISABLE_ACCESSSPEC_decodeFields (
 LLRP_tSDISABLE_ACCESSSPEC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DISABLE_ACCESSSPEC_assimilateSubParameters (
 LLRP_tSDISABLE_ACCESSSPEC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DISABLE_ACCESSSPEC_encode (
  const LLRP_tSDISABLE_ACCESSSPEC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdDISABLE_ACCESSSPEC_AccessSpecID;

extern llrp_u32_t
LLRP_DISABLE_ACCESSSPEC_getAccessSpecID (
  LLRP_tSDISABLE_ACCESSSPEC *pThis);

extern LLRP_tResultCode
LLRP_DISABLE_ACCESSSPEC_setAccessSpecID (
  LLRP_tSDISABLE_ACCESSSPEC *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SDISABLE_ACCESSSPEC_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdDISABLE_ACCESSSPEC_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdDISABLE_ACCESSSPEC_RESPONSE[];

extern LLRP_tSDISABLE_ACCESSSPEC_RESPONSE *
LLRP_DISABLE_ACCESSSPEC_RESPONSE_construct (void);

extern void
LLRP_DISABLE_ACCESSSPEC_RESPONSE_destruct (
 LLRP_tSDISABLE_ACCESSSPEC_RESPONSE * pThis);

extern void
LLRP_DISABLE_ACCESSSPEC_RESPONSE_decodeFields (
 LLRP_tSDISABLE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_DISABLE_ACCESSSPEC_RESPONSE_assimilateSubParameters (
 LLRP_tSDISABLE_ACCESSSPEC_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_DISABLE_ACCESSSPEC_RESPONSE_encode (
  const LLRP_tSDISABLE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_DISABLE_ACCESSSPEC_RESPONSE_getLLRPStatus (
  LLRP_tSDISABLE_ACCESSSPEC_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_DISABLE_ACCESSSPEC_RESPONSE_setLLRPStatus (
  LLRP_tSDISABLE_ACCESSSPEC_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SGET_ACCESSSPECS
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_ACCESSSPECS;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_ACCESSSPECS[];

extern LLRP_tSGET_ACCESSSPECS *
LLRP_GET_ACCESSSPECS_construct (void);

extern void
LLRP_GET_ACCESSSPECS_destruct (
 LLRP_tSGET_ACCESSSPECS * pThis);

extern void
LLRP_GET_ACCESSSPECS_decodeFields (
 LLRP_tSGET_ACCESSSPECS * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_ACCESSSPECS_assimilateSubParameters (
 LLRP_tSGET_ACCESSSPECS * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_ACCESSSPECS_encode (
  const LLRP_tSGET_ACCESSSPECS *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SGET_ACCESSSPECS_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

    LLRP_tSAccessSpec * listAccessSpec;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_ACCESSSPECS_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_ACCESSSPECS_RESPONSE[];

extern LLRP_tSGET_ACCESSSPECS_RESPONSE *
LLRP_GET_ACCESSSPECS_RESPONSE_construct (void);

extern void
LLRP_GET_ACCESSSPECS_RESPONSE_destruct (
 LLRP_tSGET_ACCESSSPECS_RESPONSE * pThis);

extern void
LLRP_GET_ACCESSSPECS_RESPONSE_decodeFields (
 LLRP_tSGET_ACCESSSPECS_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_ACCESSSPECS_RESPONSE_assimilateSubParameters (
 LLRP_tSGET_ACCESSSPECS_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_ACCESSSPECS_RESPONSE_encode (
  const LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_GET_ACCESSSPECS_RESPONSE_getLLRPStatus (
  LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_ACCESSSPECS_RESPONSE_setLLRPStatus (
  LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);

extern LLRP_tSAccessSpec *
LLRP_GET_ACCESSSPECS_RESPONSE_beginAccessSpec (
  LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis);

extern LLRP_tSAccessSpec *
LLRP_GET_ACCESSSPECS_RESPONSE_nextAccessSpec (
  LLRP_tSAccessSpec *pCurrent);

extern void
LLRP_GET_ACCESSSPECS_RESPONSE_clearAccessSpec (
  LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_ACCESSSPECS_RESPONSE_addAccessSpec (
  LLRP_tSGET_ACCESSSPECS_RESPONSE *pThis,
  LLRP_tSAccessSpec *pValue);




  
struct LLRP_SGET_READER_CONFIG
{
    LLRP_tSMessage hdr;
  
    llrp_u16_t  AntennaID;

    LLRP_tEGetReaderConfigRequestedData  eRequestedData;

    llrp_u16_t  GPIPortNum;

    llrp_u16_t  GPOPortNum;

  
    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_READER_CONFIG;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_READER_CONFIG[];

extern LLRP_tSGET_READER_CONFIG *
LLRP_GET_READER_CONFIG_construct (void);

extern void
LLRP_GET_READER_CONFIG_destruct (
 LLRP_tSGET_READER_CONFIG * pThis);

extern void
LLRP_GET_READER_CONFIG_decodeFields (
 LLRP_tSGET_READER_CONFIG * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_READER_CONFIG_assimilateSubParameters (
 LLRP_tSGET_READER_CONFIG * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_READER_CONFIG_encode (
  const LLRP_tSGET_READER_CONFIG *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGET_READER_CONFIG_AntennaID;

extern llrp_u16_t
LLRP_GET_READER_CONFIG_getAntennaID (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_setAntennaID (
  LLRP_tSGET_READER_CONFIG *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGET_READER_CONFIG_RequestedData;

extern LLRP_tEGetReaderConfigRequestedData
LLRP_GET_READER_CONFIG_getRequestedData (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_setRequestedData (
  LLRP_tSGET_READER_CONFIG *pThis,
  LLRP_tEGetReaderConfigRequestedData Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGET_READER_CONFIG_GPIPortNum;

extern llrp_u16_t
LLRP_GET_READER_CONFIG_getGPIPortNum (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_setGPIPortNum (
  LLRP_tSGET_READER_CONFIG *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGET_READER_CONFIG_GPOPortNum;

extern llrp_u16_t
LLRP_GET_READER_CONFIG_getGPOPortNum (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_setGPOPortNum (
  LLRP_tSGET_READER_CONFIG *pThis,
  llrp_u16_t Value);

  
extern LLRP_tSParameter *
LLRP_GET_READER_CONFIG_beginCustom (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tSParameter *
LLRP_GET_READER_CONFIG_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_GET_READER_CONFIG_clearCustom (
  LLRP_tSGET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_addCustom (
  LLRP_tSGET_READER_CONFIG *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SGET_READER_CONFIG_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

    LLRP_tSIdentification * pIdentification;

    LLRP_tSAntennaProperties * listAntennaProperties;

    LLRP_tSAntennaConfiguration * listAntennaConfiguration;

    LLRP_tSReaderEventNotificationSpec * pReaderEventNotificationSpec;

    LLRP_tSROReportSpec * pROReportSpec;

    LLRP_tSAccessReportSpec * pAccessReportSpec;

    LLRP_tSLLRPConfigurationStateValue * pLLRPConfigurationStateValue;

    LLRP_tSKeepaliveSpec * pKeepaliveSpec;

    LLRP_tSGPIPortCurrentState * listGPIPortCurrentState;

    LLRP_tSGPOWriteData * listGPOWriteData;

    LLRP_tSEventsAndReports * pEventsAndReports;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_READER_CONFIG_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_READER_CONFIG_RESPONSE[];

extern LLRP_tSGET_READER_CONFIG_RESPONSE *
LLRP_GET_READER_CONFIG_RESPONSE_construct (void);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_destruct (
 LLRP_tSGET_READER_CONFIG_RESPONSE * pThis);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_decodeFields (
 LLRP_tSGET_READER_CONFIG_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_assimilateSubParameters (
 LLRP_tSGET_READER_CONFIG_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_encode (
  const LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_GET_READER_CONFIG_RESPONSE_getLLRPStatus (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setLLRPStatus (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);

extern LLRP_tSIdentification *
LLRP_GET_READER_CONFIG_RESPONSE_getIdentification (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setIdentification (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSIdentification *pValue);

extern LLRP_tSAntennaProperties *
LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaProperties (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tSAntennaProperties *
LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaProperties (
  LLRP_tSAntennaProperties *pCurrent);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_clearAntennaProperties (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_addAntennaProperties (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSAntennaProperties *pValue);


extern LLRP_tSAntennaConfiguration *
LLRP_GET_READER_CONFIG_RESPONSE_beginAntennaConfiguration (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tSAntennaConfiguration *
LLRP_GET_READER_CONFIG_RESPONSE_nextAntennaConfiguration (
  LLRP_tSAntennaConfiguration *pCurrent);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_clearAntennaConfiguration (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_addAntennaConfiguration (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSAntennaConfiguration *pValue);


extern LLRP_tSReaderEventNotificationSpec *
LLRP_GET_READER_CONFIG_RESPONSE_getReaderEventNotificationSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setReaderEventNotificationSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSReaderEventNotificationSpec *pValue);

extern LLRP_tSROReportSpec *
LLRP_GET_READER_CONFIG_RESPONSE_getROReportSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setROReportSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSROReportSpec *pValue);

extern LLRP_tSAccessReportSpec *
LLRP_GET_READER_CONFIG_RESPONSE_getAccessReportSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setAccessReportSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSAccessReportSpec *pValue);

extern LLRP_tSLLRPConfigurationStateValue *
LLRP_GET_READER_CONFIG_RESPONSE_getLLRPConfigurationStateValue (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setLLRPConfigurationStateValue (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSLLRPConfigurationStateValue *pValue);

extern LLRP_tSKeepaliveSpec *
LLRP_GET_READER_CONFIG_RESPONSE_getKeepaliveSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setKeepaliveSpec (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSKeepaliveSpec *pValue);

extern LLRP_tSGPIPortCurrentState *
LLRP_GET_READER_CONFIG_RESPONSE_beginGPIPortCurrentState (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tSGPIPortCurrentState *
LLRP_GET_READER_CONFIG_RESPONSE_nextGPIPortCurrentState (
  LLRP_tSGPIPortCurrentState *pCurrent);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_clearGPIPortCurrentState (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_addGPIPortCurrentState (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSGPIPortCurrentState *pValue);


extern LLRP_tSGPOWriteData *
LLRP_GET_READER_CONFIG_RESPONSE_beginGPOWriteData (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tSGPOWriteData *
LLRP_GET_READER_CONFIG_RESPONSE_nextGPOWriteData (
  LLRP_tSGPOWriteData *pCurrent);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_clearGPOWriteData (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_addGPOWriteData (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSGPOWriteData *pValue);


extern LLRP_tSEventsAndReports *
LLRP_GET_READER_CONFIG_RESPONSE_getEventsAndReports (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_setEventsAndReports (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSEventsAndReports *pValue);

extern LLRP_tSParameter *
LLRP_GET_READER_CONFIG_RESPONSE_beginCustom (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tSParameter *
LLRP_GET_READER_CONFIG_RESPONSE_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_GET_READER_CONFIG_RESPONSE_clearCustom (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_GET_READER_CONFIG_RESPONSE_addCustom (
  LLRP_tSGET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SSET_READER_CONFIG
{
    LLRP_tSMessage hdr;
  
    llrp_u1_t  ResetToFactoryDefault;

  
    LLRP_tSReaderEventNotificationSpec * pReaderEventNotificationSpec;

    LLRP_tSAntennaProperties * listAntennaProperties;

    LLRP_tSAntennaConfiguration * listAntennaConfiguration;

    LLRP_tSROReportSpec * pROReportSpec;

    LLRP_tSAccessReportSpec * pAccessReportSpec;

    LLRP_tSKeepaliveSpec * pKeepaliveSpec;

    LLRP_tSGPOWriteData * listGPOWriteData;

    LLRP_tSGPIPortCurrentState * listGPIPortCurrentState;

    LLRP_tSEventsAndReports * pEventsAndReports;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdSET_READER_CONFIG;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSET_READER_CONFIG[];

extern LLRP_tSSET_READER_CONFIG *
LLRP_SET_READER_CONFIG_construct (void);

extern void
LLRP_SET_READER_CONFIG_destruct (
 LLRP_tSSET_READER_CONFIG * pThis);

extern void
LLRP_SET_READER_CONFIG_decodeFields (
 LLRP_tSSET_READER_CONFIG * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_SET_READER_CONFIG_assimilateSubParameters (
 LLRP_tSSET_READER_CONFIG * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_SET_READER_CONFIG_encode (
  const LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdSET_READER_CONFIG_ResetToFactoryDefault;

extern llrp_u1_t
LLRP_SET_READER_CONFIG_getResetToFactoryDefault (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setResetToFactoryDefault (
  LLRP_tSSET_READER_CONFIG *pThis,
  llrp_u1_t Value);

  
extern LLRP_tSReaderEventNotificationSpec *
LLRP_SET_READER_CONFIG_getReaderEventNotificationSpec (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setReaderEventNotificationSpec (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSReaderEventNotificationSpec *pValue);

extern LLRP_tSAntennaProperties *
LLRP_SET_READER_CONFIG_beginAntennaProperties (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tSAntennaProperties *
LLRP_SET_READER_CONFIG_nextAntennaProperties (
  LLRP_tSAntennaProperties *pCurrent);

extern void
LLRP_SET_READER_CONFIG_clearAntennaProperties (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_addAntennaProperties (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSAntennaProperties *pValue);


extern LLRP_tSAntennaConfiguration *
LLRP_SET_READER_CONFIG_beginAntennaConfiguration (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tSAntennaConfiguration *
LLRP_SET_READER_CONFIG_nextAntennaConfiguration (
  LLRP_tSAntennaConfiguration *pCurrent);

extern void
LLRP_SET_READER_CONFIG_clearAntennaConfiguration (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_addAntennaConfiguration (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSAntennaConfiguration *pValue);


extern LLRP_tSROReportSpec *
LLRP_SET_READER_CONFIG_getROReportSpec (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setROReportSpec (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSROReportSpec *pValue);

extern LLRP_tSAccessReportSpec *
LLRP_SET_READER_CONFIG_getAccessReportSpec (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setAccessReportSpec (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSAccessReportSpec *pValue);

extern LLRP_tSKeepaliveSpec *
LLRP_SET_READER_CONFIG_getKeepaliveSpec (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setKeepaliveSpec (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSKeepaliveSpec *pValue);

extern LLRP_tSGPOWriteData *
LLRP_SET_READER_CONFIG_beginGPOWriteData (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tSGPOWriteData *
LLRP_SET_READER_CONFIG_nextGPOWriteData (
  LLRP_tSGPOWriteData *pCurrent);

extern void
LLRP_SET_READER_CONFIG_clearGPOWriteData (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_addGPOWriteData (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSGPOWriteData *pValue);


extern LLRP_tSGPIPortCurrentState *
LLRP_SET_READER_CONFIG_beginGPIPortCurrentState (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tSGPIPortCurrentState *
LLRP_SET_READER_CONFIG_nextGPIPortCurrentState (
  LLRP_tSGPIPortCurrentState *pCurrent);

extern void
LLRP_SET_READER_CONFIG_clearGPIPortCurrentState (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_addGPIPortCurrentState (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSGPIPortCurrentState *pValue);


extern LLRP_tSEventsAndReports *
LLRP_SET_READER_CONFIG_getEventsAndReports (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_setEventsAndReports (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSEventsAndReports *pValue);

extern LLRP_tSParameter *
LLRP_SET_READER_CONFIG_beginCustom (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tSParameter *
LLRP_SET_READER_CONFIG_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_SET_READER_CONFIG_clearCustom (
  LLRP_tSSET_READER_CONFIG *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_addCustom (
  LLRP_tSSET_READER_CONFIG *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SSET_READER_CONFIG_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdSET_READER_CONFIG_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSET_READER_CONFIG_RESPONSE[];

extern LLRP_tSSET_READER_CONFIG_RESPONSE *
LLRP_SET_READER_CONFIG_RESPONSE_construct (void);

extern void
LLRP_SET_READER_CONFIG_RESPONSE_destruct (
 LLRP_tSSET_READER_CONFIG_RESPONSE * pThis);

extern void
LLRP_SET_READER_CONFIG_RESPONSE_decodeFields (
 LLRP_tSSET_READER_CONFIG_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_SET_READER_CONFIG_RESPONSE_assimilateSubParameters (
 LLRP_tSSET_READER_CONFIG_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_SET_READER_CONFIG_RESPONSE_encode (
  const LLRP_tSSET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_SET_READER_CONFIG_RESPONSE_getLLRPStatus (
  LLRP_tSSET_READER_CONFIG_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_SET_READER_CONFIG_RESPONSE_setLLRPStatus (
  LLRP_tSSET_READER_CONFIG_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SCLOSE_CONNECTION
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdCLOSE_CONNECTION;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdCLOSE_CONNECTION[];

extern LLRP_tSCLOSE_CONNECTION *
LLRP_CLOSE_CONNECTION_construct (void);

extern void
LLRP_CLOSE_CONNECTION_destruct (
 LLRP_tSCLOSE_CONNECTION * pThis);

extern void
LLRP_CLOSE_CONNECTION_decodeFields (
 LLRP_tSCLOSE_CONNECTION * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_CLOSE_CONNECTION_assimilateSubParameters (
 LLRP_tSCLOSE_CONNECTION * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_CLOSE_CONNECTION_encode (
  const LLRP_tSCLOSE_CONNECTION *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SCLOSE_CONNECTION_RESPONSE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdCLOSE_CONNECTION_RESPONSE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdCLOSE_CONNECTION_RESPONSE[];

extern LLRP_tSCLOSE_CONNECTION_RESPONSE *
LLRP_CLOSE_CONNECTION_RESPONSE_construct (void);

extern void
LLRP_CLOSE_CONNECTION_RESPONSE_destruct (
 LLRP_tSCLOSE_CONNECTION_RESPONSE * pThis);

extern void
LLRP_CLOSE_CONNECTION_RESPONSE_decodeFields (
 LLRP_tSCLOSE_CONNECTION_RESPONSE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_CLOSE_CONNECTION_RESPONSE_assimilateSubParameters (
 LLRP_tSCLOSE_CONNECTION_RESPONSE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_CLOSE_CONNECTION_RESPONSE_encode (
  const LLRP_tSCLOSE_CONNECTION_RESPONSE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_CLOSE_CONNECTION_RESPONSE_getLLRPStatus (
  LLRP_tSCLOSE_CONNECTION_RESPONSE *pThis);

extern LLRP_tResultCode
LLRP_CLOSE_CONNECTION_RESPONSE_setLLRPStatus (
  LLRP_tSCLOSE_CONNECTION_RESPONSE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SGET_REPORT
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGET_REPORT;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGET_REPORT[];

extern LLRP_tSGET_REPORT *
LLRP_GET_REPORT_construct (void);

extern void
LLRP_GET_REPORT_destruct (
 LLRP_tSGET_REPORT * pThis);

extern void
LLRP_GET_REPORT_decodeFields (
 LLRP_tSGET_REPORT * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GET_REPORT_assimilateSubParameters (
 LLRP_tSGET_REPORT * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GET_REPORT_encode (
  const LLRP_tSGET_REPORT *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SRO_ACCESS_REPORT
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSTagReportData * listTagReportData;

    LLRP_tSRFSurveyReportData * listRFSurveyReportData;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdRO_ACCESS_REPORT;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRO_ACCESS_REPORT[];

extern LLRP_tSRO_ACCESS_REPORT *
LLRP_RO_ACCESS_REPORT_construct (void);

extern void
LLRP_RO_ACCESS_REPORT_destruct (
 LLRP_tSRO_ACCESS_REPORT * pThis);

extern void
LLRP_RO_ACCESS_REPORT_decodeFields (
 LLRP_tSRO_ACCESS_REPORT * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RO_ACCESS_REPORT_assimilateSubParameters (
 LLRP_tSRO_ACCESS_REPORT * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RO_ACCESS_REPORT_encode (
  const LLRP_tSRO_ACCESS_REPORT *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSTagReportData *
LLRP_RO_ACCESS_REPORT_beginTagReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tSTagReportData *
LLRP_RO_ACCESS_REPORT_nextTagReportData (
  LLRP_tSTagReportData *pCurrent);

extern void
LLRP_RO_ACCESS_REPORT_clearTagReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tResultCode
LLRP_RO_ACCESS_REPORT_addTagReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis,
  LLRP_tSTagReportData *pValue);


extern LLRP_tSRFSurveyReportData *
LLRP_RO_ACCESS_REPORT_beginRFSurveyReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tSRFSurveyReportData *
LLRP_RO_ACCESS_REPORT_nextRFSurveyReportData (
  LLRP_tSRFSurveyReportData *pCurrent);

extern void
LLRP_RO_ACCESS_REPORT_clearRFSurveyReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tResultCode
LLRP_RO_ACCESS_REPORT_addRFSurveyReportData (
  LLRP_tSRO_ACCESS_REPORT *pThis,
  LLRP_tSRFSurveyReportData *pValue);


extern LLRP_tSParameter *
LLRP_RO_ACCESS_REPORT_beginCustom (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tSParameter *
LLRP_RO_ACCESS_REPORT_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_RO_ACCESS_REPORT_clearCustom (
  LLRP_tSRO_ACCESS_REPORT *pThis);

extern LLRP_tResultCode
LLRP_RO_ACCESS_REPORT_addCustom (
  LLRP_tSRO_ACCESS_REPORT *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SKEEPALIVE
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdKEEPALIVE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdKEEPALIVE[];

extern LLRP_tSKEEPALIVE *
LLRP_KEEPALIVE_construct (void);

extern void
LLRP_KEEPALIVE_destruct (
 LLRP_tSKEEPALIVE * pThis);

extern void
LLRP_KEEPALIVE_decodeFields (
 LLRP_tSKEEPALIVE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_KEEPALIVE_assimilateSubParameters (
 LLRP_tSKEEPALIVE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_KEEPALIVE_encode (
  const LLRP_tSKEEPALIVE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SKEEPALIVE_ACK
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdKEEPALIVE_ACK;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdKEEPALIVE_ACK[];

extern LLRP_tSKEEPALIVE_ACK *
LLRP_KEEPALIVE_ACK_construct (void);

extern void
LLRP_KEEPALIVE_ACK_destruct (
 LLRP_tSKEEPALIVE_ACK * pThis);

extern void
LLRP_KEEPALIVE_ACK_decodeFields (
 LLRP_tSKEEPALIVE_ACK * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_KEEPALIVE_ACK_assimilateSubParameters (
 LLRP_tSKEEPALIVE_ACK * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_KEEPALIVE_ACK_encode (
  const LLRP_tSKEEPALIVE_ACK *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SREADER_EVENT_NOTIFICATION
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSReaderEventNotificationData * pReaderEventNotificationData;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdREADER_EVENT_NOTIFICATION;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdREADER_EVENT_NOTIFICATION[];

extern LLRP_tSREADER_EVENT_NOTIFICATION *
LLRP_READER_EVENT_NOTIFICATION_construct (void);

extern void
LLRP_READER_EVENT_NOTIFICATION_destruct (
 LLRP_tSREADER_EVENT_NOTIFICATION * pThis);

extern void
LLRP_READER_EVENT_NOTIFICATION_decodeFields (
 LLRP_tSREADER_EVENT_NOTIFICATION * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_READER_EVENT_NOTIFICATION_assimilateSubParameters (
 LLRP_tSREADER_EVENT_NOTIFICATION * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_READER_EVENT_NOTIFICATION_encode (
  const LLRP_tSREADER_EVENT_NOTIFICATION *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSReaderEventNotificationData *
LLRP_READER_EVENT_NOTIFICATION_getReaderEventNotificationData (
  LLRP_tSREADER_EVENT_NOTIFICATION *pThis);

extern LLRP_tResultCode
LLRP_READER_EVENT_NOTIFICATION_setReaderEventNotificationData (
  LLRP_tSREADER_EVENT_NOTIFICATION *pThis,
  LLRP_tSReaderEventNotificationData *pValue);



  
struct LLRP_SENABLE_EVENTS_AND_REPORTS
{
    LLRP_tSMessage hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdENABLE_EVENTS_AND_REPORTS;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdENABLE_EVENTS_AND_REPORTS[];

extern LLRP_tSENABLE_EVENTS_AND_REPORTS *
LLRP_ENABLE_EVENTS_AND_REPORTS_construct (void);

extern void
LLRP_ENABLE_EVENTS_AND_REPORTS_destruct (
 LLRP_tSENABLE_EVENTS_AND_REPORTS * pThis);

extern void
LLRP_ENABLE_EVENTS_AND_REPORTS_decodeFields (
 LLRP_tSENABLE_EVENTS_AND_REPORTS * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ENABLE_EVENTS_AND_REPORTS_assimilateSubParameters (
 LLRP_tSENABLE_EVENTS_AND_REPORTS * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ENABLE_EVENTS_AND_REPORTS_encode (
  const LLRP_tSENABLE_EVENTS_AND_REPORTS *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SERROR_MESSAGE
{
    LLRP_tSMessage hdr;
  
  
    LLRP_tSLLRPStatus * pLLRPStatus;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdERROR_MESSAGE;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdERROR_MESSAGE[];

extern LLRP_tSERROR_MESSAGE *
LLRP_ERROR_MESSAGE_construct (void);

extern void
LLRP_ERROR_MESSAGE_destruct (
 LLRP_tSERROR_MESSAGE * pThis);

extern void
LLRP_ERROR_MESSAGE_decodeFields (
 LLRP_tSERROR_MESSAGE * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ERROR_MESSAGE_assimilateSubParameters (
 LLRP_tSERROR_MESSAGE * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ERROR_MESSAGE_encode (
  const LLRP_tSERROR_MESSAGE *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSLLRPStatus *
LLRP_ERROR_MESSAGE_getLLRPStatus (
  LLRP_tSERROR_MESSAGE *pThis);

extern LLRP_tResultCode
LLRP_ERROR_MESSAGE_setLLRPStatus (
  LLRP_tSERROR_MESSAGE *pThis,
  LLRP_tSLLRPStatus *pValue);



  
struct LLRP_SUTCTimestamp
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdUTCTimestamp;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdUTCTimestamp[];

extern LLRP_tSUTCTimestamp *
LLRP_UTCTimestamp_construct (void);

extern void
LLRP_UTCTimestamp_destruct (
 LLRP_tSUTCTimestamp * pThis);

extern void
LLRP_UTCTimestamp_decodeFields (
 LLRP_tSUTCTimestamp * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_UTCTimestamp_assimilateSubParameters (
 LLRP_tSUTCTimestamp * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_UTCTimestamp_encode (
  const LLRP_tSUTCTimestamp *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdUTCTimestamp_Microseconds;

extern llrp_u64_t
LLRP_UTCTimestamp_getMicroseconds (
  LLRP_tSUTCTimestamp *pThis);

extern LLRP_tResultCode
LLRP_UTCTimestamp_setMicroseconds (
  LLRP_tSUTCTimestamp *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_SUptime
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdUptime;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdUptime[];

extern LLRP_tSUptime *
LLRP_Uptime_construct (void);

extern void
LLRP_Uptime_destruct (
 LLRP_tSUptime * pThis);

extern void
LLRP_Uptime_decodeFields (
 LLRP_tSUptime * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_Uptime_assimilateSubParameters (
 LLRP_tSUptime * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_Uptime_encode (
  const LLRP_tSUptime *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdUptime_Microseconds;

extern llrp_u64_t
LLRP_Uptime_getMicroseconds (
  LLRP_tSUptime *pThis);

extern LLRP_tResultCode
LLRP_Uptime_setMicroseconds (
  LLRP_tSUptime *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_SCustom
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  VendorIdentifier;

    llrp_u32_t  ParameterSubtype;

    llrp_bytesToEnd_t  Data;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdCustom;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdCustom[];

extern LLRP_tSCustom *
LLRP_Custom_construct (void);

extern void
LLRP_Custom_destruct (
 LLRP_tSCustom * pThis);

extern void
LLRP_Custom_decodeFields (
 LLRP_tSCustom * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_Custom_assimilateSubParameters (
 LLRP_tSCustom * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_Custom_encode (
  const LLRP_tSCustom *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdCustom_VendorIdentifier;

extern llrp_u32_t
LLRP_Custom_getVendorIdentifier (
  LLRP_tSCustom *pThis);

extern LLRP_tResultCode
LLRP_Custom_setVendorIdentifier (
  LLRP_tSCustom *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdCustom_ParameterSubtype;

extern llrp_u32_t
LLRP_Custom_getParameterSubtype (
  LLRP_tSCustom *pThis);

extern LLRP_tResultCode
LLRP_Custom_setParameterSubtype (
  LLRP_tSCustom *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdCustom_Data;

extern llrp_bytesToEnd_t
LLRP_Custom_getData (
  LLRP_tSCustom *pThis);

extern LLRP_tResultCode
LLRP_Custom_setData (
  LLRP_tSCustom *pThis,
  llrp_bytesToEnd_t Value);

  


  
struct LLRP_SGeneralDeviceCapabilities
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  MaxNumberOfAntennaSupported;

    llrp_u1_t  CanSetAntennaProperties;

    llrp_u1_t  HasUTCClockCapability;

    llrp_u32_t  DeviceManufacturerName;

    llrp_u32_t  ModelName;

    llrp_utf8v_t  ReaderFirmwareVersion;

  
    LLRP_tSReceiveSensitivityTableEntry * listReceiveSensitivityTableEntry;

    LLRP_tSPerAntennaReceiveSensitivityRange * listPerAntennaReceiveSensitivityRange;

    LLRP_tSGPIOCapabilities * pGPIOCapabilities;

    LLRP_tSPerAntennaAirProtocol * listPerAntennaAirProtocol;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdGeneralDeviceCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGeneralDeviceCapabilities[];

extern LLRP_tSGeneralDeviceCapabilities *
LLRP_GeneralDeviceCapabilities_construct (void);

extern void
LLRP_GeneralDeviceCapabilities_destruct (
 LLRP_tSGeneralDeviceCapabilities * pThis);

extern void
LLRP_GeneralDeviceCapabilities_decodeFields (
 LLRP_tSGeneralDeviceCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GeneralDeviceCapabilities_assimilateSubParameters (
 LLRP_tSGeneralDeviceCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GeneralDeviceCapabilities_encode (
  const LLRP_tSGeneralDeviceCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_MaxNumberOfAntennaSupported;

extern llrp_u16_t
LLRP_GeneralDeviceCapabilities_getMaxNumberOfAntennaSupported (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setMaxNumberOfAntennaSupported (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_CanSetAntennaProperties;

extern llrp_u1_t
LLRP_GeneralDeviceCapabilities_getCanSetAntennaProperties (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setCanSetAntennaProperties (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_HasUTCClockCapability;

extern llrp_u1_t
LLRP_GeneralDeviceCapabilities_getHasUTCClockCapability (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setHasUTCClockCapability (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_DeviceManufacturerName;

extern llrp_u32_t
LLRP_GeneralDeviceCapabilities_getDeviceManufacturerName (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setDeviceManufacturerName (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_ModelName;

extern llrp_u32_t
LLRP_GeneralDeviceCapabilities_getModelName (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setModelName (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGeneralDeviceCapabilities_ReaderFirmwareVersion;

extern llrp_utf8v_t
LLRP_GeneralDeviceCapabilities_getReaderFirmwareVersion (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setReaderFirmwareVersion (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  llrp_utf8v_t Value);

  
extern LLRP_tSReceiveSensitivityTableEntry *
LLRP_GeneralDeviceCapabilities_beginReceiveSensitivityTableEntry (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tSReceiveSensitivityTableEntry *
LLRP_GeneralDeviceCapabilities_nextReceiveSensitivityTableEntry (
  LLRP_tSReceiveSensitivityTableEntry *pCurrent);

extern void
LLRP_GeneralDeviceCapabilities_clearReceiveSensitivityTableEntry (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_addReceiveSensitivityTableEntry (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  LLRP_tSReceiveSensitivityTableEntry *pValue);


extern LLRP_tSPerAntennaReceiveSensitivityRange *
LLRP_GeneralDeviceCapabilities_beginPerAntennaReceiveSensitivityRange (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tSPerAntennaReceiveSensitivityRange *
LLRP_GeneralDeviceCapabilities_nextPerAntennaReceiveSensitivityRange (
  LLRP_tSPerAntennaReceiveSensitivityRange *pCurrent);

extern void
LLRP_GeneralDeviceCapabilities_clearPerAntennaReceiveSensitivityRange (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_addPerAntennaReceiveSensitivityRange (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  LLRP_tSPerAntennaReceiveSensitivityRange *pValue);


extern LLRP_tSGPIOCapabilities *
LLRP_GeneralDeviceCapabilities_getGPIOCapabilities (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_setGPIOCapabilities (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  LLRP_tSGPIOCapabilities *pValue);

extern LLRP_tSPerAntennaAirProtocol *
LLRP_GeneralDeviceCapabilities_beginPerAntennaAirProtocol (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tSPerAntennaAirProtocol *
LLRP_GeneralDeviceCapabilities_nextPerAntennaAirProtocol (
  LLRP_tSPerAntennaAirProtocol *pCurrent);

extern void
LLRP_GeneralDeviceCapabilities_clearPerAntennaAirProtocol (
  LLRP_tSGeneralDeviceCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GeneralDeviceCapabilities_addPerAntennaAirProtocol (
  LLRP_tSGeneralDeviceCapabilities *pThis,
  LLRP_tSPerAntennaAirProtocol *pValue);




  
struct LLRP_SReceiveSensitivityTableEntry
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  Index;

    llrp_s16_t  ReceiveSensitivityValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdReceiveSensitivityTableEntry;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReceiveSensitivityTableEntry[];

extern LLRP_tSReceiveSensitivityTableEntry *
LLRP_ReceiveSensitivityTableEntry_construct (void);

extern void
LLRP_ReceiveSensitivityTableEntry_destruct (
 LLRP_tSReceiveSensitivityTableEntry * pThis);

extern void
LLRP_ReceiveSensitivityTableEntry_decodeFields (
 LLRP_tSReceiveSensitivityTableEntry * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReceiveSensitivityTableEntry_assimilateSubParameters (
 LLRP_tSReceiveSensitivityTableEntry * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReceiveSensitivityTableEntry_encode (
  const LLRP_tSReceiveSensitivityTableEntry *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdReceiveSensitivityTableEntry_Index;

extern llrp_u16_t
LLRP_ReceiveSensitivityTableEntry_getIndex (
  LLRP_tSReceiveSensitivityTableEntry *pThis);

extern LLRP_tResultCode
LLRP_ReceiveSensitivityTableEntry_setIndex (
  LLRP_tSReceiveSensitivityTableEntry *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdReceiveSensitivityTableEntry_ReceiveSensitivityValue;

extern llrp_s16_t
LLRP_ReceiveSensitivityTableEntry_getReceiveSensitivityValue (
  LLRP_tSReceiveSensitivityTableEntry *pThis);

extern LLRP_tResultCode
LLRP_ReceiveSensitivityTableEntry_setReceiveSensitivityValue (
  LLRP_tSReceiveSensitivityTableEntry *pThis,
  llrp_s16_t Value);

  


  
struct LLRP_SPerAntennaReceiveSensitivityRange
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  AntennaID;

    llrp_u16_t  ReceiveSensitivityIndexMin;

    llrp_u16_t  ReceiveSensitivityIndexMax;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdPerAntennaReceiveSensitivityRange;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdPerAntennaReceiveSensitivityRange[];

extern LLRP_tSPerAntennaReceiveSensitivityRange *
LLRP_PerAntennaReceiveSensitivityRange_construct (void);

extern void
LLRP_PerAntennaReceiveSensitivityRange_destruct (
 LLRP_tSPerAntennaReceiveSensitivityRange * pThis);

extern void
LLRP_PerAntennaReceiveSensitivityRange_decodeFields (
 LLRP_tSPerAntennaReceiveSensitivityRange * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_PerAntennaReceiveSensitivityRange_assimilateSubParameters (
 LLRP_tSPerAntennaReceiveSensitivityRange * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_PerAntennaReceiveSensitivityRange_encode (
  const LLRP_tSPerAntennaReceiveSensitivityRange *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdPerAntennaReceiveSensitivityRange_AntennaID;

extern llrp_u16_t
LLRP_PerAntennaReceiveSensitivityRange_getAntennaID (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis);

extern LLRP_tResultCode
LLRP_PerAntennaReceiveSensitivityRange_setAntennaID (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdPerAntennaReceiveSensitivityRange_ReceiveSensitivityIndexMin;

extern llrp_u16_t
LLRP_PerAntennaReceiveSensitivityRange_getReceiveSensitivityIndexMin (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis);

extern LLRP_tResultCode
LLRP_PerAntennaReceiveSensitivityRange_setReceiveSensitivityIndexMin (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdPerAntennaReceiveSensitivityRange_ReceiveSensitivityIndexMax;

extern llrp_u16_t
LLRP_PerAntennaReceiveSensitivityRange_getReceiveSensitivityIndexMax (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis);

extern LLRP_tResultCode
LLRP_PerAntennaReceiveSensitivityRange_setReceiveSensitivityIndexMax (
  LLRP_tSPerAntennaReceiveSensitivityRange *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SPerAntennaAirProtocol
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  AntennaID;

    llrp_u8v_t  ProtocolID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdPerAntennaAirProtocol;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdPerAntennaAirProtocol[];

extern LLRP_tSPerAntennaAirProtocol *
LLRP_PerAntennaAirProtocol_construct (void);

extern void
LLRP_PerAntennaAirProtocol_destruct (
 LLRP_tSPerAntennaAirProtocol * pThis);

extern void
LLRP_PerAntennaAirProtocol_decodeFields (
 LLRP_tSPerAntennaAirProtocol * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_PerAntennaAirProtocol_assimilateSubParameters (
 LLRP_tSPerAntennaAirProtocol * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_PerAntennaAirProtocol_encode (
  const LLRP_tSPerAntennaAirProtocol *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdPerAntennaAirProtocol_AntennaID;

extern llrp_u16_t
LLRP_PerAntennaAirProtocol_getAntennaID (
  LLRP_tSPerAntennaAirProtocol *pThis);

extern LLRP_tResultCode
LLRP_PerAntennaAirProtocol_setAntennaID (
  LLRP_tSPerAntennaAirProtocol *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdPerAntennaAirProtocol_ProtocolID;

extern llrp_u8v_t
LLRP_PerAntennaAirProtocol_getProtocolID (
  LLRP_tSPerAntennaAirProtocol *pThis);

extern LLRP_tResultCode
LLRP_PerAntennaAirProtocol_setProtocolID (
  LLRP_tSPerAntennaAirProtocol *pThis,
  llrp_u8v_t Value);

  


  
struct LLRP_SGPIOCapabilities
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  NumGPIs;

    llrp_u16_t  NumGPOs;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGPIOCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGPIOCapabilities[];

extern LLRP_tSGPIOCapabilities *
LLRP_GPIOCapabilities_construct (void);

extern void
LLRP_GPIOCapabilities_destruct (
 LLRP_tSGPIOCapabilities * pThis);

extern void
LLRP_GPIOCapabilities_decodeFields (
 LLRP_tSGPIOCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GPIOCapabilities_assimilateSubParameters (
 LLRP_tSGPIOCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GPIOCapabilities_encode (
  const LLRP_tSGPIOCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGPIOCapabilities_NumGPIs;

extern llrp_u16_t
LLRP_GPIOCapabilities_getNumGPIs (
  LLRP_tSGPIOCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GPIOCapabilities_setNumGPIs (
  LLRP_tSGPIOCapabilities *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPIOCapabilities_NumGPOs;

extern llrp_u16_t
LLRP_GPIOCapabilities_getNumGPOs (
  LLRP_tSGPIOCapabilities *pThis);

extern LLRP_tResultCode
LLRP_GPIOCapabilities_setNumGPOs (
  LLRP_tSGPIOCapabilities *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SLLRPCapabilities
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  CanDoRFSurvey;

    llrp_u1_t  CanReportBufferFillWarning;

    llrp_u1_t  SupportsClientRequestOpSpec;

    llrp_u1_t  CanDoTagInventoryStateAwareSingulation;

    llrp_u1_t  SupportsEventAndReportHolding;

    llrp_u8_t  MaxNumPriorityLevelsSupported;

    llrp_u16_t  ClientRequestOpSpecTimeout;

    llrp_u32_t  MaxNumROSpecs;

    llrp_u32_t  MaxNumSpecsPerROSpec;

    llrp_u32_t  MaxNumInventoryParameterSpecsPerAISpec;

    llrp_u32_t  MaxNumAccessSpecs;

    llrp_u32_t  MaxNumOpSpecsPerAccessSpec;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdLLRPCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLLRPCapabilities[];

extern LLRP_tSLLRPCapabilities *
LLRP_LLRPCapabilities_construct (void);

extern void
LLRP_LLRPCapabilities_destruct (
 LLRP_tSLLRPCapabilities * pThis);

extern void
LLRP_LLRPCapabilities_decodeFields (
 LLRP_tSLLRPCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LLRPCapabilities_assimilateSubParameters (
 LLRP_tSLLRPCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LLRPCapabilities_encode (
  const LLRP_tSLLRPCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_CanDoRFSurvey;

extern llrp_u1_t
LLRP_LLRPCapabilities_getCanDoRFSurvey (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setCanDoRFSurvey (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_CanReportBufferFillWarning;

extern llrp_u1_t
LLRP_LLRPCapabilities_getCanReportBufferFillWarning (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setCanReportBufferFillWarning (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_SupportsClientRequestOpSpec;

extern llrp_u1_t
LLRP_LLRPCapabilities_getSupportsClientRequestOpSpec (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setSupportsClientRequestOpSpec (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_CanDoTagInventoryStateAwareSingulation;

extern llrp_u1_t
LLRP_LLRPCapabilities_getCanDoTagInventoryStateAwareSingulation (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setCanDoTagInventoryStateAwareSingulation (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_SupportsEventAndReportHolding;

extern llrp_u1_t
LLRP_LLRPCapabilities_getSupportsEventAndReportHolding (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setSupportsEventAndReportHolding (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumPriorityLevelsSupported;

extern llrp_u8_t
LLRP_LLRPCapabilities_getMaxNumPriorityLevelsSupported (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumPriorityLevelsSupported (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u8_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_ClientRequestOpSpecTimeout;

extern llrp_u16_t
LLRP_LLRPCapabilities_getClientRequestOpSpecTimeout (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setClientRequestOpSpecTimeout (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumROSpecs;

extern llrp_u32_t
LLRP_LLRPCapabilities_getMaxNumROSpecs (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumROSpecs (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumSpecsPerROSpec;

extern llrp_u32_t
LLRP_LLRPCapabilities_getMaxNumSpecsPerROSpec (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumSpecsPerROSpec (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumInventoryParameterSpecsPerAISpec;

extern llrp_u32_t
LLRP_LLRPCapabilities_getMaxNumInventoryParameterSpecsPerAISpec (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumInventoryParameterSpecsPerAISpec (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumAccessSpecs;

extern llrp_u32_t
LLRP_LLRPCapabilities_getMaxNumAccessSpecs (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumAccessSpecs (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPCapabilities_MaxNumOpSpecsPerAccessSpec;

extern llrp_u32_t
LLRP_LLRPCapabilities_getMaxNumOpSpecsPerAccessSpec (
  LLRP_tSLLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_LLRPCapabilities_setMaxNumOpSpecsPerAccessSpec (
  LLRP_tSLLRPCapabilities *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SRegulatoryCapabilities
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  CountryCode;

    LLRP_tECommunicationsStandard  eCommunicationsStandard;

  
    LLRP_tSUHFBandCapabilities * pUHFBandCapabilities;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdRegulatoryCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRegulatoryCapabilities[];

extern LLRP_tSRegulatoryCapabilities *
LLRP_RegulatoryCapabilities_construct (void);

extern void
LLRP_RegulatoryCapabilities_destruct (
 LLRP_tSRegulatoryCapabilities * pThis);

extern void
LLRP_RegulatoryCapabilities_decodeFields (
 LLRP_tSRegulatoryCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RegulatoryCapabilities_assimilateSubParameters (
 LLRP_tSRegulatoryCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RegulatoryCapabilities_encode (
  const LLRP_tSRegulatoryCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRegulatoryCapabilities_CountryCode;

extern llrp_u16_t
LLRP_RegulatoryCapabilities_getCountryCode (
  LLRP_tSRegulatoryCapabilities *pThis);

extern LLRP_tResultCode
LLRP_RegulatoryCapabilities_setCountryCode (
  LLRP_tSRegulatoryCapabilities *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRegulatoryCapabilities_CommunicationsStandard;

extern LLRP_tECommunicationsStandard
LLRP_RegulatoryCapabilities_getCommunicationsStandard (
  LLRP_tSRegulatoryCapabilities *pThis);

extern LLRP_tResultCode
LLRP_RegulatoryCapabilities_setCommunicationsStandard (
  LLRP_tSRegulatoryCapabilities *pThis,
  LLRP_tECommunicationsStandard Value);

  
extern LLRP_tSUHFBandCapabilities *
LLRP_RegulatoryCapabilities_getUHFBandCapabilities (
  LLRP_tSRegulatoryCapabilities *pThis);

extern LLRP_tResultCode
LLRP_RegulatoryCapabilities_setUHFBandCapabilities (
  LLRP_tSRegulatoryCapabilities *pThis,
  LLRP_tSUHFBandCapabilities *pValue);

extern LLRP_tSParameter *
LLRP_RegulatoryCapabilities_beginCustom (
  LLRP_tSRegulatoryCapabilities *pThis);

extern LLRP_tSParameter *
LLRP_RegulatoryCapabilities_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_RegulatoryCapabilities_clearCustom (
  LLRP_tSRegulatoryCapabilities *pThis);

extern LLRP_tResultCode
LLRP_RegulatoryCapabilities_addCustom (
  LLRP_tSRegulatoryCapabilities *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SUHFBandCapabilities
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSTransmitPowerLevelTableEntry * listTransmitPowerLevelTableEntry;

    LLRP_tSFrequencyInformation * pFrequencyInformation;

    LLRP_tSParameter * listAirProtocolUHFRFModeTable;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdUHFBandCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdUHFBandCapabilities[];

extern LLRP_tSUHFBandCapabilities *
LLRP_UHFBandCapabilities_construct (void);

extern void
LLRP_UHFBandCapabilities_destruct (
 LLRP_tSUHFBandCapabilities * pThis);

extern void
LLRP_UHFBandCapabilities_decodeFields (
 LLRP_tSUHFBandCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_UHFBandCapabilities_assimilateSubParameters (
 LLRP_tSUHFBandCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_UHFBandCapabilities_encode (
  const LLRP_tSUHFBandCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSTransmitPowerLevelTableEntry *
LLRP_UHFBandCapabilities_beginTransmitPowerLevelTableEntry (
  LLRP_tSUHFBandCapabilities *pThis);

extern LLRP_tSTransmitPowerLevelTableEntry *
LLRP_UHFBandCapabilities_nextTransmitPowerLevelTableEntry (
  LLRP_tSTransmitPowerLevelTableEntry *pCurrent);

extern void
LLRP_UHFBandCapabilities_clearTransmitPowerLevelTableEntry (
  LLRP_tSUHFBandCapabilities *pThis);

extern LLRP_tResultCode
LLRP_UHFBandCapabilities_addTransmitPowerLevelTableEntry (
  LLRP_tSUHFBandCapabilities *pThis,
  LLRP_tSTransmitPowerLevelTableEntry *pValue);


extern LLRP_tSFrequencyInformation *
LLRP_UHFBandCapabilities_getFrequencyInformation (
  LLRP_tSUHFBandCapabilities *pThis);

extern LLRP_tResultCode
LLRP_UHFBandCapabilities_setFrequencyInformation (
  LLRP_tSUHFBandCapabilities *pThis,
  LLRP_tSFrequencyInformation *pValue);

extern LLRP_tSParameter *
LLRP_UHFBandCapabilities_beginAirProtocolUHFRFModeTable (
  LLRP_tSUHFBandCapabilities *pThis);

extern LLRP_tSParameter *
LLRP_UHFBandCapabilities_nextAirProtocolUHFRFModeTable (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_UHFBandCapabilities_clearAirProtocolUHFRFModeTable (
  LLRP_tSUHFBandCapabilities *pThis);

extern LLRP_tResultCode
LLRP_UHFBandCapabilities_addAirProtocolUHFRFModeTable (
  LLRP_tSUHFBandCapabilities *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_STransmitPowerLevelTableEntry
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  Index;

    llrp_s16_t  TransmitPowerValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdTransmitPowerLevelTableEntry;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdTransmitPowerLevelTableEntry[];

extern LLRP_tSTransmitPowerLevelTableEntry *
LLRP_TransmitPowerLevelTableEntry_construct (void);

extern void
LLRP_TransmitPowerLevelTableEntry_destruct (
 LLRP_tSTransmitPowerLevelTableEntry * pThis);

extern void
LLRP_TransmitPowerLevelTableEntry_decodeFields (
 LLRP_tSTransmitPowerLevelTableEntry * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_TransmitPowerLevelTableEntry_assimilateSubParameters (
 LLRP_tSTransmitPowerLevelTableEntry * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_TransmitPowerLevelTableEntry_encode (
  const LLRP_tSTransmitPowerLevelTableEntry *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdTransmitPowerLevelTableEntry_Index;

extern llrp_u16_t
LLRP_TransmitPowerLevelTableEntry_getIndex (
  LLRP_tSTransmitPowerLevelTableEntry *pThis);

extern LLRP_tResultCode
LLRP_TransmitPowerLevelTableEntry_setIndex (
  LLRP_tSTransmitPowerLevelTableEntry *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTransmitPowerLevelTableEntry_TransmitPowerValue;

extern llrp_s16_t
LLRP_TransmitPowerLevelTableEntry_getTransmitPowerValue (
  LLRP_tSTransmitPowerLevelTableEntry *pThis);

extern LLRP_tResultCode
LLRP_TransmitPowerLevelTableEntry_setTransmitPowerValue (
  LLRP_tSTransmitPowerLevelTableEntry *pThis,
  llrp_s16_t Value);

  


  
struct LLRP_SFrequencyInformation
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  Hopping;

  
    LLRP_tSFrequencyHopTable * listFrequencyHopTable;

    LLRP_tSFixedFrequencyTable * pFixedFrequencyTable;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdFrequencyInformation;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFrequencyInformation[];

extern LLRP_tSFrequencyInformation *
LLRP_FrequencyInformation_construct (void);

extern void
LLRP_FrequencyInformation_destruct (
 LLRP_tSFrequencyInformation * pThis);

extern void
LLRP_FrequencyInformation_decodeFields (
 LLRP_tSFrequencyInformation * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FrequencyInformation_assimilateSubParameters (
 LLRP_tSFrequencyInformation * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FrequencyInformation_encode (
  const LLRP_tSFrequencyInformation *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyInformation_Hopping;

extern llrp_u1_t
LLRP_FrequencyInformation_getHopping (
  LLRP_tSFrequencyInformation *pThis);

extern LLRP_tResultCode
LLRP_FrequencyInformation_setHopping (
  LLRP_tSFrequencyInformation *pThis,
  llrp_u1_t Value);

  
extern LLRP_tSFrequencyHopTable *
LLRP_FrequencyInformation_beginFrequencyHopTable (
  LLRP_tSFrequencyInformation *pThis);

extern LLRP_tSFrequencyHopTable *
LLRP_FrequencyInformation_nextFrequencyHopTable (
  LLRP_tSFrequencyHopTable *pCurrent);

extern void
LLRP_FrequencyInformation_clearFrequencyHopTable (
  LLRP_tSFrequencyInformation *pThis);

extern LLRP_tResultCode
LLRP_FrequencyInformation_addFrequencyHopTable (
  LLRP_tSFrequencyInformation *pThis,
  LLRP_tSFrequencyHopTable *pValue);


extern LLRP_tSFixedFrequencyTable *
LLRP_FrequencyInformation_getFixedFrequencyTable (
  LLRP_tSFrequencyInformation *pThis);

extern LLRP_tResultCode
LLRP_FrequencyInformation_setFixedFrequencyTable (
  LLRP_tSFrequencyInformation *pThis,
  LLRP_tSFixedFrequencyTable *pValue);



  
struct LLRP_SFrequencyHopTable
{
    LLRP_tSParameter hdr;
  
    llrp_u8_t  HopTableID;

    llrp_u32v_t  Frequency;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdFrequencyHopTable;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFrequencyHopTable[];

extern LLRP_tSFrequencyHopTable *
LLRP_FrequencyHopTable_construct (void);

extern void
LLRP_FrequencyHopTable_destruct (
 LLRP_tSFrequencyHopTable * pThis);

extern void
LLRP_FrequencyHopTable_decodeFields (
 LLRP_tSFrequencyHopTable * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FrequencyHopTable_assimilateSubParameters (
 LLRP_tSFrequencyHopTable * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FrequencyHopTable_encode (
  const LLRP_tSFrequencyHopTable *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyHopTable_HopTableID;

extern llrp_u8_t
LLRP_FrequencyHopTable_getHopTableID (
  LLRP_tSFrequencyHopTable *pThis);

extern LLRP_tResultCode
LLRP_FrequencyHopTable_setHopTableID (
  LLRP_tSFrequencyHopTable *pThis,
  llrp_u8_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyHopTable_Frequency;

extern llrp_u32v_t
LLRP_FrequencyHopTable_getFrequency (
  LLRP_tSFrequencyHopTable *pThis);

extern LLRP_tResultCode
LLRP_FrequencyHopTable_setFrequency (
  LLRP_tSFrequencyHopTable *pThis,
  llrp_u32v_t Value);

  


  
struct LLRP_SFixedFrequencyTable
{
    LLRP_tSParameter hdr;
  
    llrp_u32v_t  Frequency;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdFixedFrequencyTable;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFixedFrequencyTable[];

extern LLRP_tSFixedFrequencyTable *
LLRP_FixedFrequencyTable_construct (void);

extern void
LLRP_FixedFrequencyTable_destruct (
 LLRP_tSFixedFrequencyTable * pThis);

extern void
LLRP_FixedFrequencyTable_decodeFields (
 LLRP_tSFixedFrequencyTable * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FixedFrequencyTable_assimilateSubParameters (
 LLRP_tSFixedFrequencyTable * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FixedFrequencyTable_encode (
  const LLRP_tSFixedFrequencyTable *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFixedFrequencyTable_Frequency;

extern llrp_u32v_t
LLRP_FixedFrequencyTable_getFrequency (
  LLRP_tSFixedFrequencyTable *pThis);

extern LLRP_tResultCode
LLRP_FixedFrequencyTable_setFrequency (
  LLRP_tSFixedFrequencyTable *pThis,
  llrp_u32v_t Value);

  


  
struct LLRP_SROSpec
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  ROSpecID;

    llrp_u8_t  Priority;

    LLRP_tEROSpecState  eCurrentState;

  
    LLRP_tSROBoundarySpec * pROBoundarySpec;

    LLRP_tSParameter * listSpecParameter;

    LLRP_tSROReportSpec * pROReportSpec;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdROSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROSpec[];

extern LLRP_tSROSpec *
LLRP_ROSpec_construct (void);

extern void
LLRP_ROSpec_destruct (
 LLRP_tSROSpec * pThis);

extern void
LLRP_ROSpec_decodeFields (
 LLRP_tSROSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROSpec_assimilateSubParameters (
 LLRP_tSROSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROSpec_encode (
  const LLRP_tSROSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROSpec_ROSpecID;

extern llrp_u32_t
LLRP_ROSpec_getROSpecID (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_setROSpecID (
  LLRP_tSROSpec *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROSpec_Priority;

extern llrp_u8_t
LLRP_ROSpec_getPriority (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_setPriority (
  LLRP_tSROSpec *pThis,
  llrp_u8_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROSpec_CurrentState;

extern LLRP_tEROSpecState
LLRP_ROSpec_getCurrentState (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_setCurrentState (
  LLRP_tSROSpec *pThis,
  LLRP_tEROSpecState Value);

  
extern LLRP_tSROBoundarySpec *
LLRP_ROSpec_getROBoundarySpec (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_setROBoundarySpec (
  LLRP_tSROSpec *pThis,
  LLRP_tSROBoundarySpec *pValue);

extern LLRP_tSParameter *
LLRP_ROSpec_beginSpecParameter (
  LLRP_tSROSpec *pThis);

extern LLRP_tSParameter *
LLRP_ROSpec_nextSpecParameter (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_ROSpec_clearSpecParameter (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_addSpecParameter (
  LLRP_tSROSpec *pThis,
  LLRP_tSParameter *pValue);


extern LLRP_tSROReportSpec *
LLRP_ROSpec_getROReportSpec (
  LLRP_tSROSpec *pThis);

extern LLRP_tResultCode
LLRP_ROSpec_setROReportSpec (
  LLRP_tSROSpec *pThis,
  LLRP_tSROReportSpec *pValue);



  
struct LLRP_SROBoundarySpec
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSROSpecStartTrigger * pROSpecStartTrigger;

    LLRP_tSROSpecStopTrigger * pROSpecStopTrigger;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdROBoundarySpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROBoundarySpec[];

extern LLRP_tSROBoundarySpec *
LLRP_ROBoundarySpec_construct (void);

extern void
LLRP_ROBoundarySpec_destruct (
 LLRP_tSROBoundarySpec * pThis);

extern void
LLRP_ROBoundarySpec_decodeFields (
 LLRP_tSROBoundarySpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROBoundarySpec_assimilateSubParameters (
 LLRP_tSROBoundarySpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROBoundarySpec_encode (
  const LLRP_tSROBoundarySpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSROSpecStartTrigger *
LLRP_ROBoundarySpec_getROSpecStartTrigger (
  LLRP_tSROBoundarySpec *pThis);

extern LLRP_tResultCode
LLRP_ROBoundarySpec_setROSpecStartTrigger (
  LLRP_tSROBoundarySpec *pThis,
  LLRP_tSROSpecStartTrigger *pValue);

extern LLRP_tSROSpecStopTrigger *
LLRP_ROBoundarySpec_getROSpecStopTrigger (
  LLRP_tSROBoundarySpec *pThis);

extern LLRP_tResultCode
LLRP_ROBoundarySpec_setROSpecStopTrigger (
  LLRP_tSROBoundarySpec *pThis,
  LLRP_tSROSpecStopTrigger *pValue);



  
struct LLRP_SROSpecStartTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tEROSpecStartTriggerType  eROSpecStartTriggerType;

  
    LLRP_tSPeriodicTriggerValue * pPeriodicTriggerValue;

    LLRP_tSGPITriggerValue * pGPITriggerValue;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdROSpecStartTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROSpecStartTrigger[];

extern LLRP_tSROSpecStartTrigger *
LLRP_ROSpecStartTrigger_construct (void);

extern void
LLRP_ROSpecStartTrigger_destruct (
 LLRP_tSROSpecStartTrigger * pThis);

extern void
LLRP_ROSpecStartTrigger_decodeFields (
 LLRP_tSROSpecStartTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROSpecStartTrigger_assimilateSubParameters (
 LLRP_tSROSpecStartTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROSpecStartTrigger_encode (
  const LLRP_tSROSpecStartTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecStartTrigger_ROSpecStartTriggerType;

extern LLRP_tEROSpecStartTriggerType
LLRP_ROSpecStartTrigger_getROSpecStartTriggerType (
  LLRP_tSROSpecStartTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStartTrigger_setROSpecStartTriggerType (
  LLRP_tSROSpecStartTrigger *pThis,
  LLRP_tEROSpecStartTriggerType Value);

  
extern LLRP_tSPeriodicTriggerValue *
LLRP_ROSpecStartTrigger_getPeriodicTriggerValue (
  LLRP_tSROSpecStartTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStartTrigger_setPeriodicTriggerValue (
  LLRP_tSROSpecStartTrigger *pThis,
  LLRP_tSPeriodicTriggerValue *pValue);

extern LLRP_tSGPITriggerValue *
LLRP_ROSpecStartTrigger_getGPITriggerValue (
  LLRP_tSROSpecStartTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStartTrigger_setGPITriggerValue (
  LLRP_tSROSpecStartTrigger *pThis,
  LLRP_tSGPITriggerValue *pValue);



  
struct LLRP_SPeriodicTriggerValue
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  Offset;

    llrp_u32_t  Period;

  
    LLRP_tSUTCTimestamp * pUTCTimestamp;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdPeriodicTriggerValue;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdPeriodicTriggerValue[];

extern LLRP_tSPeriodicTriggerValue *
LLRP_PeriodicTriggerValue_construct (void);

extern void
LLRP_PeriodicTriggerValue_destruct (
 LLRP_tSPeriodicTriggerValue * pThis);

extern void
LLRP_PeriodicTriggerValue_decodeFields (
 LLRP_tSPeriodicTriggerValue * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_PeriodicTriggerValue_assimilateSubParameters (
 LLRP_tSPeriodicTriggerValue * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_PeriodicTriggerValue_encode (
  const LLRP_tSPeriodicTriggerValue *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdPeriodicTriggerValue_Offset;

extern llrp_u32_t
LLRP_PeriodicTriggerValue_getOffset (
  LLRP_tSPeriodicTriggerValue *pThis);

extern LLRP_tResultCode
LLRP_PeriodicTriggerValue_setOffset (
  LLRP_tSPeriodicTriggerValue *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdPeriodicTriggerValue_Period;

extern llrp_u32_t
LLRP_PeriodicTriggerValue_getPeriod (
  LLRP_tSPeriodicTriggerValue *pThis);

extern LLRP_tResultCode
LLRP_PeriodicTriggerValue_setPeriod (
  LLRP_tSPeriodicTriggerValue *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSUTCTimestamp *
LLRP_PeriodicTriggerValue_getUTCTimestamp (
  LLRP_tSPeriodicTriggerValue *pThis);

extern LLRP_tResultCode
LLRP_PeriodicTriggerValue_setUTCTimestamp (
  LLRP_tSPeriodicTriggerValue *pThis,
  LLRP_tSUTCTimestamp *pValue);



  
struct LLRP_SGPITriggerValue
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  GPIPortNum;

    llrp_u1_t  GPIEvent;

    llrp_u32_t  Timeout;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGPITriggerValue;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGPITriggerValue[];

extern LLRP_tSGPITriggerValue *
LLRP_GPITriggerValue_construct (void);

extern void
LLRP_GPITriggerValue_destruct (
 LLRP_tSGPITriggerValue * pThis);

extern void
LLRP_GPITriggerValue_decodeFields (
 LLRP_tSGPITriggerValue * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GPITriggerValue_assimilateSubParameters (
 LLRP_tSGPITriggerValue * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GPITriggerValue_encode (
  const LLRP_tSGPITriggerValue *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGPITriggerValue_GPIPortNum;

extern llrp_u16_t
LLRP_GPITriggerValue_getGPIPortNum (
  LLRP_tSGPITriggerValue *pThis);

extern LLRP_tResultCode
LLRP_GPITriggerValue_setGPIPortNum (
  LLRP_tSGPITriggerValue *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPITriggerValue_GPIEvent;

extern llrp_u1_t
LLRP_GPITriggerValue_getGPIEvent (
  LLRP_tSGPITriggerValue *pThis);

extern LLRP_tResultCode
LLRP_GPITriggerValue_setGPIEvent (
  LLRP_tSGPITriggerValue *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPITriggerValue_Timeout;

extern llrp_u32_t
LLRP_GPITriggerValue_getTimeout (
  LLRP_tSGPITriggerValue *pThis);

extern LLRP_tResultCode
LLRP_GPITriggerValue_setTimeout (
  LLRP_tSGPITriggerValue *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SROSpecStopTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tEROSpecStopTriggerType  eROSpecStopTriggerType;

    llrp_u32_t  DurationTriggerValue;

  
    LLRP_tSGPITriggerValue * pGPITriggerValue;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdROSpecStopTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROSpecStopTrigger[];

extern LLRP_tSROSpecStopTrigger *
LLRP_ROSpecStopTrigger_construct (void);

extern void
LLRP_ROSpecStopTrigger_destruct (
 LLRP_tSROSpecStopTrigger * pThis);

extern void
LLRP_ROSpecStopTrigger_decodeFields (
 LLRP_tSROSpecStopTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROSpecStopTrigger_assimilateSubParameters (
 LLRP_tSROSpecStopTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROSpecStopTrigger_encode (
  const LLRP_tSROSpecStopTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecStopTrigger_ROSpecStopTriggerType;

extern LLRP_tEROSpecStopTriggerType
LLRP_ROSpecStopTrigger_getROSpecStopTriggerType (
  LLRP_tSROSpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStopTrigger_setROSpecStopTriggerType (
  LLRP_tSROSpecStopTrigger *pThis,
  LLRP_tEROSpecStopTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecStopTrigger_DurationTriggerValue;

extern llrp_u32_t
LLRP_ROSpecStopTrigger_getDurationTriggerValue (
  LLRP_tSROSpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStopTrigger_setDurationTriggerValue (
  LLRP_tSROSpecStopTrigger *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSGPITriggerValue *
LLRP_ROSpecStopTrigger_getGPITriggerValue (
  LLRP_tSROSpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_ROSpecStopTrigger_setGPITriggerValue (
  LLRP_tSROSpecStopTrigger *pThis,
  LLRP_tSGPITriggerValue *pValue);



  
struct LLRP_SAISpec
{
    LLRP_tSParameter hdr;
  
    llrp_u16v_t  AntennaIDs;

  
    LLRP_tSAISpecStopTrigger * pAISpecStopTrigger;

    LLRP_tSInventoryParameterSpec * listInventoryParameterSpec;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAISpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAISpec[];

extern LLRP_tSAISpec *
LLRP_AISpec_construct (void);

extern void
LLRP_AISpec_destruct (
 LLRP_tSAISpec * pThis);

extern void
LLRP_AISpec_decodeFields (
 LLRP_tSAISpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AISpec_assimilateSubParameters (
 LLRP_tSAISpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AISpec_encode (
  const LLRP_tSAISpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAISpec_AntennaIDs;

extern llrp_u16v_t
LLRP_AISpec_getAntennaIDs (
  LLRP_tSAISpec *pThis);

extern LLRP_tResultCode
LLRP_AISpec_setAntennaIDs (
  LLRP_tSAISpec *pThis,
  llrp_u16v_t Value);

  
extern LLRP_tSAISpecStopTrigger *
LLRP_AISpec_getAISpecStopTrigger (
  LLRP_tSAISpec *pThis);

extern LLRP_tResultCode
LLRP_AISpec_setAISpecStopTrigger (
  LLRP_tSAISpec *pThis,
  LLRP_tSAISpecStopTrigger *pValue);

extern LLRP_tSInventoryParameterSpec *
LLRP_AISpec_beginInventoryParameterSpec (
  LLRP_tSAISpec *pThis);

extern LLRP_tSInventoryParameterSpec *
LLRP_AISpec_nextInventoryParameterSpec (
  LLRP_tSInventoryParameterSpec *pCurrent);

extern void
LLRP_AISpec_clearInventoryParameterSpec (
  LLRP_tSAISpec *pThis);

extern LLRP_tResultCode
LLRP_AISpec_addInventoryParameterSpec (
  LLRP_tSAISpec *pThis,
  LLRP_tSInventoryParameterSpec *pValue);


extern LLRP_tSParameter *
LLRP_AISpec_beginCustom (
  LLRP_tSAISpec *pThis);

extern LLRP_tSParameter *
LLRP_AISpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_AISpec_clearCustom (
  LLRP_tSAISpec *pThis);

extern LLRP_tResultCode
LLRP_AISpec_addCustom (
  LLRP_tSAISpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SAISpecStopTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tEAISpecStopTriggerType  eAISpecStopTriggerType;

    llrp_u32_t  DurationTrigger;

  
    LLRP_tSGPITriggerValue * pGPITriggerValue;

    LLRP_tSTagObservationTrigger * pTagObservationTrigger;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAISpecStopTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAISpecStopTrigger[];

extern LLRP_tSAISpecStopTrigger *
LLRP_AISpecStopTrigger_construct (void);

extern void
LLRP_AISpecStopTrigger_destruct (
 LLRP_tSAISpecStopTrigger * pThis);

extern void
LLRP_AISpecStopTrigger_decodeFields (
 LLRP_tSAISpecStopTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AISpecStopTrigger_assimilateSubParameters (
 LLRP_tSAISpecStopTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AISpecStopTrigger_encode (
  const LLRP_tSAISpecStopTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAISpecStopTrigger_AISpecStopTriggerType;

extern LLRP_tEAISpecStopTriggerType
LLRP_AISpecStopTrigger_getAISpecStopTriggerType (
  LLRP_tSAISpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AISpecStopTrigger_setAISpecStopTriggerType (
  LLRP_tSAISpecStopTrigger *pThis,
  LLRP_tEAISpecStopTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAISpecStopTrigger_DurationTrigger;

extern llrp_u32_t
LLRP_AISpecStopTrigger_getDurationTrigger (
  LLRP_tSAISpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AISpecStopTrigger_setDurationTrigger (
  LLRP_tSAISpecStopTrigger *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSGPITriggerValue *
LLRP_AISpecStopTrigger_getGPITriggerValue (
  LLRP_tSAISpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AISpecStopTrigger_setGPITriggerValue (
  LLRP_tSAISpecStopTrigger *pThis,
  LLRP_tSGPITriggerValue *pValue);

extern LLRP_tSTagObservationTrigger *
LLRP_AISpecStopTrigger_getTagObservationTrigger (
  LLRP_tSAISpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AISpecStopTrigger_setTagObservationTrigger (
  LLRP_tSAISpecStopTrigger *pThis,
  LLRP_tSTagObservationTrigger *pValue);



  
struct LLRP_STagObservationTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tETagObservationTriggerType  eTriggerType;

    llrp_u16_t  NumberOfTags;

    llrp_u16_t  NumberOfAttempts;

    llrp_u16_t  T;

    llrp_u32_t  Timeout;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdTagObservationTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdTagObservationTrigger[];

extern LLRP_tSTagObservationTrigger *
LLRP_TagObservationTrigger_construct (void);

extern void
LLRP_TagObservationTrigger_destruct (
 LLRP_tSTagObservationTrigger * pThis);

extern void
LLRP_TagObservationTrigger_decodeFields (
 LLRP_tSTagObservationTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_TagObservationTrigger_assimilateSubParameters (
 LLRP_tSTagObservationTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_TagObservationTrigger_encode (
  const LLRP_tSTagObservationTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdTagObservationTrigger_TriggerType;

extern LLRP_tETagObservationTriggerType
LLRP_TagObservationTrigger_getTriggerType (
  LLRP_tSTagObservationTrigger *pThis);

extern LLRP_tResultCode
LLRP_TagObservationTrigger_setTriggerType (
  LLRP_tSTagObservationTrigger *pThis,
  LLRP_tETagObservationTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagObservationTrigger_NumberOfTags;

extern llrp_u16_t
LLRP_TagObservationTrigger_getNumberOfTags (
  LLRP_tSTagObservationTrigger *pThis);

extern LLRP_tResultCode
LLRP_TagObservationTrigger_setNumberOfTags (
  LLRP_tSTagObservationTrigger *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagObservationTrigger_NumberOfAttempts;

extern llrp_u16_t
LLRP_TagObservationTrigger_getNumberOfAttempts (
  LLRP_tSTagObservationTrigger *pThis);

extern LLRP_tResultCode
LLRP_TagObservationTrigger_setNumberOfAttempts (
  LLRP_tSTagObservationTrigger *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagObservationTrigger_T;

extern llrp_u16_t
LLRP_TagObservationTrigger_getT (
  LLRP_tSTagObservationTrigger *pThis);

extern LLRP_tResultCode
LLRP_TagObservationTrigger_setT (
  LLRP_tSTagObservationTrigger *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagObservationTrigger_Timeout;

extern llrp_u32_t
LLRP_TagObservationTrigger_getTimeout (
  LLRP_tSTagObservationTrigger *pThis);

extern LLRP_tResultCode
LLRP_TagObservationTrigger_setTimeout (
  LLRP_tSTagObservationTrigger *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SInventoryParameterSpec
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  InventoryParameterSpecID;

    LLRP_tEAirProtocols  eProtocolID;

  
    LLRP_tSAntennaConfiguration * listAntennaConfiguration;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdInventoryParameterSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdInventoryParameterSpec[];

extern LLRP_tSInventoryParameterSpec *
LLRP_InventoryParameterSpec_construct (void);

extern void
LLRP_InventoryParameterSpec_destruct (
 LLRP_tSInventoryParameterSpec * pThis);

extern void
LLRP_InventoryParameterSpec_decodeFields (
 LLRP_tSInventoryParameterSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_InventoryParameterSpec_assimilateSubParameters (
 LLRP_tSInventoryParameterSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_InventoryParameterSpec_encode (
  const LLRP_tSInventoryParameterSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdInventoryParameterSpec_InventoryParameterSpecID;

extern llrp_u16_t
LLRP_InventoryParameterSpec_getInventoryParameterSpecID (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tResultCode
LLRP_InventoryParameterSpec_setInventoryParameterSpecID (
  LLRP_tSInventoryParameterSpec *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdInventoryParameterSpec_ProtocolID;

extern LLRP_tEAirProtocols
LLRP_InventoryParameterSpec_getProtocolID (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tResultCode
LLRP_InventoryParameterSpec_setProtocolID (
  LLRP_tSInventoryParameterSpec *pThis,
  LLRP_tEAirProtocols Value);

  
extern LLRP_tSAntennaConfiguration *
LLRP_InventoryParameterSpec_beginAntennaConfiguration (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tSAntennaConfiguration *
LLRP_InventoryParameterSpec_nextAntennaConfiguration (
  LLRP_tSAntennaConfiguration *pCurrent);

extern void
LLRP_InventoryParameterSpec_clearAntennaConfiguration (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tResultCode
LLRP_InventoryParameterSpec_addAntennaConfiguration (
  LLRP_tSInventoryParameterSpec *pThis,
  LLRP_tSAntennaConfiguration *pValue);


extern LLRP_tSParameter *
LLRP_InventoryParameterSpec_beginCustom (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tSParameter *
LLRP_InventoryParameterSpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_InventoryParameterSpec_clearCustom (
  LLRP_tSInventoryParameterSpec *pThis);

extern LLRP_tResultCode
LLRP_InventoryParameterSpec_addCustom (
  LLRP_tSInventoryParameterSpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SRFSurveySpec
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  AntennaID;

    llrp_u32_t  StartFrequency;

    llrp_u32_t  EndFrequency;

  
    LLRP_tSRFSurveySpecStopTrigger * pRFSurveySpecStopTrigger;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFSurveySpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFSurveySpec[];

extern LLRP_tSRFSurveySpec *
LLRP_RFSurveySpec_construct (void);

extern void
LLRP_RFSurveySpec_destruct (
 LLRP_tSRFSurveySpec * pThis);

extern void
LLRP_RFSurveySpec_decodeFields (
 LLRP_tSRFSurveySpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFSurveySpec_assimilateSubParameters (
 LLRP_tSRFSurveySpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFSurveySpec_encode (
  const LLRP_tSRFSurveySpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpec_AntennaID;

extern llrp_u16_t
LLRP_RFSurveySpec_getAntennaID (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpec_setAntennaID (
  LLRP_tSRFSurveySpec *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpec_StartFrequency;

extern llrp_u32_t
LLRP_RFSurveySpec_getStartFrequency (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpec_setStartFrequency (
  LLRP_tSRFSurveySpec *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpec_EndFrequency;

extern llrp_u32_t
LLRP_RFSurveySpec_getEndFrequency (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpec_setEndFrequency (
  LLRP_tSRFSurveySpec *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSRFSurveySpecStopTrigger *
LLRP_RFSurveySpec_getRFSurveySpecStopTrigger (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpec_setRFSurveySpecStopTrigger (
  LLRP_tSRFSurveySpec *pThis,
  LLRP_tSRFSurveySpecStopTrigger *pValue);

extern LLRP_tSParameter *
LLRP_RFSurveySpec_beginCustom (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tSParameter *
LLRP_RFSurveySpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_RFSurveySpec_clearCustom (
  LLRP_tSRFSurveySpec *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpec_addCustom (
  LLRP_tSRFSurveySpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SRFSurveySpecStopTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tERFSurveySpecStopTriggerType  eStopTriggerType;

    llrp_u32_t  DurationPeriod;

    llrp_u32_t  N;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFSurveySpecStopTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFSurveySpecStopTrigger[];

extern LLRP_tSRFSurveySpecStopTrigger *
LLRP_RFSurveySpecStopTrigger_construct (void);

extern void
LLRP_RFSurveySpecStopTrigger_destruct (
 LLRP_tSRFSurveySpecStopTrigger * pThis);

extern void
LLRP_RFSurveySpecStopTrigger_decodeFields (
 LLRP_tSRFSurveySpecStopTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFSurveySpecStopTrigger_assimilateSubParameters (
 LLRP_tSRFSurveySpecStopTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFSurveySpecStopTrigger_encode (
  const LLRP_tSRFSurveySpecStopTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpecStopTrigger_StopTriggerType;

extern LLRP_tERFSurveySpecStopTriggerType
LLRP_RFSurveySpecStopTrigger_getStopTriggerType (
  LLRP_tSRFSurveySpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpecStopTrigger_setStopTriggerType (
  LLRP_tSRFSurveySpecStopTrigger *pThis,
  LLRP_tERFSurveySpecStopTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpecStopTrigger_DurationPeriod;

extern llrp_u32_t
LLRP_RFSurveySpecStopTrigger_getDurationPeriod (
  LLRP_tSRFSurveySpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpecStopTrigger_setDurationPeriod (
  LLRP_tSRFSurveySpecStopTrigger *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveySpecStopTrigger_N;

extern llrp_u32_t
LLRP_RFSurveySpecStopTrigger_getN (
  LLRP_tSRFSurveySpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_RFSurveySpecStopTrigger_setN (
  LLRP_tSRFSurveySpecStopTrigger *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SLoopSpec
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  LoopCount;

  
    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdLoopSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLoopSpec[];

extern LLRP_tSLoopSpec *
LLRP_LoopSpec_construct (void);

extern void
LLRP_LoopSpec_destruct (
 LLRP_tSLoopSpec * pThis);

extern void
LLRP_LoopSpec_decodeFields (
 LLRP_tSLoopSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LoopSpec_assimilateSubParameters (
 LLRP_tSLoopSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LoopSpec_encode (
  const LLRP_tSLoopSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLoopSpec_LoopCount;

extern llrp_u32_t
LLRP_LoopSpec_getLoopCount (
  LLRP_tSLoopSpec *pThis);

extern LLRP_tResultCode
LLRP_LoopSpec_setLoopCount (
  LLRP_tSLoopSpec *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSParameter *
LLRP_LoopSpec_beginCustom (
  LLRP_tSLoopSpec *pThis);

extern LLRP_tSParameter *
LLRP_LoopSpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_LoopSpec_clearCustom (
  LLRP_tSLoopSpec *pThis);

extern LLRP_tResultCode
LLRP_LoopSpec_addCustom (
  LLRP_tSLoopSpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SAccessSpec
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  AccessSpecID;

    llrp_u16_t  AntennaID;

    LLRP_tEAirProtocols  eProtocolID;

    LLRP_tEAccessSpecState  eCurrentState;

    llrp_u32_t  ROSpecID;

  
    LLRP_tSAccessSpecStopTrigger * pAccessSpecStopTrigger;

    LLRP_tSAccessCommand * pAccessCommand;

    LLRP_tSAccessReportSpec * pAccessReportSpec;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAccessSpec[];

extern LLRP_tSAccessSpec *
LLRP_AccessSpec_construct (void);

extern void
LLRP_AccessSpec_destruct (
 LLRP_tSAccessSpec * pThis);

extern void
LLRP_AccessSpec_decodeFields (
 LLRP_tSAccessSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AccessSpec_assimilateSubParameters (
 LLRP_tSAccessSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AccessSpec_encode (
  const LLRP_tSAccessSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpec_AccessSpecID;

extern llrp_u32_t
LLRP_AccessSpec_getAccessSpecID (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setAccessSpecID (
  LLRP_tSAccessSpec *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpec_AntennaID;

extern llrp_u16_t
LLRP_AccessSpec_getAntennaID (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setAntennaID (
  LLRP_tSAccessSpec *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpec_ProtocolID;

extern LLRP_tEAirProtocols
LLRP_AccessSpec_getProtocolID (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setProtocolID (
  LLRP_tSAccessSpec *pThis,
  LLRP_tEAirProtocols Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpec_CurrentState;

extern LLRP_tEAccessSpecState
LLRP_AccessSpec_getCurrentState (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setCurrentState (
  LLRP_tSAccessSpec *pThis,
  LLRP_tEAccessSpecState Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpec_ROSpecID;

extern llrp_u32_t
LLRP_AccessSpec_getROSpecID (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setROSpecID (
  LLRP_tSAccessSpec *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSAccessSpecStopTrigger *
LLRP_AccessSpec_getAccessSpecStopTrigger (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setAccessSpecStopTrigger (
  LLRP_tSAccessSpec *pThis,
  LLRP_tSAccessSpecStopTrigger *pValue);

extern LLRP_tSAccessCommand *
LLRP_AccessSpec_getAccessCommand (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setAccessCommand (
  LLRP_tSAccessSpec *pThis,
  LLRP_tSAccessCommand *pValue);

extern LLRP_tSAccessReportSpec *
LLRP_AccessSpec_getAccessReportSpec (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_setAccessReportSpec (
  LLRP_tSAccessSpec *pThis,
  LLRP_tSAccessReportSpec *pValue);

extern LLRP_tSParameter *
LLRP_AccessSpec_beginCustom (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tSParameter *
LLRP_AccessSpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_AccessSpec_clearCustom (
  LLRP_tSAccessSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessSpec_addCustom (
  LLRP_tSAccessSpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SAccessSpecStopTrigger
{
    LLRP_tSParameter hdr;
  
    LLRP_tEAccessSpecStopTriggerType  eAccessSpecStopTrigger;

    llrp_u16_t  OperationCountValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessSpecStopTrigger;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAccessSpecStopTrigger[];

extern LLRP_tSAccessSpecStopTrigger *
LLRP_AccessSpecStopTrigger_construct (void);

extern void
LLRP_AccessSpecStopTrigger_destruct (
 LLRP_tSAccessSpecStopTrigger * pThis);

extern void
LLRP_AccessSpecStopTrigger_decodeFields (
 LLRP_tSAccessSpecStopTrigger * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AccessSpecStopTrigger_assimilateSubParameters (
 LLRP_tSAccessSpecStopTrigger * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AccessSpecStopTrigger_encode (
  const LLRP_tSAccessSpecStopTrigger *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpecStopTrigger_AccessSpecStopTrigger;

extern LLRP_tEAccessSpecStopTriggerType
LLRP_AccessSpecStopTrigger_getAccessSpecStopTrigger (
  LLRP_tSAccessSpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AccessSpecStopTrigger_setAccessSpecStopTrigger (
  LLRP_tSAccessSpecStopTrigger *pThis,
  LLRP_tEAccessSpecStopTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpecStopTrigger_OperationCountValue;

extern llrp_u16_t
LLRP_AccessSpecStopTrigger_getOperationCountValue (
  LLRP_tSAccessSpecStopTrigger *pThis);

extern LLRP_tResultCode
LLRP_AccessSpecStopTrigger_setOperationCountValue (
  LLRP_tSAccessSpecStopTrigger *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SAccessCommand
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSParameter * pAirProtocolTagSpec;

    LLRP_tSParameter * listAccessCommandOpSpec;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessCommand;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAccessCommand[];

extern LLRP_tSAccessCommand *
LLRP_AccessCommand_construct (void);

extern void
LLRP_AccessCommand_destruct (
 LLRP_tSAccessCommand * pThis);

extern void
LLRP_AccessCommand_decodeFields (
 LLRP_tSAccessCommand * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AccessCommand_assimilateSubParameters (
 LLRP_tSAccessCommand * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AccessCommand_encode (
  const LLRP_tSAccessCommand *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSParameter *
LLRP_AccessCommand_getAirProtocolTagSpec (
  LLRP_tSAccessCommand *pThis);

extern LLRP_tResultCode
LLRP_AccessCommand_setAirProtocolTagSpec (
  LLRP_tSAccessCommand *pThis,
  LLRP_tSParameter *pValue);

extern LLRP_tSParameter *
LLRP_AccessCommand_beginAccessCommandOpSpec (
  LLRP_tSAccessCommand *pThis);

extern LLRP_tSParameter *
LLRP_AccessCommand_nextAccessCommandOpSpec (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_AccessCommand_clearAccessCommandOpSpec (
  LLRP_tSAccessCommand *pThis);

extern LLRP_tResultCode
LLRP_AccessCommand_addAccessCommandOpSpec (
  LLRP_tSAccessCommand *pThis,
  LLRP_tSParameter *pValue);


extern LLRP_tSParameter *
LLRP_AccessCommand_beginCustom (
  LLRP_tSAccessCommand *pThis);

extern LLRP_tSParameter *
LLRP_AccessCommand_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_AccessCommand_clearCustom (
  LLRP_tSAccessCommand *pThis);

extern LLRP_tResultCode
LLRP_AccessCommand_addCustom (
  LLRP_tSAccessCommand *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SLLRPConfigurationStateValue
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  LLRPConfigurationStateValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdLLRPConfigurationStateValue;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLLRPConfigurationStateValue[];

extern LLRP_tSLLRPConfigurationStateValue *
LLRP_LLRPConfigurationStateValue_construct (void);

extern void
LLRP_LLRPConfigurationStateValue_destruct (
 LLRP_tSLLRPConfigurationStateValue * pThis);

extern void
LLRP_LLRPConfigurationStateValue_decodeFields (
 LLRP_tSLLRPConfigurationStateValue * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LLRPConfigurationStateValue_assimilateSubParameters (
 LLRP_tSLLRPConfigurationStateValue * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LLRPConfigurationStateValue_encode (
  const LLRP_tSLLRPConfigurationStateValue *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPConfigurationStateValue_LLRPConfigurationStateValue;

extern llrp_u32_t
LLRP_LLRPConfigurationStateValue_getLLRPConfigurationStateValue (
  LLRP_tSLLRPConfigurationStateValue *pThis);

extern LLRP_tResultCode
LLRP_LLRPConfigurationStateValue_setLLRPConfigurationStateValue (
  LLRP_tSLLRPConfigurationStateValue *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SIdentification
{
    LLRP_tSParameter hdr;
  
    LLRP_tEIdentificationType  eIDType;

    llrp_u8v_t  ReaderID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdIdentification;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdIdentification[];

extern LLRP_tSIdentification *
LLRP_Identification_construct (void);

extern void
LLRP_Identification_destruct (
 LLRP_tSIdentification * pThis);

extern void
LLRP_Identification_decodeFields (
 LLRP_tSIdentification * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_Identification_assimilateSubParameters (
 LLRP_tSIdentification * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_Identification_encode (
  const LLRP_tSIdentification *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdIdentification_IDType;

extern LLRP_tEIdentificationType
LLRP_Identification_getIDType (
  LLRP_tSIdentification *pThis);

extern LLRP_tResultCode
LLRP_Identification_setIDType (
  LLRP_tSIdentification *pThis,
  LLRP_tEIdentificationType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdIdentification_ReaderID;

extern llrp_u8v_t
LLRP_Identification_getReaderID (
  LLRP_tSIdentification *pThis);

extern LLRP_tResultCode
LLRP_Identification_setReaderID (
  LLRP_tSIdentification *pThis,
  llrp_u8v_t Value);

  


  
struct LLRP_SGPOWriteData
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  GPOPortNumber;

    llrp_u1_t  GPOData;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGPOWriteData;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGPOWriteData[];

extern LLRP_tSGPOWriteData *
LLRP_GPOWriteData_construct (void);

extern void
LLRP_GPOWriteData_destruct (
 LLRP_tSGPOWriteData * pThis);

extern void
LLRP_GPOWriteData_decodeFields (
 LLRP_tSGPOWriteData * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GPOWriteData_assimilateSubParameters (
 LLRP_tSGPOWriteData * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GPOWriteData_encode (
  const LLRP_tSGPOWriteData *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGPOWriteData_GPOPortNumber;

extern llrp_u16_t
LLRP_GPOWriteData_getGPOPortNumber (
  LLRP_tSGPOWriteData *pThis);

extern LLRP_tResultCode
LLRP_GPOWriteData_setGPOPortNumber (
  LLRP_tSGPOWriteData *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPOWriteData_GPOData;

extern llrp_u1_t
LLRP_GPOWriteData_getGPOData (
  LLRP_tSGPOWriteData *pThis);

extern LLRP_tResultCode
LLRP_GPOWriteData_setGPOData (
  LLRP_tSGPOWriteData *pThis,
  llrp_u1_t Value);

  


  
struct LLRP_SKeepaliveSpec
{
    LLRP_tSParameter hdr;
  
    LLRP_tEKeepaliveTriggerType  eKeepaliveTriggerType;

    llrp_u32_t  PeriodicTriggerValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdKeepaliveSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdKeepaliveSpec[];

extern LLRP_tSKeepaliveSpec *
LLRP_KeepaliveSpec_construct (void);

extern void
LLRP_KeepaliveSpec_destruct (
 LLRP_tSKeepaliveSpec * pThis);

extern void
LLRP_KeepaliveSpec_decodeFields (
 LLRP_tSKeepaliveSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_KeepaliveSpec_assimilateSubParameters (
 LLRP_tSKeepaliveSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_KeepaliveSpec_encode (
  const LLRP_tSKeepaliveSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdKeepaliveSpec_KeepaliveTriggerType;

extern LLRP_tEKeepaliveTriggerType
LLRP_KeepaliveSpec_getKeepaliveTriggerType (
  LLRP_tSKeepaliveSpec *pThis);

extern LLRP_tResultCode
LLRP_KeepaliveSpec_setKeepaliveTriggerType (
  LLRP_tSKeepaliveSpec *pThis,
  LLRP_tEKeepaliveTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdKeepaliveSpec_PeriodicTriggerValue;

extern llrp_u32_t
LLRP_KeepaliveSpec_getPeriodicTriggerValue (
  LLRP_tSKeepaliveSpec *pThis);

extern LLRP_tResultCode
LLRP_KeepaliveSpec_setPeriodicTriggerValue (
  LLRP_tSKeepaliveSpec *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SAntennaProperties
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  AntennaConnected;

    llrp_u16_t  AntennaID;

    llrp_s16_t  AntennaGain;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAntennaProperties;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAntennaProperties[];

extern LLRP_tSAntennaProperties *
LLRP_AntennaProperties_construct (void);

extern void
LLRP_AntennaProperties_destruct (
 LLRP_tSAntennaProperties * pThis);

extern void
LLRP_AntennaProperties_decodeFields (
 LLRP_tSAntennaProperties * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AntennaProperties_assimilateSubParameters (
 LLRP_tSAntennaProperties * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AntennaProperties_encode (
  const LLRP_tSAntennaProperties *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaProperties_AntennaConnected;

extern llrp_u1_t
LLRP_AntennaProperties_getAntennaConnected (
  LLRP_tSAntennaProperties *pThis);

extern LLRP_tResultCode
LLRP_AntennaProperties_setAntennaConnected (
  LLRP_tSAntennaProperties *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaProperties_AntennaID;

extern llrp_u16_t
LLRP_AntennaProperties_getAntennaID (
  LLRP_tSAntennaProperties *pThis);

extern LLRP_tResultCode
LLRP_AntennaProperties_setAntennaID (
  LLRP_tSAntennaProperties *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaProperties_AntennaGain;

extern llrp_s16_t
LLRP_AntennaProperties_getAntennaGain (
  LLRP_tSAntennaProperties *pThis);

extern LLRP_tResultCode
LLRP_AntennaProperties_setAntennaGain (
  LLRP_tSAntennaProperties *pThis,
  llrp_s16_t Value);

  


  
struct LLRP_SAntennaConfiguration
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  AntennaID;

  
    LLRP_tSRFReceiver * pRFReceiver;

    LLRP_tSRFTransmitter * pRFTransmitter;

    LLRP_tSParameter * listAirProtocolInventoryCommandSettings;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAntennaConfiguration;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAntennaConfiguration[];

extern LLRP_tSAntennaConfiguration *
LLRP_AntennaConfiguration_construct (void);

extern void
LLRP_AntennaConfiguration_destruct (
 LLRP_tSAntennaConfiguration * pThis);

extern void
LLRP_AntennaConfiguration_decodeFields (
 LLRP_tSAntennaConfiguration * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AntennaConfiguration_assimilateSubParameters (
 LLRP_tSAntennaConfiguration * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AntennaConfiguration_encode (
  const LLRP_tSAntennaConfiguration *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaConfiguration_AntennaID;

extern llrp_u16_t
LLRP_AntennaConfiguration_getAntennaID (
  LLRP_tSAntennaConfiguration *pThis);

extern LLRP_tResultCode
LLRP_AntennaConfiguration_setAntennaID (
  LLRP_tSAntennaConfiguration *pThis,
  llrp_u16_t Value);

  
extern LLRP_tSRFReceiver *
LLRP_AntennaConfiguration_getRFReceiver (
  LLRP_tSAntennaConfiguration *pThis);

extern LLRP_tResultCode
LLRP_AntennaConfiguration_setRFReceiver (
  LLRP_tSAntennaConfiguration *pThis,
  LLRP_tSRFReceiver *pValue);

extern LLRP_tSRFTransmitter *
LLRP_AntennaConfiguration_getRFTransmitter (
  LLRP_tSAntennaConfiguration *pThis);

extern LLRP_tResultCode
LLRP_AntennaConfiguration_setRFTransmitter (
  LLRP_tSAntennaConfiguration *pThis,
  LLRP_tSRFTransmitter *pValue);

extern LLRP_tSParameter *
LLRP_AntennaConfiguration_beginAirProtocolInventoryCommandSettings (
  LLRP_tSAntennaConfiguration *pThis);

extern LLRP_tSParameter *
LLRP_AntennaConfiguration_nextAirProtocolInventoryCommandSettings (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_AntennaConfiguration_clearAirProtocolInventoryCommandSettings (
  LLRP_tSAntennaConfiguration *pThis);

extern LLRP_tResultCode
LLRP_AntennaConfiguration_addAirProtocolInventoryCommandSettings (
  LLRP_tSAntennaConfiguration *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SRFReceiver
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  ReceiverSensitivity;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFReceiver;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFReceiver[];

extern LLRP_tSRFReceiver *
LLRP_RFReceiver_construct (void);

extern void
LLRP_RFReceiver_destruct (
 LLRP_tSRFReceiver * pThis);

extern void
LLRP_RFReceiver_decodeFields (
 LLRP_tSRFReceiver * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFReceiver_assimilateSubParameters (
 LLRP_tSRFReceiver * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFReceiver_encode (
  const LLRP_tSRFReceiver *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRFReceiver_ReceiverSensitivity;

extern llrp_u16_t
LLRP_RFReceiver_getReceiverSensitivity (
  LLRP_tSRFReceiver *pThis);

extern LLRP_tResultCode
LLRP_RFReceiver_setReceiverSensitivity (
  LLRP_tSRFReceiver *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SRFTransmitter
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  HopTableID;

    llrp_u16_t  ChannelIndex;

    llrp_u16_t  TransmitPower;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFTransmitter;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFTransmitter[];

extern LLRP_tSRFTransmitter *
LLRP_RFTransmitter_construct (void);

extern void
LLRP_RFTransmitter_destruct (
 LLRP_tSRFTransmitter * pThis);

extern void
LLRP_RFTransmitter_decodeFields (
 LLRP_tSRFTransmitter * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFTransmitter_assimilateSubParameters (
 LLRP_tSRFTransmitter * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFTransmitter_encode (
  const LLRP_tSRFTransmitter *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRFTransmitter_HopTableID;

extern llrp_u16_t
LLRP_RFTransmitter_getHopTableID (
  LLRP_tSRFTransmitter *pThis);

extern LLRP_tResultCode
LLRP_RFTransmitter_setHopTableID (
  LLRP_tSRFTransmitter *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFTransmitter_ChannelIndex;

extern llrp_u16_t
LLRP_RFTransmitter_getChannelIndex (
  LLRP_tSRFTransmitter *pThis);

extern LLRP_tResultCode
LLRP_RFTransmitter_setChannelIndex (
  LLRP_tSRFTransmitter *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFTransmitter_TransmitPower;

extern llrp_u16_t
LLRP_RFTransmitter_getTransmitPower (
  LLRP_tSRFTransmitter *pThis);

extern LLRP_tResultCode
LLRP_RFTransmitter_setTransmitPower (
  LLRP_tSRFTransmitter *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SGPIPortCurrentState
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  GPIPortNum;

    llrp_u1_t  Config;

    LLRP_tEGPIPortState  eState;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGPIPortCurrentState;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGPIPortCurrentState[];

extern LLRP_tSGPIPortCurrentState *
LLRP_GPIPortCurrentState_construct (void);

extern void
LLRP_GPIPortCurrentState_destruct (
 LLRP_tSGPIPortCurrentState * pThis);

extern void
LLRP_GPIPortCurrentState_decodeFields (
 LLRP_tSGPIPortCurrentState * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GPIPortCurrentState_assimilateSubParameters (
 LLRP_tSGPIPortCurrentState * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GPIPortCurrentState_encode (
  const LLRP_tSGPIPortCurrentState *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGPIPortCurrentState_GPIPortNum;

extern llrp_u16_t
LLRP_GPIPortCurrentState_getGPIPortNum (
  LLRP_tSGPIPortCurrentState *pThis);

extern LLRP_tResultCode
LLRP_GPIPortCurrentState_setGPIPortNum (
  LLRP_tSGPIPortCurrentState *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPIPortCurrentState_Config;

extern llrp_u1_t
LLRP_GPIPortCurrentState_getConfig (
  LLRP_tSGPIPortCurrentState *pThis);

extern LLRP_tResultCode
LLRP_GPIPortCurrentState_setConfig (
  LLRP_tSGPIPortCurrentState *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPIPortCurrentState_State;

extern LLRP_tEGPIPortState
LLRP_GPIPortCurrentState_getState (
  LLRP_tSGPIPortCurrentState *pThis);

extern LLRP_tResultCode
LLRP_GPIPortCurrentState_setState (
  LLRP_tSGPIPortCurrentState *pThis,
  LLRP_tEGPIPortState Value);

  


  
struct LLRP_SEventsAndReports
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  HoldEventsAndReportsUponReconnect;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdEventsAndReports;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdEventsAndReports[];

extern LLRP_tSEventsAndReports *
LLRP_EventsAndReports_construct (void);

extern void
LLRP_EventsAndReports_destruct (
 LLRP_tSEventsAndReports * pThis);

extern void
LLRP_EventsAndReports_decodeFields (
 LLRP_tSEventsAndReports * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_EventsAndReports_assimilateSubParameters (
 LLRP_tSEventsAndReports * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_EventsAndReports_encode (
  const LLRP_tSEventsAndReports *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdEventsAndReports_HoldEventsAndReportsUponReconnect;

extern llrp_u1_t
LLRP_EventsAndReports_getHoldEventsAndReportsUponReconnect (
  LLRP_tSEventsAndReports *pThis);

extern LLRP_tResultCode
LLRP_EventsAndReports_setHoldEventsAndReportsUponReconnect (
  LLRP_tSEventsAndReports *pThis,
  llrp_u1_t Value);

  


  
struct LLRP_SROReportSpec
{
    LLRP_tSParameter hdr;
  
    LLRP_tEROReportTriggerType  eROReportTrigger;

    llrp_u16_t  N;

  
    LLRP_tSTagReportContentSelector * pTagReportContentSelector;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdROReportSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROReportSpec[];

extern LLRP_tSROReportSpec *
LLRP_ROReportSpec_construct (void);

extern void
LLRP_ROReportSpec_destruct (
 LLRP_tSROReportSpec * pThis);

extern void
LLRP_ROReportSpec_decodeFields (
 LLRP_tSROReportSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROReportSpec_assimilateSubParameters (
 LLRP_tSROReportSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROReportSpec_encode (
  const LLRP_tSROReportSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROReportSpec_ROReportTrigger;

extern LLRP_tEROReportTriggerType
LLRP_ROReportSpec_getROReportTrigger (
  LLRP_tSROReportSpec *pThis);

extern LLRP_tResultCode
LLRP_ROReportSpec_setROReportTrigger (
  LLRP_tSROReportSpec *pThis,
  LLRP_tEROReportTriggerType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROReportSpec_N;

extern llrp_u16_t
LLRP_ROReportSpec_getN (
  LLRP_tSROReportSpec *pThis);

extern LLRP_tResultCode
LLRP_ROReportSpec_setN (
  LLRP_tSROReportSpec *pThis,
  llrp_u16_t Value);

  
extern LLRP_tSTagReportContentSelector *
LLRP_ROReportSpec_getTagReportContentSelector (
  LLRP_tSROReportSpec *pThis);

extern LLRP_tResultCode
LLRP_ROReportSpec_setTagReportContentSelector (
  LLRP_tSROReportSpec *pThis,
  LLRP_tSTagReportContentSelector *pValue);

extern LLRP_tSParameter *
LLRP_ROReportSpec_beginCustom (
  LLRP_tSROReportSpec *pThis);

extern LLRP_tSParameter *
LLRP_ROReportSpec_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_ROReportSpec_clearCustom (
  LLRP_tSROReportSpec *pThis);

extern LLRP_tResultCode
LLRP_ROReportSpec_addCustom (
  LLRP_tSROReportSpec *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_STagReportContentSelector
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  EnableROSpecID;

    llrp_u1_t  EnableSpecIndex;

    llrp_u1_t  EnableInventoryParameterSpecID;

    llrp_u1_t  EnableAntennaID;

    llrp_u1_t  EnableChannelIndex;

    llrp_u1_t  EnablePeakRSSI;

    llrp_u1_t  EnableFirstSeenTimestamp;

    llrp_u1_t  EnableLastSeenTimestamp;

    llrp_u1_t  EnableTagSeenCount;

    llrp_u1_t  EnableAccessSpecID;

  
    LLRP_tSParameter * listAirProtocolEPCMemorySelector;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdTagReportContentSelector;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdTagReportContentSelector[];

extern LLRP_tSTagReportContentSelector *
LLRP_TagReportContentSelector_construct (void);

extern void
LLRP_TagReportContentSelector_destruct (
 LLRP_tSTagReportContentSelector * pThis);

extern void
LLRP_TagReportContentSelector_decodeFields (
 LLRP_tSTagReportContentSelector * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_TagReportContentSelector_assimilateSubParameters (
 LLRP_tSTagReportContentSelector * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_TagReportContentSelector_encode (
  const LLRP_tSTagReportContentSelector *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableROSpecID;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableROSpecID (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableROSpecID (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableSpecIndex;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableSpecIndex (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableSpecIndex (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableInventoryParameterSpecID;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableInventoryParameterSpecID (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableInventoryParameterSpecID (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableAntennaID;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableAntennaID (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableAntennaID (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableChannelIndex;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableChannelIndex (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableChannelIndex (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnablePeakRSSI;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnablePeakRSSI (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnablePeakRSSI (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableFirstSeenTimestamp;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableFirstSeenTimestamp (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableFirstSeenTimestamp (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableLastSeenTimestamp;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableLastSeenTimestamp (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableLastSeenTimestamp (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableTagSeenCount;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableTagSeenCount (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableTagSeenCount (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdTagReportContentSelector_EnableAccessSpecID;

extern llrp_u1_t
LLRP_TagReportContentSelector_getEnableAccessSpecID (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_setEnableAccessSpecID (
  LLRP_tSTagReportContentSelector *pThis,
  llrp_u1_t Value);

  
extern LLRP_tSParameter *
LLRP_TagReportContentSelector_beginAirProtocolEPCMemorySelector (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tSParameter *
LLRP_TagReportContentSelector_nextAirProtocolEPCMemorySelector (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_TagReportContentSelector_clearAirProtocolEPCMemorySelector (
  LLRP_tSTagReportContentSelector *pThis);

extern LLRP_tResultCode
LLRP_TagReportContentSelector_addAirProtocolEPCMemorySelector (
  LLRP_tSTagReportContentSelector *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SAccessReportSpec
{
    LLRP_tSParameter hdr;
  
    LLRP_tEAccessReportTriggerType  eAccessReportTrigger;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessReportSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAccessReportSpec[];

extern LLRP_tSAccessReportSpec *
LLRP_AccessReportSpec_construct (void);

extern void
LLRP_AccessReportSpec_destruct (
 LLRP_tSAccessReportSpec * pThis);

extern void
LLRP_AccessReportSpec_decodeFields (
 LLRP_tSAccessReportSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AccessReportSpec_assimilateSubParameters (
 LLRP_tSAccessReportSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AccessReportSpec_encode (
  const LLRP_tSAccessReportSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAccessReportSpec_AccessReportTrigger;

extern LLRP_tEAccessReportTriggerType
LLRP_AccessReportSpec_getAccessReportTrigger (
  LLRP_tSAccessReportSpec *pThis);

extern LLRP_tResultCode
LLRP_AccessReportSpec_setAccessReportTrigger (
  LLRP_tSAccessReportSpec *pThis,
  LLRP_tEAccessReportTriggerType Value);

  


  
struct LLRP_STagReportData
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSParameter * pEPCParameter;

    LLRP_tSROSpecID * pROSpecID;

    LLRP_tSSpecIndex * pSpecIndex;

    LLRP_tSInventoryParameterSpecID * pInventoryParameterSpecID;

    LLRP_tSAntennaID * pAntennaID;

    LLRP_tSPeakRSSI * pPeakRSSI;

    LLRP_tSChannelIndex * pChannelIndex;

    LLRP_tSFirstSeenTimestampUTC * pFirstSeenTimestampUTC;

    LLRP_tSFirstSeenTimestampUptime * pFirstSeenTimestampUptime;

    LLRP_tSLastSeenTimestampUTC * pLastSeenTimestampUTC;

    LLRP_tSLastSeenTimestampUptime * pLastSeenTimestampUptime;

    LLRP_tSTagSeenCount * pTagSeenCount;

    LLRP_tSParameter * listAirProtocolTagData;

    LLRP_tSAccessSpecID * pAccessSpecID;

    LLRP_tSParameter * listAccessCommandOpSpecResult;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdTagReportData;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdTagReportData[];

extern LLRP_tSTagReportData *
LLRP_TagReportData_construct (void);

extern void
LLRP_TagReportData_destruct (
 LLRP_tSTagReportData * pThis);

extern void
LLRP_TagReportData_decodeFields (
 LLRP_tSTagReportData * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_TagReportData_assimilateSubParameters (
 LLRP_tSTagReportData * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_TagReportData_encode (
  const LLRP_tSTagReportData *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSParameter *
LLRP_TagReportData_getEPCParameter (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setEPCParameter (
  LLRP_tSTagReportData *pThis,
  LLRP_tSParameter *pValue);

extern LLRP_tSROSpecID *
LLRP_TagReportData_getROSpecID (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setROSpecID (
  LLRP_tSTagReportData *pThis,
  LLRP_tSROSpecID *pValue);

extern LLRP_tSSpecIndex *
LLRP_TagReportData_getSpecIndex (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setSpecIndex (
  LLRP_tSTagReportData *pThis,
  LLRP_tSSpecIndex *pValue);

extern LLRP_tSInventoryParameterSpecID *
LLRP_TagReportData_getInventoryParameterSpecID (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setInventoryParameterSpecID (
  LLRP_tSTagReportData *pThis,
  LLRP_tSInventoryParameterSpecID *pValue);

extern LLRP_tSAntennaID *
LLRP_TagReportData_getAntennaID (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setAntennaID (
  LLRP_tSTagReportData *pThis,
  LLRP_tSAntennaID *pValue);

extern LLRP_tSPeakRSSI *
LLRP_TagReportData_getPeakRSSI (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setPeakRSSI (
  LLRP_tSTagReportData *pThis,
  LLRP_tSPeakRSSI *pValue);

extern LLRP_tSChannelIndex *
LLRP_TagReportData_getChannelIndex (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setChannelIndex (
  LLRP_tSTagReportData *pThis,
  LLRP_tSChannelIndex *pValue);

extern LLRP_tSFirstSeenTimestampUTC *
LLRP_TagReportData_getFirstSeenTimestampUTC (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setFirstSeenTimestampUTC (
  LLRP_tSTagReportData *pThis,
  LLRP_tSFirstSeenTimestampUTC *pValue);

extern LLRP_tSFirstSeenTimestampUptime *
LLRP_TagReportData_getFirstSeenTimestampUptime (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setFirstSeenTimestampUptime (
  LLRP_tSTagReportData *pThis,
  LLRP_tSFirstSeenTimestampUptime *pValue);

extern LLRP_tSLastSeenTimestampUTC *
LLRP_TagReportData_getLastSeenTimestampUTC (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setLastSeenTimestampUTC (
  LLRP_tSTagReportData *pThis,
  LLRP_tSLastSeenTimestampUTC *pValue);

extern LLRP_tSLastSeenTimestampUptime *
LLRP_TagReportData_getLastSeenTimestampUptime (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setLastSeenTimestampUptime (
  LLRP_tSTagReportData *pThis,
  LLRP_tSLastSeenTimestampUptime *pValue);

extern LLRP_tSTagSeenCount *
LLRP_TagReportData_getTagSeenCount (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setTagSeenCount (
  LLRP_tSTagReportData *pThis,
  LLRP_tSTagSeenCount *pValue);

extern LLRP_tSParameter *
LLRP_TagReportData_beginAirProtocolTagData (
  LLRP_tSTagReportData *pThis);

extern LLRP_tSParameter *
LLRP_TagReportData_nextAirProtocolTagData (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_TagReportData_clearAirProtocolTagData (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_addAirProtocolTagData (
  LLRP_tSTagReportData *pThis,
  LLRP_tSParameter *pValue);


extern LLRP_tSAccessSpecID *
LLRP_TagReportData_getAccessSpecID (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_setAccessSpecID (
  LLRP_tSTagReportData *pThis,
  LLRP_tSAccessSpecID *pValue);

extern LLRP_tSParameter *
LLRP_TagReportData_beginAccessCommandOpSpecResult (
  LLRP_tSTagReportData *pThis);

extern LLRP_tSParameter *
LLRP_TagReportData_nextAccessCommandOpSpecResult (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_TagReportData_clearAccessCommandOpSpecResult (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_addAccessCommandOpSpecResult (
  LLRP_tSTagReportData *pThis,
  LLRP_tSParameter *pValue);


extern LLRP_tSParameter *
LLRP_TagReportData_beginCustom (
  LLRP_tSTagReportData *pThis);

extern LLRP_tSParameter *
LLRP_TagReportData_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_TagReportData_clearCustom (
  LLRP_tSTagReportData *pThis);

extern LLRP_tResultCode
LLRP_TagReportData_addCustom (
  LLRP_tSTagReportData *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SEPCData
{
    LLRP_tSParameter hdr;
  
    llrp_u1v_t  EPC;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdEPCData;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdEPCData[];

extern LLRP_tSEPCData *
LLRP_EPCData_construct (void);

extern void
LLRP_EPCData_destruct (
 LLRP_tSEPCData * pThis);

extern void
LLRP_EPCData_decodeFields (
 LLRP_tSEPCData * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_EPCData_assimilateSubParameters (
 LLRP_tSEPCData * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_EPCData_encode (
  const LLRP_tSEPCData *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdEPCData_EPC;

extern llrp_u1v_t
LLRP_EPCData_getEPC (
  LLRP_tSEPCData *pThis);

extern LLRP_tResultCode
LLRP_EPCData_setEPC (
  LLRP_tSEPCData *pThis,
  llrp_u1v_t Value);

  


  
struct LLRP_SEPC_96
{
    LLRP_tSParameter hdr;
  
    llrp_u96_t  EPC;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdEPC_96;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdEPC_96[];

extern LLRP_tSEPC_96 *
LLRP_EPC_96_construct (void);

extern void
LLRP_EPC_96_destruct (
 LLRP_tSEPC_96 * pThis);

extern void
LLRP_EPC_96_decodeFields (
 LLRP_tSEPC_96 * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_EPC_96_assimilateSubParameters (
 LLRP_tSEPC_96 * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_EPC_96_encode (
  const LLRP_tSEPC_96 *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdEPC_96_EPC;

extern llrp_u96_t
LLRP_EPC_96_getEPC (
  LLRP_tSEPC_96 *pThis);

extern LLRP_tResultCode
LLRP_EPC_96_setEPC (
  LLRP_tSEPC_96 *pThis,
  llrp_u96_t Value);

  


  
struct LLRP_SROSpecID
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  ROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdROSpecID;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROSpecID[];

extern LLRP_tSROSpecID *
LLRP_ROSpecID_construct (void);

extern void
LLRP_ROSpecID_destruct (
 LLRP_tSROSpecID * pThis);

extern void
LLRP_ROSpecID_decodeFields (
 LLRP_tSROSpecID * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROSpecID_assimilateSubParameters (
 LLRP_tSROSpecID * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROSpecID_encode (
  const LLRP_tSROSpecID *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecID_ROSpecID;

extern llrp_u32_t
LLRP_ROSpecID_getROSpecID (
  LLRP_tSROSpecID *pThis);

extern LLRP_tResultCode
LLRP_ROSpecID_setROSpecID (
  LLRP_tSROSpecID *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SSpecIndex
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  SpecIndex;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdSpecIndex;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdSpecIndex[];

extern LLRP_tSSpecIndex *
LLRP_SpecIndex_construct (void);

extern void
LLRP_SpecIndex_destruct (
 LLRP_tSSpecIndex * pThis);

extern void
LLRP_SpecIndex_decodeFields (
 LLRP_tSSpecIndex * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_SpecIndex_assimilateSubParameters (
 LLRP_tSSpecIndex * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_SpecIndex_encode (
  const LLRP_tSSpecIndex *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdSpecIndex_SpecIndex;

extern llrp_u16_t
LLRP_SpecIndex_getSpecIndex (
  LLRP_tSSpecIndex *pThis);

extern LLRP_tResultCode
LLRP_SpecIndex_setSpecIndex (
  LLRP_tSSpecIndex *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SInventoryParameterSpecID
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  InventoryParameterSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdInventoryParameterSpecID;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdInventoryParameterSpecID[];

extern LLRP_tSInventoryParameterSpecID *
LLRP_InventoryParameterSpecID_construct (void);

extern void
LLRP_InventoryParameterSpecID_destruct (
 LLRP_tSInventoryParameterSpecID * pThis);

extern void
LLRP_InventoryParameterSpecID_decodeFields (
 LLRP_tSInventoryParameterSpecID * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_InventoryParameterSpecID_assimilateSubParameters (
 LLRP_tSInventoryParameterSpecID * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_InventoryParameterSpecID_encode (
  const LLRP_tSInventoryParameterSpecID *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdInventoryParameterSpecID_InventoryParameterSpecID;

extern llrp_u16_t
LLRP_InventoryParameterSpecID_getInventoryParameterSpecID (
  LLRP_tSInventoryParameterSpecID *pThis);

extern LLRP_tResultCode
LLRP_InventoryParameterSpecID_setInventoryParameterSpecID (
  LLRP_tSInventoryParameterSpecID *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SAntennaID
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  AntennaID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAntennaID;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAntennaID[];

extern LLRP_tSAntennaID *
LLRP_AntennaID_construct (void);

extern void
LLRP_AntennaID_destruct (
 LLRP_tSAntennaID * pThis);

extern void
LLRP_AntennaID_decodeFields (
 LLRP_tSAntennaID * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AntennaID_assimilateSubParameters (
 LLRP_tSAntennaID * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AntennaID_encode (
  const LLRP_tSAntennaID *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaID_AntennaID;

extern llrp_u16_t
LLRP_AntennaID_getAntennaID (
  LLRP_tSAntennaID *pThis);

extern LLRP_tResultCode
LLRP_AntennaID_setAntennaID (
  LLRP_tSAntennaID *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SPeakRSSI
{
    LLRP_tSParameter hdr;
  
    llrp_s8_t  PeakRSSI;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdPeakRSSI;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdPeakRSSI[];

extern LLRP_tSPeakRSSI *
LLRP_PeakRSSI_construct (void);

extern void
LLRP_PeakRSSI_destruct (
 LLRP_tSPeakRSSI * pThis);

extern void
LLRP_PeakRSSI_decodeFields (
 LLRP_tSPeakRSSI * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_PeakRSSI_assimilateSubParameters (
 LLRP_tSPeakRSSI * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_PeakRSSI_encode (
  const LLRP_tSPeakRSSI *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdPeakRSSI_PeakRSSI;

extern llrp_s8_t
LLRP_PeakRSSI_getPeakRSSI (
  LLRP_tSPeakRSSI *pThis);

extern LLRP_tResultCode
LLRP_PeakRSSI_setPeakRSSI (
  LLRP_tSPeakRSSI *pThis,
  llrp_s8_t Value);

  


  
struct LLRP_SChannelIndex
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  ChannelIndex;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdChannelIndex;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdChannelIndex[];

extern LLRP_tSChannelIndex *
LLRP_ChannelIndex_construct (void);

extern void
LLRP_ChannelIndex_destruct (
 LLRP_tSChannelIndex * pThis);

extern void
LLRP_ChannelIndex_decodeFields (
 LLRP_tSChannelIndex * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ChannelIndex_assimilateSubParameters (
 LLRP_tSChannelIndex * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ChannelIndex_encode (
  const LLRP_tSChannelIndex *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdChannelIndex_ChannelIndex;

extern llrp_u16_t
LLRP_ChannelIndex_getChannelIndex (
  LLRP_tSChannelIndex *pThis);

extern LLRP_tResultCode
LLRP_ChannelIndex_setChannelIndex (
  LLRP_tSChannelIndex *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SFirstSeenTimestampUTC
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdFirstSeenTimestampUTC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFirstSeenTimestampUTC[];

extern LLRP_tSFirstSeenTimestampUTC *
LLRP_FirstSeenTimestampUTC_construct (void);

extern void
LLRP_FirstSeenTimestampUTC_destruct (
 LLRP_tSFirstSeenTimestampUTC * pThis);

extern void
LLRP_FirstSeenTimestampUTC_decodeFields (
 LLRP_tSFirstSeenTimestampUTC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FirstSeenTimestampUTC_assimilateSubParameters (
 LLRP_tSFirstSeenTimestampUTC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FirstSeenTimestampUTC_encode (
  const LLRP_tSFirstSeenTimestampUTC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFirstSeenTimestampUTC_Microseconds;

extern llrp_u64_t
LLRP_FirstSeenTimestampUTC_getMicroseconds (
  LLRP_tSFirstSeenTimestampUTC *pThis);

extern LLRP_tResultCode
LLRP_FirstSeenTimestampUTC_setMicroseconds (
  LLRP_tSFirstSeenTimestampUTC *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_SFirstSeenTimestampUptime
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdFirstSeenTimestampUptime;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFirstSeenTimestampUptime[];

extern LLRP_tSFirstSeenTimestampUptime *
LLRP_FirstSeenTimestampUptime_construct (void);

extern void
LLRP_FirstSeenTimestampUptime_destruct (
 LLRP_tSFirstSeenTimestampUptime * pThis);

extern void
LLRP_FirstSeenTimestampUptime_decodeFields (
 LLRP_tSFirstSeenTimestampUptime * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FirstSeenTimestampUptime_assimilateSubParameters (
 LLRP_tSFirstSeenTimestampUptime * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FirstSeenTimestampUptime_encode (
  const LLRP_tSFirstSeenTimestampUptime *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFirstSeenTimestampUptime_Microseconds;

extern llrp_u64_t
LLRP_FirstSeenTimestampUptime_getMicroseconds (
  LLRP_tSFirstSeenTimestampUptime *pThis);

extern LLRP_tResultCode
LLRP_FirstSeenTimestampUptime_setMicroseconds (
  LLRP_tSFirstSeenTimestampUptime *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_SLastSeenTimestampUTC
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdLastSeenTimestampUTC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLastSeenTimestampUTC[];

extern LLRP_tSLastSeenTimestampUTC *
LLRP_LastSeenTimestampUTC_construct (void);

extern void
LLRP_LastSeenTimestampUTC_destruct (
 LLRP_tSLastSeenTimestampUTC * pThis);

extern void
LLRP_LastSeenTimestampUTC_decodeFields (
 LLRP_tSLastSeenTimestampUTC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LastSeenTimestampUTC_assimilateSubParameters (
 LLRP_tSLastSeenTimestampUTC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LastSeenTimestampUTC_encode (
  const LLRP_tSLastSeenTimestampUTC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLastSeenTimestampUTC_Microseconds;

extern llrp_u64_t
LLRP_LastSeenTimestampUTC_getMicroseconds (
  LLRP_tSLastSeenTimestampUTC *pThis);

extern LLRP_tResultCode
LLRP_LastSeenTimestampUTC_setMicroseconds (
  LLRP_tSLastSeenTimestampUTC *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_SLastSeenTimestampUptime
{
    LLRP_tSParameter hdr;
  
    llrp_u64_t  Microseconds;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdLastSeenTimestampUptime;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLastSeenTimestampUptime[];

extern LLRP_tSLastSeenTimestampUptime *
LLRP_LastSeenTimestampUptime_construct (void);

extern void
LLRP_LastSeenTimestampUptime_destruct (
 LLRP_tSLastSeenTimestampUptime * pThis);

extern void
LLRP_LastSeenTimestampUptime_decodeFields (
 LLRP_tSLastSeenTimestampUptime * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LastSeenTimestampUptime_assimilateSubParameters (
 LLRP_tSLastSeenTimestampUptime * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LastSeenTimestampUptime_encode (
  const LLRP_tSLastSeenTimestampUptime *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLastSeenTimestampUptime_Microseconds;

extern llrp_u64_t
LLRP_LastSeenTimestampUptime_getMicroseconds (
  LLRP_tSLastSeenTimestampUptime *pThis);

extern LLRP_tResultCode
LLRP_LastSeenTimestampUptime_setMicroseconds (
  LLRP_tSLastSeenTimestampUptime *pThis,
  llrp_u64_t Value);

  


  
struct LLRP_STagSeenCount
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  TagCount;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdTagSeenCount;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdTagSeenCount[];

extern LLRP_tSTagSeenCount *
LLRP_TagSeenCount_construct (void);

extern void
LLRP_TagSeenCount_destruct (
 LLRP_tSTagSeenCount * pThis);

extern void
LLRP_TagSeenCount_decodeFields (
 LLRP_tSTagSeenCount * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_TagSeenCount_assimilateSubParameters (
 LLRP_tSTagSeenCount * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_TagSeenCount_encode (
  const LLRP_tSTagSeenCount *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdTagSeenCount_TagCount;

extern llrp_u16_t
LLRP_TagSeenCount_getTagCount (
  LLRP_tSTagSeenCount *pThis);

extern LLRP_tResultCode
LLRP_TagSeenCount_setTagCount (
  LLRP_tSTagSeenCount *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SAccessSpecID
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  AccessSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessSpecID;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAccessSpecID[];

extern LLRP_tSAccessSpecID *
LLRP_AccessSpecID_construct (void);

extern void
LLRP_AccessSpecID_destruct (
 LLRP_tSAccessSpecID * pThis);

extern void
LLRP_AccessSpecID_decodeFields (
 LLRP_tSAccessSpecID * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AccessSpecID_assimilateSubParameters (
 LLRP_tSAccessSpecID * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AccessSpecID_encode (
  const LLRP_tSAccessSpecID *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAccessSpecID_AccessSpecID;

extern llrp_u32_t
LLRP_AccessSpecID_getAccessSpecID (
  LLRP_tSAccessSpecID *pThis);

extern LLRP_tResultCode
LLRP_AccessSpecID_setAccessSpecID (
  LLRP_tSAccessSpecID *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SRFSurveyReportData
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSROSpecID * pROSpecID;

    LLRP_tSSpecIndex * pSpecIndex;

    LLRP_tSFrequencyRSSILevelEntry * listFrequencyRSSILevelEntry;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFSurveyReportData;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFSurveyReportData[];

extern LLRP_tSRFSurveyReportData *
LLRP_RFSurveyReportData_construct (void);

extern void
LLRP_RFSurveyReportData_destruct (
 LLRP_tSRFSurveyReportData * pThis);

extern void
LLRP_RFSurveyReportData_decodeFields (
 LLRP_tSRFSurveyReportData * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFSurveyReportData_assimilateSubParameters (
 LLRP_tSRFSurveyReportData * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFSurveyReportData_encode (
  const LLRP_tSRFSurveyReportData *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSROSpecID *
LLRP_RFSurveyReportData_getROSpecID (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyReportData_setROSpecID (
  LLRP_tSRFSurveyReportData *pThis,
  LLRP_tSROSpecID *pValue);

extern LLRP_tSSpecIndex *
LLRP_RFSurveyReportData_getSpecIndex (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyReportData_setSpecIndex (
  LLRP_tSRFSurveyReportData *pThis,
  LLRP_tSSpecIndex *pValue);

extern LLRP_tSFrequencyRSSILevelEntry *
LLRP_RFSurveyReportData_beginFrequencyRSSILevelEntry (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tSFrequencyRSSILevelEntry *
LLRP_RFSurveyReportData_nextFrequencyRSSILevelEntry (
  LLRP_tSFrequencyRSSILevelEntry *pCurrent);

extern void
LLRP_RFSurveyReportData_clearFrequencyRSSILevelEntry (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyReportData_addFrequencyRSSILevelEntry (
  LLRP_tSRFSurveyReportData *pThis,
  LLRP_tSFrequencyRSSILevelEntry *pValue);


extern LLRP_tSParameter *
LLRP_RFSurveyReportData_beginCustom (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tSParameter *
LLRP_RFSurveyReportData_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_RFSurveyReportData_clearCustom (
  LLRP_tSRFSurveyReportData *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyReportData_addCustom (
  LLRP_tSRFSurveyReportData *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SFrequencyRSSILevelEntry
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  Frequency;

    llrp_u32_t  Bandwidth;

    llrp_s8_t  AverageRSSI;

    llrp_s8_t  PeakRSSI;

  
    LLRP_tSParameter * pTimestamp;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdFrequencyRSSILevelEntry;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFrequencyRSSILevelEntry[];

extern LLRP_tSFrequencyRSSILevelEntry *
LLRP_FrequencyRSSILevelEntry_construct (void);

extern void
LLRP_FrequencyRSSILevelEntry_destruct (
 LLRP_tSFrequencyRSSILevelEntry * pThis);

extern void
LLRP_FrequencyRSSILevelEntry_decodeFields (
 LLRP_tSFrequencyRSSILevelEntry * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FrequencyRSSILevelEntry_assimilateSubParameters (
 LLRP_tSFrequencyRSSILevelEntry * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FrequencyRSSILevelEntry_encode (
  const LLRP_tSFrequencyRSSILevelEntry *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyRSSILevelEntry_Frequency;

extern llrp_u32_t
LLRP_FrequencyRSSILevelEntry_getFrequency (
  LLRP_tSFrequencyRSSILevelEntry *pThis);

extern LLRP_tResultCode
LLRP_FrequencyRSSILevelEntry_setFrequency (
  LLRP_tSFrequencyRSSILevelEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyRSSILevelEntry_Bandwidth;

extern llrp_u32_t
LLRP_FrequencyRSSILevelEntry_getBandwidth (
  LLRP_tSFrequencyRSSILevelEntry *pThis);

extern LLRP_tResultCode
LLRP_FrequencyRSSILevelEntry_setBandwidth (
  LLRP_tSFrequencyRSSILevelEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyRSSILevelEntry_AverageRSSI;

extern llrp_s8_t
LLRP_FrequencyRSSILevelEntry_getAverageRSSI (
  LLRP_tSFrequencyRSSILevelEntry *pThis);

extern LLRP_tResultCode
LLRP_FrequencyRSSILevelEntry_setAverageRSSI (
  LLRP_tSFrequencyRSSILevelEntry *pThis,
  llrp_s8_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdFrequencyRSSILevelEntry_PeakRSSI;

extern llrp_s8_t
LLRP_FrequencyRSSILevelEntry_getPeakRSSI (
  LLRP_tSFrequencyRSSILevelEntry *pThis);

extern LLRP_tResultCode
LLRP_FrequencyRSSILevelEntry_setPeakRSSI (
  LLRP_tSFrequencyRSSILevelEntry *pThis,
  llrp_s8_t Value);

  
extern LLRP_tSParameter *
LLRP_FrequencyRSSILevelEntry_getTimestamp (
  LLRP_tSFrequencyRSSILevelEntry *pThis);

extern LLRP_tResultCode
LLRP_FrequencyRSSILevelEntry_setTimestamp (
  LLRP_tSFrequencyRSSILevelEntry *pThis,
  LLRP_tSParameter *pValue);



  
struct LLRP_SReaderEventNotificationSpec
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSEventNotificationState * listEventNotificationState;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdReaderEventNotificationSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReaderEventNotificationSpec[];

extern LLRP_tSReaderEventNotificationSpec *
LLRP_ReaderEventNotificationSpec_construct (void);

extern void
LLRP_ReaderEventNotificationSpec_destruct (
 LLRP_tSReaderEventNotificationSpec * pThis);

extern void
LLRP_ReaderEventNotificationSpec_decodeFields (
 LLRP_tSReaderEventNotificationSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReaderEventNotificationSpec_assimilateSubParameters (
 LLRP_tSReaderEventNotificationSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReaderEventNotificationSpec_encode (
  const LLRP_tSReaderEventNotificationSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSEventNotificationState *
LLRP_ReaderEventNotificationSpec_beginEventNotificationState (
  LLRP_tSReaderEventNotificationSpec *pThis);

extern LLRP_tSEventNotificationState *
LLRP_ReaderEventNotificationSpec_nextEventNotificationState (
  LLRP_tSEventNotificationState *pCurrent);

extern void
LLRP_ReaderEventNotificationSpec_clearEventNotificationState (
  LLRP_tSReaderEventNotificationSpec *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationSpec_addEventNotificationState (
  LLRP_tSReaderEventNotificationSpec *pThis,
  LLRP_tSEventNotificationState *pValue);




  
struct LLRP_SEventNotificationState
{
    LLRP_tSParameter hdr;
  
    LLRP_tENotificationEventType  eEventType;

    llrp_u1_t  NotificationState;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdEventNotificationState;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdEventNotificationState[];

extern LLRP_tSEventNotificationState *
LLRP_EventNotificationState_construct (void);

extern void
LLRP_EventNotificationState_destruct (
 LLRP_tSEventNotificationState * pThis);

extern void
LLRP_EventNotificationState_decodeFields (
 LLRP_tSEventNotificationState * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_EventNotificationState_assimilateSubParameters (
 LLRP_tSEventNotificationState * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_EventNotificationState_encode (
  const LLRP_tSEventNotificationState *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdEventNotificationState_EventType;

extern LLRP_tENotificationEventType
LLRP_EventNotificationState_getEventType (
  LLRP_tSEventNotificationState *pThis);

extern LLRP_tResultCode
LLRP_EventNotificationState_setEventType (
  LLRP_tSEventNotificationState *pThis,
  LLRP_tENotificationEventType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdEventNotificationState_NotificationState;

extern llrp_u1_t
LLRP_EventNotificationState_getNotificationState (
  LLRP_tSEventNotificationState *pThis);

extern LLRP_tResultCode
LLRP_EventNotificationState_setNotificationState (
  LLRP_tSEventNotificationState *pThis,
  llrp_u1_t Value);

  


  
struct LLRP_SReaderEventNotificationData
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSParameter * pTimestamp;

    LLRP_tSHoppingEvent * pHoppingEvent;

    LLRP_tSGPIEvent * pGPIEvent;

    LLRP_tSROSpecEvent * pROSpecEvent;

    LLRP_tSReportBufferLevelWarningEvent * pReportBufferLevelWarningEvent;

    LLRP_tSReportBufferOverflowErrorEvent * pReportBufferOverflowErrorEvent;

    LLRP_tSReaderExceptionEvent * pReaderExceptionEvent;

    LLRP_tSRFSurveyEvent * pRFSurveyEvent;

    LLRP_tSAISpecEvent * pAISpecEvent;

    LLRP_tSAntennaEvent * pAntennaEvent;

    LLRP_tSConnectionAttemptEvent * pConnectionAttemptEvent;

    LLRP_tSConnectionCloseEvent * pConnectionCloseEvent;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdReaderEventNotificationData;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReaderEventNotificationData[];

extern LLRP_tSReaderEventNotificationData *
LLRP_ReaderEventNotificationData_construct (void);

extern void
LLRP_ReaderEventNotificationData_destruct (
 LLRP_tSReaderEventNotificationData * pThis);

extern void
LLRP_ReaderEventNotificationData_decodeFields (
 LLRP_tSReaderEventNotificationData * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReaderEventNotificationData_assimilateSubParameters (
 LLRP_tSReaderEventNotificationData * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReaderEventNotificationData_encode (
  const LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSParameter *
LLRP_ReaderEventNotificationData_getTimestamp (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setTimestamp (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSParameter *pValue);

extern LLRP_tSHoppingEvent *
LLRP_ReaderEventNotificationData_getHoppingEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setHoppingEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSHoppingEvent *pValue);

extern LLRP_tSGPIEvent *
LLRP_ReaderEventNotificationData_getGPIEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setGPIEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSGPIEvent *pValue);

extern LLRP_tSROSpecEvent *
LLRP_ReaderEventNotificationData_getROSpecEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setROSpecEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSROSpecEvent *pValue);

extern LLRP_tSReportBufferLevelWarningEvent *
LLRP_ReaderEventNotificationData_getReportBufferLevelWarningEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setReportBufferLevelWarningEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSReportBufferLevelWarningEvent *pValue);

extern LLRP_tSReportBufferOverflowErrorEvent *
LLRP_ReaderEventNotificationData_getReportBufferOverflowErrorEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setReportBufferOverflowErrorEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSReportBufferOverflowErrorEvent *pValue);

extern LLRP_tSReaderExceptionEvent *
LLRP_ReaderEventNotificationData_getReaderExceptionEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setReaderExceptionEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSReaderExceptionEvent *pValue);

extern LLRP_tSRFSurveyEvent *
LLRP_ReaderEventNotificationData_getRFSurveyEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setRFSurveyEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSRFSurveyEvent *pValue);

extern LLRP_tSAISpecEvent *
LLRP_ReaderEventNotificationData_getAISpecEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setAISpecEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSAISpecEvent *pValue);

extern LLRP_tSAntennaEvent *
LLRP_ReaderEventNotificationData_getAntennaEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setAntennaEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSAntennaEvent *pValue);

extern LLRP_tSConnectionAttemptEvent *
LLRP_ReaderEventNotificationData_getConnectionAttemptEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setConnectionAttemptEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSConnectionAttemptEvent *pValue);

extern LLRP_tSConnectionCloseEvent *
LLRP_ReaderEventNotificationData_getConnectionCloseEvent (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_setConnectionCloseEvent (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSConnectionCloseEvent *pValue);

extern LLRP_tSParameter *
LLRP_ReaderEventNotificationData_beginCustom (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tSParameter *
LLRP_ReaderEventNotificationData_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_ReaderEventNotificationData_clearCustom (
  LLRP_tSReaderEventNotificationData *pThis);

extern LLRP_tResultCode
LLRP_ReaderEventNotificationData_addCustom (
  LLRP_tSReaderEventNotificationData *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SHoppingEvent
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  HopTableID;

    llrp_u16_t  NextChannelIndex;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdHoppingEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdHoppingEvent[];

extern LLRP_tSHoppingEvent *
LLRP_HoppingEvent_construct (void);

extern void
LLRP_HoppingEvent_destruct (
 LLRP_tSHoppingEvent * pThis);

extern void
LLRP_HoppingEvent_decodeFields (
 LLRP_tSHoppingEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_HoppingEvent_assimilateSubParameters (
 LLRP_tSHoppingEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_HoppingEvent_encode (
  const LLRP_tSHoppingEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdHoppingEvent_HopTableID;

extern llrp_u16_t
LLRP_HoppingEvent_getHopTableID (
  LLRP_tSHoppingEvent *pThis);

extern LLRP_tResultCode
LLRP_HoppingEvent_setHopTableID (
  LLRP_tSHoppingEvent *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdHoppingEvent_NextChannelIndex;

extern llrp_u16_t
LLRP_HoppingEvent_getNextChannelIndex (
  LLRP_tSHoppingEvent *pThis);

extern LLRP_tResultCode
LLRP_HoppingEvent_setNextChannelIndex (
  LLRP_tSHoppingEvent *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SGPIEvent
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  GPIPortNumber;

    llrp_u1_t  GPIEvent;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdGPIEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdGPIEvent[];

extern LLRP_tSGPIEvent *
LLRP_GPIEvent_construct (void);

extern void
LLRP_GPIEvent_destruct (
 LLRP_tSGPIEvent * pThis);

extern void
LLRP_GPIEvent_decodeFields (
 LLRP_tSGPIEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_GPIEvent_assimilateSubParameters (
 LLRP_tSGPIEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_GPIEvent_encode (
  const LLRP_tSGPIEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdGPIEvent_GPIPortNumber;

extern llrp_u16_t
LLRP_GPIEvent_getGPIPortNumber (
  LLRP_tSGPIEvent *pThis);

extern LLRP_tResultCode
LLRP_GPIEvent_setGPIPortNumber (
  LLRP_tSGPIEvent *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdGPIEvent_GPIEvent;

extern llrp_u1_t
LLRP_GPIEvent_getGPIEvent (
  LLRP_tSGPIEvent *pThis);

extern LLRP_tResultCode
LLRP_GPIEvent_setGPIEvent (
  LLRP_tSGPIEvent *pThis,
  llrp_u1_t Value);

  


  
struct LLRP_SROSpecEvent
{
    LLRP_tSParameter hdr;
  
    LLRP_tEROSpecEventType  eEventType;

    llrp_u32_t  ROSpecID;

    llrp_u32_t  PreemptingROSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdROSpecEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdROSpecEvent[];

extern LLRP_tSROSpecEvent *
LLRP_ROSpecEvent_construct (void);

extern void
LLRP_ROSpecEvent_destruct (
 LLRP_tSROSpecEvent * pThis);

extern void
LLRP_ROSpecEvent_decodeFields (
 LLRP_tSROSpecEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ROSpecEvent_assimilateSubParameters (
 LLRP_tSROSpecEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ROSpecEvent_encode (
  const LLRP_tSROSpecEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecEvent_EventType;

extern LLRP_tEROSpecEventType
LLRP_ROSpecEvent_getEventType (
  LLRP_tSROSpecEvent *pThis);

extern LLRP_tResultCode
LLRP_ROSpecEvent_setEventType (
  LLRP_tSROSpecEvent *pThis,
  LLRP_tEROSpecEventType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecEvent_ROSpecID;

extern llrp_u32_t
LLRP_ROSpecEvent_getROSpecID (
  LLRP_tSROSpecEvent *pThis);

extern LLRP_tResultCode
LLRP_ROSpecEvent_setROSpecID (
  LLRP_tSROSpecEvent *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdROSpecEvent_PreemptingROSpecID;

extern llrp_u32_t
LLRP_ROSpecEvent_getPreemptingROSpecID (
  LLRP_tSROSpecEvent *pThis);

extern LLRP_tResultCode
LLRP_ROSpecEvent_setPreemptingROSpecID (
  LLRP_tSROSpecEvent *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SReportBufferLevelWarningEvent
{
    LLRP_tSParameter hdr;
  
    llrp_u8_t  ReportBufferPercentageFull;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdReportBufferLevelWarningEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReportBufferLevelWarningEvent[];

extern LLRP_tSReportBufferLevelWarningEvent *
LLRP_ReportBufferLevelWarningEvent_construct (void);

extern void
LLRP_ReportBufferLevelWarningEvent_destruct (
 LLRP_tSReportBufferLevelWarningEvent * pThis);

extern void
LLRP_ReportBufferLevelWarningEvent_decodeFields (
 LLRP_tSReportBufferLevelWarningEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReportBufferLevelWarningEvent_assimilateSubParameters (
 LLRP_tSReportBufferLevelWarningEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReportBufferLevelWarningEvent_encode (
  const LLRP_tSReportBufferLevelWarningEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdReportBufferLevelWarningEvent_ReportBufferPercentageFull;

extern llrp_u8_t
LLRP_ReportBufferLevelWarningEvent_getReportBufferPercentageFull (
  LLRP_tSReportBufferLevelWarningEvent *pThis);

extern LLRP_tResultCode
LLRP_ReportBufferLevelWarningEvent_setReportBufferPercentageFull (
  LLRP_tSReportBufferLevelWarningEvent *pThis,
  llrp_u8_t Value);

  


  
struct LLRP_SReportBufferOverflowErrorEvent
{
    LLRP_tSParameter hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdReportBufferOverflowErrorEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReportBufferOverflowErrorEvent[];

extern LLRP_tSReportBufferOverflowErrorEvent *
LLRP_ReportBufferOverflowErrorEvent_construct (void);

extern void
LLRP_ReportBufferOverflowErrorEvent_destruct (
 LLRP_tSReportBufferOverflowErrorEvent * pThis);

extern void
LLRP_ReportBufferOverflowErrorEvent_decodeFields (
 LLRP_tSReportBufferOverflowErrorEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReportBufferOverflowErrorEvent_assimilateSubParameters (
 LLRP_tSReportBufferOverflowErrorEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReportBufferOverflowErrorEvent_encode (
  const LLRP_tSReportBufferOverflowErrorEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SReaderExceptionEvent
{
    LLRP_tSParameter hdr;
  
    llrp_utf8v_t  Message;

  
    LLRP_tSROSpecID * pROSpecID;

    LLRP_tSSpecIndex * pSpecIndex;

    LLRP_tSInventoryParameterSpecID * pInventoryParameterSpecID;

    LLRP_tSAntennaID * pAntennaID;

    LLRP_tSAccessSpecID * pAccessSpecID;

    LLRP_tSOpSpecID * pOpSpecID;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdReaderExceptionEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdReaderExceptionEvent[];

extern LLRP_tSReaderExceptionEvent *
LLRP_ReaderExceptionEvent_construct (void);

extern void
LLRP_ReaderExceptionEvent_destruct (
 LLRP_tSReaderExceptionEvent * pThis);

extern void
LLRP_ReaderExceptionEvent_decodeFields (
 LLRP_tSReaderExceptionEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ReaderExceptionEvent_assimilateSubParameters (
 LLRP_tSReaderExceptionEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ReaderExceptionEvent_encode (
  const LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdReaderExceptionEvent_Message;

extern llrp_utf8v_t
LLRP_ReaderExceptionEvent_getMessage (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setMessage (
  LLRP_tSReaderExceptionEvent *pThis,
  llrp_utf8v_t Value);

  
extern LLRP_tSROSpecID *
LLRP_ReaderExceptionEvent_getROSpecID (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setROSpecID (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSROSpecID *pValue);

extern LLRP_tSSpecIndex *
LLRP_ReaderExceptionEvent_getSpecIndex (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setSpecIndex (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSSpecIndex *pValue);

extern LLRP_tSInventoryParameterSpecID *
LLRP_ReaderExceptionEvent_getInventoryParameterSpecID (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setInventoryParameterSpecID (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSInventoryParameterSpecID *pValue);

extern LLRP_tSAntennaID *
LLRP_ReaderExceptionEvent_getAntennaID (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setAntennaID (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSAntennaID *pValue);

extern LLRP_tSAccessSpecID *
LLRP_ReaderExceptionEvent_getAccessSpecID (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setAccessSpecID (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSAccessSpecID *pValue);

extern LLRP_tSOpSpecID *
LLRP_ReaderExceptionEvent_getOpSpecID (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_setOpSpecID (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSOpSpecID *pValue);

extern LLRP_tSParameter *
LLRP_ReaderExceptionEvent_beginCustom (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tSParameter *
LLRP_ReaderExceptionEvent_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_ReaderExceptionEvent_clearCustom (
  LLRP_tSReaderExceptionEvent *pThis);

extern LLRP_tResultCode
LLRP_ReaderExceptionEvent_addCustom (
  LLRP_tSReaderExceptionEvent *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SOpSpecID
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdOpSpecID;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdOpSpecID[];

extern LLRP_tSOpSpecID *
LLRP_OpSpecID_construct (void);

extern void
LLRP_OpSpecID_destruct (
 LLRP_tSOpSpecID * pThis);

extern void
LLRP_OpSpecID_decodeFields (
 LLRP_tSOpSpecID * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_OpSpecID_assimilateSubParameters (
 LLRP_tSOpSpecID * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_OpSpecID_encode (
  const LLRP_tSOpSpecID *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdOpSpecID_OpSpecID;

extern llrp_u16_t
LLRP_OpSpecID_getOpSpecID (
  LLRP_tSOpSpecID *pThis);

extern LLRP_tResultCode
LLRP_OpSpecID_setOpSpecID (
  LLRP_tSOpSpecID *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SRFSurveyEvent
{
    LLRP_tSParameter hdr;
  
    LLRP_tERFSurveyEventType  eEventType;

    llrp_u32_t  ROSpecID;

    llrp_u16_t  SpecIndex;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdRFSurveyEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdRFSurveyEvent[];

extern LLRP_tSRFSurveyEvent *
LLRP_RFSurveyEvent_construct (void);

extern void
LLRP_RFSurveyEvent_destruct (
 LLRP_tSRFSurveyEvent * pThis);

extern void
LLRP_RFSurveyEvent_decodeFields (
 LLRP_tSRFSurveyEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_RFSurveyEvent_assimilateSubParameters (
 LLRP_tSRFSurveyEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_RFSurveyEvent_encode (
  const LLRP_tSRFSurveyEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveyEvent_EventType;

extern LLRP_tERFSurveyEventType
LLRP_RFSurveyEvent_getEventType (
  LLRP_tSRFSurveyEvent *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyEvent_setEventType (
  LLRP_tSRFSurveyEvent *pThis,
  LLRP_tERFSurveyEventType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveyEvent_ROSpecID;

extern llrp_u32_t
LLRP_RFSurveyEvent_getROSpecID (
  LLRP_tSRFSurveyEvent *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyEvent_setROSpecID (
  LLRP_tSRFSurveyEvent *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdRFSurveyEvent_SpecIndex;

extern llrp_u16_t
LLRP_RFSurveyEvent_getSpecIndex (
  LLRP_tSRFSurveyEvent *pThis);

extern LLRP_tResultCode
LLRP_RFSurveyEvent_setSpecIndex (
  LLRP_tSRFSurveyEvent *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SAISpecEvent
{
    LLRP_tSParameter hdr;
  
    LLRP_tEAISpecEventType  eEventType;

    llrp_u32_t  ROSpecID;

    llrp_u16_t  SpecIndex;

  
    LLRP_tSParameter * pAirProtocolSingulationDetails;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdAISpecEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAISpecEvent[];

extern LLRP_tSAISpecEvent *
LLRP_AISpecEvent_construct (void);

extern void
LLRP_AISpecEvent_destruct (
 LLRP_tSAISpecEvent * pThis);

extern void
LLRP_AISpecEvent_decodeFields (
 LLRP_tSAISpecEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AISpecEvent_assimilateSubParameters (
 LLRP_tSAISpecEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AISpecEvent_encode (
  const LLRP_tSAISpecEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAISpecEvent_EventType;

extern LLRP_tEAISpecEventType
LLRP_AISpecEvent_getEventType (
  LLRP_tSAISpecEvent *pThis);

extern LLRP_tResultCode
LLRP_AISpecEvent_setEventType (
  LLRP_tSAISpecEvent *pThis,
  LLRP_tEAISpecEventType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAISpecEvent_ROSpecID;

extern llrp_u32_t
LLRP_AISpecEvent_getROSpecID (
  LLRP_tSAISpecEvent *pThis);

extern LLRP_tResultCode
LLRP_AISpecEvent_setROSpecID (
  LLRP_tSAISpecEvent *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAISpecEvent_SpecIndex;

extern llrp_u16_t
LLRP_AISpecEvent_getSpecIndex (
  LLRP_tSAISpecEvent *pThis);

extern LLRP_tResultCode
LLRP_AISpecEvent_setSpecIndex (
  LLRP_tSAISpecEvent *pThis,
  llrp_u16_t Value);

  
extern LLRP_tSParameter *
LLRP_AISpecEvent_getAirProtocolSingulationDetails (
  LLRP_tSAISpecEvent *pThis);

extern LLRP_tResultCode
LLRP_AISpecEvent_setAirProtocolSingulationDetails (
  LLRP_tSAISpecEvent *pThis,
  LLRP_tSParameter *pValue);



  
struct LLRP_SAntennaEvent
{
    LLRP_tSParameter hdr;
  
    LLRP_tEAntennaEventType  eEventType;

    llrp_u16_t  AntennaID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdAntennaEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdAntennaEvent[];

extern LLRP_tSAntennaEvent *
LLRP_AntennaEvent_construct (void);

extern void
LLRP_AntennaEvent_destruct (
 LLRP_tSAntennaEvent * pThis);

extern void
LLRP_AntennaEvent_decodeFields (
 LLRP_tSAntennaEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_AntennaEvent_assimilateSubParameters (
 LLRP_tSAntennaEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_AntennaEvent_encode (
  const LLRP_tSAntennaEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaEvent_EventType;

extern LLRP_tEAntennaEventType
LLRP_AntennaEvent_getEventType (
  LLRP_tSAntennaEvent *pThis);

extern LLRP_tResultCode
LLRP_AntennaEvent_setEventType (
  LLRP_tSAntennaEvent *pThis,
  LLRP_tEAntennaEventType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdAntennaEvent_AntennaID;

extern llrp_u16_t
LLRP_AntennaEvent_getAntennaID (
  LLRP_tSAntennaEvent *pThis);

extern LLRP_tResultCode
LLRP_AntennaEvent_setAntennaID (
  LLRP_tSAntennaEvent *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SConnectionAttemptEvent
{
    LLRP_tSParameter hdr;
  
    LLRP_tEConnectionAttemptStatusType  eStatus;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdConnectionAttemptEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdConnectionAttemptEvent[];

extern LLRP_tSConnectionAttemptEvent *
LLRP_ConnectionAttemptEvent_construct (void);

extern void
LLRP_ConnectionAttemptEvent_destruct (
 LLRP_tSConnectionAttemptEvent * pThis);

extern void
LLRP_ConnectionAttemptEvent_decodeFields (
 LLRP_tSConnectionAttemptEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ConnectionAttemptEvent_assimilateSubParameters (
 LLRP_tSConnectionAttemptEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ConnectionAttemptEvent_encode (
  const LLRP_tSConnectionAttemptEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdConnectionAttemptEvent_Status;

extern LLRP_tEConnectionAttemptStatusType
LLRP_ConnectionAttemptEvent_getStatus (
  LLRP_tSConnectionAttemptEvent *pThis);

extern LLRP_tResultCode
LLRP_ConnectionAttemptEvent_setStatus (
  LLRP_tSConnectionAttemptEvent *pThis,
  LLRP_tEConnectionAttemptStatusType Value);

  


  
struct LLRP_SConnectionCloseEvent
{
    LLRP_tSParameter hdr;
  
  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdConnectionCloseEvent;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdConnectionCloseEvent[];

extern LLRP_tSConnectionCloseEvent *
LLRP_ConnectionCloseEvent_construct (void);

extern void
LLRP_ConnectionCloseEvent_destruct (
 LLRP_tSConnectionCloseEvent * pThis);

extern void
LLRP_ConnectionCloseEvent_decodeFields (
 LLRP_tSConnectionCloseEvent * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ConnectionCloseEvent_assimilateSubParameters (
 LLRP_tSConnectionCloseEvent * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ConnectionCloseEvent_encode (
  const LLRP_tSConnectionCloseEvent *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  


  
struct LLRP_SLLRPStatus
{
    LLRP_tSParameter hdr;
  
    LLRP_tEStatusCode  eStatusCode;

    llrp_utf8v_t  ErrorDescription;

  
    LLRP_tSFieldError * pFieldError;

    LLRP_tSParameterError * pParameterError;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdLLRPStatus;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdLLRPStatus[];

extern LLRP_tSLLRPStatus *
LLRP_LLRPStatus_construct (void);

extern void
LLRP_LLRPStatus_destruct (
 LLRP_tSLLRPStatus * pThis);

extern void
LLRP_LLRPStatus_decodeFields (
 LLRP_tSLLRPStatus * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_LLRPStatus_assimilateSubParameters (
 LLRP_tSLLRPStatus * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_LLRPStatus_encode (
  const LLRP_tSLLRPStatus *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPStatus_StatusCode;

extern LLRP_tEStatusCode
LLRP_LLRPStatus_getStatusCode (
  LLRP_tSLLRPStatus *pThis);

extern LLRP_tResultCode
LLRP_LLRPStatus_setStatusCode (
  LLRP_tSLLRPStatus *pThis,
  LLRP_tEStatusCode Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdLLRPStatus_ErrorDescription;

extern llrp_utf8v_t
LLRP_LLRPStatus_getErrorDescription (
  LLRP_tSLLRPStatus *pThis);

extern LLRP_tResultCode
LLRP_LLRPStatus_setErrorDescription (
  LLRP_tSLLRPStatus *pThis,
  llrp_utf8v_t Value);

  
extern LLRP_tSFieldError *
LLRP_LLRPStatus_getFieldError (
  LLRP_tSLLRPStatus *pThis);

extern LLRP_tResultCode
LLRP_LLRPStatus_setFieldError (
  LLRP_tSLLRPStatus *pThis,
  LLRP_tSFieldError *pValue);

extern LLRP_tSParameterError *
LLRP_LLRPStatus_getParameterError (
  LLRP_tSLLRPStatus *pThis);

extern LLRP_tResultCode
LLRP_LLRPStatus_setParameterError (
  LLRP_tSLLRPStatus *pThis,
  LLRP_tSParameterError *pValue);



  
struct LLRP_SFieldError
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  FieldNum;

    LLRP_tEStatusCode  eErrorCode;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdFieldError;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdFieldError[];

extern LLRP_tSFieldError *
LLRP_FieldError_construct (void);

extern void
LLRP_FieldError_destruct (
 LLRP_tSFieldError * pThis);

extern void
LLRP_FieldError_decodeFields (
 LLRP_tSFieldError * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_FieldError_assimilateSubParameters (
 LLRP_tSFieldError * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_FieldError_encode (
  const LLRP_tSFieldError *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdFieldError_FieldNum;

extern llrp_u16_t
LLRP_FieldError_getFieldNum (
  LLRP_tSFieldError *pThis);

extern LLRP_tResultCode
LLRP_FieldError_setFieldNum (
  LLRP_tSFieldError *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdFieldError_ErrorCode;

extern LLRP_tEStatusCode
LLRP_FieldError_getErrorCode (
  LLRP_tSFieldError *pThis);

extern LLRP_tResultCode
LLRP_FieldError_setErrorCode (
  LLRP_tSFieldError *pThis,
  LLRP_tEStatusCode Value);

  


  
struct LLRP_SParameterError
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  ParameterType;

    LLRP_tEStatusCode  eErrorCode;

  
    LLRP_tSFieldError * pFieldError;

    LLRP_tSParameterError * pParameterError;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdParameterError;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdParameterError[];

extern LLRP_tSParameterError *
LLRP_ParameterError_construct (void);

extern void
LLRP_ParameterError_destruct (
 LLRP_tSParameterError * pThis);

extern void
LLRP_ParameterError_decodeFields (
 LLRP_tSParameterError * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_ParameterError_assimilateSubParameters (
 LLRP_tSParameterError * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_ParameterError_encode (
  const LLRP_tSParameterError *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdParameterError_ParameterType;

extern llrp_u16_t
LLRP_ParameterError_getParameterType (
  LLRP_tSParameterError *pThis);

extern LLRP_tResultCode
LLRP_ParameterError_setParameterType (
  LLRP_tSParameterError *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdParameterError_ErrorCode;

extern LLRP_tEStatusCode
LLRP_ParameterError_getErrorCode (
  LLRP_tSParameterError *pThis);

extern LLRP_tResultCode
LLRP_ParameterError_setErrorCode (
  LLRP_tSParameterError *pThis,
  LLRP_tEStatusCode Value);

  
extern LLRP_tSFieldError *
LLRP_ParameterError_getFieldError (
  LLRP_tSParameterError *pThis);

extern LLRP_tResultCode
LLRP_ParameterError_setFieldError (
  LLRP_tSParameterError *pThis,
  LLRP_tSFieldError *pValue);

extern LLRP_tSParameterError *
LLRP_ParameterError_getParameterError (
  LLRP_tSParameterError *pThis);

extern LLRP_tResultCode
LLRP_ParameterError_setParameterError (
  LLRP_tSParameterError *pThis,
  LLRP_tSParameterError *pValue);



  
struct LLRP_SC1G2LLRPCapabilities
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  CanSupportBlockErase;

    llrp_u1_t  CanSupportBlockWrite;

    llrp_u16_t  MaxNumSelectFiltersPerQuery;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2LLRPCapabilities;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2LLRPCapabilities[];

extern LLRP_tSC1G2LLRPCapabilities *
LLRP_C1G2LLRPCapabilities_construct (void);

extern void
LLRP_C1G2LLRPCapabilities_destruct (
 LLRP_tSC1G2LLRPCapabilities * pThis);

extern void
LLRP_C1G2LLRPCapabilities_decodeFields (
 LLRP_tSC1G2LLRPCapabilities * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2LLRPCapabilities_assimilateSubParameters (
 LLRP_tSC1G2LLRPCapabilities * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2LLRPCapabilities_encode (
  const LLRP_tSC1G2LLRPCapabilities *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LLRPCapabilities_CanSupportBlockErase;

extern llrp_u1_t
LLRP_C1G2LLRPCapabilities_getCanSupportBlockErase (
  LLRP_tSC1G2LLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_C1G2LLRPCapabilities_setCanSupportBlockErase (
  LLRP_tSC1G2LLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LLRPCapabilities_CanSupportBlockWrite;

extern llrp_u1_t
LLRP_C1G2LLRPCapabilities_getCanSupportBlockWrite (
  LLRP_tSC1G2LLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_C1G2LLRPCapabilities_setCanSupportBlockWrite (
  LLRP_tSC1G2LLRPCapabilities *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LLRPCapabilities_MaxNumSelectFiltersPerQuery;

extern llrp_u16_t
LLRP_C1G2LLRPCapabilities_getMaxNumSelectFiltersPerQuery (
  LLRP_tSC1G2LLRPCapabilities *pThis);

extern LLRP_tResultCode
LLRP_C1G2LLRPCapabilities_setMaxNumSelectFiltersPerQuery (
  LLRP_tSC1G2LLRPCapabilities *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2UHFRFModeTable
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSC1G2UHFRFModeTableEntry * listC1G2UHFRFModeTableEntry;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2UHFRFModeTable;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2UHFRFModeTable[];

extern LLRP_tSC1G2UHFRFModeTable *
LLRP_C1G2UHFRFModeTable_construct (void);

extern void
LLRP_C1G2UHFRFModeTable_destruct (
 LLRP_tSC1G2UHFRFModeTable * pThis);

extern void
LLRP_C1G2UHFRFModeTable_decodeFields (
 LLRP_tSC1G2UHFRFModeTable * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2UHFRFModeTable_assimilateSubParameters (
 LLRP_tSC1G2UHFRFModeTable * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2UHFRFModeTable_encode (
  const LLRP_tSC1G2UHFRFModeTable *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSC1G2UHFRFModeTableEntry *
LLRP_C1G2UHFRFModeTable_beginC1G2UHFRFModeTableEntry (
  LLRP_tSC1G2UHFRFModeTable *pThis);

extern LLRP_tSC1G2UHFRFModeTableEntry *
LLRP_C1G2UHFRFModeTable_nextC1G2UHFRFModeTableEntry (
  LLRP_tSC1G2UHFRFModeTableEntry *pCurrent);

extern void
LLRP_C1G2UHFRFModeTable_clearC1G2UHFRFModeTableEntry (
  LLRP_tSC1G2UHFRFModeTable *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTable_addC1G2UHFRFModeTableEntry (
  LLRP_tSC1G2UHFRFModeTable *pThis,
  LLRP_tSC1G2UHFRFModeTableEntry *pValue);




  
struct LLRP_SC1G2UHFRFModeTableEntry
{
    LLRP_tSParameter hdr;
  
    llrp_u32_t  ModeIdentifier;

    LLRP_tEC1G2DRValue  eDRValue;

    llrp_u1_t  EPCHAGTCConformance;

    LLRP_tEC1G2MValue  eMValue;

    LLRP_tEC1G2ForwardLinkModulation  eForwardLinkModulation;

    LLRP_tEC1G2SpectralMaskIndicator  eSpectralMaskIndicator;

    llrp_u32_t  BDRValue;

    llrp_u32_t  PIEValue;

    llrp_u32_t  MinTariValue;

    llrp_u32_t  MaxTariValue;

    llrp_u32_t  StepTariValue;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2UHFRFModeTableEntry;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2UHFRFModeTableEntry[];

extern LLRP_tSC1G2UHFRFModeTableEntry *
LLRP_C1G2UHFRFModeTableEntry_construct (void);

extern void
LLRP_C1G2UHFRFModeTableEntry_destruct (
 LLRP_tSC1G2UHFRFModeTableEntry * pThis);

extern void
LLRP_C1G2UHFRFModeTableEntry_decodeFields (
 LLRP_tSC1G2UHFRFModeTableEntry * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2UHFRFModeTableEntry_assimilateSubParameters (
 LLRP_tSC1G2UHFRFModeTableEntry * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2UHFRFModeTableEntry_encode (
  const LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_ModeIdentifier;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getModeIdentifier (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setModeIdentifier (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_DRValue;

extern LLRP_tEC1G2DRValue
LLRP_C1G2UHFRFModeTableEntry_getDRValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setDRValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  LLRP_tEC1G2DRValue Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_EPCHAGTCConformance;

extern llrp_u1_t
LLRP_C1G2UHFRFModeTableEntry_getEPCHAGTCConformance (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setEPCHAGTCConformance (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_MValue;

extern LLRP_tEC1G2MValue
LLRP_C1G2UHFRFModeTableEntry_getMValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setMValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  LLRP_tEC1G2MValue Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_ForwardLinkModulation;

extern LLRP_tEC1G2ForwardLinkModulation
LLRP_C1G2UHFRFModeTableEntry_getForwardLinkModulation (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setForwardLinkModulation (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  LLRP_tEC1G2ForwardLinkModulation Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_SpectralMaskIndicator;

extern LLRP_tEC1G2SpectralMaskIndicator
LLRP_C1G2UHFRFModeTableEntry_getSpectralMaskIndicator (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setSpectralMaskIndicator (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  LLRP_tEC1G2SpectralMaskIndicator Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_BDRValue;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getBDRValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setBDRValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_PIEValue;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getPIEValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setPIEValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_MinTariValue;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getMinTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setMinTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_MaxTariValue;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getMaxTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setMaxTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2UHFRFModeTableEntry_StepTariValue;

extern llrp_u32_t
LLRP_C1G2UHFRFModeTableEntry_getStepTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis);

extern LLRP_tResultCode
LLRP_C1G2UHFRFModeTableEntry_setStepTariValue (
  LLRP_tSC1G2UHFRFModeTableEntry *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SC1G2InventoryCommand
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  TagInventoryStateAware;

  
    LLRP_tSC1G2Filter * listC1G2Filter;

    LLRP_tSC1G2RFControl * pC1G2RFControl;

    LLRP_tSC1G2SingulationControl * pC1G2SingulationControl;

    LLRP_tSParameter * listCustom;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2InventoryCommand;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2InventoryCommand[];

extern LLRP_tSC1G2InventoryCommand *
LLRP_C1G2InventoryCommand_construct (void);

extern void
LLRP_C1G2InventoryCommand_destruct (
 LLRP_tSC1G2InventoryCommand * pThis);

extern void
LLRP_C1G2InventoryCommand_decodeFields (
 LLRP_tSC1G2InventoryCommand * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2InventoryCommand_assimilateSubParameters (
 LLRP_tSC1G2InventoryCommand * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2InventoryCommand_encode (
  const LLRP_tSC1G2InventoryCommand *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2InventoryCommand_TagInventoryStateAware;

extern llrp_u1_t
LLRP_C1G2InventoryCommand_getTagInventoryStateAware (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tResultCode
LLRP_C1G2InventoryCommand_setTagInventoryStateAware (
  LLRP_tSC1G2InventoryCommand *pThis,
  llrp_u1_t Value);

  
extern LLRP_tSC1G2Filter *
LLRP_C1G2InventoryCommand_beginC1G2Filter (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tSC1G2Filter *
LLRP_C1G2InventoryCommand_nextC1G2Filter (
  LLRP_tSC1G2Filter *pCurrent);

extern void
LLRP_C1G2InventoryCommand_clearC1G2Filter (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tResultCode
LLRP_C1G2InventoryCommand_addC1G2Filter (
  LLRP_tSC1G2InventoryCommand *pThis,
  LLRP_tSC1G2Filter *pValue);


extern LLRP_tSC1G2RFControl *
LLRP_C1G2InventoryCommand_getC1G2RFControl (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tResultCode
LLRP_C1G2InventoryCommand_setC1G2RFControl (
  LLRP_tSC1G2InventoryCommand *pThis,
  LLRP_tSC1G2RFControl *pValue);

extern LLRP_tSC1G2SingulationControl *
LLRP_C1G2InventoryCommand_getC1G2SingulationControl (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tResultCode
LLRP_C1G2InventoryCommand_setC1G2SingulationControl (
  LLRP_tSC1G2InventoryCommand *pThis,
  LLRP_tSC1G2SingulationControl *pValue);

extern LLRP_tSParameter *
LLRP_C1G2InventoryCommand_beginCustom (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tSParameter *
LLRP_C1G2InventoryCommand_nextCustom (
  LLRP_tSParameter *pCurrent);

extern void
LLRP_C1G2InventoryCommand_clearCustom (
  LLRP_tSC1G2InventoryCommand *pThis);

extern LLRP_tResultCode
LLRP_C1G2InventoryCommand_addCustom (
  LLRP_tSC1G2InventoryCommand *pThis,
  LLRP_tSParameter *pValue);




  
struct LLRP_SC1G2Filter
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2TruncateAction  eT;

  
    LLRP_tSC1G2TagInventoryMask * pC1G2TagInventoryMask;

    LLRP_tSC1G2TagInventoryStateAwareFilterAction * pC1G2TagInventoryStateAwareFilterAction;

    LLRP_tSC1G2TagInventoryStateUnawareFilterAction * pC1G2TagInventoryStateUnawareFilterAction;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2Filter;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2Filter[];

extern LLRP_tSC1G2Filter *
LLRP_C1G2Filter_construct (void);

extern void
LLRP_C1G2Filter_destruct (
 LLRP_tSC1G2Filter * pThis);

extern void
LLRP_C1G2Filter_decodeFields (
 LLRP_tSC1G2Filter * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2Filter_assimilateSubParameters (
 LLRP_tSC1G2Filter * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2Filter_encode (
  const LLRP_tSC1G2Filter *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Filter_T;

extern LLRP_tEC1G2TruncateAction
LLRP_C1G2Filter_getT (
  LLRP_tSC1G2Filter *pThis);

extern LLRP_tResultCode
LLRP_C1G2Filter_setT (
  LLRP_tSC1G2Filter *pThis,
  LLRP_tEC1G2TruncateAction Value);

  
extern LLRP_tSC1G2TagInventoryMask *
LLRP_C1G2Filter_getC1G2TagInventoryMask (
  LLRP_tSC1G2Filter *pThis);

extern LLRP_tResultCode
LLRP_C1G2Filter_setC1G2TagInventoryMask (
  LLRP_tSC1G2Filter *pThis,
  LLRP_tSC1G2TagInventoryMask *pValue);

extern LLRP_tSC1G2TagInventoryStateAwareFilterAction *
LLRP_C1G2Filter_getC1G2TagInventoryStateAwareFilterAction (
  LLRP_tSC1G2Filter *pThis);

extern LLRP_tResultCode
LLRP_C1G2Filter_setC1G2TagInventoryStateAwareFilterAction (
  LLRP_tSC1G2Filter *pThis,
  LLRP_tSC1G2TagInventoryStateAwareFilterAction *pValue);

extern LLRP_tSC1G2TagInventoryStateUnawareFilterAction *
LLRP_C1G2Filter_getC1G2TagInventoryStateUnawareFilterAction (
  LLRP_tSC1G2Filter *pThis);

extern LLRP_tResultCode
LLRP_C1G2Filter_setC1G2TagInventoryStateUnawareFilterAction (
  LLRP_tSC1G2Filter *pThis,
  LLRP_tSC1G2TagInventoryStateUnawareFilterAction *pValue);



  
struct LLRP_SC1G2TagInventoryMask
{
    LLRP_tSParameter hdr;
  
    llrp_u2_t  MB;

    llrp_u16_t  Pointer;

    llrp_u1v_t  TagMask;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TagInventoryMask;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TagInventoryMask[];

extern LLRP_tSC1G2TagInventoryMask *
LLRP_C1G2TagInventoryMask_construct (void);

extern void
LLRP_C1G2TagInventoryMask_destruct (
 LLRP_tSC1G2TagInventoryMask * pThis);

extern void
LLRP_C1G2TagInventoryMask_decodeFields (
 LLRP_tSC1G2TagInventoryMask * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TagInventoryMask_assimilateSubParameters (
 LLRP_tSC1G2TagInventoryMask * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TagInventoryMask_encode (
  const LLRP_tSC1G2TagInventoryMask *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryMask_MB;

extern llrp_u2_t
LLRP_C1G2TagInventoryMask_getMB (
  LLRP_tSC1G2TagInventoryMask *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryMask_setMB (
  LLRP_tSC1G2TagInventoryMask *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryMask_Pointer;

extern llrp_u16_t
LLRP_C1G2TagInventoryMask_getPointer (
  LLRP_tSC1G2TagInventoryMask *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryMask_setPointer (
  LLRP_tSC1G2TagInventoryMask *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryMask_TagMask;

extern llrp_u1v_t
LLRP_C1G2TagInventoryMask_getTagMask (
  LLRP_tSC1G2TagInventoryMask *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryMask_setTagMask (
  LLRP_tSC1G2TagInventoryMask *pThis,
  llrp_u1v_t Value);

  


  
struct LLRP_SC1G2TagInventoryStateAwareFilterAction
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2StateAwareTarget  eTarget;

    LLRP_tEC1G2StateAwareAction  eAction;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TagInventoryStateAwareFilterAction;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TagInventoryStateAwareFilterAction[];

extern LLRP_tSC1G2TagInventoryStateAwareFilterAction *
LLRP_C1G2TagInventoryStateAwareFilterAction_construct (void);

extern void
LLRP_C1G2TagInventoryStateAwareFilterAction_destruct (
 LLRP_tSC1G2TagInventoryStateAwareFilterAction * pThis);

extern void
LLRP_C1G2TagInventoryStateAwareFilterAction_decodeFields (
 LLRP_tSC1G2TagInventoryStateAwareFilterAction * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TagInventoryStateAwareFilterAction_assimilateSubParameters (
 LLRP_tSC1G2TagInventoryStateAwareFilterAction * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TagInventoryStateAwareFilterAction_encode (
  const LLRP_tSC1G2TagInventoryStateAwareFilterAction *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryStateAwareFilterAction_Target;

extern LLRP_tEC1G2StateAwareTarget
LLRP_C1G2TagInventoryStateAwareFilterAction_getTarget (
  LLRP_tSC1G2TagInventoryStateAwareFilterAction *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryStateAwareFilterAction_setTarget (
  LLRP_tSC1G2TagInventoryStateAwareFilterAction *pThis,
  LLRP_tEC1G2StateAwareTarget Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryStateAwareFilterAction_Action;

extern LLRP_tEC1G2StateAwareAction
LLRP_C1G2TagInventoryStateAwareFilterAction_getAction (
  LLRP_tSC1G2TagInventoryStateAwareFilterAction *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryStateAwareFilterAction_setAction (
  LLRP_tSC1G2TagInventoryStateAwareFilterAction *pThis,
  LLRP_tEC1G2StateAwareAction Value);

  


  
struct LLRP_SC1G2TagInventoryStateUnawareFilterAction
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2StateUnawareAction  eAction;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TagInventoryStateUnawareFilterAction;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TagInventoryStateUnawareFilterAction[];

extern LLRP_tSC1G2TagInventoryStateUnawareFilterAction *
LLRP_C1G2TagInventoryStateUnawareFilterAction_construct (void);

extern void
LLRP_C1G2TagInventoryStateUnawareFilterAction_destruct (
 LLRP_tSC1G2TagInventoryStateUnawareFilterAction * pThis);

extern void
LLRP_C1G2TagInventoryStateUnawareFilterAction_decodeFields (
 LLRP_tSC1G2TagInventoryStateUnawareFilterAction * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TagInventoryStateUnawareFilterAction_assimilateSubParameters (
 LLRP_tSC1G2TagInventoryStateUnawareFilterAction * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TagInventoryStateUnawareFilterAction_encode (
  const LLRP_tSC1G2TagInventoryStateUnawareFilterAction *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryStateUnawareFilterAction_Action;

extern LLRP_tEC1G2StateUnawareAction
LLRP_C1G2TagInventoryStateUnawareFilterAction_getAction (
  LLRP_tSC1G2TagInventoryStateUnawareFilterAction *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryStateUnawareFilterAction_setAction (
  LLRP_tSC1G2TagInventoryStateUnawareFilterAction *pThis,
  LLRP_tEC1G2StateUnawareAction Value);

  


  
struct LLRP_SC1G2RFControl
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  ModeIndex;

    llrp_u16_t  Tari;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2RFControl;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2RFControl[];

extern LLRP_tSC1G2RFControl *
LLRP_C1G2RFControl_construct (void);

extern void
LLRP_C1G2RFControl_destruct (
 LLRP_tSC1G2RFControl * pThis);

extern void
LLRP_C1G2RFControl_decodeFields (
 LLRP_tSC1G2RFControl * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2RFControl_assimilateSubParameters (
 LLRP_tSC1G2RFControl * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2RFControl_encode (
  const LLRP_tSC1G2RFControl *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2RFControl_ModeIndex;

extern llrp_u16_t
LLRP_C1G2RFControl_getModeIndex (
  LLRP_tSC1G2RFControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2RFControl_setModeIndex (
  LLRP_tSC1G2RFControl *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2RFControl_Tari;

extern llrp_u16_t
LLRP_C1G2RFControl_getTari (
  LLRP_tSC1G2RFControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2RFControl_setTari (
  LLRP_tSC1G2RFControl *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2SingulationControl
{
    LLRP_tSParameter hdr;
  
    llrp_u2_t  Session;

    llrp_u16_t  TagPopulation;

    llrp_u32_t  TagTransitTime;

  
    LLRP_tSC1G2TagInventoryStateAwareSingulationAction * pC1G2TagInventoryStateAwareSingulationAction;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2SingulationControl;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2SingulationControl[];

extern LLRP_tSC1G2SingulationControl *
LLRP_C1G2SingulationControl_construct (void);

extern void
LLRP_C1G2SingulationControl_destruct (
 LLRP_tSC1G2SingulationControl * pThis);

extern void
LLRP_C1G2SingulationControl_decodeFields (
 LLRP_tSC1G2SingulationControl * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2SingulationControl_assimilateSubParameters (
 LLRP_tSC1G2SingulationControl * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2SingulationControl_encode (
  const LLRP_tSC1G2SingulationControl *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2SingulationControl_Session;

extern llrp_u2_t
LLRP_C1G2SingulationControl_getSession (
  LLRP_tSC1G2SingulationControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationControl_setSession (
  LLRP_tSC1G2SingulationControl *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2SingulationControl_TagPopulation;

extern llrp_u16_t
LLRP_C1G2SingulationControl_getTagPopulation (
  LLRP_tSC1G2SingulationControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationControl_setTagPopulation (
  LLRP_tSC1G2SingulationControl *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2SingulationControl_TagTransitTime;

extern llrp_u32_t
LLRP_C1G2SingulationControl_getTagTransitTime (
  LLRP_tSC1G2SingulationControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationControl_setTagTransitTime (
  LLRP_tSC1G2SingulationControl *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSC1G2TagInventoryStateAwareSingulationAction *
LLRP_C1G2SingulationControl_getC1G2TagInventoryStateAwareSingulationAction (
  LLRP_tSC1G2SingulationControl *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationControl_setC1G2TagInventoryStateAwareSingulationAction (
  LLRP_tSC1G2SingulationControl *pThis,
  LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pValue);



  
struct LLRP_SC1G2TagInventoryStateAwareSingulationAction
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2TagInventoryStateAwareI  eI;

    LLRP_tEC1G2TagInventoryStateAwareS  eS;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TagInventoryStateAwareSingulationAction;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TagInventoryStateAwareSingulationAction[];

extern LLRP_tSC1G2TagInventoryStateAwareSingulationAction *
LLRP_C1G2TagInventoryStateAwareSingulationAction_construct (void);

extern void
LLRP_C1G2TagInventoryStateAwareSingulationAction_destruct (
 LLRP_tSC1G2TagInventoryStateAwareSingulationAction * pThis);

extern void
LLRP_C1G2TagInventoryStateAwareSingulationAction_decodeFields (
 LLRP_tSC1G2TagInventoryStateAwareSingulationAction * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TagInventoryStateAwareSingulationAction_assimilateSubParameters (
 LLRP_tSC1G2TagInventoryStateAwareSingulationAction * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TagInventoryStateAwareSingulationAction_encode (
  const LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryStateAwareSingulationAction_I;

extern LLRP_tEC1G2TagInventoryStateAwareI
LLRP_C1G2TagInventoryStateAwareSingulationAction_getI (
  LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryStateAwareSingulationAction_setI (
  LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pThis,
  LLRP_tEC1G2TagInventoryStateAwareI Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TagInventoryStateAwareSingulationAction_S;

extern LLRP_tEC1G2TagInventoryStateAwareS
LLRP_C1G2TagInventoryStateAwareSingulationAction_getS (
  LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagInventoryStateAwareSingulationAction_setS (
  LLRP_tSC1G2TagInventoryStateAwareSingulationAction *pThis,
  LLRP_tEC1G2TagInventoryStateAwareS Value);

  


  
struct LLRP_SC1G2TagSpec
{
    LLRP_tSParameter hdr;
  
  
    LLRP_tSC1G2TargetTag * listC1G2TargetTag;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TagSpec;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TagSpec[];

extern LLRP_tSC1G2TagSpec *
LLRP_C1G2TagSpec_construct (void);

extern void
LLRP_C1G2TagSpec_destruct (
 LLRP_tSC1G2TagSpec * pThis);

extern void
LLRP_C1G2TagSpec_decodeFields (
 LLRP_tSC1G2TagSpec * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TagSpec_assimilateSubParameters (
 LLRP_tSC1G2TagSpec * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TagSpec_encode (
  const LLRP_tSC1G2TagSpec *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
  
extern LLRP_tSC1G2TargetTag *
LLRP_C1G2TagSpec_beginC1G2TargetTag (
  LLRP_tSC1G2TagSpec *pThis);

extern LLRP_tSC1G2TargetTag *
LLRP_C1G2TagSpec_nextC1G2TargetTag (
  LLRP_tSC1G2TargetTag *pCurrent);

extern void
LLRP_C1G2TagSpec_clearC1G2TargetTag (
  LLRP_tSC1G2TagSpec *pThis);

extern LLRP_tResultCode
LLRP_C1G2TagSpec_addC1G2TargetTag (
  LLRP_tSC1G2TagSpec *pThis,
  LLRP_tSC1G2TargetTag *pValue);




  
struct LLRP_SC1G2TargetTag
{
    LLRP_tSParameter hdr;
  
    llrp_u2_t  MB;

    llrp_u1_t  Match;

    llrp_u16_t  Pointer;

    llrp_u1v_t  TagMask;

    llrp_u1v_t  TagData;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2TargetTag;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2TargetTag[];

extern LLRP_tSC1G2TargetTag *
LLRP_C1G2TargetTag_construct (void);

extern void
LLRP_C1G2TargetTag_destruct (
 LLRP_tSC1G2TargetTag * pThis);

extern void
LLRP_C1G2TargetTag_decodeFields (
 LLRP_tSC1G2TargetTag * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2TargetTag_assimilateSubParameters (
 LLRP_tSC1G2TargetTag * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2TargetTag_encode (
  const LLRP_tSC1G2TargetTag *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TargetTag_MB;

extern llrp_u2_t
LLRP_C1G2TargetTag_getMB (
  LLRP_tSC1G2TargetTag *pThis);

extern LLRP_tResultCode
LLRP_C1G2TargetTag_setMB (
  LLRP_tSC1G2TargetTag *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TargetTag_Match;

extern llrp_u1_t
LLRP_C1G2TargetTag_getMatch (
  LLRP_tSC1G2TargetTag *pThis);

extern LLRP_tResultCode
LLRP_C1G2TargetTag_setMatch (
  LLRP_tSC1G2TargetTag *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TargetTag_Pointer;

extern llrp_u16_t
LLRP_C1G2TargetTag_getPointer (
  LLRP_tSC1G2TargetTag *pThis);

extern LLRP_tResultCode
LLRP_C1G2TargetTag_setPointer (
  LLRP_tSC1G2TargetTag *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TargetTag_TagMask;

extern llrp_u1v_t
LLRP_C1G2TargetTag_getTagMask (
  LLRP_tSC1G2TargetTag *pThis);

extern LLRP_tResultCode
LLRP_C1G2TargetTag_setTagMask (
  LLRP_tSC1G2TargetTag *pThis,
  llrp_u1v_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2TargetTag_TagData;

extern llrp_u1v_t
LLRP_C1G2TargetTag_getTagData (
  LLRP_tSC1G2TargetTag *pThis);

extern LLRP_tResultCode
LLRP_C1G2TargetTag_setTagData (
  LLRP_tSC1G2TargetTag *pThis,
  llrp_u1v_t Value);

  


  
struct LLRP_SC1G2Read
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  AccessPassword;

    llrp_u2_t  MB;

    llrp_u16_t  WordPointer;

    llrp_u16_t  WordCount;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2Read;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2Read[];

extern LLRP_tSC1G2Read *
LLRP_C1G2Read_construct (void);

extern void
LLRP_C1G2Read_destruct (
 LLRP_tSC1G2Read * pThis);

extern void
LLRP_C1G2Read_decodeFields (
 LLRP_tSC1G2Read * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2Read_assimilateSubParameters (
 LLRP_tSC1G2Read * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2Read_encode (
  const LLRP_tSC1G2Read *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Read_OpSpecID;

extern llrp_u16_t
LLRP_C1G2Read_getOpSpecID (
  LLRP_tSC1G2Read *pThis);

extern LLRP_tResultCode
LLRP_C1G2Read_setOpSpecID (
  LLRP_tSC1G2Read *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Read_AccessPassword;

extern llrp_u32_t
LLRP_C1G2Read_getAccessPassword (
  LLRP_tSC1G2Read *pThis);

extern LLRP_tResultCode
LLRP_C1G2Read_setAccessPassword (
  LLRP_tSC1G2Read *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Read_MB;

extern llrp_u2_t
LLRP_C1G2Read_getMB (
  LLRP_tSC1G2Read *pThis);

extern LLRP_tResultCode
LLRP_C1G2Read_setMB (
  LLRP_tSC1G2Read *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Read_WordPointer;

extern llrp_u16_t
LLRP_C1G2Read_getWordPointer (
  LLRP_tSC1G2Read *pThis);

extern LLRP_tResultCode
LLRP_C1G2Read_setWordPointer (
  LLRP_tSC1G2Read *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Read_WordCount;

extern llrp_u16_t
LLRP_C1G2Read_getWordCount (
  LLRP_tSC1G2Read *pThis);

extern LLRP_tResultCode
LLRP_C1G2Read_setWordCount (
  LLRP_tSC1G2Read *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2Write
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  AccessPassword;

    llrp_u2_t  MB;

    llrp_u16_t  WordPointer;

    llrp_u16v_t  WriteData;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2Write;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2Write[];

extern LLRP_tSC1G2Write *
LLRP_C1G2Write_construct (void);

extern void
LLRP_C1G2Write_destruct (
 LLRP_tSC1G2Write * pThis);

extern void
LLRP_C1G2Write_decodeFields (
 LLRP_tSC1G2Write * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2Write_assimilateSubParameters (
 LLRP_tSC1G2Write * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2Write_encode (
  const LLRP_tSC1G2Write *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Write_OpSpecID;

extern llrp_u16_t
LLRP_C1G2Write_getOpSpecID (
  LLRP_tSC1G2Write *pThis);

extern LLRP_tResultCode
LLRP_C1G2Write_setOpSpecID (
  LLRP_tSC1G2Write *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Write_AccessPassword;

extern llrp_u32_t
LLRP_C1G2Write_getAccessPassword (
  LLRP_tSC1G2Write *pThis);

extern LLRP_tResultCode
LLRP_C1G2Write_setAccessPassword (
  LLRP_tSC1G2Write *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Write_MB;

extern llrp_u2_t
LLRP_C1G2Write_getMB (
  LLRP_tSC1G2Write *pThis);

extern LLRP_tResultCode
LLRP_C1G2Write_setMB (
  LLRP_tSC1G2Write *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Write_WordPointer;

extern llrp_u16_t
LLRP_C1G2Write_getWordPointer (
  LLRP_tSC1G2Write *pThis);

extern LLRP_tResultCode
LLRP_C1G2Write_setWordPointer (
  LLRP_tSC1G2Write *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Write_WriteData;

extern llrp_u16v_t
LLRP_C1G2Write_getWriteData (
  LLRP_tSC1G2Write *pThis);

extern LLRP_tResultCode
LLRP_C1G2Write_setWriteData (
  LLRP_tSC1G2Write *pThis,
  llrp_u16v_t Value);

  


  
struct LLRP_SC1G2Kill
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  KillPassword;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2Kill;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2Kill[];

extern LLRP_tSC1G2Kill *
LLRP_C1G2Kill_construct (void);

extern void
LLRP_C1G2Kill_destruct (
 LLRP_tSC1G2Kill * pThis);

extern void
LLRP_C1G2Kill_decodeFields (
 LLRP_tSC1G2Kill * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2Kill_assimilateSubParameters (
 LLRP_tSC1G2Kill * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2Kill_encode (
  const LLRP_tSC1G2Kill *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Kill_OpSpecID;

extern llrp_u16_t
LLRP_C1G2Kill_getOpSpecID (
  LLRP_tSC1G2Kill *pThis);

extern LLRP_tResultCode
LLRP_C1G2Kill_setOpSpecID (
  LLRP_tSC1G2Kill *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Kill_KillPassword;

extern llrp_u32_t
LLRP_C1G2Kill_getKillPassword (
  LLRP_tSC1G2Kill *pThis);

extern LLRP_tResultCode
LLRP_C1G2Kill_setKillPassword (
  LLRP_tSC1G2Kill *pThis,
  llrp_u32_t Value);

  


  
struct LLRP_SC1G2Lock
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  AccessPassword;

  
    LLRP_tSC1G2LockPayload * listC1G2LockPayload;

};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2Lock;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2Lock[];

extern LLRP_tSC1G2Lock *
LLRP_C1G2Lock_construct (void);

extern void
LLRP_C1G2Lock_destruct (
 LLRP_tSC1G2Lock * pThis);

extern void
LLRP_C1G2Lock_decodeFields (
 LLRP_tSC1G2Lock * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2Lock_assimilateSubParameters (
 LLRP_tSC1G2Lock * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2Lock_encode (
  const LLRP_tSC1G2Lock *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Lock_OpSpecID;

extern llrp_u16_t
LLRP_C1G2Lock_getOpSpecID (
  LLRP_tSC1G2Lock *pThis);

extern LLRP_tResultCode
LLRP_C1G2Lock_setOpSpecID (
  LLRP_tSC1G2Lock *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2Lock_AccessPassword;

extern llrp_u32_t
LLRP_C1G2Lock_getAccessPassword (
  LLRP_tSC1G2Lock *pThis);

extern LLRP_tResultCode
LLRP_C1G2Lock_setAccessPassword (
  LLRP_tSC1G2Lock *pThis,
  llrp_u32_t Value);

  
extern LLRP_tSC1G2LockPayload *
LLRP_C1G2Lock_beginC1G2LockPayload (
  LLRP_tSC1G2Lock *pThis);

extern LLRP_tSC1G2LockPayload *
LLRP_C1G2Lock_nextC1G2LockPayload (
  LLRP_tSC1G2LockPayload *pCurrent);

extern void
LLRP_C1G2Lock_clearC1G2LockPayload (
  LLRP_tSC1G2Lock *pThis);

extern LLRP_tResultCode
LLRP_C1G2Lock_addC1G2LockPayload (
  LLRP_tSC1G2Lock *pThis,
  LLRP_tSC1G2LockPayload *pValue);




  
struct LLRP_SC1G2LockPayload
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2LockPrivilege  ePrivilege;

    LLRP_tEC1G2LockDataField  eDataField;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2LockPayload;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2LockPayload[];

extern LLRP_tSC1G2LockPayload *
LLRP_C1G2LockPayload_construct (void);

extern void
LLRP_C1G2LockPayload_destruct (
 LLRP_tSC1G2LockPayload * pThis);

extern void
LLRP_C1G2LockPayload_decodeFields (
 LLRP_tSC1G2LockPayload * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2LockPayload_assimilateSubParameters (
 LLRP_tSC1G2LockPayload * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2LockPayload_encode (
  const LLRP_tSC1G2LockPayload *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LockPayload_Privilege;

extern LLRP_tEC1G2LockPrivilege
LLRP_C1G2LockPayload_getPrivilege (
  LLRP_tSC1G2LockPayload *pThis);

extern LLRP_tResultCode
LLRP_C1G2LockPayload_setPrivilege (
  LLRP_tSC1G2LockPayload *pThis,
  LLRP_tEC1G2LockPrivilege Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LockPayload_DataField;

extern LLRP_tEC1G2LockDataField
LLRP_C1G2LockPayload_getDataField (
  LLRP_tSC1G2LockPayload *pThis);

extern LLRP_tResultCode
LLRP_C1G2LockPayload_setDataField (
  LLRP_tSC1G2LockPayload *pThis,
  LLRP_tEC1G2LockDataField Value);

  


  
struct LLRP_SC1G2BlockErase
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  AccessPassword;

    llrp_u2_t  MB;

    llrp_u16_t  WordPointer;

    llrp_u16_t  WordCount;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2BlockErase;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2BlockErase[];

extern LLRP_tSC1G2BlockErase *
LLRP_C1G2BlockErase_construct (void);

extern void
LLRP_C1G2BlockErase_destruct (
 LLRP_tSC1G2BlockErase * pThis);

extern void
LLRP_C1G2BlockErase_decodeFields (
 LLRP_tSC1G2BlockErase * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2BlockErase_assimilateSubParameters (
 LLRP_tSC1G2BlockErase * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2BlockErase_encode (
  const LLRP_tSC1G2BlockErase *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockErase_OpSpecID;

extern llrp_u16_t
LLRP_C1G2BlockErase_getOpSpecID (
  LLRP_tSC1G2BlockErase *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockErase_setOpSpecID (
  LLRP_tSC1G2BlockErase *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockErase_AccessPassword;

extern llrp_u32_t
LLRP_C1G2BlockErase_getAccessPassword (
  LLRP_tSC1G2BlockErase *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockErase_setAccessPassword (
  LLRP_tSC1G2BlockErase *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockErase_MB;

extern llrp_u2_t
LLRP_C1G2BlockErase_getMB (
  LLRP_tSC1G2BlockErase *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockErase_setMB (
  LLRP_tSC1G2BlockErase *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockErase_WordPointer;

extern llrp_u16_t
LLRP_C1G2BlockErase_getWordPointer (
  LLRP_tSC1G2BlockErase *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockErase_setWordPointer (
  LLRP_tSC1G2BlockErase *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockErase_WordCount;

extern llrp_u16_t
LLRP_C1G2BlockErase_getWordCount (
  LLRP_tSC1G2BlockErase *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockErase_setWordCount (
  LLRP_tSC1G2BlockErase *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2BlockWrite
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  OpSpecID;

    llrp_u32_t  AccessPassword;

    llrp_u2_t  MB;

    llrp_u16_t  WordPointer;

    llrp_u16v_t  WriteData;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2BlockWrite;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2BlockWrite[];

extern LLRP_tSC1G2BlockWrite *
LLRP_C1G2BlockWrite_construct (void);

extern void
LLRP_C1G2BlockWrite_destruct (
 LLRP_tSC1G2BlockWrite * pThis);

extern void
LLRP_C1G2BlockWrite_decodeFields (
 LLRP_tSC1G2BlockWrite * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2BlockWrite_assimilateSubParameters (
 LLRP_tSC1G2BlockWrite * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2BlockWrite_encode (
  const LLRP_tSC1G2BlockWrite *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWrite_OpSpecID;

extern llrp_u16_t
LLRP_C1G2BlockWrite_getOpSpecID (
  LLRP_tSC1G2BlockWrite *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWrite_setOpSpecID (
  LLRP_tSC1G2BlockWrite *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWrite_AccessPassword;

extern llrp_u32_t
LLRP_C1G2BlockWrite_getAccessPassword (
  LLRP_tSC1G2BlockWrite *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWrite_setAccessPassword (
  LLRP_tSC1G2BlockWrite *pThis,
  llrp_u32_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWrite_MB;

extern llrp_u2_t
LLRP_C1G2BlockWrite_getMB (
  LLRP_tSC1G2BlockWrite *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWrite_setMB (
  LLRP_tSC1G2BlockWrite *pThis,
  llrp_u2_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWrite_WordPointer;

extern llrp_u16_t
LLRP_C1G2BlockWrite_getWordPointer (
  LLRP_tSC1G2BlockWrite *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWrite_setWordPointer (
  LLRP_tSC1G2BlockWrite *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWrite_WriteData;

extern llrp_u16v_t
LLRP_C1G2BlockWrite_getWriteData (
  LLRP_tSC1G2BlockWrite *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWrite_setWriteData (
  LLRP_tSC1G2BlockWrite *pThis,
  llrp_u16v_t Value);

  


  
struct LLRP_SC1G2EPCMemorySelector
{
    LLRP_tSParameter hdr;
  
    llrp_u1_t  EnableCRC;

    llrp_u1_t  EnablePCBits;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2EPCMemorySelector;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2EPCMemorySelector[];

extern LLRP_tSC1G2EPCMemorySelector *
LLRP_C1G2EPCMemorySelector_construct (void);

extern void
LLRP_C1G2EPCMemorySelector_destruct (
 LLRP_tSC1G2EPCMemorySelector * pThis);

extern void
LLRP_C1G2EPCMemorySelector_decodeFields (
 LLRP_tSC1G2EPCMemorySelector * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2EPCMemorySelector_assimilateSubParameters (
 LLRP_tSC1G2EPCMemorySelector * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2EPCMemorySelector_encode (
  const LLRP_tSC1G2EPCMemorySelector *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2EPCMemorySelector_EnableCRC;

extern llrp_u1_t
LLRP_C1G2EPCMemorySelector_getEnableCRC (
  LLRP_tSC1G2EPCMemorySelector *pThis);

extern LLRP_tResultCode
LLRP_C1G2EPCMemorySelector_setEnableCRC (
  LLRP_tSC1G2EPCMemorySelector *pThis,
  llrp_u1_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2EPCMemorySelector_EnablePCBits;

extern llrp_u1_t
LLRP_C1G2EPCMemorySelector_getEnablePCBits (
  LLRP_tSC1G2EPCMemorySelector *pThis);

extern LLRP_tResultCode
LLRP_C1G2EPCMemorySelector_setEnablePCBits (
  LLRP_tSC1G2EPCMemorySelector *pThis,
  llrp_u1_t Value);

  


  
struct LLRP_SC1G2_PC
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  PC_Bits;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2_PC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2_PC[];

extern LLRP_tSC1G2_PC *
LLRP_C1G2_PC_construct (void);

extern void
LLRP_C1G2_PC_destruct (
 LLRP_tSC1G2_PC * pThis);

extern void
LLRP_C1G2_PC_decodeFields (
 LLRP_tSC1G2_PC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2_PC_assimilateSubParameters (
 LLRP_tSC1G2_PC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2_PC_encode (
  const LLRP_tSC1G2_PC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2_PC_PC_Bits;

extern llrp_u16_t
LLRP_C1G2_PC_getPC_Bits (
  LLRP_tSC1G2_PC *pThis);

extern LLRP_tResultCode
LLRP_C1G2_PC_setPC_Bits (
  LLRP_tSC1G2_PC *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2_CRC
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  CRC;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2_CRC;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2_CRC[];

extern LLRP_tSC1G2_CRC *
LLRP_C1G2_CRC_construct (void);

extern void
LLRP_C1G2_CRC_destruct (
 LLRP_tSC1G2_CRC * pThis);

extern void
LLRP_C1G2_CRC_decodeFields (
 LLRP_tSC1G2_CRC * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2_CRC_assimilateSubParameters (
 LLRP_tSC1G2_CRC * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2_CRC_encode (
  const LLRP_tSC1G2_CRC *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2_CRC_CRC;

extern llrp_u16_t
LLRP_C1G2_CRC_getCRC (
  LLRP_tSC1G2_CRC *pThis);

extern LLRP_tResultCode
LLRP_C1G2_CRC_setCRC (
  LLRP_tSC1G2_CRC *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2SingulationDetails
{
    LLRP_tSParameter hdr;
  
    llrp_u16_t  NumCollisionSlots;

    llrp_u16_t  NumEmptySlots;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2SingulationDetails;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2SingulationDetails[];

extern LLRP_tSC1G2SingulationDetails *
LLRP_C1G2SingulationDetails_construct (void);

extern void
LLRP_C1G2SingulationDetails_destruct (
 LLRP_tSC1G2SingulationDetails * pThis);

extern void
LLRP_C1G2SingulationDetails_decodeFields (
 LLRP_tSC1G2SingulationDetails * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2SingulationDetails_assimilateSubParameters (
 LLRP_tSC1G2SingulationDetails * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2SingulationDetails_encode (
  const LLRP_tSC1G2SingulationDetails *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2SingulationDetails_NumCollisionSlots;

extern llrp_u16_t
LLRP_C1G2SingulationDetails_getNumCollisionSlots (
  LLRP_tSC1G2SingulationDetails *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationDetails_setNumCollisionSlots (
  LLRP_tSC1G2SingulationDetails *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2SingulationDetails_NumEmptySlots;

extern llrp_u16_t
LLRP_C1G2SingulationDetails_getNumEmptySlots (
  LLRP_tSC1G2SingulationDetails *pThis);

extern LLRP_tResultCode
LLRP_C1G2SingulationDetails_setNumEmptySlots (
  LLRP_tSC1G2SingulationDetails *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2ReadOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2ReadResultType  eResult;

    llrp_u16_t  OpSpecID;

    llrp_u16v_t  ReadData;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2ReadOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2ReadOpSpecResult[];

extern LLRP_tSC1G2ReadOpSpecResult *
LLRP_C1G2ReadOpSpecResult_construct (void);

extern void
LLRP_C1G2ReadOpSpecResult_destruct (
 LLRP_tSC1G2ReadOpSpecResult * pThis);

extern void
LLRP_C1G2ReadOpSpecResult_decodeFields (
 LLRP_tSC1G2ReadOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2ReadOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2ReadOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2ReadOpSpecResult_encode (
  const LLRP_tSC1G2ReadOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2ReadOpSpecResult_Result;

extern LLRP_tEC1G2ReadResultType
LLRP_C1G2ReadOpSpecResult_getResult (
  LLRP_tSC1G2ReadOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2ReadOpSpecResult_setResult (
  LLRP_tSC1G2ReadOpSpecResult *pThis,
  LLRP_tEC1G2ReadResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2ReadOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2ReadOpSpecResult_getOpSpecID (
  LLRP_tSC1G2ReadOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2ReadOpSpecResult_setOpSpecID (
  LLRP_tSC1G2ReadOpSpecResult *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2ReadOpSpecResult_ReadData;

extern llrp_u16v_t
LLRP_C1G2ReadOpSpecResult_getReadData (
  LLRP_tSC1G2ReadOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2ReadOpSpecResult_setReadData (
  LLRP_tSC1G2ReadOpSpecResult *pThis,
  llrp_u16v_t Value);

  


  
struct LLRP_SC1G2WriteOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2WriteResultType  eResult;

    llrp_u16_t  OpSpecID;

    llrp_u16_t  NumWordsWritten;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2WriteOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2WriteOpSpecResult[];

extern LLRP_tSC1G2WriteOpSpecResult *
LLRP_C1G2WriteOpSpecResult_construct (void);

extern void
LLRP_C1G2WriteOpSpecResult_destruct (
 LLRP_tSC1G2WriteOpSpecResult * pThis);

extern void
LLRP_C1G2WriteOpSpecResult_decodeFields (
 LLRP_tSC1G2WriteOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2WriteOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2WriteOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2WriteOpSpecResult_encode (
  const LLRP_tSC1G2WriteOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2WriteOpSpecResult_Result;

extern LLRP_tEC1G2WriteResultType
LLRP_C1G2WriteOpSpecResult_getResult (
  LLRP_tSC1G2WriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2WriteOpSpecResult_setResult (
  LLRP_tSC1G2WriteOpSpecResult *pThis,
  LLRP_tEC1G2WriteResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2WriteOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2WriteOpSpecResult_getOpSpecID (
  LLRP_tSC1G2WriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2WriteOpSpecResult_setOpSpecID (
  LLRP_tSC1G2WriteOpSpecResult *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2WriteOpSpecResult_NumWordsWritten;

extern llrp_u16_t
LLRP_C1G2WriteOpSpecResult_getNumWordsWritten (
  LLRP_tSC1G2WriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2WriteOpSpecResult_setNumWordsWritten (
  LLRP_tSC1G2WriteOpSpecResult *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2KillOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2KillResultType  eResult;

    llrp_u16_t  OpSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2KillOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2KillOpSpecResult[];

extern LLRP_tSC1G2KillOpSpecResult *
LLRP_C1G2KillOpSpecResult_construct (void);

extern void
LLRP_C1G2KillOpSpecResult_destruct (
 LLRP_tSC1G2KillOpSpecResult * pThis);

extern void
LLRP_C1G2KillOpSpecResult_decodeFields (
 LLRP_tSC1G2KillOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2KillOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2KillOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2KillOpSpecResult_encode (
  const LLRP_tSC1G2KillOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2KillOpSpecResult_Result;

extern LLRP_tEC1G2KillResultType
LLRP_C1G2KillOpSpecResult_getResult (
  LLRP_tSC1G2KillOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2KillOpSpecResult_setResult (
  LLRP_tSC1G2KillOpSpecResult *pThis,
  LLRP_tEC1G2KillResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2KillOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2KillOpSpecResult_getOpSpecID (
  LLRP_tSC1G2KillOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2KillOpSpecResult_setOpSpecID (
  LLRP_tSC1G2KillOpSpecResult *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2LockOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2LockResultType  eResult;

    llrp_u16_t  OpSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2LockOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2LockOpSpecResult[];

extern LLRP_tSC1G2LockOpSpecResult *
LLRP_C1G2LockOpSpecResult_construct (void);

extern void
LLRP_C1G2LockOpSpecResult_destruct (
 LLRP_tSC1G2LockOpSpecResult * pThis);

extern void
LLRP_C1G2LockOpSpecResult_decodeFields (
 LLRP_tSC1G2LockOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2LockOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2LockOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2LockOpSpecResult_encode (
  const LLRP_tSC1G2LockOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LockOpSpecResult_Result;

extern LLRP_tEC1G2LockResultType
LLRP_C1G2LockOpSpecResult_getResult (
  LLRP_tSC1G2LockOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2LockOpSpecResult_setResult (
  LLRP_tSC1G2LockOpSpecResult *pThis,
  LLRP_tEC1G2LockResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2LockOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2LockOpSpecResult_getOpSpecID (
  LLRP_tSC1G2LockOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2LockOpSpecResult_setOpSpecID (
  LLRP_tSC1G2LockOpSpecResult *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2BlockEraseOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2BlockEraseResultType  eResult;

    llrp_u16_t  OpSpecID;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2BlockEraseOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2BlockEraseOpSpecResult[];

extern LLRP_tSC1G2BlockEraseOpSpecResult *
LLRP_C1G2BlockEraseOpSpecResult_construct (void);

extern void
LLRP_C1G2BlockEraseOpSpecResult_destruct (
 LLRP_tSC1G2BlockEraseOpSpecResult * pThis);

extern void
LLRP_C1G2BlockEraseOpSpecResult_decodeFields (
 LLRP_tSC1G2BlockEraseOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2BlockEraseOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2BlockEraseOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2BlockEraseOpSpecResult_encode (
  const LLRP_tSC1G2BlockEraseOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockEraseOpSpecResult_Result;

extern LLRP_tEC1G2BlockEraseResultType
LLRP_C1G2BlockEraseOpSpecResult_getResult (
  LLRP_tSC1G2BlockEraseOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockEraseOpSpecResult_setResult (
  LLRP_tSC1G2BlockEraseOpSpecResult *pThis,
  LLRP_tEC1G2BlockEraseResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockEraseOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2BlockEraseOpSpecResult_getOpSpecID (
  LLRP_tSC1G2BlockEraseOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockEraseOpSpecResult_setOpSpecID (
  LLRP_tSC1G2BlockEraseOpSpecResult *pThis,
  llrp_u16_t Value);

  


  
struct LLRP_SC1G2BlockWriteOpSpecResult
{
    LLRP_tSParameter hdr;
  
    LLRP_tEC1G2BlockWriteResultType  eResult;

    llrp_u16_t  OpSpecID;

    llrp_u16_t  NumWordsWritten;

  
};

extern const LLRP_tSTypeDescriptor
LLRP_tdC1G2BlockWriteOpSpecResult;

extern const LLRP_tSFieldDescriptor *
LLRP_apfdC1G2BlockWriteOpSpecResult[];

extern LLRP_tSC1G2BlockWriteOpSpecResult *
LLRP_C1G2BlockWriteOpSpecResult_construct (void);

extern void
LLRP_C1G2BlockWriteOpSpecResult_destruct (
 LLRP_tSC1G2BlockWriteOpSpecResult * pThis);

extern void
LLRP_C1G2BlockWriteOpSpecResult_decodeFields (
 LLRP_tSC1G2BlockWriteOpSpecResult * pThis,
 LLRP_tSDecoderStream * pDecoderStream);

extern void
LLRP_C1G2BlockWriteOpSpecResult_assimilateSubParameters (
 LLRP_tSC1G2BlockWriteOpSpecResult * pThis,
 LLRP_tSErrorDetails *          pError);

extern void
LLRP_C1G2BlockWriteOpSpecResult_encode (
  const LLRP_tSC1G2BlockWriteOpSpecResult *pThis,
  LLRP_tSEncoderStream *        pEncoderStream);

  

  
extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWriteOpSpecResult_Result;

extern LLRP_tEC1G2BlockWriteResultType
LLRP_C1G2BlockWriteOpSpecResult_getResult (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWriteOpSpecResult_setResult (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis,
  LLRP_tEC1G2BlockWriteResultType Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWriteOpSpecResult_OpSpecID;

extern llrp_u16_t
LLRP_C1G2BlockWriteOpSpecResult_getOpSpecID (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWriteOpSpecResult_setOpSpecID (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis,
  llrp_u16_t Value);

extern const LLRP_tSFieldDescriptor
LLRP_fdC1G2BlockWriteOpSpecResult_NumWordsWritten;

extern llrp_u16_t
LLRP_C1G2BlockWriteOpSpecResult_getNumWordsWritten (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis);

extern LLRP_tResultCode
LLRP_C1G2BlockWriteOpSpecResult_setNumWordsWritten (
  LLRP_tSC1G2BlockWriteOpSpecResult *pThis,
  llrp_u16_t Value);

  



extern llrp_bool_t
LLRP_SpecParameter_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdSpecParameter;


extern llrp_bool_t
LLRP_AccessCommandOpSpec_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessCommandOpSpec;


extern llrp_bool_t
LLRP_AccessCommandOpSpecResult_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAccessCommandOpSpecResult;


extern llrp_bool_t
LLRP_EPCParameter_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdEPCParameter;


extern llrp_bool_t
LLRP_Timestamp_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdTimestamp;


extern llrp_bool_t
LLRP_AirProtocolLLRPCapabilities_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolLLRPCapabilities;


extern llrp_bool_t
LLRP_AirProtocolUHFRFModeTable_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolUHFRFModeTable;


extern llrp_bool_t
LLRP_AirProtocolInventoryCommandSettings_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolInventoryCommandSettings;


extern llrp_bool_t
LLRP_AirProtocolTagSpec_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolTagSpec;


extern llrp_bool_t
LLRP_AirProtocolEPCMemorySelector_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolEPCMemorySelector;


extern llrp_bool_t
LLRP_AirProtocolTagData_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolTagData;


extern llrp_bool_t
LLRP_AirProtocolSingulationDetails_isMember (
  LLRP_tSParameter *            pParameter);

extern const LLRP_tSTypeDescriptor
LLRP_tdAirProtocolSingulationDetails;


void
LLRP_enrollCoreTypesIntoRegistry (
  LLRP_tSTypeRegistry *         pTypeRegistry);

