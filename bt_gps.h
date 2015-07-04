#ifndef __BT_GPS_PS_HH__
#define __BT_GPS_PS_HH__


#define PARSE_GPRMC     	(0)

#define NMEA_SIG_BAD        (0)
#define NMEA_SIG_LOW        (1)
#define NMEA_SIG_MID        (2)
#define NMEA_SIG_HIGH       (3)

#define NMEA_FIX_BAD        (1)
#define NMEA_FIX_2D         (2)
#define NMEA_FIX_3D         (3)

#define NMEA_MAXSAT         (12)
#define NMEA_SATINPACK      (4)

#define NMEA_DEF_LAT        (5001.2621)
#define NMEA_DEF_LON        (3613.0595)


#define HEARD_GPRMC    "$GPRMC"
#define END_RETRUE    "\r\n"

/*
 * Distance units
 */

#define NMEA_TUD_YARDS      (1.0936)        /**< Yeards, meter * NMEA_TUD_YARDS = yard */
#define NMEA_TUD_KNOTS      (1.852)         /**< Knots, kilometer / NMEA_TUD_KNOTS = knot */
#define NMEA_TUD_MILES      (1.609)         /**< Miles, kilometer / NMEA_TUD_MILES = mile */

/*
 * Speed units
 */

#define NMEA_TUS_MS         (3.6)           /**< Meters per seconds, (k/h) / NMEA_TUS_MS= (m/s) */


/**
 * Date and time data
 * @see nmea_time_now
 */
typedef struct _nmeaTIME
{
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     hsec;       /**< Hundredth part of second - [0,99] */

} nmeaTIME;

typedef struct _nmeaGPRMC
{
    nmeaTIME utc;       /**< UTC of position */
    char    status;     /**< Status (A = active or V = void) */
	double  lat;        /**< Latitude in NDEG - [degree*/
    char    ns;         /**< [N]orth or [S]outh */
	double  lon;        /**< Longitude in NDEG - [degree */
    char    ew;         /**< [E]ast or [W]est */
    double  speed;      /**< Speed over the ground in knots */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */
    char    declin_ew;  /**< [E]ast or [W]est */
    char    mode;       /**< Mode indicator of fix type (A = autonomous, D = differential, E = estimated, N = not valid, S = simulator) */

} BTnmeaGPRMC;

typedef struct _btGPSInfo
{
	BTnmeaGPRMC GPRMC_pack;

    int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
	
	MYTIME utc_time;

	double lat;   //纬度

	char ns;      //南北
	
	double lon;   //经度

	char ew;      //东西

	double speed; //千米/时

	double  direction;
}btGPSInfo;

#endif
