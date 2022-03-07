#include "mfrc522.h"
#include "driver/spi.h"
#include "debug.h"
#include "sspi.h"

uint8_t spi_addr_tx8(uint8_t addr_t,uint8_t val)
{
	uint32_t data[2];
	data[0] = addr_t;
	data[1] = val;
	/* spi_master_send_length(2); */
	/* spi_master_transmit(SPI_SEND, data, 2); */


	SSPI_CS_LOW;
	uint8_t k = 0;
	for( k = 0; k < 2; k++ )
	{
		txByteSSPI(data[k]);
	}

	SSPI_CS_HIGH;
	/* spi_master_send_length(0); */
	return 0;
}
uint8_t spi_rx8(uint8_t addr)
{
	SSPI_CS_LOW;
	uint8_t p1 = txByteSSPI(addr);
	uint8_t data = txByteSSPI(addr+1);
	SSPI_CS_HIGH;
	return 	data;
}

void MFRC522_InitPins(void)
{

	initSSPI();
	/* gpio_config_t io_conf; */
    /* //disable interrupt */
    /* io_conf.intr_type = GPIO_INTR_DISABLE; */
    /* //set as output mode */
    /* io_conf.mode = GPIO_MODE_OUTPUT; */
    /* //bit mask of the pins that you want to set,e.g.GPIO15/16 */
	/* // */
    /* io_conf.pin_bit_mask =  (1 << 5); */
    /* //disable pull-down mode */
    /* io_conf.pull_down_en = 0; */
    /* //disable pull-up mode */
    /* io_conf.pull_up_en = 0; */
    /* //configure GPIO with the given settings */
    /* gpio_config(&io_conf); */
	/* gpio_set_level(5,0); */
	/* vTaskDelay(10/portTICK_PERIOD_MS); */
	/* gpio_set_level(5,1); */
	/* vTaskDelay(50/portTICK_PERIOD_MS); */


	/* spi_init(1); */
	 /* spi_config_t spi_config; */
    /* // Load default interface parameters */
    /* // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0 */
    /* spi_config.interface.val = SPI_DEFAULT_INTERFACE; */

    /* // Load default interrupt enable */
    /* // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false */
    /* spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE; */
    /* // Set SPI to master mode */
    /* // ESP8266 Only support half-duplex */
    /* spi_config.mode = SPI_MASTER_MODE; */
    /* // Set the SPI clock frequency division factor */
    /* spi_config.clk_div = SPI_4MHz_DIV; */
    /* spi_config.event_cb = NULL; */
    /* spi_init(HSPI_HOST, &spi_config); */

}

void MFRC522_Init(void)
{
	MFRC522_InitPins();

	MFRC522_Reset();

	MFRC522_WriteRegister(MFRC522_REG_TX_MODE, 0x00);
	MFRC522_WriteRegister(MFRC522_REG_RX_MODE, 0x00);
	// Reset ModWidthReg
	MFRC522_WriteRegister(MFRC522_REG_MOD_WIDTH, 0x26);

	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x80);
	MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0xA9);
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0x03);
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 0xE8);

	MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

	MFRC522_WriteRegister(MFRC522_REG_RF_CFG,0x70);

	MFRC522_AntennaOn();
}

void MFRC522_WriteRegister(uint8_t addr, uint8_t val)
{
	uint32_t res;
	addr = (addr << 1) & 0x7E;
	res = spi_addr_tx8(addr,val);
}
uint8_t MFRC522_ReadRegister(uint8_t addr)
{
	uint32_t res;
	addr = ((addr << 1) & 0x7E) | 0x80;
	res = spi_rx8(addr);
    return res;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp;
	tmp = MFRC522_ReadRegister(reg);
	MFRC522_WriteRegister(reg, tmp | mask);
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
	uint8_t tmp;
	tmp = MFRC522_ReadRegister(reg);
	MFRC522_WriteRegister(reg, tmp & ~mask);  // clear bit mask
}

void MFRC522_AntennaOn(void) {
	uint8_t temp = MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03))
	{
		MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL,0x03);
	}
}

void MFRC522_AntennaOff(void) {
	MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_Reset() {
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
	// The datasheet does not mention how long the SoftRest command takes to complete.
	// But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
	// Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74μs. 	Let us be generous: 50ms.
	vTaskDelay(200/portTICK_PERIOD_MS);
}

MFRC522_Status_t MFRC522_Check(uint8_t* id)
{
	MFRC522_Status_t status;
	//Find cards, return card type
	/* MFRC522_WriteRegister(MFRC522_REG_TX_MODE, 0x00); */
	/* MFRC522_WriteRegister(MFRC522_REG_RX_MODE, 0x00); */

	/* MFRC522_WriteRegister(MFRC522_REG_MOD_WIDTH, 0x26); */
	status = MFRC522_Request(PICC_REQIDL, id);
	if (status == MI_OK)
	{
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = MFRC522_Anticoll(id);
	}
	return status;
}

MFRC522_Status_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
	MFRC522_Status_t status;
	uint16_t backBits;			//The received data bits
	uint8_t   BufRc[8];
	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		//TxLastBists = BitFramingReg[2..0]	???

	BufRc[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, BufRc, 1, BufRc, &backBits);

	if ((status == MI_OK) && (backBits == 0x10))
	{
		*TagType = BufRc[0];
		*(TagType+1) = BufRc[1];
	}
	return status;
}

MFRC522_Status_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen)
{
	MFRC522_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command)
	{
		case PCD_AUTHENT:
		{
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:
		{
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn|0x80);
	MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ,0x80);
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL,0x80);
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);
	for (i=0; i<sendLen; i++)
	{
		MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);
	}
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) 
	{    
		MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts
	}   

	//Waiting to receive data to complete
	i = 150;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);			//StartSend=0

	if (i != 0)
	{
		if (!(MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x11))
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{   
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE)
			{
				n = MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits)
				{
					*backLen = (n - 1) * 8 + lastBits;   
				} else {   
					*backLen = n * 8;   
				}

				if (n == 0) 
				{   
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN)
				{   
					n = MFRC522_MAX_LEN;   
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++)
				{   
					backData[i] = MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);
				}
			}
		}
		else
		{   
			status = MI_ERR;  
		}
	}

	return status;
}

MFRC522_Status_t MFRC522_Anticoll(uint8_t* serNum) {
	MFRC522_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);		//TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK)
	{
		for (i = 0; i < 4; i++)
		{   
			serNumCheck ^= serNum[i];
		}
	}
	return status;
} 

void MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
	for (i = 0; i < len; i++) {   
		MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));
	}
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 255;
	do {
		n = MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

uint8_t MFRC522_SelectTag(uint8_t* serNum) {
	uint8_t i;
	MFRC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = *(serNum+i);
	}
	MFRC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {   
		size = buffer[0]; 
	} else {   
		size = 0;    
	}

	return size;
}

MFRC522_Status_t MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	MFRC522_Status_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	MFRC522_CalculateCRC(recvData,2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}

void MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);

	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

