## 資料夾
`./tlm`是SystemC版本的程式\
`./syn`是用來給Stratus合成的程式

## SystemC模擬
在`./tlm/build/`裡面打`make run`指令來進行SystemC模擬

## Stratus合成與verilog模擬
在`./syn/`裡面\
`./syn/def.h`定義了輸入矩陣大小、小數位數、整數位數、迭代次數

在`./tlm/stratus/`裡面打以下指令\
`make sim_B`執行SystemC模擬\
`make sim_V`執行合成與verilog模擬\
`make sim_V_DPA`執行開啟dpopt的合成與verilog模擬
