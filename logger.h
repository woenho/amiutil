/* ---------------------
 * UTF-8 한글확인 용
 * ---------------------*/
 
#if !defined(__LOGGER_H__)
#define __LOGGER_H__
/*
    conp(...)       타임스탬프(X) 줄 바꿈(O), 무조건 콘솔은 출력
    conpt(...)      타임스탬프(O) 줄 바꿈(O), 무조건 콘솔은 출력
    conf(...)       타임스탬프(X) 줄 바꿈(O)
    conft(...)      타임스탬프(O) 줄 바꿈(O)
    confn(...)      타임스탬프(X) 줄 바꿈(X), 로그데몬에서는 사용금지
    conftn(...)     타임스탬프(O) 줄 바꿈(X), 로그데몬에서는 사용금지
*/

enum con_callmode {
    CON_CALLMODE_CONF = 1,
    CON_CALLMODE_CONFN = 2,
    CON_CALLMODE_CONP = 3,
    CON_CALLMODE_CONPN = 4,
    // 아래는 시간 있음
    CON_CALLMODE_CONFT = 5,
    CON_CALLMODE_CONFTN = 6,
    CON_CALLMODE_CONPT = 7,
    CON_CALLMODE_CONPTN = 8
};

#define concat(dst, src) strncat((dst), (src), ((sizeof(dst)) - strlen(dst) - 1))
#define concatf(dst, ...) snprintf((dst) + strlen((dst)), sizeof((dst)) - strlen((dst)), __VA_ARGS__)


typedef enum {logmode_file, logmode_udp} LOG_MODE;
void con_logmode(LOG_MODE mode);
void con_udpclose();
void con_logfile (const char *file);
void con_logudp(const char* server, unsigned short port);
void _con_writef (enum con_callmode cm, const char *file, int line, const char *function, const char *fmt, ...);

#if defined(CON_DEBUG)
#define con_debug(...) _con_writef(CON_CALLMODE_CONPT, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define con_debug(...)
#endif

#define conf(...) _con_writef(CON_CALLMODE_CONF, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define conft(...) _con_writef(CON_CALLMODE_CONFT, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define confn(...) _con_writef(CON_CALLMODE_CONFN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define conftn(...) _con_writef(CON_CALLMODE_CONFTN, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define conp(...) _con_writef(CON_CALLMODE_CONP, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define conpt(...) _con_writef(CON_CALLMODE_CONPT, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define conpn(...) _con_writef(CON_CALLMODE_CONPN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define conptn(...) _con_writef(CON_CALLMODE_CONPTN, __FILE__, __LINE__, __func__, __VA_ARGS__)

#if defined(DEBUGTRACE)
#define CONF(...)   conp(__VA_ARGS__)
#define CONFN(...)  conpn(__VA_ARGS__)
#define CONFT(...)  conpt(__VA_ARGS__)
#define CONFTN(...) conptn(__VA_ARGS__)
#else
#define CONF(...)   conf(__VA_ARGS__)
#define CONFN(...)  confn(__VA_ARGS__)
#define CONFT(...)  conft(__VA_ARGS__)
#define CONFTN(...) conftn(__VA_ARGS__)
#endif
#define CONP(...)   conp(__VA_ARGS__)
#define CONPN(...)  conpn(__VA_ARGS__)
#define CONPT(...)  conpt(__VA_ARGS__)
#define CONPTN(...) conptn(__VA_ARGS__)

#ifndef _DEBUGTRACE_
#define _DEBUGTRACE_
#if defined(DEBUGTRACE)
#define TRACE(...) \
	/* do while(0) 문은 블록이 없는 if문에서도 구문 없이 사용하기 위한 방법이다 */ \
	do { \
		struct timeval debug_now; \
		struct	tm debug_tm; \
		gettimeofday(&debug_now, NULL); \
		localtime_r(&debug_now.tv_sec,&debug_tm);\
		char debug_buf[4096+24]; \
		int debug_len = sprintf(debug_buf,"%04d-%02d-%02d %02d:%02d:%02d.%.3ld " \
			, debug_tm.tm_year + 1900 \
			, debug_tm.tm_mon + 1 \
			, debug_tm.tm_mday \
			, debug_tm.tm_hour \
			, debug_tm.tm_min \
			, debug_tm.tm_sec \
			, debug_now.tv_usec / 1000 \
			); \
		snprintf(debug_buf+debug_len,sizeof(debug_buf)-debug_len,__VA_ARGS__); \
		fwrite(debug_buf,sizeof(char),strlen(debug_buf),stdout); \
		fflush(stdout); \
	}while(0) 
#else
#define TRACE(...) 
#endif
#endif


#endif