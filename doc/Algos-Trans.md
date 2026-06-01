# Transformation Algorithms
To tranform next order (level) elements from previous one

## T_Algo_Arr_Symm
`T_Algo_Arr_Symm = std::array<DRecSymm, cFrac::NrOfElements>;`
Shortest.
Used in TOML confguration file (default: `log/fractal-anim-cfg.toml`)
and in a hardcoded array `preCalcAlgoData` .
Symmetrical that is assumes Up and Down branches are to being transformed
the same way (but with mirroring angles).
`T_Algo_Arr_Symm = std::array<DRecSymm, cFrac::NrOfElements>;`
  - repos  // move (reposition) in promile
  - angle  // of a rotation in 0.1 deg
  - scale. 

## T_Algo_Arr
`T_Algo_Arr = std::array<DRec, cFrac::NrOfElements>;`
Enables assymetrical transformation thus look.
Needed for wind/wobbling effect
Opening/Closing animation modyfies this directly
F3 Loads from file to this format (cfg_toml module implementation)
### Function
Obtained from previous structure by use of function
  `conv_to_assym()` 
  - repos
  - angle
  - angle_down // initially just -angle
  - scale

## T_Fluctuate_Algo_Arr
`T_Fluctuate_Algo_Arr = std::array<T_Algo_Arr, cFrac::NrOfOrders +1>;`
Enables assymetrical and *level dependent* transformation thus look.
Necessary for fluctuation effects (other than animation: close, open):
- wind/wobbling effect
- initial growing.
Used now for core tranformations even if fluctuations are not active.
### Function
Initial identically working tranformations
obtained from previous algo structure by use of function:
  `conv_to_fluctuate()`

## NrOfOrders vs NrOfElements
### NrOfOrders
Following orders, levels, generations of stems transformed from previous.
Array size is `cFrac::NrOfOrders +1` as result of:
- 0th level is reserved for primary stem (not transformed from previous);
- 1..NrOfOrders - following levels/generations of objects/stems.
### NrOfElements
Following branches Up and Down.
They are counted as usually in C: 0..NrOfElements-1

