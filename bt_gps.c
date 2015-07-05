#if defined(__BLUE_TRCK__)
#include "bt_ps.h"
#include "bt_gps.h"

btGPSInfo GPSInfo = {0};
U8 * ubloxAssitBuff = NULL;
U8 soc_rev_buff[1024*5] = {0};
U32	soc_rev_lenth = 0;

int nmea_scanf(const char *buff, int buff_sz, const char *format, ...);


char buff_gps[1024] = {0};
/*{
	 "$GPGGA,111609.14,5001.27,N,3613.06,E,3,08,0.0,10.2,M,0.0,M,0.0,0000*70\r\n$GPGSV,2,1,08,01,05,005,80,02,05,050,80,03,05,095,80,04,05,140,80*7f\r\n$GPGSV,2,2,08,05,05,185,80,06,05,230,80,07,05,275,80,08,05,320,80*71\r\n$GPGSA,A,3,01,02,03,04,05,06,07,08,00,00,00,00,0.0,0.0,0.0*3a\r\n$GPRMC,111609.14,A,5001.27,N,3613.06,E,11.2,0.0,261206,0.0,E*50\r\n" };

*/
int _nmea_parse_time(const char *buff, int buff_sz, nmeaTIME *res)
{
	 int success = 0;
 
	 switch(buff_sz)
	 {
	 case sizeof("hhmmss") - 1:
		 success = (3 == nmea_scanf(buff, buff_sz,
			 "%2d%2d%2d", &(res->hour), &(res->min), &(res->sec)
			 ));
		 break;
	 case sizeof("hhmmss.s") - 1:
	 case sizeof("hhmmss.ss") - 1:
	 case sizeof("hhmmss.sss") - 1:
		 success = (4 == nmea_scanf(buff, buff_sz,
			 "%2d%2d%2d.%d", &(res->hour), &(res->min), &(res->sec), &(res->hsec)
			 ));
		 break;
	 default:
		 //nmea_error("Parse of time error (format error)!");
		 success = 0;
		 break;
	 }
 
	 return (success?0:-1); 	   
 }
 /**
  * \brief gps_covers_str_dddd
  * @param 用于不支持浮点运算时.
  * @param 在线查询转换:http://www.earthpoint.us/Convert.aspx
  * @param
  */
 
  double szl_gps_covers_str_dddd(double pdegree)
  {
  
	int lat_degree ;
	double lat_cent ;

	if(pdegree == 0.0) return;
	lat_degree = (int)pdegree/100;

	lat_cent = (pdegree - lat_degree*100.0)/60.0;

	return (double)(lat_degree+lat_cent);

  }


 /**
 * \brief Parse RMC packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPRMC(const char *buff, int buff_sz, BTnmeaGPRMC *pack)
{
    int nsen;
    char time_buff[256];


    memset(pack, 0, sizeof(BTnmeaGPRMC));

    nsen = nmea_scanf(buff, buff_sz,
        "$GPRMC,%s,%C,%f,%C,%f,%C,%f,%f,%2d%2d%2d,%f,%C,%C*",
        &(time_buff[0]),
        &(pack->status), &(pack->lat), &(pack->ns), &(pack->lon), &(pack->ew),
        &(pack->speed), &(pack->direction),
        &(pack->utc.day), &(pack->utc.mon), &(pack->utc.year),
        &(pack->declination), &(pack->declin_ew), &(pack->mode));

    if(nsen != 13 && nsen != 14)
    {
        //"GPRMC parse error!");
        return 0;
    }

    if(0 != _nmea_parse_time(&time_buff[0], (int)strlen(&time_buff[0]), &(pack->utc)))
    {
        //nmea_error("GPRMC time parse error!");
        return 0;
    }

	pack->lat = szl_gps_covers_str_dddd(pack->lat);
	pack->lon = szl_gps_covers_str_dddd(pack->lon);
	
   // if(pack->utc.year < 90)
   //     pack->utc.year += 100;
   // pack->utc.mon -= 1;

	pack->utc.year += 2000;
    return 1;
}
 BOOL nmea_parser_push_ex(void *parser, const char *buff, int buff_sz, int parse_type)
 {
	if((buff_sz == 0)&&(buff == NULL))
	return FALSE;

	switch(parse_type)
	{
		case PARSE_GPRMC:
		{
			nmea_parse_GPRMC((const char *)buff,buff_sz,(BTnmeaGPRMC *)parser);
		}break;

		default:
		break;
	}
 }
 /**
 * \brief Fill nmeaINFO structure by RMC packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPRMC2info(BTnmeaGPRMC *pack, btGPSInfo *info)
{

    if('A' == pack->status)
    {
        info->sig = NMEA_SIG_HIGH;
    }
    else if('V' == pack->status)
    {
        info->sig = NMEA_SIG_BAD;
    }
    info->utc_time.nYear = pack->utc.year;
    info->utc_time.nMonth= pack->utc.mon;
    info->utc_time.nDay= pack->utc.day;
    info->utc_time.nHour= pack->utc.hour;
    info->utc_time.nMin= pack->utc.min;
    info->utc_time.nSec= pack->utc.sec;
    
    info->lat = ((pack->ns == 'N')?pack->lat:-(pack->lat));
    info->lon = ((pack->ew == 'E')?pack->lon:-(pack->lon));
    info->speed = pack->speed * NMEA_TUD_KNOTS;
    info->direction = pack->direction;
}
void GPS_ActiveCallBack_ex(btGPSInfo *GPS_info)
{
//	if(gBtps.AT_getGpsInfoFlg == TRUE)
//	{
//		bt_print("GPS call back to respond AT command");
//		gBtps.AT_getGpsInfoFlg = FALSE;
		AT_SendLocGPS_ex(GPS_info);
//	}
}


 void bt_gps_parse_loop(void)
 {
	 btGPSInfo gpsInfo = {0};
	 char * addr_heard = NULL;
	 char * addr_end = NULL;

	 addr_heard = strstr(buff_gps,HEARD_GPRMC);
     //解析GPRMC
	  if(addr_heard != NULL)
	  {
	  	BTnmeaGPRMC pack;
		char nmeaGPRMCbuff[80] = {0};

		if((addr_end = strstr(addr_heard,END_RETRUE))!=NULL);
		{
			strncpy(nmeaGPRMCbuff,addr_heard,addr_end - addr_heard);
		}
		//bt_print("GPS:%s",nmeaGPRMCbuff);
		memset(&GPSInfo,0,sizeof(GPSInfo));
		if(nmea_parser_push_ex((void *)&pack,(const char *)nmeaGPRMCbuff,strlen(nmeaGPRMCbuff),PARSE_GPRMC))
		{
			//解析正确
			memcpy(&GPSInfo.GPRMC_pack,&pack,sizeof(BTnmeaGPRMC));
			nmea_GPRMC2info(&pack,&GPSInfo);
			//if(GPSInfo.sig== NMEA_SIG_HIGH)
			{
				bt_print("****GPS active lat:%d,lon:%d,speed:%d*******",pack.lat,pack.lon,pack.speed);
				//lb_make_call_out("13554757572");
				//GPS_ActiveCallBack_ex(&GPSInfo);
			}
		}
		else
		{
			//parse error;
		}
	  }

}


#define NMEA_CONVSTR_BUF    (256)
#define NMEA_TOKS_COMPARE   (1)
#define NMEA_TOKS_PERCENT   (2)
#define NMEA_TOKS_WIDTH     (3)
#define NMEA_TOKS_TYPE      (4)

/**
 * \brief Convert string to fraction number
 */
double nmea_atof(const char *str, int str_sz)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    double res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtod(&buff[0], &tmp_ptr);
    }

    return res;
}

/**
 * \brief Convert string to number
 */
int nmea_atoi(const char *str, int str_sz, int radix)
{
    char *tmp_ptr;
    char buff[NMEA_CONVSTR_BUF];
    int res = 0;

    if(str_sz < NMEA_CONVSTR_BUF)
    {
        memcpy(&buff[0], str, str_sz);
        buff[str_sz] = '\0';
        res = strtol(&buff[0], &tmp_ptr, radix);
    }

    return res;
}
/**
 * \brief Analyse string (specificate for NMEA sentences)
 */
int nmea_scanf(const char *buff, int buff_sz, const char *format, ...)
{
    const char *beg_tok;
    const char *end_buf = buff + buff_sz;

    va_list arg_ptr;
    int tok_type = NMEA_TOKS_COMPARE;
    int width = 0;
    const char *beg_fmt = 0;
    int snum = 0, unum = 0;

    int tok_count = 0;
    void *parg_target;

    va_start(arg_ptr, format);
    
    for(; *format && buff < end_buf; ++format)
    {
        switch(tok_type)
        {
        case NMEA_TOKS_COMPARE:
            if('%' == *format)
                tok_type = NMEA_TOKS_PERCENT;
            else if(*buff++ != *format)
                goto fail;
            break;
        case NMEA_TOKS_PERCENT:
            width = 0;
            beg_fmt = format;
            tok_type = NMEA_TOKS_WIDTH;
        case NMEA_TOKS_WIDTH:
            if(isdigit(*format))
                break;
            {
                tok_type = NMEA_TOKS_TYPE;
                if(format > beg_fmt)
                    width = nmea_atoi(beg_fmt, (int)(format - beg_fmt), 10);
            }
        case NMEA_TOKS_TYPE:
            beg_tok = buff;

            if(!width && ('c' == *format || 'C' == *format) && *buff != format[1])
                width = 1;

            if(width)
            {
                if(buff + width <= end_buf)
                    buff += width;
                else
                    goto fail;
            }
            else
            {
                if(!format[1] || (0 == (buff = (char *)memchr(buff, format[1], end_buf - buff))))
                    buff = end_buf;
            }

            if(buff > end_buf)
                goto fail;

            tok_type = NMEA_TOKS_COMPARE;
            tok_count++;

            parg_target = 0; width = (int)(buff - beg_tok);

            switch(*format)
            {
            case 'c':
            case 'C':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                    *((char *)parg_target) = *beg_tok;
                break;
            case 's':
            case 'S':
                parg_target = (void *)va_arg(arg_ptr, char *);
                if(width && 0 != (parg_target))
                {
                    memcpy(parg_target, beg_tok, width);
                    ((char *)parg_target)[width] = '\0';
                }
                break;
            case 'f':
            case 'g':
            case 'G':
            case 'e':
            case 'E':
                parg_target = (void *)va_arg(arg_ptr, double *);
                if(width && 0 != (parg_target))
                    *((double *)parg_target) = nmea_atof(beg_tok, width);
                break;
            };

            if(parg_target)
                break;
            if(0 == (parg_target = (void *)va_arg(arg_ptr, int *)))
                break;
            if(!width)
                break;

            switch(*format)
            {
            case 'd':
            case 'i':
                snum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &snum, sizeof(int));
                break;
            case 'u':
                unum = nmea_atoi(beg_tok, width, 10);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            case 'x':
            case 'X':
                unum = nmea_atoi(beg_tok, width, 16);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            case 'o':
                unum = nmea_atoi(beg_tok, width, 8);
                memcpy(parg_target, &unum, sizeof(unsigned int));
                break;
            default:
                goto fail;
            };

            break;
        };
    }

fail:

    va_end(arg_ptr);

    return tok_count;
}


void bt_init_gps(void)
{
	
}

#endif

