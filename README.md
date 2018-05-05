# modbus_port_esp32
lib freemodbus porting for esp32 

you must know thise lib should be used for modbus master role.


implemented:(see components/esp-modbus/functions/)


## 一、简述

FreeModbus是一款开源的Modbus协议栈，但是只有从机开源，主机源码是需要**收费**的。同时网上也没有发现比较好的开源的Modbus主机协议栈，所以才开发这款支持主机模式的FreeModbus协议栈。本版FreeModbus版本号更改为V1.6,支持Modbus主机协议栈，特性如下：


### CMD - 0x01
eMBMasterReqReadCoils

### CMD - 0x02
eMBMasterReqReadDiscreteInputs

### CMD - 0x03
eMBMasterReqReadHoldingRegister

### CMD - 0x04
eMBMasterReqReadInputRegister

### CMD - 0x05
eMBMasterReqWriteCoil

### CMD - 0x06
eMBMasterReqWriteHoldingRegister

### CMD - 0x0f
eMBMasterReqWriteMultipleCoils

### CMD - 0x10
eMBMasterReqWriteMultipleHoldingRegister

### CMD - 0x17
eMBMasterReqReadWriteMultipleHoldingRegister


--now add rs485 support (note:this demo rs485 en pin using gpio12)--



thanks armink ! more reference seeing his git>https://github.com/armink/FreeModbus_Slave-Master-RTT-STM32




