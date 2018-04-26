# modbus_port_esp32
lib freemodbus porting for esp32 

you must know thise lib should be used for modbus master role.



implemented:(see components/esp-modbus/functions/)

##CMD - 0x01
eMBMasterReqReadCoils

##CMD - 0x02
eMBMasterReqReadDiscreteInputs

##CMD - 0x03
eMBMasterReqReadHoldingRegister

##CMD - 0x04
eMBMasterReqReadInputRegister

##CMD - 0x05
eMBMasterReqWriteCoil

##CMD - 0x06
eMBMasterReqWriteHoldingRegister

##CMD - 0x0f
eMBMasterReqWriteMultipleCoils

##CMD - 0x10
eMBMasterReqWriteMultipleHoldingRegister

##CMD - 0x17
eMBMasterReqReadWriteMultipleHoldingRegister













thanks armink ! his git>https://github.com/armink/FreeModbus_Slave-Master-RTT-STM32
