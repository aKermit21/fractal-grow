---
title: "Keyboard actions"
description: "Key functions"
---

# ⌨ Keyboard Actions

Application is driven by keyboard keys. 
Below is a complete list of Key Functions divided by Class (struct)
which implements appropriate action:

| Key | Class | Function |
| -----------| -------------| --------|
| `F1` | Help | Print Key Functions on screen (short list) |
| `X` | main()  | E**X**it |
| | | |
|  `↑Up` or `W`    | |  Light color rotation
|  `↓Down` or `S`  | |  rotation reversed
|  `←Left` or `A`  | |  left smooth movement
|  `→Right` or `D` | |  right smooth movement
| | | |
| `Space` | LogKey  | Pause Animation  |
| `Enter` |         | Resume Animation  |
| `R` | main() | **R**eset - go to base configuration |
| `F3` |  CfgToml  |  Load next initial config from toml file |
| | | |
| PageUp | TranAlg |  Speed Up, less details  |
| PageDown |       |  Speed Down, more details |
| | | |
| `L` | LightS |  **L**ight on/off toggle |
| `G`              | |  Switch mode of **G**rid rays visualisation: off, on, on move.
| `J` |   ColorPal |  Dimm all colors (d**J**imm) by 20%
| `U` |            |  **U**p all colors by 20% (reverse to previous)
| `H` |            |  Global flash of active (**H**igh) color
| | | |
| `~` |   MovFluctuate  | Animation: Trembling (wind) on/off
| | | |
| `C` | (developer) |  Display **C**ore Elements
| `E` |             |  **E**xtended D**E**velopment info display
