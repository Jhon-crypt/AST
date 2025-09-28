//**************************************************************************************************
/*!
@file    ItfBloInCur.h
@version 1.17.2.0
@brief   \htmlonly
<span><img class="imgBriefImage" src="BloInCur32x32.png" alt="BloInCur32x32.png"></span>
<span class="spanBriefText">Block "Current Input" (<b>BloInCur</b>)</span>
\endhtmlonly

\n
\htmlonly
<img src="BloInCurSketch.png" width="500"  alt="into", align="left" >
\endhtmlonly
\n\n\n\n\n\n\n\n\n\n\n\n\n\n
The InCur block can be attached to a PIN delivering an analog signal (e.g. 0...25 [mA]). The block\n
will deliver a signal and a direction. The signal can be customized within the limits of Tint16. The\n
input block can be configured in order to deliver a single direction signal (default 0...1000 [?]) or\n
a double direction signal (default -1000...+1000 [?]). A predefined direction output is delivered\n
in addition to the signal. This is used as a parallel path to detect controller errors.Both\n
characteristics are arrays that describe a positive(POS) and a negative(NEG) area as well as a \n
neutral(NEU) position, in &mu;A, e.g.:
|ai16InChar         |[3]        |
|:--:               |--         |
|       20000       |[0] POS    |
|       12000       |[1] NEU    |
|       4000        |[2] NEG    |
\b Examples: \n
\b a) The configuration can be chosen in a way that an input range gets scaled to 0..1000:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       20000       |[0] POS    |1000       |[0]|
|       4000        |[1] NEU    |0          |[1]|
|       4000        |[2] NEG    |0          |[2]|
\n\n\n
\htmlonly <img src="BloInCurDirPos1000.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n

\b Examples: \n
\b b) The configuration can be chosen in a way that an input range gets scaled to -1000..1000:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       20000       |[0] POS    |1000       |[0]|
|       12000       |[1] NEU    |0          |[1]|
|       4000        |[2] NEG    |-1000      |[2]|
\n\n\n
\htmlonly <img src="BloInCurDualDir1000.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n
\n\b Examples: \n
\b c) The configuration can be chosen in a way that an input range gets scaled to 1000..0:
|ai16InChar         |[3]        |ai16OutChar|[3]|
|:-:                |-          |:-:        |-  |
|       4000        |[0] POS    |0          |[0]|
|       4000        |[1] NEU    |0          |[1]|
|       20000       |[2] NEG    |-1000      |[2]|
\n\n\n
\htmlonly <img src="BloInCurDirNeg1000.png" width="600" alt="into", align="left"> \endhtmlonly
\n\n\n\n\n\n\n\n\n\n

@created    22.10.2013
@changelog
-           12.01.2016
    - Defines names were extended because same defines exist in other blocks
-           19.01.2016  
    - ItfSigBasEle.h renamed to ItfBasStruc.h in MS 06.09.XX.XX.
-   1.6.0.0 09.02.2017  
    - The MS block interface version is updated to 1.4.
-   1.6.2.0 15.05.2017  
    - The state of a block pin is added in the output structure.
-   1.7.0.0 16.05.2017  
    - The MS block interface version is updated to 1.5.
-   1.7.1.0 15.08.2018  
    - Improved error detection (#33824).
-   1.8.0.0 26.06.2018  
    - The MS block interface version updated to 1.6.
-   1.8.2.0 14.05.2019  
    - Improved pointer error diagnostics (#26971).
-   1.9.0.0 17.06.2019
    - The MS block interface version updated to 1.7.
-   1.10.0.0    25.02.2020
    - The MS block interface version is updated to 1.8.    
-   1.11.0.0    26.06.2020
    - The MS block interface version is updated to 1.9.
-   1.12.1.0    25.08.2020  
    - The MS block interface version is updated to 1.10.
-   1.13.0.0  23.04.2021  
    - The MS block interface version is updated to 1.11
-   1.13.1.0  07.06.2021
    - \ref DM_INCUR_PARAM is now reset when valid update is requested. (#33862)
    - Revised debug messages
    - Revised doxygen
    - Improved checking of parameters of input characteristic
-   1.14.0.0  10.01.2022  
    - The MS block interface version is updated to 1.12
-   1.15.0.0  27.07.2022  
    - The MS block interface version is updated to 1.13
-   1.16.1.0  07.09.2023  
    - The MS block interface version is updated to 1.14
-   1.17.0.0  29.05.2024  
    - The MS block interface version is updated to 1.15
-   1.17.1.0  20.09.2024  
    - Incremented block version because of certification reasons
-   1.17.2.0  03.06.2025
    - Added \ref DM_INCUR_OUT_OF_RANGE_LO (warning) (#108320)
    - Added \ref DM_INCUR_OUT_OF_RANGE_HI (warning) (#108320)
*/
//**************************************************************************************************

#ifndef __ITFBLOINCUR__
    #define __ITFBLOINCUR__

// INCLUDES ========================================================================================
    #include <ItfCore.h>
    #include <ItfBasEleErr.h>
    #include <ItfBasStruc.h>
    #include <ItfCoreDb.h>


// DEFINES & ENUMS =================================================================================
     enum
     {
         DM_INCUR_SP,                   //!< 0 - Master input signal short to power
         DM_INCUR_SG_OL,                //!< 1 - Master input signal short to ground
         DM_INCUR_PARAM,                //!< 2 - Parameter not correct
         DM_INCUR_UNKNOWN,              //!< 3 - Unknown internal error

         //! @brief 4 - Pin current too low (warning)
         //!
         //! This error is detected (detect conditions), when (OR):
         //! - Case1 (\ref TInCurPrp#ai16InChar "ai16InChar[0]" < \ref TInCurPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin current is < than \ref TInCurPrp#ai16InChar "ai16InChar[0]"
         //! - Case2 (\ref TInCurPrp#ai16InChar "ai16InChar[0]" > \ref TInCurPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin current is < than \ref TInCurPrp#ai16InChar "ai16InChar[2]"
         //!
         //! This error is not detected (exclude conditions), when:
         //! - Error is active (debounced) \ref DM_INCUR_SG_OL
         //!
         //! @warning
         //! It is recommended to set debounce time of \ref DM_INCUR_OUT_OF_RANGE_LO higher (at least one block call cycle e.g. 10ms)
         //! than the  debounce times of following detection method:
         //! - \ref DM_INCUR_SG_OL
         //! Reason: \ref DM_INCUR_SG_OL should be activated as first in case of open load or short circuit to ground
         //!         (to be exclude condition for \ref DM_INCUR_OUT_OF_RANGE_LO).
         DM_INCUR_OUT_OF_RANGE_LO,

         //! @brief 5 - Pin current too high (warning)
         //!
         //! This error is detected (detect conditions), when (OR):
         //! - Case1 (\ref TInCurPrp#ai16InChar "ai16InChar[0]" < \ref TInCurPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin current is > than \ref TInCurPrp#ai16InChar "ai16InChar[2]"
         //! - Case2 (\ref TInCurPrp#ai16InChar "ai16InChar[0]" > \ref TInCurPrp#ai16InChar "ai16InChar[2]"):
         //!   measured pin current is > than \ref TInCurPrp#ai16InChar "ai16InChar[0]"
         //!
         //! This error is not detected (exclude conditions), when:
         //! - Error is active (debounced) \ref DM_INCUR_SP
         //!
         //! @warning
         //! It is recommended to set debounce time of \ref DM_INCUR_OUT_OF_RANGE_HI higher (at least one block call cycle e.g. 10ms)
         //! than the  debounce times of following detection method:
         //! - \ref DM_INCUR_SP
         //! Reason: \ref DM_INCUR_SP should be activated as first in case of short circuit to power
         //!         (to be exclude condition for \ref DM_INCUR_OUT_OF_RANGE_HI).
         DM_INCUR_OUT_OF_RANGE_HI,

         DM_INCUR_CNT_MAX
     };

// STRUCTURES ======================================================================================

     //! FIXED CONFIGURATION SETTINGS
     typedef struct
     {
         // Dummy
         TBoolean    boDummy;                    //!< [BOO] - Stuck detection activation/deactivation
     }TInCurFix;


     //! Properties
         /*!
         \htmlonly
         <img src="BloInCurGraph.png" width="600" alt="into" align="left">
         \endhtmlonly
         \n\n\n\n\n\n\n\n\n\n\n\n\n
         */
         /*!

     <table> <tr><th>         tPrp               </th><th>           Default             </th><th>        Range                          </th></tr>
             <tr><td>    \ref eInpBeh            </td><td align="centre">    INBEH_ERR_TO_OUT            </td><td  align="left">INPBEH_ERR_TO_OUT<br>
                                                                                                                                 INPBEH_FREEZE_INP<br>
                                                                                                                             INPBEH_PAR_TO_INP</td></tr>
             <tr><td>    \ref ePin               </td><td align="right"> PIN_NA          </td><td align="right"> PIN_xxx                 </td></tr>
             <tr><td>    \ref ai16InLim[2]       </td><td align="right"> {21000,1000}    </td><td align="right">0..32767                 </td></tr>
             <tr><td>    \ref u8DevLim           </td><td align="right"> 1               </td><td align="right"> 0..100                  </td></tr>
             <tr><td>    \ref u8DeadZone         </td><td align="right"> 1               </td><td align="right"> 0..100                  </td></tr>
             <tr><td>    \ref ai16OutChar [3]    </td><td align="right">{2800,0,-2800}   </td><td align="right"> -32768..32767           </td></tr>
     </table>
         */

         typedef struct
         {
             /*!
             \htmlonly
              <img src="BloInCurInpBeh.png" width="700" alt="into", align="right">
              \endhtmlonly
             <table><tr><th></th><th></th><th></th></tr>
             <tr><td>    eInpBeh         </td><td>
             INPBEH_ERR_TO_OUT<br>INPBEH_FREEZE_INP<br>INPBEH_PAR_TO_INP</td><td>
             A detected error at one of the input pins will set the output in an error state<br>
             A detected error at one of the input pins will set the last valid input value at the faulty input<br>
             A detected error at one of the input pins will set a predefined value at the faulty input</td></tr>
             </table>
             \n\n\n\n\n
             */
             EInpBeh   eInpBeh;                          //!< [ENU] - Input behavior at fault input signal
             EPin      ePin;                             //!< [ENU] - Pin (Cfg-Idx) master channel
             /*!
             |       au16InLim           |       [2]         |
             |------------------         | ----------------- |
             |Short to Power Detection   |       [0]         |
             |Short to Ground Detection  |       [1]         |
             */
             TDbLinkU16Var atDbInLim[2];
             /*!
             The Deadzone is an area around the neutral point of the input characteristics (e.g. \ref TInCurPar#ai16InChar "ai16InChar"[1]).\n
             It is a percentage of the positive or negative area. With default values the positive and\n
             negative area is 8000&mu;A wide and the Deadzone is 1%, this results that the neutral area is\n
             from 12080&mu;A to 11920&mu;A.
             */
             TUint8     u8DeadZone;                 //!< [%]   - Dead zone
             TDbLink    tDbDeadZone;                //!< [STU] - Dead zone
             TDbLinkI16Var atDbOutChar[3];          //!< [STU] - Output characteristic
         }TInCurPrp;

     //! PARAMETER
     typedef struct
     {
         TDbLinkI16Var atDbInChar[3];               //!< [uA] - VAL: Input Master characteristic + DBL
         TUint16    u16InpValDefault;               //!< [uA] - VAL: Default input value for input behavior INPBEH_VAL_TO_INP
         TDbLink    tDbInpValDefault;               //!< [STU] - DBL: Default input value for input behavior INPBEH_VAL_TO_INP
     }TInCurPar;

    //! BLOCK CONFIGURATION
    typedef struct
    {
        // Common
        TChar      achName[BLO_NAME_STR_LEN];       //!< [STR] - Block Name
        EBloStatus eBloProc;                        //!< [ENU] - Block Process

        // Fixed configuration settings
        TInCurFix tFix;                             //!< [STU] - Fixed settings

        // Properties
        TInCurPrp tPrp;                             //!< [STU] - Properties

        // Parameter
        TInCurPar tPar;                             //!< [STU] - Parameter

        // Errors
        TUint8     u8ErrCnt;                        //!< [NUM] - Number of defined errors
        TErrCfgFea atErrFea[DM_INCUR_CNT_MAX];      //!< [CLA] - Error Configurations
    }TInCurCfg;

    //! INPUT SUB-BLOCK
    typedef struct
    {
        // Common
        EBloStatus   eBehProc;                       //!< [ENU] - Block Process Behavior
        EUpdate      eUpdatePar;                     //!< [ENU] - Parameter update option

        // Error handling
        TBehErrCntl  tBehErrCntl;                    //!< [STU] - Error Control Behavior
    }TInCurInp;

    //! OUTPUT SUB-BLOCK
    typedef struct
    {
        // Common
        EBloStatus eBehProc;                         //!< [ENU] - Block Process Behavior
        EPinStatus ePinSta;                          //!< [ENU] - Actual pin status

        // Output
        TSigDir tOutVal;                             //!< [CUS] - Output value
        TUint16 u16RawVal;                           //!< [uA] - Raw Value

        // Error handling
        TBehErrSta tBehErrSta;                       //!< [STU] - Error Behavior Status
    }TInCurOut;

    //! ADDRESS SUB-BLOCK
    typedef struct
    {
        const TInCurCfg* cptCfg;                       //!< [STU] - Configuration Struct
        TVoid*   pvObj;                              //!< [STU] - Pointer to the private Object
        TUint16  u16Stamp;                           //!< Registration stamp
    }TInCurAdr;

    //! BLOCK
    typedef struct
    {
        TInCurInp   tInp;                              //!< [STU] - Input vector
        TInCurOut   tOut;                              //!< [STU] - Output vector
        TInCurPrp   tPrp;                              //!< [STU] - Properties
        TInCurPar   tPar;                              //!< [STU] - Parameter
        TInCurAdr   tXAdr;                             //!< [STU] - Private Address Information
    }TBloInCur;

// LIBRARY PROTOTYPES ==============================================================================
    //--------------------------------------------------------------------------------------------------
    //! @brief      Create the InCur-Block  (*** for manually block creation without PDT ***)
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @param[in]  cptCfg         - [PNT] Pointer to the config struct
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_MEMORY       - [ENU] Out of memory
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInCurCreate( TBloInCur* ptBlo, const TInCurCfg* cptCfg );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Initialization funktion for InCur-Block Interface  (*** for manually block creation without PDT ***)
    //! @param[in]  ptBlo   - [PNT] Pointer to Block
    //! @retval     R_OKAY          - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER  - [ENU] Null Pointer
    //! @retval     R_ADDRESS       - [ENU] Wrong Block address
    //! @retval     R_NOACT         - [ENU] Function aborted. Initialization already succeeded.
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInCurInit(TBloInCur* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Create and Initialization funktion for InCur-Block Interface (*** only for block creation by PDT ***)
    //! @param[in]  pvBlo          - [PNT] Pointer to output block sturcture
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_ADDRESS      - [ENU] Wrong Block address
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered MinCylLenFront
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInCurCreateInitRegistry( TVoid *pvBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block InCur Version information
    //! @retval     *cptVer - Pointer of Versions information
    //--------------------------------------------------------------------------------------------------
    extern const TVerChapCom *cptBloInCurVersionsInfo( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block InCur Version check
    //! @retval     TRUE   - is correct
    //! @retval     FALSE  - is incorrect
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInCurVersionCheck( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      InCur Block Function
    //! @param[in]  ptBlo           - [PNT] Pointer to Block
    //! @retval     R_OKAY          - [ENU] Functions execute without error
    //! @retval     R_ADDRESS       - [ENU] Wrong block adress
    //! @retval     R_NULL_POINTER  - [ENU] Null Pointer
    //! @retval     R_ADDRESS       - [ENU] Wrong Block address
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInCur( TBloInCur* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InCur Status on a specified Bit-Position
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
    //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '1'
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInCurGetErrStaBit( TBloInCur* ptBlo, TUint8 u8BitPos );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the sum InCur Status Information
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @retval     OKAY      - [BIT] Error State Bit Coded
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '1'
    //--------------------------------------------------------------------------------------------------
    extern TUint16 u16BloInCurGetErrStaAll( TBloInCur* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InCur Event on a specified Bit-Position
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
    //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
    //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '1'
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInCurGetErrEveBit( TBloInCur* ptBlo, TBoolean boDetect, TUint8 u8BitPos );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Get the InCur Event on a specified Bit-Position
    //! @param[in]  ptBlo     - [PNT] Pointer to Block
    //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
    //! @retval     OKAY      - [BIT] Error State Bit Coded
    //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '1'
    //--------------------------------------------------------------------------------------------------
    extern TUint16 u16BloInCurGetErrEveAll( TBloInCur* ptBlo, TBoolean boDetect );

#endif
