#ifndef __SUNXI_IR_H__
#define __SUNXI_IR_H__

#if (defined CONFIG_ARCH_SUN7I)
#define IR_BASE 		R_CIR_BASE
#elif (defined CONFIG_ARCH_SUN6I)

#endif

#define IR_CHECK_ADDR_CODE

//Registers
#define IR_REG(x) 			(x)
#define IR_CTRL_REG			IR_REG(0x00) 	//IR Control
#define IR_RXCFG_REG		IR_REG(0x10) 	//Rx Config
#define IR_RXDAT_REG		IR_REG(0x20) 	//Rx Data
#define IR_RXINTE_REG		IR_REG(0x2C) 	//Rx Interrupt Enable
#define IR_RXINTS_REG		IR_REG(0x30) 	//Rx Interrupt Status
#define IR_SPLCFG_REG		IR_REG(0x34) 	//IR Sample Config


//Frequency of Sample Clock = 23437.5Hz, Cycle is 42.7us
//Pulse of NEC Remote >560us
#define IR_RXFILT_VAL		(8)						//Filter Threshold = 8*42.7 = ~341us	< 500us
#define IR_RXIDLE_VAL		(2)   				//Idle Threshold = (2+1)*128*42.7 = ~16.4ms > 9ms

#define IR_L1_MIN				(80)					//80*42.7 = ~3.4ms, Lead1(4.5ms) > IR_L1_MIN
#define IR_L0_MIN				(40)					//40*42.7 = ~1.7ms, Lead0(4.5ms) Lead0R(2.25ms)> IR_L0_MIN
#define IR_PMAX					(26)					//26*42.7 = ~1109us ~= 561*2, Pluse < IR_PMAX
#define IR_DMID					(26)					//26*42.7 = ~1109us ~= 561*2, D1 > IR_DMID, D0 =< IR_DMID
#define IR_DMAX					(53)					//53*42.7 = ~2263us ~= 561*4, D < IR_DMAX

#define IR_ERROR_CODE		(0xffffffff)
#define IR_REPEAT_CODE	(0x00000000)

#define IR_FIFO_SIZE		(8)    	//16Bytes

//Bit Definition of IR_RXINTS_REG Register
#define IR_RXINTS_RXOF		(0x1<<0)	//Rx FIFO Overflow
#define IR_RXINTS_RXPE		(0x1<<1)	//Rx Packet End
#define IR_RXINTS_RXDA		(0x1<<4)	//Rx FIFO Data Available

// key
#define POWER_KEY   		0x57
#define OK_KEY					0x02




struct ir_raw_buffer {
	unsigned long dcnt;                  		/*Packet Count*/
	#define	IR_RAW_BUF_SIZE		128
	unsigned char buf[IR_RAW_BUF_SIZE];	
};

#endif