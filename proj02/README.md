## 資料夾
`./vp`是RISCV VP的CPU和硬體PE部分的程式(只包括和這個專題有關的部分)\
`./sw`是RISCV VP的軟體部分的程式\
`./synthesis_stratus`是用來給Stratus合成的程式\
把`vp`和`sw`資料夾放進lab中的riscv-vp資料夾即可使用下列功能

## RISCV VP建造
在`./vp/src/platform/basic-gaussseidel`裡面的`def.h`定義了硬體PE可容納的矩陣大小，此數值的4倍應該要與合成的設定一樣。\
在`./vp/build`裡面打`make install`指令來進行RISCV VP編譯，結果會產生`./vp/build/bin/basic-gaussseidel`這個編譯完的RISCV VP CPU。

## RISCV VP模擬
`./sw/basic-gaussseidel/main.cpp`裡面定義了矩陣大小N、硬體PE大小MAX_N、PE數量N_PE，可供修改。\
在`./sw/basic-gaussseidel`裡面打`make sim`可執行main.cpp的編譯與RISCV VP模擬

## Stratus合成與verilog模擬
在`./syn/`裡面\
`./syn/def.h`定義了輸入矩陣大小、小數位數、整數位數、運算模式(0是解線性方程式、1是矩陣乘以向量的乘法)

在`./tlm/stratus/`裡面打以下指令\
`make sim_V_DPA`執行開啟dpopt的合成與verilog模擬
