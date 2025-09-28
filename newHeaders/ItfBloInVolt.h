//**************************************************************************************************
/*!
@file    ItfBloInVolt.h
@version 1.16.3.0
@brief   \htmlonly
<span><img class="imgBriefImage" src="BloInVolt32x32.png" alt="BloInVolt32x32.png"></span>
<span class="spanBriefText">Block "Voltage Input" (<b>BloInVolt</b>)</span>
\endhtmlonly

\n
\htmlonly
<img src="BloInVoltSchematic.png" width="500"  alt="into", align="left" > 
\endhtmlonly
\n\n\n\n\n\n\n\n\n\n\n
The InVolt block can be attached to a PIN delivering an analog signal (e.g. 0...5 [V]). The block\n 
will deliver a signal and a direction. The signal can be customized within the limits of Tint16. The\n 
input block can be configured in order to deliver a single direction signal (default 0...1000 [?]) or\n
a double direction signal (default -1000...+1000 [?]). A predefined direction output is delivered\n 
in addition to the signal. This is used as a parallel path to detect controller errors.Both\n 
characteristics are arrays that describe a positive(POS) and a negative(NEG) area as well as a \n
neutral(NEU) position, in mV, e.g.:
|ai16InChar         |[3]        |
|:--:               |--         |
|       4500        |[0] POS    |
|       2500        |[1] NEU    |
|       500         |[2] NEG    | 

\b Examples: \n
\b a) The configuration can be chosen in a way that an input range gets scaled to 0..1000:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       4500        |[0] POS    |1000       |[0]|
|       500         |[1] NEU    |0          |[1]|
|       500         |[2] NEG    |0          |[2]|
\n
\htmlonly <img src="BloInVoltSingleDirPos.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n\n

\b Examples: \n
\b b) The configuration can be chosen in a way that an input range gets scaled to -1000..1000:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       4500        |[0] POS    |1000       |[0]|
|       2500        |[1] NEU    |0          |[1]|
|       500         |[2] NEG    |-1000      |[2]|
\n
\htmlonly <img src="BloInVoltDualDir.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n\n
\n\b Examples:
\b c) The configuration can be chosen in a way that an input range gets scaled to 1000..0:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       500         |[0] POS    |0          |[0]|
|       500         |[1] NEU    |0          |[1]|
|       4500        |[2] NEG    |-1000      |[2]|
\n
\htmlonly <img src="BloInVoltSingleDirNeg.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n\n

@created    22.10.2013
@changelog
-   1.5.0.0  09.02.2017  
    - The MATCH block interface version is updated to 1.4.
-   1.5.2.0  15.05.2017  
    - The state of a block pin is added in the output structure.
-   1.6.0.0  16.05.2017  
    - The MATCH block interface version is updated to 1.5.
-   1.7.0.0  26.06.2018  
    - The MATCH block interface version is updated to 1.6.
-   1.7.1.0  14.05.2019  
    - Improved pointer error diagnostics (#26971).
-   1.8.0.0 18.06.2019
    - The MATCH block interface version is updated to 1.7.
-   1.9.0.0 25.02.2020
    - The MATCH block interface version is updated to 1.8.    
-   1.10.0.0    26.06.2020
    - The MATCH block interface version is updated to 1.9.
-   1.11.1.0    25.08.2020  
    - The MATCH block interface version is updated to 1.10.
-   1.12.0.0  04.05.2021  
    - The MATCH block interface version is updated to 1.11
-   1.12.1.0    11.06.2021
    - \ref DM_INVOLT_PARAM is now reset when valid update is requested. (#50422)
-   1.13.0.0  10.01.2022  
    - The MATCH block interface version is updated to 1.12
-   1.14.0.0  01.08.2022  
    - The MATCH block interface version is updated to 1.13
-   1.15.1.0  07.09.2023  
    - The MATCH block interface version is updated to 1.14
-   1.16.1.0  27.09.2024 
    - The MATCH block interface version is updated to 1.15
-   1.16.3.0  02.06.2025
    - Added \ref DM_INVOLT_OUT_OF_RANGE_LO (warning) (#108320)
    - Added \ref DM_INVOLT_OUT_OF_RANGE_HI (warning) (#108320)
*/
//**************************************************************************************************

#ifndef __ITFBLOINVOLT__
    #define __ITFBLOINVOLT__

// INCLUDES ========================================================================================
    #include <ItfCore.h>
    #include <ItfBasEleErr.h>
    #include <ItfBasStruc.h>
    #include <ItfCoreDb.h>

// DEFINES & ENUMS =================================================================================

     enum
     {
         DM_INVOLT_SP,                      //!< 0 - Pin voltage is higher than \ref TInVoltPrp#au16InLim "au16InLim [0]"
         DM_INVOLT_SG_OL,                   //!< 1 - Pin voltage is lower than \ref TInVoltPrp#au16InLim "au16InLim [1]"
         DM_INVOLT_PARAM,                   //!< 2 - The parameters violate the constrain
         DM_INVOLT_UNKNOWN,                 //!< 3 - Internal Error in case a Match function is faulty
         
         //! @brief 4 - Pin voltage too low (warning)
         //!
         //! This error is detected (detect conditions), when (OR):
         //! - Case1 (\ref TInVoltPrp#ai16InChar "ai16InChar[0]" < \ref TInVoltPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin voltage is < than \ref TInVoltPrp#ai16InChar "ai16InChar[0]"
         //! - Case2 (\ref TInVoltPrp#ai16InChar "ai16InChar[0]" > \ref TInVoltPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin voltage is < than \ref TInVoltPrp#ai16InChar "ai16InChar[2]"
         //!
         //! This error is not detected (exclude conditions), when:
         //! - Error is active (debounced) \ref DM_INVOLT_SG_OL
         //!
         //! @warning
         //! It is recommended to set debounce time of \ref DM_INVOLT_OUT_OF_RANGE_LO higher (at least one block call cycle e.g. 10ms)
         //! than the  debounce times of following detection method:
         //! - \ref DM_INVOLT_SG_OL
         //! Reason: \ref DM_INVOLT_SG_OL should be activated as first in case of open load or short circuit to ground 
         //!         (to be exclude condition for \ref DM_INVOLT_OUT_OF_RANGE_LO).
         DM_INVOLT_OUT_OF_RANGE_LO,
         
         //! @brief 5 - Pin voltage too high (warning)
         //!
         //! This error is detected (detect conditions), when (OR):
         //! - Case1 (\ref TInVoltPrp#ai16InChar "ai16InChar[0]" < \ref TInVoltPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin voltage is > than \ref TInVoltPrp#ai16InChar "ai16InChar[2]"
         //! - Case2 (\ref TInVoltPrp#ai16InChar "ai16InChar[0]" > \ref TInVoltPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin voltage is > than \ref TInVoltPrp#ai16InChar "ai16InChar[0]"
         //!
         //! This error is not detected (exclude conditions), when:
         //! - Error is active (debounced) \ref DM_INVOLT_SP
         //!
         //! @warning
         //! It is recommended to set debounce time of \ref DM_INVOLT_OUT_OF_RANGE_HI higher (at least one block call cycle e.g. 10ms)
         //! than the  debounce times of following detection method:
         //! - \ref DM_INVOLT_SP
         //! Reason: \ref DM_INVOLT_SP should be activated as first in case of short circuit to power 
         //!         (to be exclude condition for \ref DM_INVOLT_OUT_OF_RANGE_HI).
         DM_INVOLT_OUT_OF_RANGE_HI,
         
         DM_INVOLT_CNT_MAX
     };

// STRUCTURES ======================================================================================

     //! Fixed configuration settings
     typedef struct
     {
         // Dummy
         TBoolean    boDummy;                    //!< [BOO] - Stuck detection activation/deactivation
     }TInVoltFix;
    
    //! Properties
    /*!
    \htmlonly
    <img src="BloInVoltGraph.png" width="600" alt="into" align="left"> 
    \endhtmlonly
    \n\n\n\n\n\n\n\n\n\n\n\n
    */
    /*!
        
<table> <tr><th>         tPrp               </th><th>           Default             </th><th>        Range                          </th></tr> 
        <tr><td>    \ref eInpBeh            </td><td align="centre">    INBEH_ERR_TO_OUT            </td><td  align="left">INPBEH_ERR_TO_OUT<br>
                                                                                                                            INPBEH_FREEZE_INP<br>
                                                                                                                        INPBEH_PAR_TO_INP</td></tr>
        <tr><td>    \ref ePin               </td><td align="right"> PIN_NA          </td><td align="right"> PIN_xxx                 </td></tr>
        <tr><td>    \ref ai16InLim[2]       </td><td align="right"> {4900,100}                      </td><td align="right">0..32767 </td></tr>
        <tr><td>    \ref u8DevLim           </td><td align="right"> 1               </td><td align="right"> 0..100                  </td></tr>
        <tr><td>    \ref u8DeadZone         </td><td align="right"> 1               </td><td align="right"> 0..100                  </td></tr>
        <tr><td>    \ref ai16OutChar [3]    </td><td align="right">{1000,0,-1000}   </td><td align="right"> -32768..32767           </td></tr>
</table>
    */
    //! TInVoltPrp
    typedef struct
    {
        EInpBeh   eInpBeh;                          //!< [ENU] - Input behavior at fault input signal
        EPin      ePin;                             //!< [ENU] - Pin (Cfg-Idx) master channel
        /*!
        |       au16InLim           |       [2]         | 
        |------------------         | ----------------- | 
        |Short to Power Detection   |       [0]         |
        |Short to Ground Detection  |       [1]         |
        */

        TDbLinkU16Var   atDbInLim[2];                       //!< [mA]  - Input Master limits for error detection
        /*!
        The Deadzone is an area around the neutral point of the input characteristics (e.g. \ref TInVoltPar#ai16InChar "ai16InChar"[1]).\n
        It is a percentage of the positive or negative area. With default values the positive and\n
        negative area is 2000mv wide and the Deadzone is 1%, this results that the neutral area is\n
        from 2480mV to 2520mV.
        */
        TUint8    u8DeadZone;                       //!< [%]   - Dead zone
        TDbLink   tDbDeadZone;                      //!< Dead zone
        TDbLinkI16Var    atDbOutChar[3];                   //!< [CUS] - Output characteristic
    }TInVoltPrp;
    
    //! Parameter
    typedef struct
    {
        TDbLinkI16Var     atDbInChar[3];                     //!< [mV]  - Input Master characteristic
        /*!
        In case of an input error at the input pin and the option \ref eInpBeh is set\n
        to \c INPBEH_PAR_TO_INP , this value will be used as static input as long as the\n
        failure occurs at the Masterpin.
        */ 
        TUint16    u16InpValDefault;                 //!< [mV]    - Default input value for input behavior INPBEH_VAL_TO_INP
        TDbLink     tDbInpValDefault;                //!< tDbInpValDefault
    }TInVoltPar;

    //! Block Configuration
    typedef struct
    {
        // Common
        TChar      achName[BLO_NAME_STR_LEN];         //!< [STR] - Block Name
        EBloStatus eBloProc;                          //!< [ENU] - Block Process

        // Fixed configuration settings
        TInVoltFix tFix;                             //!< [STU] - Fixed settings
        
        // Properties
        TInVoltPrp tPrp;                             //!< [STU] - Properties

        // Parameter
        TInVoltPar tPar;                             //!< [STU] - Parameter
        
        // Errors
        TUint8     u8ErrCnt;                            //!< [NUM] - Number of defined errors
        TErrCfgFea atErrFea[DM_INVOLT_CNT_MAX];      //!< [CLA] - Error Configurations
    }TInVoltCfg;

    //! Input Sub-Block
    typedef struct
    {
        // Common
        EBloStatus   eBehProc;                       //!< [ENU] - Block Process Behavior
        EUpdate      eUpdatePar;                     //!< [ENU] - Parameter update option

        // Error handling
        TBehErrCntl  tBehErrCntl;                    //!< [STU] - Error Control Behavior
    }TInVoltInp;

    //! Output Sub-Block
    typedef struct
    {
        // Common
        EBloStatus eBehProc;                         //!< [ENU] - Block Process Behavior
        EPinStatus ePinSta;                          //!< [ENU] - Actual pin status
        
        // Output
        TSigDir tOutVal;                             //!< [CUS] - Output value
        TUint16 u16RawVal;                           //!< [mV] - Raw Value
        // Error handling
        TBehErrSta tBehErrSta;                       //!< [STU] - Error Behavior Status
    }TInVoltOut;

    //! Address Sub-Block
    typedef struct
    {
        const TInVoltCfg* cptCfg;                       //!< [STU] - Configuration Struct
        TVoid*   pvObj;                              //!< [STU] - Pointer to the private Object
        TUint16  u16Stamp;                           //!< Registration stamp
    }TInVoltAdr;

    //! Block
    typedef struct
    {
        TInVoltInp   tInp;                              //!< [STU] - Input vector
        TInVoltOut   tOut;                              //!< [STU] - Output vector
        TInVoltPrp   tPrp;                              //!< [STU] - Properties
        TInVoltPar   tPar;                              //!< [STU] - Parameter
        TInVoltAdr   tXAdr;                             //!< [STU] - Private Address Information
    }TBloInVolt;

// LIBRARY PROTOTYPES ==============================================================================
    //--------------------------------------------------------------------------------------------------
    //! @brief      Create the InVolt-Block  (*** for manually block creation without PDT ***)
    /*! The Create-function will supply the block with memory to store all parameters for initialization.
    */
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @param[in]  cptCfg         - [PNT] Pointer to the config struct
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_MEMORY       - [ENU] Out of memory
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInVoltCreate( TBloInVolt* ptBlo, const TInVoltCfg* cptCfg );

    //------------------------------------------------------------------------------
    //! @brief      Initialization.
    //! @details    Object constructor. <a href="blo.html#block_init">Only for manual initialization</a>.
    //! @pre
    //!             - Function `eBloInVoltCreate()` must be already successfully 
    //!               executed.
    //! @note
    //!             - Method must be used in application initialization phase 
    //!               `eAppInit()`.
    //! @param[in,out]  ptBlo           - [stu] Pointer to a Block interface structure.
    //! @retval     R_OKAY              - Faultless execution.
    //! @retval     R_NULL_POINTER      - An argument is `NULL` pointer.
    //! @retval     R_ADDRESS           - Invalid address of object.
    //! @retval     R_NOT_REGISTRY      - Object is not registered.
    //! @retval     R_NOACT             - Block is already initialized.
    //! @retval     R_NOT_INITIALIZED   - Invalid configuration values.
    //! @retval     R_MONOTONY          - Characteristics are not monotonic.
    //! @retval     R_PARAMETER         - Output characteristic monotony invalid.
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInVoltInit(TBloInVolt* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Create and Initialization funktion for InVolt-Block Interface (*** only for block creation by PDT ***)
    //! @param[in]  pvBlo          - [PNT] Pointer to output block sturcture
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_ADDRESS      - [ENU] Wrong block adresse
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInVoltCreateInitRegistry( TVoid *pvBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block InVolt Version information
    //! @retval     *cptVer - Pointer of Versions information
    //--------------------------------------------------------------------------------------------------
    extern const TVerChapCom *cptBloInVoltVersionsInfo( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block  InVol Version check
    //! @retval     TRUE   - is correct
    //! @retval     FALSE  - is incorrect
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInVoltVersionCheck( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      InVolt Block Function
    /*! This function represents the cyclical part of the block. It calculates the output from the inputs.\n
        Additionally this function checks for the update flag \ref EUpdatePar and whether error occur. 
    */
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @retval R_OKAY              - Faultless execution.
    //! @retval R_ADDRESS           - Invalid address of object.
    //! @retval R_NULL_POINTER      - An argument is `NULL` pointer.
    //! @retval R_NOT_REGISTRY      - Object is not registered.
    //! @retval R_NOT_INITIALIZED   
    //! - \ref TInVoltInp::eBloSta "Block's input state" is `BLO_NOT_INIT`
    //! - `eBloInVoltInit()` was not performed successfully.
    //! @retval R_UNKNOWN
    //! - \ref TInVoltInp::eBloSta "Block's input state" is `BLO_LOCKED` . 
    //! - \ref TInVoltInp::eBloSta "Block's input state" is out of bounds.
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInVolt( TBloInVolt* ptBlo );
    
    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InVolt Status on a specified Bit-Position
    /*! True - Error bit active\n
        False- Error bit inactive
    */
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
    //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInVoltGetErrStaBit( TBloInVolt* ptBlo, TUint8 u8BitPos );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the sum InVolt Status Information
    /*! This function returns all error bits in one bit code. This bit combination can be masked to look\n 
        for the status of specific errors.  
    */
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @retval     OKAY      - [BIT] Error State Bit Coded
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
    //--------------------------------------------------------------------------------------------------
    extern TUint16 u16BloInVoltGetErrStaAll( TBloInVolt* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InVolt Event on a specified Bit-Position
    /*! This function returns information whether an error got activated or reset to inactive.
    */
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
    //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
    //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInVoltGetErrEveBit( TBloInVolt* ptBlo, TBoolean boDetect, TUint8 u8BitPos );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InVolt Event on a specified Bit-Position
    /*! This function returns information whether all errors got activated or reset to inactive.
    */
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
    //! @retval     OKAY      - [BIT] Error State Bit Coded
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
    //--------------------------------------------------------------------------------------------------
    extern TUint16 u16BloInVoltGetErrEveAll( TBloInVolt* ptBlo, TBoolean boDetect );

#endif