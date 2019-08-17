#pragma once

#define HTU21DF_I2CADDR         (0x40)/** Default I2C address for the HTU21D. */
#define HTU21DF_READTEMP        (0xE3)/** Read temperature register. */
#define HTU21DF_READHUM         (0xE5)/** Read humidity register. */
#define HTU21DF_WRITEREG        (0xE6)/** Write register command. */
#define HTU21DF_READREG         (0xE7)/** Read register command. */
#define HTU21DF_RESET           (0xFE)/** Reset command. */

/*
 * Driver for the HTU21DF board.
 */
class HTU21 
{
    public:
        HTU21(): lasthumidity(0.0f), lasttemp(0.0f){}

        bool	init(void);
        float   readTemperature(void);
        float   readHumidity(void);
        void    reset(void);

    private:
        //bool	readData(void);
        float 	lasthumidity, lasttemp;
};