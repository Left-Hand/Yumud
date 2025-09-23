#pragma once

namespace ymd::drivers::ch9434_details{

static constexpr auto GPIO_NUMBER 0

static constexpr auto IOCTL_CMD_GPIOENABLE    = _IOW(IOCTL_MAGIC, 0x80, u16);
static constexpr auto IOCTL_CMD_GPIODIR       = _IOW(IOCTL_MAGIC, 0x81, u16);
static constexpr auto IOCTL_CMD_GPIOPULLUP    = _IOW(IOCTL_MAGIC, 0x82, u16);
static constexpr auto IOCTL_CMD_GPIOPULLDOWN  = _IOW(IOCTL_MAGIC, 0x83, u16);
static constexpr auto IOCTL_CMD_GPIOSET       = _IOW(IOCTL_MAGIC, 0x84, u16);
static constexpr auto IOCTL_CMD_GPIOGET       = _IOWR(IOCTL_MAGIC, 0x85, u16);
static constexpr auto IOCTL_CMD_GRS485        = _IOR(IOCTL_MAGIC, 0x86, u16);
static constexpr auto IOCTL_CMD_SRS485        = _IOW(IOCTL_MAGIC, 0x87, u16);

/* CH943X register definitions */
static constexpr uint8_t CH943X_RHR_REG = (0x00); /* RX FIFO */
static constexpr uint8_t CH943X_THR_REG = (0x00); /* TX FIFO */
static constexpr uint8_t CH943X_IER_REG = (0x01); /* Interrupt enable */
static constexpr uint8_t CH943X_IIR_REG = (0x02); /* Interrupt Identification */
static constexpr uint8_t CH943X_FCR_REG = (0x02); /* FIFO control */
static constexpr uint8_t CH943X_LCR_REG = (0x03); /* Line Control */
static constexpr uint8_t CH943X_MCR_REG = (0x04); /* Modem Control */
static constexpr uint8_t CH943X_LSR_REG = (0x05); /* Line Status */
static constexpr uint8_t CH943X_MSR_REG = (0x06); /* Modem Status */
static constexpr uint8_t CH943X_SPR_REG = (0x07); /* Scratch Pad */

static constexpr uint8_t CH943X_CLK_REG	=   (0x48); /* Clock Set */
static constexpr uint8_t CH943X_RS485_REG =  (0x41); /* RS485 Control */
static constexpr uint8_t CH943X_FIFO_REG	=   (0x42); /* FIFO Control */
static constexpr uint8_t CH943X_FIFOCL_REG = (0x43); /* FIFO Count Low */
static constexpr uint8_t CH943X_FIFOCH_REG = (0x44); /* FIFO Count High */

static constexpr uint8_t CH943X_GPIOEN_REG =  (0x50); /* GPIO Enable Set */
static constexpr uint8_t CH943X_GPIODIR_REG = (0x54); /* GPIO Direction Set */
static constexpr uint8_t CH943X_GPIOPU_REG =  (0x58); /* GPIO PullUp Set */
static constexpr uint8_t CH943X_GPIOPD_REG =  (0x5C); /* GPIO PullDown Set */
static constexpr uint8_t CH943X_GPIOVAL_REG = (0x60); /* GPIO Value Set */

static constexpr uint8_t CH943X_SPI_CONT_MODE_REG = (0x64); /* SPI transfer mode Set */
static constexpr uint8_t CH943X_CHIP_VER_REG	=  (0x65); /* Firmware Version */

/* Special Register set: Only if (LCR[7] == 1) */
static constexpr uint8_t CH943X_DLL_REG = (0x00); /* Divisor Latch Low */
static constexpr uint8_t CH943X_DLH_REG = (0x01); /* Divisor Latch High */

/* IER register bits */
static constexpr uint8_t CH943X_IER_RDI_BIT =  (1 << 0); /* Enable RX data interrupt */
static constexpr uint8_t CH943X_IER_THRI_BIT = (1 << 1); /* Enable TX holding register interrupt */
static constexpr uint8_t CH943X_IER_RLSI_BIT = (1 << 2); /* Enable RX line status interrupt */
static constexpr uint8_t CH943X_IER_MSI_BIT =  (1 << 3); /* Enable Modem status interrupt */

/* IER enhanced register bits */
static constexpr uint8_t CH943X_IER_RESET_BIT	= (1 << 7); /* Enable Soft reset */
static constexpr uint8_t CH943X_IER_LOWPOWER_BIT = (1 << 6); /* Enable low power mode */
static constexpr uint8_t CH943X_IER_SLEEP_BIT	= (1 << 5); /* Enable sleep mode */

/* FCR register bits */
static constexpr uint8_t CH943X_FCR_FIFO_BIT =    (1 << 0); /* Enable FIFO */
static constexpr uint8_t CH943X_FCR_RXRESET_BIT = (1 << 1); /* Reset RX FIFO */
static constexpr uint8_t CH943X_FCR_TXRESET_BIT = (1 << 2); /* Reset TX FIFO */
static constexpr uint8_t CH943X_FCR_RXLVLL_BIT =  (1 << 6); /* RX Trigger level LSB */
static constexpr uint8_t CH943X_FCR_RXLVLH_BIT =  (1 << 7); /* RX Trigger level MSB */

/* IIR register bits */
static constexpr uint8_t CH943X_IIR_NO_INT_BIT = (1 << 0); /* No interrupts pending */
static constexpr uint8_t CH943X_IIR_ID_MASK    = 0x0e;     /* Mask for the interrupt ID */
static constexpr uint8_t CH943X_IIR_THRI_SRC   = 0x02;     /* TX holding register empty */
static constexpr uint8_t CH943X_IIR_RDI_SRC    = 0x04;     /* RX data interrupt */
static constexpr uint8_t CH943X_IIR_RLSE_SRC   = 0x06;     /* RX line status error */
static constexpr uint8_t CH943X_IIR_RTOI_SRC   = 0x0c;     /* RX time-out interrupt */
static constexpr uint8_t CH943X_IIR_MSI_SRC    = 0x00;     /* Modem status interrupt */

static constexpr uint8_t CH943X_LCR_PARITY_BIT	    = (1 << 3); /* Parity bit enable */
static constexpr uint8_t CH943X_LCR_ODDPARITY_BIT   = (0);	    /* Odd parity bit enable */
static constexpr uint8_t CH943X_LCR_EVENPARITY_BIT  = (1 << 4); /* Even parity bit enable */
static constexpr uint8_t CH943X_LCR_MARKPARITY_BIT  = (1 << 5); /* Mark parity bit enable */
static constexpr uint8_t CH943X_LCR_SPACEPARITY_BIT = (3 << 4); /* Space parity bit enable */

static constexpr uint8_t CH943X_LCR_TXBREAK_BIT = (1 << 6); /* TX break enable */
static constexpr uint8_t CH943X_LCR_DLAB_BIT    = (1 << 7); /* Divisor Latch enable */
static constexpr uint8_t CH943X_LCR_WORD_LEN_5  = (0x00);
static constexpr uint8_t CH943X_LCR_WORD_LEN_6  = (0x01);
static constexpr uint8_t CH943X_LCR_WORD_LEN_7  = (0x02);
static constexpr uint8_t CH943X_LCR_WORD_LEN_8  = (0x03);

/* MCR register bits */
static constexpr uint8_t CH943X_MCR_DTR_BIT  = (1 << 0); /* DTR complement */
static constexpr uint8_t CH943X_MCR_RTS_BIT  = (1 << 1); /* RTS complement */
static constexpr uint8_t CH943X_MCR_OUT1	  =   (1 << 2); /* OUT1 */
static constexpr uint8_t CH943X_MCR_OUT2	  =   (1 << 3); /* OUT2 */
static constexpr uint8_t CH943X_MCR_LOOP_BIT = (1 << 4); /* Enable loopback test mode */
static constexpr uint8_t CH943X_MCR_AFE	  =   (1 << 5); /* Enable Hardware Flow control */

/* LSR register bits */
static constexpr uint8_t CH943X_LSR_DR_BIT	 = (1 << 0); /* Receiver data ready */
static constexpr uint8_t CH943X_LSR_OE_BIT	 = (1 << 1); /* Overrun Error */
static constexpr uint8_t CH943X_LSR_PE_BIT	 = (1 << 2); /* Parity Error */
static constexpr uint8_t CH943X_LSR_FE_BIT	 = (1 << 3); /* Frame Error */
static constexpr uint8_t CH943X_LSR_BI_BIT	 = (1 << 4); /* Break Interrupt */
static constexpr uint8_t CH943X_LSR_BRK_ERROR_MASK = 0x1E;	   /* BI, FE, PE, OE bits */
static constexpr uint8_t CH943X_LSR_THRE_BIT	 = (1 << 5); /* TX holding register empty */
static constexpr uint8_t CH943X_LSR_TEMT_BIT	 = (1 << 6); /* Transmitter empty */
static constexpr uint8_t CH943X_LSR_FIFOE_BIT	 = (1 << 7); /* Fifo Error */

/* MSR register bits */
static constexpr uint8_t CH943X_MSR_DCTS_BIT   = (1 << 0); /* Delta CTS Clear To Send */
static constexpr uint8_t CH943X_MSR_DDSR_BIT   = (1 << 1); /* Delta DSR Data Set Ready */
static constexpr uint8_t CH943X_MSR_DRI_BIT    = (1 << 2); /* Delta RI Ring Indicator */
static constexpr uint8_t CH943X_MSR_DCD_BIT    = (1 << 3); /* Delta CD Carrier Detect */
static constexpr uint8_t CH943X_MSR_CTS_BIT    = (1 << 4); /* CTS */
static constexpr uint8_t CH943X_MSR_DSR_BIT    = (1 << 5); /* DSR */
static constexpr uint8_t CH943X_MSR_RI_BIT     = (1 << 6); /* RI */
static constexpr uint8_t CH943X_MSR_CD_BIT     = (1 << 7); /* CD */
static constexpr uint8_t CH943X_MSR_DELTA_MASK = 0x0F;     /* Any of the delta bits! */

/* Clock Set */
static constexpr uint8_t CH943X_CLK_PLL_BIT = (1 << 7); /* PLL Enable */
static constexpr uint8_t CH943X_CLK_EXT_BIT = (1 << 6); /* Extenal Clock Enable */

/* FIFO */
static constexpr uint8_t CH943X_FIFO_RD_BIT = (0 << 4); /* Receive FIFO */
static constexpr uint8_t CH943X_FIFO_WR_BIT = (1 << 4); /* Receive FIFO */

/* SPI Cont Mode Set */
static constexpr uint8_t CH943X_SPI_CONTE_BIT = (1 << 0); /* SPI Cont Enable */

/* Misc definitions */
static constexpr uint8_t CH943X_FIFO_SIZE = (1536);
static constexpr uint8_t CH943X_CMD_DELAY 3

}