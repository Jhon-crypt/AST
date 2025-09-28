//**************************************************************************************************
/*!
@file    ItfBloInFreq.h
@version 1.19.1.0
@brief   \htmlonly
<span><img class="imgBriefImage" src="BloInFreq32x32.png" alt="BloInFreq32x32.png"></span>
<span class="spanBriefText">Block "Frequency Input" (<b>BloInFreq</b>)</span>
\endhtmlonly

\n
\htmlonly
<img src="BloInFreqSketch.png" width="500"  alt="into", align="left" > 
\endhtmlonly
\htmlonly <br clear="all"> \endhtmlonly
This &quot;InFreq&quot; block is designed to read in signal pulses and convert them in to a frequency signal.\n
The connected pin has to be used as a \ref TInFreqPrp#ePinNum "complex timer input pin".\n
With the various configuration options it is possible to adapt the pin to different sensor types.\n
Several parameters, which allow you to adapt the input signal to the corresponding physical magnitude, are available.\n
The block can detect up to four errors, depending of the connected sensor type (or sensor type connection).\n\n

In case of TTC3x, TTC5x, TTC7x, TTC5xx the output signal is calculated depending on the configured \ref TInFreqPrp.ePulsMode "pulse mode" as follows: \n

<table boarder=0 >
<tr><td align= "left">

- \ref TInFreqPrp.ePulsMode "FIN_PULS_PERIOD_TIME"

    \f{eqnarray*}{
        i32Frequency [\frac{Hz}{10}] &=& \frac{1000000}{u32MeasuredPeriodTime[us]} \cdot
        \frac{10}{u16PulsesPerRevolution} \cdot \frac{u16TransmRatioMul}{u16TransmRatioDiv}
    \f}

- \ref TInFreqPrp.ePulsMode "FIN_PULS_HIGH_TIME or FIN_PULS_LOW_TIME" (no calculation takes place, frequency measured by HW is applied)

    \f{eqnarray*}{
        i32Frequency [\frac{Hz}{10}] &=& u32MeasuredFrequency[Hz] * 10
    \f}

</td></tr>
</table>

In case of TTC2xxx the output signal is calculated depending on the measured period time of input signal as follows: \n

<table boarder=0 >
<tr><td align= "left">

- Measured period time is always available on TTC2xxx (not dependent from block configuration)

    \f{eqnarray*}{
        i32Frequency [\frac{Hz}{10}] &=& \frac{1000000}{u32MeasuredPeriodTime[us]} \cdot
        \frac{10}{u16PulsesPerRevolution} \cdot \frac{u16TransmRatioMul}{u16TransmRatioDiv}
    \f}

</td></tr>
</table>

@created    20.11.2013
@changelog
-   1.8.0.0 09.02.2017
    - MS block interface version updated to 1.4.
-   1.8.2.0 15.05.2017
    - The initial state of pin is set to UNDEF.
-   1.9.0.0 16.05.2017
    - The MS block interface version updated to 1.5.
-   1.10.0.0    26.06.2018
    - The MS block interface version updated to 1.6.
-   1.10.1.0    14.05.2019
    - Improved pointer error diagnostics (#26971).
-   1.11.0.0    17.06.2019
    - The MS block interface version updated to 1.7.
-   1.12.0.0    25.02.2020
    - The MS block interface version is updated to 1.8.
-   1.13.0.0    26.06.2020
    - The MS block interface version is updated to 1.9.
-   1.14.1.0    25.08.2020 
    - The MS block interface version is updated to 1.10.
    - Added configuration for timer pulse measurement mode \ref TInFreqPrp.ePulsMode
    - Rename output u32PulseWidth to u32TimerMeasurement
    - Revised doxygen
-   1.15.0.0    23.04.2021  
    - The MS block interface version is updated to 1.11
-   1.15.1.0    28.05.2021
    - Rename implementation of function i32BloInFreqGetFrequency.
-   1.15.2.0    14.06.2021
    - Using two adjacent pins with the same configuration no longer causes group conflict errors (#15843)
-   1.16.0.0    10.01.2022  
    - The MS block interface version is updated to 1.12
-   1.16.1.0    15.03.2022 
    - Removed TTC30XH support
-   1.16.2.0    16.05.2022
    - Added database link to \ref TInFreqPar "parameters"
-   1.16.3.0    24.05.2022
    - Fixed initialization error when using PIN_CHA (#65682)
-   1.17.0.0  01.08.2022  
    - The MS block interface version is updated to 1.13
-   1.17.1.0  03.08.2022
    - Created new structure for DbLink parameters
-   1.17.3.0  11.09.2023
    - Updated doxygen ( \ref TInFreqOut )
    - Added support for TTC2xxx (#57930)
    - TInFreqOut.u16Cnt "Counter" is now initialized as intended (#86807)
    - Fixed crashes due to invalid pin configuration (#86806)
    - Added: \ref TInFreqOut "u32PulseWidth and u32PulsePeriod" (only TTC2xxx) (#86973)
    - Renamed `u32TimerMeasurement` to \ref TInFreqOut "u32PulseWidthOrPeriod" (#86804)
-   1.18.1.0  14.09.2023  
    - The MS block interface version is updated to 1.14
-   1.19.0.0  29.05.2024  
    - The MS block interface version is updated to 1.15
-   1.19.1.0  20.09.2024  
    - Revised doxygen
*/
//**************************************************************************************************

#ifndef ITFBLOINFREQ_H_
    #define ITFBLOINFREQ_H_


// INCLUDES ========================================================================================
    #include <ItfCore.h>
    #include <ItfCoreLib.h>
    #include <ItfBasEleErr.h>
    #include <ItfCoreDb.h>

// DEFINES & ENUMS =================================================================================

    //! EInFreqCaptureCnt
    /*! 
    The value Average of measurements directly defines how many measuremnt sample have 
    to be accumulated before a valid measurement is returned. E.g.: Configuring CAPTURE_CNT_3_MEASURMENT 
    will reutrn a new valid measurement as soon as 3 frequency samples have been acquired 
   */
    typedef enum
    {
        #ifndef COMPILER_SWITCH_FAM_TTC5XX
        #ifndef COMPILER_SWITCH_PLATFORM_TTC2XXX
        CAPTURE_CNT_NO = 0,                         //!< 0 - No average
        #endif
        #endif
        CAPTURE_CNT_1_MEASURMENT = 1,               //!< 1 - average of 2 measurements
        CAPTURE_CNT_2_MEASURMENT = 2,               //!< 2 - average of 3 measurements
        CAPTURE_CNT_3_MEASURMENT = 3,               //!< 3 - average of 4 measurements
        CAPTURE_CNT_4_MEASURMENT = 4,               //!< 4 - average of 5 measurements
        CAPTURE_CNT_5_MEASURMENT = 5,               //!< 5 - average of 6 measurements
        CAPTURE_CNT_6_MEASURMENT = 6,               //!< 6 - average of 7 measurements
        CAPTURE_CNT_7_MEASURMENT = 7,               //!< 7 - average of 8 measurements
        CAPTURE_CNT_8_MEASURMENT = 8                //!< 8 - average of 9 measurements
    }EInFreqCaptureCnt;


     //! Errors
     enum
     {
        /*!
        Detection method becomes active, when the
         - \ref TInFreqOut.u16Vin "input voltage" is below \ref TInFreqPrp.u16SigLowTolMin
        (only applicable for TTC3x, TTC7x, TTC5xx and TTC2xxx)
         - pin state is `PINSTA_SCGND` (not applicable for TTC2xxx)
         - pin state is `PINSTA_PWD_INVALID_VOLTAGE` (only applicable for TTC5xx)
         - pin state is `PINSTA_PWD_CURRENT_THRESHOLD_LOW`
         (only applicable for TTC5x when \ref TInFreqPrp.eIntResist "FIN_RES_PD_110" is configured) 
        */
        DM_INFREQ_THRLOW,                               //!< 0 - Error Threshold low
        /*!
        Detection method becomes active, when the
         - \ref TInFreqOut.u16Vin "input voltage" exceeds \ref TInFreqPrp.u16SigHighTolMax
         (only applicable for TTC3x, TTC7x, TTC5xx and TTC2xxx)
         - pin state is `PINSTA_SCPOW` (not applicable for TTC2xxx)
         - pin state is `PINSTA_PWD_INVALID_VOLTAGE` (only applicable for TTC5xx)
         - pin state is `PINSTA_OPEN_SCPOW` (only applicable for TTC5xx)
         - pin state is `PINSTA_PWD_CURRENT_THRESHOLD_HIGH`
         (only applicable for TTC5x when \ref TInFreqPrp.eIntResist "FIN_RES_PD_110" is configured)
        */
        DM_INFREQ_THRHIGH,                              //!< 1 - Error Threshold high
        /*!
        If this error occurs, the block goes into error state until it is updated with the correct parameter data. <br>
        All outputs are forced to take the  error state
        */
        DM_INFREQ_PAR,                                  //!< 2 - Error Parameter out of range
        DM_INFREQ_UNKNOWN,                              //!< 3 - Error Unknown
        DM_INFREQ_CNT_MAX                               //!< 4 - Maximum number of Errors
     };


// STRUCTURES ======================================================================================
    //! Properties
    /*! 
    The figure below shows the common connection between sensor and controller. \n
    The pin have to be configured as complex timer input.
    It is possible to connect ABS/NPN 2 Pole sensors and PNP 3 Pole sensors to the \n
    controller input pin. A \ref TInFreqPrp#eIntResist "resistor" can be selected for each type. \n\n
    
    \htmlonly <img src="BloInFreqCPXInputSchematic.png" width="500px"  alt="into", align="left" > \endhtmlonly
    \htmlonly <br clear="all"> \endhtmlonly
    
    The voltage high \ref TInFreqPrp#eThreshold "threshold" defines the level for detecting a logic high signal.\n
    Additional it is possible to define the \ref TInFreqPrp#eCaptureMode "capture edge" signal. \n
    In this case, the measurement for the input pulse starts at the defined signal edge.\n\n
    
    \htmlonly <img src="BloInFreqPrp.png" width="500px"  alt="into", align="left" > \endhtmlonly
    \htmlonly <br clear="all"> \endhtmlonly

    */
    typedef struct
    {
        // Common
        
        /*! 
        <table><tr><th></th><th></th><th></th></tr>
        <tr><td>    eInpBeh         </td><td>
        INPBEH_ERR_TO_OUT<br>INPBEH_FREEZE_INP<br>INPBEH_PAR_TO_INP</td><td>
        A detected error at one of the input pins will set the output in an error state<br> 
        A detected error at one of the input pins will set the last valid input value at the faulty input<br> 
        A detected error at one of the input pins will set a predefined value at the faulty input|</td></tr>
        */
        EInpBeh    eInBeh;                      //!< [ENU] - Input behavior at fault input signal

        /*!
        This pin connects the sensor to the controller 
        */
        EPin ePinNum;                           //!< [ENU] - Input pin number
        
        #ifndef COMPILER_SWITCH_PLATFORM_TTC2XXX // 2XXX has no configurable threshold
        /*!
        Describes the level of input voltage where the controller detects a logical high signal. (only for TTC5x)
        */
        EFinThrHold eThreshold;                 //!< [ENU] - Voltage threshold for digital input
        #endif
        
        /*!
        Depending of the selected hardware, it is possible to configure pull-up/down resistors. \n
        A pull-down resistor has to be selected, when using NPN sensor types (highlighted in red).  \n
        The 110Ohm pull-down operation is generally required for operation with ABS sensors. \n
        A pull-up resistor is required for PNP sensor types (highlighted in green).  \n
        Case of TTC3X, TTC7X, TTC5X, TTC5XX: If no re-sistor is available for the used hardware, the pin is permanently connected to a pull-up resistor (10kOhm) internally. \n
        Case of TTC2XXX: If no re-sistor is available for the used hardware, please refer to HW manual. \n
        */
        EFinResistor eIntResist;                //!< [ENU] - Internal resistance
        
        
        #ifndef COMPILER_SWITCH_PLATFORM_TTC2XXX     // 2XXX measurement mode is dependent on detected edge
        //! Pulse time measurement mode
        //!
        //! @attention
        //! Using FIN_PULS_HIGH_TIME or FIN_PULS_LOW_TIME reduces the minimum frequency measurement
        //! value to 1 Hz and the frequency resolution to 1 Hz.
        //!
        EFinPulsMode ePulsMode;                 //!< [enu] - Pulse timer measurement mode
        #endif

        //!
        #if   defined COMPILER_SWITCH_PLATFORM_TTC2XXX
        //! | Resolution [us] |  Min. frequency [Hz] | Max. frequency [Hz] | Max. period [s] | Min. period [us] |
        //! |---------------- | -------------------- | ------------------- | --------------- | ---------------- |
        //! | 1               | 0,4768               | 20 000              | 16,7785         | 20               |
        //! | 0,125           | 0,1                  | 20 000              | 2,0973          | 20               |
        //!
        #elif defined COMPILER_SWITCH_FAM_TTC5XX
        //! | Resolution [us] |  Min. frequency [Hz] | Max. frequency [Hz] | Max. period [s] | Min. period [us] | Pin group    |
        //! |---------------- | -------------------- | ------------------- | --------------- | ---------------- | ------------ |
        //! | 0,5             | 0,1                  | 20 000              | 16,7785         | 20               | Pin 115..141 |
        //! | 0,5             | 0,1                  | 20 000              | 10              | 20               | Pin 122..148 |
        //! | 1               | 0,1                  | 10 000              | 10              | 20               | Pin 101..175 |

        #else // TTC3X, TTC5x, TTC7x
        //! | Resolution [us] |  Min. frequency [Hz] | Max. frequency [Hz] | Max. period [s] | Min. period [us] |
        //! |---------------- | -------------------- | ------------------- | --------------- | ---------------- |
        //! | 0,2             | 0,1                  | 10 000              | 3,342           | 20               |
        //! | 0,4             | 0,1                  | 10 000              | 6,684           | 20               |
        //! | 0,8             | 0,1                  | 10 000              | 13,369          | 20               |
        //! | 1,6             | 0,1                  | 10 000              | 26,738          | 20               |
        //! | 3,2             | 0,1                  | 10 000              | 53,476          | 20               |

        #endif
        //! In most applications, you can use the standard settings of this parameter. \n
        //! Adjusting this mainly impacts the maximum period time.

        EFinTimRes eTimerResol;                 //!< [ENU] - Timer resolution
        
        /*!
        Define the threshold voltage for recognizing level changes of the measured signal. \n
        It is possible to select between a rising (red) and a falling (blue) signal edge.
        */
        EFinFreqMode eCaptureMode;              //!< [ENU] - Capture mode (detection for timer)
        
        /*!
        Configure the number of frequency measurements (from 0 to 8) for obtaining the measurement value.\n
        0: Obtain as many frequency measurements (up to a maximum of 8) available until the next driver call and output the averaged value. \n
        0...8: Take the specified number of frequency measurements by force and return the calculated average when the process is complete.
        */
        EInFreqCaptureCnt eCaptureCnt;          //!< [ENU] - Average of measurements

        /*!
        The value describes the lower voltage limit of a low level input signal.
        If the input value is below this level an short to ground error will occur.
        */
        TUint16 u16SigLowTolMin;                        //!< [mV]   - Lower Limit for low level signal

        /*!
        The value describes the upper voltage limit of a high level input signal.
        If the input value is greater than this level an short to power error will occur.
        */
        TUint16 u16SigHighTolMax;                       //!< [mV]   - Upper Limit for high level signal

    }TInFreqPrp;
    //! Parameter
    /*!
    Parameters are available for creating a connection between the input signal and physical magnitude. \n
    It is, thus, possible to define a transmission ratio. The ration can be defined with the \n 
    \ref TInFreqPar#u16TransmRatioMul "Transmission Ratio Multiplier" (e.g. number of teethes of the driven gear) and the \n
    \ref TInFreqPar#u16TransmRatioDiv "Transmission Ratio Divisor" (e.g. number of teethes of the drive gear). \n
    The ratio have to be at least one. A ratio below one will result in a parameter error. \n
    You also have to define the count of \ref TInFreqPar#u16PulsesPerRevolution "Pulses Per Revolution". \n
    It is normally equal to the number of magnets which generate a signal puls at the sensor. \n
    The value for \ref TInFreqPar#u16PulsesPerRevolution "Pulses Per Revolution" have to be at least one, too.\n
    If the input signal is zero for an given \ref TInFreqPar#u16TimeoutSignal "time", the block output is set to zero.\n\n
    
    \htmlonly <img src="BloInFreqPrpPar.png" width="700px"  alt="into", align="left" > \endhtmlonly
    \htmlonly <br clear="all"> \endhtmlonly

    In the example below the drive gear has 24 teeth and the driven gear has 16 teeth. \n
    The relation between both gears describes the transmission ratio. \n
    The input revolution n<sub>1</sub> is 1000 rpm. \n
    So the number of revolution on the driven gear is calculated as follows: \n


    \f{eqnarray*}{
        n_{2} [rpm] &=& \frac{24}{16} \times 1000 rpm = 1500 rpm
    \f}

    The number of pulses is 16. It depends from the number of markers which generate a signal pulse. \n
    So the frequency on the input pin is calculated as follows: \n

    \f{eqnarray*}{
        f_{pin} [Hz] &=& \frac{1500 \times 16}{60s} = 400Hz
    \f}

    This frequency corresponds with a period time of 2500 &mu;s. \n
    Now it is possible to calculate the frequency of the input shaft (n<sub>1</sub>).

    \f{eqnarray*}{
        i32Frequency [\frac{Hz}{10}] &=& \frac{1000000}{2500us} \times ,
        \frac{10}{16} \times \frac{16}{24} = 166,66 [\frac{Hz}{10}]
    \f}

    */
    typedef struct
    {
        // Common
        #ifdef COMPILER_SWITCH_PLATFORM_TTC2XXX
        /*!
        If the input behavior \ref TInFreqPrp#eInBeh "eInBeh" is configured as &quot;INBEH_PAR_TO_IN&quot;,\n
        the &quot;default input value&quot; is used for further block operation in case of an error.\n
        \n
        Default value is the period time = 1 / frequency.\n
        \n
        Range   :   0 - 1000000 us\n
        */
        #else
        /*!
        If the input behavior \ref TInFreqPrp#eInBeh "eInBeh" is configured as &quot;INBEH_PAR_TO_IN&quot;,\n
        the &quot;default input value&quot; is used for further block operation in case of an error.\n
        \n
        Depending on selected measurement mode:
        @note
            - \ref TInFreqPrp.ePulsMode "FIN_PULS_PERIOD_TIME"
                - Default value is the period time = 1 / frequency.\n
                \n
                Range   :   0 - 1000000 us\n

        @note
            - \ref TInFreqPrp.ePulsMode "FIN_PULS_HIGH_TIME or FIN_PULS_LOW_TIME"
                - Default value is the frequency.\n
                \n
                Range   :   0 - 1000000 Hz\n
        */
        #endif
        TUint32     u32InpValDefault;           //!< [us]    - Default input value for input behavior INPBEH_VAL_TO_INP
        
        #ifdef COMPILER_SWITCH_PLATFORM_TTC2XXX
        /*!
        The &quot;pulses per revolution&quot; parameter describes the number \n
        of pulses of the frequency input during one revolution.\n\n
        Range   :   1  - 65529
        
        */
        #else
        /*!
        The &quot;pulses per revolution&quot; parameter describes the number \n
        of pulses of the frequency input during one revolution.\n\n
        Range   :   1  - 65529

        @note
        Only required when \ref TInFreqPrp.ePulsMode "FIN_PULS_PERIOD_TIME" is configured.

        */
        #endif
        TUint16 u16PulsesPerRevolution;         //!< [-]    - Pulses per revolution (e.g. 10 pulses per revolution)
        
        #ifdef COMPILER_SWITCH_PLATFORM_TTC2XXX
        /*!
        The transmission ratio multiplier is a parameter \n
        which is used to adapt the input signal to the hardware\n\n
        Range   :   1  - 65529

        */
        #else
        /*!
        The transmission ratio multiplier is a parameter \n
        which is used to adapt the input signal to the hardware\n\n
        Range   :   1  - 65529

        @note
        Only required when \ref TInFreqPrp.ePulsMode "FIN_PULS_PERIOD_TIME" is configured.

        */
        #endif
        TUint16 u16TransmRatioMul;              //!< [-]    - Transmission ratio multiplier
        
        #ifdef COMPILER_SWITCH_PLATFORM_TTC2XXX
        /*!
        The transmission ratio divisor is a parameter \n
        which is used to adapt the input signal to the hardware \n\n
        Range   :   1  - 65529

        */
        #else
        /*!
        The transmission ratio divisor is a parameter \n
        which is used to adapt the input signal to the hardware \n\n
        Range   :   1  - 65529

        @note
        Only required when \ref TInFreqPrp.ePulsMode "FIN_PULS_PERIOD_TIME" is configured.

        */
        #endif
        TUint16 u16TransmRatioDiv;              //!< [-]    - Transmission ratio divisor
        
        /*!
        The signal timeout time starts after the last valid input signal. \n
        During this time, the output holds retains the last valid value. \n
        When the time expires, the output is set to zero.\n\n

        Range   :   0 ms - 65529 ms
        */
        TUint16 u16TimeoutSignal;               //!< [ms]  - Timeout of input signal
    }TInFreqPar;

    typedef struct
    {
        TDbLinkU32Var     tInpValDefault;
        TDbLinkU16Var     tPulsesPerRevolution;
        TDbLinkU16Var     tTransmRatioMul;
        TDbLinkU16Var     tTransmRatioDiv;
        TDbLinkU16Var     tTimeoutSignal;
    }TInFreqParCfg;
    //! Block Configuration
    typedef struct
    {
        // Common
        TChar      achName[BLO_NAME_STR_LEN];   //!< [STR] - Block name
        EBloStatus eBloProc;                    //!< [ENU] - Block Process

        // Properties
        TInFreqPrp    tPrp;                     //!< [STU] - InFreq Properties

        // Parameter
        TInFreqParCfg    tPar;                     //!< [STU] - InFreq Parameter

        //Errors
         TUint8     u8ErrCnt;                   //!< [NUM] - Number of defined errors
         TErrCfgFea atErrFea[DM_INFREQ_CNT_MAX];//!< [CLA] - Error Configurations
    }TInFreqCfg;

    //! Input Sub-Block
    typedef struct
    {
        EBloStatus   eBehProc;                  //!< [ENU] - Block Process Behavior
        EUpdate      eUpdatePar;                //!< [ENU] - Parameter update option

        TBehErrCntl  tBehErrCntl;               //!< [STU] - Error Control Behavior

    }TInFreqInp;

    //! Output Sub-Block
    /*! Output
    \n\n
    \htmlonly <img src="BloInFreqOut.png" width="200"  alt="into", align="left" > \endhtmlonly \n
    */
    typedef struct
    {
        EBloStatus eBehProc;                    //!< [ENU] - Block Process Behavior

        TBehErrSta tBehErrSta;                  //!< [STU] - Error Behavior Status

        #if defined(COMPILER_SWITCH_PLATFORM_TTC2XXX)
        //! @brief [Hz/10] - Frequency output signal
        //!
        //! Frequency is calculated from \ref TInFreqOut#u32PulsePeriod
        #else
        //! @brief [Hz/10] - Frequency output signal
        //!
        //! if (tPrp.ePulsMode == FIN_PULS_PERIOD_TIME):\n
        //! Frequency is calculated from \ref TInFreqOut#u32TimerMeasurement 
        //!
        //! if ((tPrp.ePulsMode == FIN_PULS_HIGH_TIME) or (tPrp.ePulsMode == FIN_PULS_LOW_TIME)):\n
        //! Frequency value measured by HW directly is used.
        #endif
        TInt32  i32Frequency;                

        #ifdef COMPILER_SWITCH_PLATFORM_TTC2XXX
        TUint32 u32PulseWidth;                  //!< [us] - Measured time: pulse-high-time or pulse-low-time  ( @sa \ref TInFreqPrp#eCaptureMode).
        TUint32 u32PulsePeriod;                 //!< [us] - Measured time: pulse period.
        #else
        TUint32 u32PulseWidthOrPeriod;          //!< [us] - Measured time: pulse-high-time, pulse-low-time or pulse period  ( @sa \ref TInFreqPrp#ePulsMode).
        #endif
        
        TUint16 u16Vin;                         //!< [mV] - Voltage on Cpx pin (only for TTC3X, TTC7X, TTC5XX and TTC2XXX)
        TUint16 u16Cnt;                         //!< [cnt]  Current count value (only for TTC5XX: PIN_115, PIN_139, PIN_116, PIN_140, PIN_117, PIN_141)
        EPinStatus ePinState;                   //!< [STU] - Complex Pin State (input raw value)
    }TInFreqOut;

    //! Address Sub-Block
    typedef struct
    {
        const TInFreqCfg* cptCfg;                //!< [STU] - Configuration Struct
        TVoid*   pvObj;                          //!< [STU] - Pointer to the private Object
        TUint16  u16Stamp;                       //!< Registration stamp
    }TInFreqAdr;

    //! Block
    typedef struct
    {
        TInFreqInp   tInp;                      //!< [STU] - Input vector
        TInFreqOut   tOut;                      //!< [STU] - Output vector
        TInFreqPrp   tPrp;                      //!< [STU] - Properties
        TInFreqPar   tPar;                      //!< [STU] - Parameter
        TInFreqAdr   tXAdr;                     //!< [STU] - Private Address Information
    }TBloInFreq;

// LIBRARY PROTOTYPES ==============================================================================
    //--------------------------------------------------------------------------------------------------
    //! @brief      Create the InFreq-Block  (*** for manually block creation without PDT ***)
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @param[in]  cptCfg         - [PNT] Pointer to the config struct
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_MEMORY       - [ENU] Out of memory
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInFreqCreate( TBloInFreq* ptBlo, const TInFreqCfg* cptCfg );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Initialization function for InFreq-Block Interface  (*** for manually block creation without PDT ***)
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_ADDRESS      - [ENU] Wrong block address
    //! @retval     R_MINIMUM               - Parameter below lower limit
    //! @retval     R_MAXIMUM               - Parameter greater than upper limit
    //! @retval     R_NULL_POINTER - pointer of ptCpxInit is NULL
    //! @retval     R_PIN              - Pin type is not PINTYP_NA or _CPX
    //!                                  PIN_BLO is not configured
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInFreqInit(TBloInFreq* ptBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Create and Initialization function for InFreq-Block Interface (*** only for block creation by PDT ***)
    //! @param[in]  pvBlo          - [PNT] Pointer to output block structure
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_ADDRESS      - [ENU] Wrong block address
    //! @retval     R_NOT_REGISTRY - [ENU] Block not registered
    //! @retval     R_MINIMUM               - Parameter below lower limit
    //! @retval     R_MAXIMUM               - Parameter greater than upper limit
    //! @retval     R_NULL_POINTER - pointer of ptCpxInit is NULL
    //! @retval     R_PIN              - Pin type is not PINTYP_NA or _CPX
    //!                                  PIN_BLO is not configured
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInFreqCreateInitRegistry( TVoid *pvBlo );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block InFreq Version information
    //! @retval     *cptVer - Pointer of Versions information
    //--------------------------------------------------------------------------------------------------
    extern const TVerChapCom *cptBloInFreqVersionsInfo( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      Block InFreq Version check
    //! @retval     TRUE   - is correct
    //! @retval     FALSE  - is incorrect
    //--------------------------------------------------------------------------------------------------
    extern TBoolean boBloInFreqVersionCheck( TVoid );

    //--------------------------------------------------------------------------------------------------
    //! @brief      InFreq Block Function
    //! @param[in]  ptBlo          - [PNT] Pointer to Block
    //! @retval     R_OKAY         - [ENU] Functions execute without error
    //! @retval     R_ADDRESS      - [ENU] Wrong block address
    //! @retval     R_NULL_POINTER - [ENU] Null Pointer
    //! @retval     R_ADDRESS      - [ENU] Wrong block address
    //--------------------------------------------------------------------------------------------------
    extern ERetVal eBloInFreq( TBloInFreq* ptBlo );

    // -------------------------------------------------------------------------------------------------
    //! @brief          i32BloInFreqGetFrequency - Get Frequency value
    //! @param[in,out]  ptBlo           - [PNT] Block Pointer
    //! @retval         TInt32          - [Hz/10]Calculated Frequency
    //! @retval         I32_ERROR       - Wrong block address or other errors
    //! @retval         R_OKAY          - Pointer okay
    //--------------------------------------------------------------------------------------------------
    TInt32 i32BloInFreqGetFrequency( TBloInFreq* ptBlo );

     //--------------------------------------------------------------------------------------------------
     //! @brief      Get the InFreq Status on a specified Bit-Position
     //! @param[in]  ptBlo     - [PNT] Pointer to Block
     //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
     //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
     //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
     //--------------------------------------------------------------------------------------------------
     extern TBoolean boBloInFreqGetErrStaBit( TBloInFreq* ptBlo, TUint8 u8BitPos );

     //--------------------------------------------------------------------------------------------------
     //! @brief      Get the sum InFreq Status Information
     //! @param[in]  ptBlo     - [PNT] Pointer to Block
     //! @retval     OKAY      - [BIT] Error State Bit Coded
     //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
     //--------------------------------------------------------------------------------------------------
     extern TUint16 u16BloInFreqGetErrStaAll( TBloInFreq* ptBlo );

     //--------------------------------------------------------------------------------------------------
     //! @brief      Get the InFreq Event on a specified Bit-Position
     //! @param[in]  ptBlo     - [PNT] Pointer to Block
     //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
     //! @param[in]  u8BitPos  - [IDX] Bitnumber 0-7
     //! @retval     OKAY      - [BIT] Error State of the specific Bit-Position
     //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
     //--------------------------------------------------------------------------------------------------
     extern TBoolean boBloInFreqGetErrEveBit( TBloInFreq* ptBlo, TBoolean boDetect, TUint8 u8BitPos );

     //--------------------------------------------------------------------------------------------------
     //! @brief      Get the InFreq Event on a specified Bit-Position
     //! @param[in]  ptBlo     - [PNT] Pointer to Block
     //! @param[in]  boDetect  - [BOO] TRUE = Detect-Bit-Event, FALSE = Delete-Bit-Event
     //! @retval     OKAY      - [BIT] Error State Bit Coded
     //! @retval     ERROR     - If wrong ptBlo-Addr, return value is '0'
     //--------------------------------------------------------------------------------------------------
     extern TUint16 u16BloInFreqGetErrEveAll( TBloInFreq* ptBlo, TBoolean boDetect );


#endif /* ITFBLOINFREQ_H_ */
