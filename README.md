# RL78_F24_XmodemTransfer
 RL78_F24_XmodemTransfer

1. initial F24 EVB , to test xmodem

- UART0 : P15/TX , P16/RX , to printf message

- UART1 : P74/TX , P75/RX , to recive xmodem data from PC HOST with uart bridge tool and teraterm


2. press keyboard : 1 to entry receive xmodem flow 

below is different debug message to monitor xmodem data

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/log_debug1.jpg)

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/log_debug2.jpg)

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/log_debug3.jpg)


3. use teraterm tool , to send xmodem data (ex : test_for_xmodem.bin )

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/xmodem_send.jpg)

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/xmodem_send_under_process.jpg)

4. below is LA capture about xmodem data

first frame

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/LA_1st_frame.jpg)

last frame

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/LA_last_frame.jpg)

5. below is log when finish to receive xmodem data  , will reset MCU

![image](https://github.com/released/RL78_F24_XmodemTransfer/blob/main/log_finish.jpg)


