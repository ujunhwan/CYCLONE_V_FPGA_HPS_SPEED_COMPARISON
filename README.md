# Comparision of Audio Compression Process Rate between FPGA and HPS
This project compares the speed differences when audio raw datas compressed between FPGAs and HPS.
## Structural features
---
![Alt text](img/portion.png "Optional title")
- Cyclone V has two portions which are FPGA Portion and HPS Portion.    
- The two parts made it possible to compare the processing speed.
## Overall Processing
---
![Alt text](img/process.png)
- The sound is transmitted through a microphone connected to the FPGA.
- Audio data is processed by the HPS and the FPGA, respectively, through the DMA 
- Compressed audio data is stored again as a SDRAM of the FPGA.
## Result
---
![Alt text](img/result_data.png)
![Alt text](img/result.png)
### The Hardware (FPGA) indicates faster processing speed.