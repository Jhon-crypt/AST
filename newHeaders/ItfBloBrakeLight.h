//**************************************************************************************************
/*!
@version 1.4.1.0
*/
//! @file    ItfBloBrakeLight.h
//! @briefWithImage{BloBrakeLight32x32.png,Block "Block for state of brake lights",BloBrakeLight}
//!
//! @details
//! \htmlonly<img width="50%" src="BrakeLight_Unit.png" alt="BrakeLight_Unit.png">\endhtmlonly
//! \htmlonly <br clear="all"> \endhtmlonly
//!\n\n\n
//! This BrakeLight block is designed to read input values and generate brake light state depending on \n
//! vehicle acceleration or deflection of the brake pedal.\n
//! Also it is possible to suppress the velocity signal than the brake light state will depend only \n
//! on the deflection of the brake pedal. \n
//! In this case brake lights will be deactivated after the delay time \ ref u32DelayTimer. \n
//! If the velocity signal is not suppressed than brake lights will be deactivated after the delay expired \n
//! and the absolute deceleration will be under the absolute threshold \ ref u16AbsDeactDecThr \n
//!
//!
//! ### Input/Output example
//! Here is one input example where the current velocity stay the same. \n
//! So the brake light depends only on deflection of the brake pedal: \n
//! \htmlonly<img width="60%" src="BrakeLight_Input.png" alt="BrakeLight_Input.png">\endhtmlonly
//! \htmlonly <br clear="all"> \endhtmlonly
//! \n
//! The output will be the following for state of brake light: \n
//! \htmlonly<img width="60%" src="BrakeLight_outputBL.png" alt="BrakeLight_outputBL.png">\endhtmlonly
//! \htmlonly <br clear="all"> \endhtmlonly
//! \n
//! And the output for filtered acceleration: \n
//! \htmlonly<img width="60%" src="BrakeLight_OutputAccCur.png" alt="BrakeLight_OutputAccCur.png">\endhtmlonly
//! \htmlonly <br clear="all"> \endhtmlonly
//!\n
//! ### Behavior on input error
//! The Block has special behavior on input error.\n
//! The table below shows additional input characteristics, which defines this behavior.\n
//! \n
//! \htmlonly<img width="70%" src="BrakeLight_Tabelle.png" alt="BrakeLight_Tabelle.png">\endhtmlonly
//! \htmlonly <br clear="all"> \endhtmlonly
//! \n
//! If input is out of valid range and in total value range than calculation will be done with limit values. \n
//! If input is not critical and is set to error / undefined values or out of range than input will be set to default.\n
//! Calculation will be done with default values. \n
//! If input value as critical value defined and set to error, undefined values or out of total range than\n
//! no calculation will be done. The last valid acceleration will be set to output. The state of brake light \n
//! should be set to ON. \n
//! \n
//! ### PDT block settings
//!
//! <div class="divMono first_column_left"></div>
//!| Name                                                                                                      | Unit        | Range            | Default value |
//!|-----------------------------------------------------------------------------------------------------------|-------------|------------------|---------------|
//!| \b Common                                                                                                 |             |                  |               |
//!| Block name                                                                                                | char        | 0 ... 32         | N/A           |
//!| <a href="blo.html#eBloSta">eBloProc - Initial Block state</a>                                             | enum        |                  | BLO_RELEASE   |
//!| Block description                                                                                         | char        | 0 ... 32         | Description   |
//!| <b>Parameters</b>                                                                                         |             |                  |               |
//!| \ref TBrakeLightPar::u16BrakeMin "Minimum brake pedal deflection"                                         | [0.001]     | 0 ... 65535      | 200           |
//!| \ref TBrakeLightPar::u16AbsActDecThr "Threshold value of deceleration to activate the brake lights"       | [0.01m/s2]  | 0 ... 65535      | 100           |
//!| \ref TBrakeLightPar::u16AbsDeactDecThr "Threshold value of deceleration to deactivate the brake lights"   | [0.01m/s2]  | 0 ... 65535      | 100           |
//!| <b>Properties</b>                                                                                         |             |                  |               |
//!| \ref TBrakeLightPrp::u32DelayTimer "Delay to deactivate brake lights"                                     | [ms]        | 0 ... 4294967295 | 1000          |
//!| \ref TBrakeLightPrp::u16AccFilterConst "Acceleration low pass filter constant"                            | [num]       | 0 ... 65535      | 2000          |
//! \n
//! ### How to Use
//! 1. A Block has to be created in "Project Definition Tool" (PDT).
//! 2. Main input value have to be set to the \ref TBrakeLightInp "input structure" .
//! 3. The Block calculation function `eBloBrakeLight()` has to be called
//!    in run-time phase `vAppRun`.
//! 4. \ref TBrakeLightOut "Output values" may be used for a further calculation.
//!
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//! TVoid vAppRun(TCoreInp *ptCoreInp, TCoreOut *ptCoreOut)
//! {
//!     // ...
//!     ERetVal eRetVal;
//!
//!     // define variable that will store output value
//!     TInt16 i16AccCur;
//!
//!     // get a Block pointer
//!     TBloBrakeLight* ptBlo = &gBrakeLight_tBrakeLight_01;
//!
//!     // set obligatory inputs
//!     ptBlo->tInp.i16CurrentVelocity = 1500;
//!     //...
//!
//!     // call a Block
//!     eRetVal = eBloBrakeLight(ptBlo);
//!
//!     // get output values
//!     i16AccCur = ptBlo->tOut.i16AccCur;
//!
//!     // ...
//! }
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//!
//! ### Update of parameters
//! Any parameter that is stored in parameter structure `tPar` can be\n
//! updated at the run-time. \n
//! For this can the function eBloBrakeLightSetPar() can be used.\n
//!
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//! TVoid vAppRun(TCoreInp *ptCoreInp, TCoreOut *ptCoreOut)
//! {
//!      // ...
//!      ERetVal eRetVal;
//!
//!      // get Block pointer
//!      TBloBrakeLight* ptBlo = &gBrakeLight_tBrakeLight_01;
//!
//!      // set new parameter to ptPar
//!      TBrakeLightPar tPar;
//!      tPar.u16BrakeMin = 200;
//!
//!      // get Parameter structure pointer
//!      const TBrakeLightPar * const cptPar = &tPar;
//!
//!      // set new parameters
//!      eBloBrakeLightSetPar(ptBlo, cptPar);
//!
//!      // call a Block
//!      eRetVal = eBloBrakeLight(ptBlo);
//!
//!      // check returned value
//!      if (R_OKAY != eRetVal)
//!      {
//!        // define behavior for an update error case
//!      }
//!
//!      // ...
//! }
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//!
//!
//! ### Initialization of properties and parameters
//! It is also possible to change the properties `tPrp` and parameter `tPar` \n
//! defined in PDT using manual call of re-initialization function eBloBrakeLightReInit(). \n
//! - All changed properties have to be set into a block data bank \n
//! - All changed parameters should be passed with the function \n
//! - All values will be checked and saved inside of the Block.
//!
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//! TVoid vAppRun(TVoid)
//! {
//!        // ...
//!        ERetVal eRetVal;
//!        // get block pointer
//!        TBloBrakeLight* ptBlo = &gBrakeLight_tBrakeLight_01;
//!
//!        // set new parameter to ptPar structure
//!        TBrakeLightPar tPar;
//!        tPar.u16BrakeMin = 200;
//!
//!        // get parameter structure pointer
//!        const TBrakeLightPar * const cptPar = &tPar;
//!
//!        // call re-initialization
//!         eRetVal = eBloBrakeLightReInit(ptBlo, cptPar);
//!
//!        // check returned value
//!        if (R_OKAY != eRetVal)
//!        {
//!          // define behavior for an update error case
//!        }
//!
//!        // ...
//! }
//! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//!
//! @warning
//! This function eBloBrakeLightReInit will reset the entire block state.
//! If any parameter values are invalid the old parameter values are restored
//! and the re-initialization of the block will not be performed.
//! Be aware that it will reset all input variables, output variables and error states.\n
//!
//! @created    05.07.21
//!
//! @changelog
//! -   1.0.0.0  05.07.21
//!     Created Block BloBrakeLight
//! -   1.0.1.0  05.04.24
//!     Added boSuppressVeloSig to input structure
//!-    1.3.0.0  05.04.24
//!    - The MS block interface version is updated to 1.14
//!-    1.3.1.0  31.05.24
//!    - Renamed input u16Brake to u16BrakePedal
//!-    1.3.2.0  20.08.24
//!    - Added deceleration threshold to deactivate the brake light
//!-   1.4.0.0  02.10.2024
//!    - The MS block interface version is updated to 1.15
//!-   1.4.1.0 20.02.2025
//!    - Updated after the last standard
//**************************************************************************************************

#ifndef __ITFBLOBRAKELIGHT__
    #define __ITFBLOBRAKELIGHT__


// Includes ========================================================================================

#include <ItfGlobal.h>
#include <ItfTypes.h>
#include <ItfCoreDb.h>
#include <ItfCoreLib.h>
#include <ItfBasOut.h>
#include <ItfBasConv.h>


#include <ItfBasEleTimer.h>
#include <ItfSigGdGlobal.h>
#include <ItfSigGdGlobalPrv.h>

#ifndef __FUNCBLOCK__   // Define error structs once for functions blocks globally
    #define __FUNCBLOCK__

typedef struct{
    TUint16 u16Warning;    // bit - Blocks warning triggered
    TUint16 u16Error;      // bit - Blocks error triggered
 } TBlockErrorState;

 typedef struct{
    TUint16 u16Warning;    // bit - input is out of range
    TUint16 u16Error;      // bit - input is out of range and out of tolerance
    TUint16 u16StopFunc;   // bit - input triggers function stop
 } TInputErrorState;
#endif


// Enumerations ====================================================================================

// =================================================================================================
//! @brief  Bit positions to identify each input signal within the block info structure
//!
//! @details  The following enumerations define the position in the bit fields u8InpError
//!           that correspond to the notifications regarding a particular input
//!           variable.
// =================================================================================================
typedef enum
{
    //! @brief Bit position for u16BrakePedal
    //!
    INP_BRAKELIGHT_BRAKE_PEDAL = 0,
    
    //! @brief Bit position for i16CurrentVelocity
    //!
    INP_BRAKELIGHT_CURRENT_VELOCITY = 1
    
}EBloBrakeLightInpVar;



// =================================================================================================
//! @brief Bit positions to identify individual block errors
//!
//! @details  The following enumerations define the bit positions in the bit field u16BloError
//!           to identify individual errors that may have been detected by the block
//!
// =================================================================================================
typedef enum
{
    
    //! @brief Unexpected error 
    //!
    //! @details This error is triggered if an unexpected error is detected. If this error occurs 
    //!          the blocks functionality will be locked. The block will go into a safe state. 
    //!          Please refer to the blocks documentation to find out more about the safe state of 
    //!          this particular block. 
    DM_ERR_BRAKELIGHT_INTERNAL = 0,
   
    //! @brief Invalid configuration values 
    //!
    //! @details This error occurs if any of the configuration values are invalid or can not be read 
    //!          from the configuration structure while the block is being created. If this error 
    //!          occurs the creation process of the block will be aborted. For additional 
    //!          information check the debug message or the return value of the create function.  
    DM_ERR_BRAKELIGHT_CONFIG = 1

}EBloBrakeLightErr;



// =================================================================================================
//! @brief Bit positions to identify individual block warnings
//!
//! @details The following enumerations define the bit positions in the bit field u16BloWarning that
//!          are set if a particular warning is active
// =================================================================================================
typedef enum
{
    
    //! @brief Block has not successfully been created 
    //!
    //! @details This warning is active as long as the block has not successfully been created.
    DM_WAR_BRAKELIGHT_NOT_CREATED = 0,
    
    //! @brief Block has not successfully been created 
    //!
    //! @details This warning is active as long as the block has not successfully been initialized.
    DM_WAR_BRAKELIGHT_NOT_INITIALIZED  = 1,
    
    //! @brief Invalid configuration values 
    //!
    //! @details This warning is triggered if the configuration of the block was attempted to be 
    //!          updated with at least one invalid configuration value after the block was 
    //!          successfully initialized. If this warning occurs the update or reinitialization 
    //!          process will be aborted and the block will continue to function with the last 
    //!          valid configuration. The warning will be reset the next time the configuration 
    //!          values are successfully updated
    DM_WAR_BRAKELIGHT_CONFIG = 2

}EBloBrakeLightWar;


// Structures ======================================================================================

// =================================================================================================
//! @brief Input structure
//!
//! @details See the documentation for the valid range and individual error behavior of each input
//!          signal.
//!
//! @note Note, that all input variables will be set to the type specific value for undefined during
//!       the start-up and (re-)initialization phase. This however does not apply for the block
//!       status (eBloSta)
// =================================================================================================
typedef struct
{
    //! @brief [enu] Input Block State
    //!
    EBloStatus  eBloSta;

    //! @brief [0,1%] Deflection of the brake pedal 
    //!
    TUint16  u16BrakePedal;

    //! @brief [0.01km/h] Current velocity of the vehicle 
    //!
    TInt16  i16CurrentVelocity;

    //! @brief [boo] Suppress the velocity signal 
    //!
    //! @detailsFor determination of brake light state will be only the deflection of the brake 
    //!pedal used 
    TBoolean  boSuppressVeloSig;

    

}TBrakeLightInp;



//! @brief [stu] Current Block input, warning and error state
//!
//! @note The error bit information is not debounced.
//!
//! @sa EBloBrakeLightWar, EBloBrakeLightErr
typedef struct{
    TUint16 u16InpWarning;  //!< @brief [bit] Input value out of range
    TUint16 u16InpError;    //!< @brief [bit] Input value out of tolerance range
    TUint16 u16BloWarning;  //!< @brief [bit] Block warning detected
    TUint16 u16BloError;    //!< @brief [bit] Block error detected 

} TBrakeLightInfo;


// =================================================================================================
//! @brief Output structure of the block
//!
//! @details The values within the output structure will be recalculated every time the main block
//!          function eBloBrakeLight(...) is executed.
//!
//! @note Note, that all output variables will be set to the type specific value for undefined
//!       during the start-up and (re-)initialization phase. This however does not apply for the
//!       block status (eBloSta) and the block information field (tBloInfo)
// =================================================================================================
typedef struct
{
    //! @brief [enu] Current block State
    //!
    EBloStatus  eBloSta;
    
    
    //! @brief [0.01m/s^2] Current acceleration 
    //!
    //! @detailsThis current acceleration is filtered with a low pass filter 
    TInt16  i16AccCur;

    //! @brief [ON/OFF] State of brake light 
    //!
    TBit2  bi2BrakeLight;

    
    TBrakeLightInfo tBloInfo; //!< current Block input, warning and error state

    

}TBrakeLightOut;



// =================================================================================================
//! @brief Parameter values
//!
//! @details Parameters are configuration values that may be modified at any time during the
//!          application life cycle.
//!
//! @note Note, that if a parameter update fails, the block will continue with the last valid
//!       configuration. A failed update attempt will be indicated via the block's info structure.
// =================================================================================================
typedef struct
{
    //! @brief [0.01m/s^2] Absolute threshold value of deceleration to activate the brake lights 
    //!
    //! @detailsThis value shall be above the u16AbsDeactDecThr parameter.If the deceleration above 
    //!this value than the brake light is activated  
    TUint16 u16AbsActDecThr;

    //! @brief [0.01m/s^2]  Absolute threshold value of deceleration to deactivate the brake lights 
    //!
    //! @detailsThis value shall be under the u16AbsActDecThr parameter. If the deceleration under 
    //!this value and the timer is expired than the brake light is deactivated 
    TUint16 u16AbsDeactDecThr;

    //! @brief [0.001] Minimum brake pedal deflection 
    //!
    TUint16 u16BrakeMin;

}TBrakeLightPar;



// =================================================================================================
//! @brief Parameter configuration structure
//!
//! @details This structure contains the database links and default values for all parameters as
//!          they were defined within the PDT. The value of a database link may be modified via
//!          the MST.
//!
//! @note Note, that these values will automatically be used for the block configuration, when the
//!       block is being initialized
// =================================================================================================
typedef struct
{
    //! @brief [0.01m/s^2] Absolute threshold value of deceleration to activate the brake lights 
    //!
    //! @detailsThis value shall be above the u16AbsDeactDecThr parameter.If the deceleration above 
    //!this value than the brake light is activated  
    TDbLinkU16Var tAbsActDecThr;
    
    //! @brief [0.01m/s^2]  Absolute threshold value of deceleration to deactivate the brake lights 
    //!
    //! @detailsThis value shall be under the u16AbsActDecThr parameter. If the deceleration under 
    //!this value and the timer is expired than the brake light is deactivated 
    TDbLinkU16Var tAbsDeactDecThr;
    
    //! @brief [0.001] Minimum brake pedal deflection 
    //!
    TDbLinkU16Var tBrakeMin;
    
}TBrakeLightParCfg;



// =================================================================================================
//! @brief Property values
//!
//! @details Properties are configuration values which cannot be modified during run time. The block
//!          will copy the property values from the configuration structure only when the block is
//!          being (re-)initialized
//!
//! @note Note, that if a property update fails, the block will continue with the last valid
//!       configuration. A failed update attempt will be indicated via the block's info structure.
// =================================================================================================
typedef struct
{
    
    //! @brief [num] Acceleration low pass filter constant 
    //!
    TUint16 u16AccFilterConst;

    //! @brief [ms] Delay to deactivate brake lights 
    //!
    //! @detailsIf the deceleration under this value and the timer is expired than the brake light 
    //!is deactivated 
    TUint32 u32DelayTimer;

}TBrakeLightPrp;



// =================================================================================================
//! @brief Property configuration structure
//!
//! @details This structure contains the database links and default values for all properties as
//!          they were defined within the PDT. The value of a database link may be modified via
//!          the MST.
//!
//! @note Note, that these values will automatically be used for the block configuration, when the
//!       block is being (re-)initialized
// =================================================================================================
typedef struct
{
    
    //! @brief [num] Acceleration low pass filter constant 
    //!
    TDbLinkU16Var tAccFilterConst;
    
    //! @brief [ms] Delay to deactivate brake lights 
    //!
    //! @detailsIf the deceleration under this value and the timer is expired than the brake light 
    //!is deactivated 
    TDbLinkU32Var tDelayTimer;
    

}TBrakeLightPrpCfg;



// =================================================================================================
//! @brief Overall Block configuration structure
//!
// =================================================================================================
typedef struct
{
    //! @brief [str] Block name.
    //!
    //! @details Custom name for a particular block instance
    TChar achName[BLO_NAME_STR_LEN];

    //! @brief [enu] Block Initial State
    //!
    //! @details Depending on the selected initial block state the block will automatically be
    //!          created and initialized. The following table gives an overview of the different
    //!          options that are available available:
    //!
    //! | Option         | Creation       | Initialization | Configuration Structure         |
    //! |----------------|----------------|----------------|---------------------------------|
    //! | BLO_RELEASE    |  automatically | automatically  | const (stored in FLASH memory)  |
    //! | BLO_LOCKED     |  automatically | automatically  | const (stored in FLASH memory)  |
    //! | BLO_FREEZE_INP |  automatically | automatically  | const (stored in FLASH memory)  |
    //! | BLO_FREEZE_OUT |  automatically | automatically  | const (stored in FLASH memory)  |
    //! | BLO_NOT_INIT   |  automatically | manually       | const (stored in FLASH memory)  |
    //! | BLO_NA         |  manually      | manually       | not const (stored in RAM memory)|
    //!
    EBloStatus eBloSta;

    //! @brief [-] \ref TBrakeLightPar PDT interface structure for block parameter
    //!
    TBrakeLightParCfg tPar;

    //! @brief [-] \ref TBrakeLightPrp PDT interface structure for block properties
    //!
    TBrakeLightPrpCfg tPrp;

    

}TBrakeLightCfg;



// =================================================================================================
//! @brief Block address structure
//!
// =================================================================================================
typedef struct
{
    //! @brief [stu] Pointer to the block configuration structure.
    //!
    const TBrakeLightCfg * cptCfg;

    //! @brief [stu] Pointer to the private block object.
    //!
    TVoid * pvObj;

    //! @brief [num] Block Stamp.
    //!
    //! Registration stamp
    TUint16 u16Stamp;

}TBrakeLightAdr;



// =================================================================================================
//! @brief Main overall block structure
//!
// =================================================================================================
typedef struct
{
    TBrakeLightInp  tInp;   //!< [stu] Input structure
    TBrakeLightOut  tOut;   //!< [stu] Output structure
    TBrakeLightAdr  tXAdr;  //!< [stu] Address structure. !For internal use only!

}TBloBrakeLight;



// Block Interfaces ================================================================================

// =================================================================================================
//! @brief Get the version details of the block
//!
//! @retval *cptVer - Pointer to version-date information structure.
// =================================================================================================
extern const TVerChapCom *cptBloBrakeLightVersionsInfo(TVoid);



// =================================================================================================
//! @brief Check if block is compatible with the currently used MS version
//!
//! @retval TRUE  - Block is compatible with the currently used MS version
//! @retval FALSE - Block is not compatible with the currently used MS version
// =================================================================================================
extern TBoolean boBloBrakeLightVersionCheck(TVoid);



// =================================================================================================
//! @brief  Create, initialize and register the block.
//!
//! @pre  Block must be defined in the Project Definition Tool (PDT).
//!
//! @note  This function will be called by the "auto code".
//!
//! @param[in,out] cpvBlo - [PNT] Pointer to a Block interface structure.
//!
//! @retval R_OKAY         - The block was successfully created and initialized.
//! @retval R_NULL_POINTER - Null pointer argument.
//! @retval R_NOT_REGISTRY - The block is not registered.
//! @retval R_MEMORY       - No enough memory is available to create the block.
//! @retval R_ADDRESS      - The address pointer of the block is invalid.
//! @retval R_CONFIG       - Error while copying the configuration values. See error bit field.
//!                          or debug message for more details.
//! @retval R_UNKOWN       - Internal error. See debug message for more details.
// =================================================================================================
extern ERetVal eBloBrakeLightCreateInitRegistry(TVoid * const pcvBlo);



// =================================================================================================
//! @brief  Create block.
//!
//! @details  <a href="blo.html#block_create">Only for manual creation</a>.
//!
//! @pre Blocktype must be registered in MS environment.
//!
//! @note  This function must be called in the startup phase.
//!
//! @param[in,out] pctBlo    - [PNT] Main block structure
//! @param[in]     cpctCfg   - [PNT] Block configuration structure
//!
//! @retval R_OKAY          - The block was successfully created.
//! @retval R_NULL_POINTER  - At least one of the provided arguments is a null pointer.
//! @retval R_NOT_REGISTRY  - The block is not registered.
//! @retval R_MEMORY        - No enough memory is available to create the block.
//! @retval R_CONFIG
//!     - Error when copying property or parameter values. 
//!       See error bit field or debug message for more details.
//!     - Error element could not be created.
//! @retval R_PHASE         - Wrong application phase to create the block. The create function must
//!                           be called in the start-up phase.
//! @See also eBloBrakeLightAppItfCreate
// =================================================================================================
extern  ERetVal eBloBrakeLightCreate(      TBloBrakeLight * const pctBlo,
                                     const TBrakeLightCfg * const cpctCfg);



// =================================================================================================
//! @brief  Initialize block.
//!
//! @details  <a href="blo.html#block_init">Only for manual initialization</a>.
//!
//! @pre  The block must have been successfully created beforehand.
//!
//! @param[in,out] pctBlo - [PNT] Pointer to a Block interface structure.
//!
//! @retval R_OKAY         - The block was successfully initialized.
//! @retval R_NULL_POINTER - Null pointer argument.
//! @retval R_CONFIG       - No valid configuration available. See error bit field for more details.
//! @retval R_NOT_REGISTRY - The block is not registered.
//! @retval R_ADDRESS      - The address pointer of the block is invalid.
//! @retval R_UNKNOWN      - Block is not created
//! @See also eBloBrakeLightAppItfInit
// =================================================================================================
extern ERetVal eBloBrakeLightInit(TBloBrakeLight * const pctBlo);



// =================================================================================================
//! @brief Main block function to calculate all output values.
//!
//! @details  Function has to be called in every ecu cycle.
//!           <a href="blo.html#block_run">See also</a>.\n
//!
//! @pre   The block must have been successfully created and initialized before calling this
//!        function.
//!
//! @note  - Function should be called only once per controller cycle.
//!        - Function should be used in application run(-time) phase (vAppRun(...)).
//!
//! @param[in,out] pctBlo - [PNT] Main block structure.
//!
//! @retval R_OKAY            - The block was successfully executed.
//! @retval R_NULL_POINTER    - Null pointer argument.
//! @retval R_NOT_INITIALIZED - The block has not yet been successfully initialized.
//! @retval R_NOT_REGISTRY    - The block is not registered.
//! @retval R_ADDRESS         - The address pointer of the block is invalid.
//! @retval R_NOACT           - The block functionality is locked, because the state is either set
//!                             to BLO_LOCKED or BLO_NA.
//! @retval R_LOCKED          - The block functionality was locked due to an input error.
// =================================================================================================
extern ERetVal  eBloBrakeLight(TBloBrakeLight * const pctBlo);



// =================================================================================================
//! @brief  Reinitialize the block.
//!
//! @details  The block will be reinitialized by using the provided parameter values and 
//!
//! @note  Resets the entire block state. If any parameter values are
//!        invalid the old parameter values are restored and the re-initialization of the block will
//!        not be performed.  
//!
//! @param[in,out] pctBlo  - [PNT] Main block interface structure.
//! @param[in]     cpctPar - [PNT] New parameter for the block
//!
//! @retval R_OKAY            - The block was successfully reinitialized.
//! @retval R_NULL_POINTER    - Null pointer argument.
//! @retval R_NOT_INITIALIZED - The block has not yet been successfully initialized.
//! @retval R_NOT_REGISTRY    - The block is not registered.
//! @retval R_ADDRESS         - The address pointer of the block is invalid.
//! @retval R_DB_LIST         - Cannot read configuration values from database links
//! @retval R_CONFIG          - At least one of the configuration values is invalid. See block info
//!                             field for more details.
//! @retval R_UNKNOWN         - The block not created
//! @See also eBloBrakeLightAppItfInit
// =================================================================================================
extern ERetVal eBloBrakeLightReInit(      TBloBrakeLight    * const pctBlo,
                                    const TBrakeLightPar * const cpctPar);



// =================================================================================================
//! @brief Checks if a given set of parameter values meet the mandatory restrictions
//!
//! @param[in] cpctPar - [PNT] Parameter values
//!
//! @retval R_OKAY         - All parameter values meet the mandatory restrictions
//! @retval R_CONFIG       - At least one parameter value does not meet the mandatory restrictions
//! @retval R_NULL_POINTER - Null pointer argument.
// =================================================================================================
extern ERetVal eBloBrakeLightCheckPar(TBloBrakeLight * const pctBlo,
		                        const TBrakeLightPar * const cpctPar);



// =================================================================================================
//! @brief Checks if a given set of property values meet the mandatory restrictions
//!
//! @param[in] cpctPrp - [PNT] Property values
//!
//! @retval R_OKAY         - All property values meet the mandatory restrictions
//! @retval R_CONFIG       - At least one property value does not meet the mandatory restrictions
//! @retval R_NULL_POINTER - Null pointer argument.
// =================================================================================================
extern ERetVal eBloBrakeLightCheckPrp(TBloBrakeLight * const pctBlo,
		                        const TBrakeLightPrp * const cpctPrp);



// =================================================================================================
//! @brief Set new parameter values.
//!
//! @pre   The block must have been successfully created and initialized before calling this
//!        function.
//!
//! @param[in,out] pctBlo     - [PNT] Main block interface structure.
//! @param[in]     cpctParSrc - [PNT] New parameter values.
//!
//! @retval R_OKAY            - All parameter values have successfully been updated.
//! @retval R_NULL_POINTER    - Null pointer argument.
//! @retval R_NOT_INITIALIZED - The block has not yet been successfully initialized.
//! @retval R_ADDRESS         - Invalid address of object.
//! @retval R_NOT_REGISTRY    - Object is not registered.
//! @retval R_CONFIG          - At least one parameter value is invalid.
// =================================================================================================
extern ERetVal eBloBrakeLightSetPar(      TBloBrakeLight    * const pctBlo,
                                    const TBrakeLightPar * const cpctParSrc);



// =================================================================================================
//! @brief Get currently used parameter values.
//!
//! @pre   The block must have been successfully created and initialized before calling this
//!        function.
//!
//! @param[in]  cpctBlo   - [PNT] Main block interface structure.
//! @param[out] cptParDSt - [PNT] Target structure for parameter values.
//!
//! @retval R_OKAY            - All parameter values have successfully been copied to the target
//!                             structure.
//! @retval R_NULL_POINTER    - Null pointer argument.
//! @retval R_NOT_INITIALIZED - The block has not yet been successfully initialized.
//! @retval R_ADDRESS         - Invalid address of object.
//! @retval R_NOT_REGISTRY    - Object is not registered.
// =================================================================================================
extern ERetVal eBloBrakeLightGetPar(const TBloBrakeLight    * const cpctBlo,
                                          TBrakeLightPar * const pctParDst);



// =================================================================================================
//! @brief Get currently used property values.
//!
//! @pre   The block must have been successfully created and initialized before calling this
//!        function.
//!
//! @param[in]  cpctBlo   - [PNT] Main block interface structure.
//! @param[out] cptPrpDst - [PNT] Target structure for property values.
//!
//! @retval R_OKAY            - All property values have successfully been copied to the target
//!                             structure.
//! @retval R_NULL_POINTER    - Null pointer argument.
//! @retval R_NOT_INITIALIZED - The block has not yet been successfully initialized.
//! @retval R_ADDRESS         - Invalid address of object.
//! @retval R_NOT_REGISTRY    - Object is not registered.
// =================================================================================================
extern ERetVal eBloBrakeLightGetPrp(const TBloBrakeLight    * const cpctBlo,
                                          TBrakeLightPrp * const pctPrpDst);



// =================================================================================================
//! @brief Get the parameter values that are stored in the configuration structure.
//!
//! @param[in]  cpctBlo   - [PNT] Main block interface structure.
//! @param[out] cptParDst - [PNT] Target structure for the parameter values.
//!
//! @retval R_OKAY         - All parameter values were successfully copied to the target structure.
//! @retval R_NULL_POINTER - Null pointer argument.
//! @retval R_ADDRESS         - Invalid address of object.
//! @retval R_NOT_REGISTRY    - Object is not registered.
//! @retval R_DB_LIST      - List index out of range.
//! @retval R_DB_VAR       - Variable index out of range.
//! @retval R_DB_DIM       - Array index out of range.
//! @retval R_SUPPORT      - no variable function supported.
//! @retval R_INCONSISTENT - DB list is inconsistent.
//! @retval R_PARAMETER    - eVarTyp don't MS with the type in the db-list.
// =================================================================================================
extern ERetVal eBloBrakeLightGetCfgPar(const TBloBrakeLight    * const cpctBlo,
                                             TBrakeLightPar * const pctParDst);



// =================================================================================================
//! @brief Get the property values that are stored in the configuration structure.
//!
//! @param[in]  cpctBlo   - [PNT] Main block interface structure.
//! @param[out] cptPrpDst - [PNT] Target structure for the property values.
//!
//! @retval R_OKAY         - All parameter values were successfully copied to the target structure.
//! @retval R_NULL_POINTER - Null pointer argument.
//! @retval R_ADDRESS         - Invalid address of object.
//! @retval R_NOT_REGISTRY    - Object is not registered.
//! @retval R_DB_LIST      - List index out of range.
//! @retval R_DB_VAR       - Variable index out of range.
//! @retval R_DB_DIM       - Array index out of range.
//! @retval R_SUPPORT      - no variable function supported.
//! @retval R_INCONSISTENT - DB list is inconsistent.
//! @retval R_PARAMETER    - eVarTyp don't MS with the type in the db-list.
// =================================================================================================
extern ERetVal eBloBrakeLightGetCfgPrp(const TBloBrakeLight    * const cpctBlo,
                                             TBrakeLightPrp * const pctPrpDst);



// =================================================================================================
//! @brief Get the input warning state for a particular input variable
//!
//! @details An input warning is triggered if the value of a particular input variable is outside 
//!          the specified valid range, but still within the tolerance range.
//!
//! @note In case a warning is active, the value of that particular input variable will be bound to 
//!       the specified valid range for all internal calculations
//!
//! @param[in] cpctAdr   - [PNT] Block address structure
//! @param[in] ceInpVar - [ENU] Selected input variable
//!
//! @retval BI2_ON    - Warning is active
//! @retval BI2_OFF   - Warning is not active
//! @retval BI2_UNDEF - The address pointer is a null pointer
//! @retval BI2_ERROR - The block has not yet successfully been created
// =================================================================================================
extern TBit2 bi2BloBrakeLightGetInpWar(const TBrakeLightAdr    * const cpctAdr,
                                       const EBloBrakeLightInpVar         ceInpVar);



// =================================================================================================
//! @brief Get the input error state for a particular input variable
//!
//! @details An input error is triggered if the value of a particular input variable is outside 
//!          the specified valid and tolerance range.
//!
//! @note Refer to the documentation for more details about the resulting error reaction regarding a
//!       particular input variable
//!
//! @param[in] cpctAdr   - [PNT] Block address structure
//! @param[in] ceInpVar - [ENU] Selected input variable
//!
//! @retval BI2_ON    - Error is active
//! @retval BI2_OFF   - Error is not active
//! @retval BI2_UNDEF - The address pointer is a null pointer
//! @retval BI2_ERROR - The block has not yet successfully been created
// =================================================================================================
extern TBit2 bi2BloBrakeLightGetInpErr(const TBrakeLightAdr    * const cpctAdr,
                                       const EBloBrakeLightInpVar         ceInpVar);



// =================================================================================================
//! @brief Get the state of a particular block warning
//!
//! @details See EBloBrakeLightWar for a list of all available warnings
//!
//! @param[in] cpctAdr   - [PNT] Block address structure
//! @param[in] ceBloWar - [ENU] Selected block warning
//!
//! @retval BI2_ON    - Warning is active
//! @retval BI2_OFF   - Warning is not active
//! @retval BI2_UNDEF - The address pointer is a null pointer
//! @retval BI2_ERROR - The block has not yet successfully been created
// =================================================================================================
extern TBit2 bi2BloBrakeLightGetBloWar(const TBrakeLightAdr * const cpctAdr,
                                       const EBloBrakeLightWar         ceBloWar);



// =================================================================================================
//! @brief Get the state of a particular block error
//!
//! @details See EBloBrakeLightErr for a list of all available errors
//!
//! @note Refer to the documentation for more details about the resulting error reaction regarding a
//!       particular block error
//!
//! @param[in] cpctAdr   - [PNT] Block address structure
//! @param[in] ceBloErr - [ENU] Selected block error
//!
//! @retval BI2_ON    - Error is active
//! @retval BI2_OFF   - Error is not active
//! @retval BI2_UNDEF - The address pointer is a null pointer
//! @retval BI2_ERROR - The block has not yet successfully been created
// =================================================================================================
extern TBit2 bi2BloBrakeLightGetBloErr(const TBrakeLightAdr * const cpctAdr,
                                       const EBloBrakeLightErr         ceBloErr);



// =================================================================================================
//! @brief Set the state of a particular block warning
//!
//! @details See EBloBrakeLightWar for a list of all available warnings
//!
//! @param[in] cptAdr   - [PNT] Block address structure
//! @param[in] ceBloWar - [ENU] Selected block warning
//! @param[in] cboState - [BOO] Target state of the warning (TRUE = Active, FALSE = Inactive)
//!
//! @retval R_OKAY         - Error state was successfully modified
//! @retval R_NULL_POINTER - The adress pointer is a null pointer or the block has not successfully
//!                          been created
// =================================================================================================
extern ERetVal eBloBrakeLightSetBloWar(      TBrakeLightAdr * const pctAdr,
                                       const EBloBrakeLightWar         ceBloWar,
                                       const TBoolean                  cboState);



// =================================================================================================
//! @brief Set the state of a particular block error
//!
//! @details See EBloBrakeLightErr for a list of all available errors
//!
//! @note Refer to the documentation for more details about the resulting error reaction regarding a
//!       particular block error
//!
//! @param[in] cptAdr   - [PNT] Block address structure
//! @param[in] ceBloErr - [ENU] Selected block error
//! @param[in] cboState - [BOO] Target state of the error (TRUE = Active, FALSE = Inactive)
//!
//! @retval R_OKAY         - Error state was successfully modified
//! @retval R_NULL_POINTER - The adress pointer is a null pointer or the block has not successfully
//!                          been created
// =================================================================================================
extern ERetVal eBloBrakeLightSetBloErr(      TBrakeLightAdr * const pctAdr,
                                       const EBloBrakeLightErr         ceBloErr,
                                       const TBoolean                  cboState);








#endif
