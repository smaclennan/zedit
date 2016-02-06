#ifndef _TINIT_H_
#define _TINIT_H_

void tinit(void);

/* These are weak functions that can be overridden by the app */
void tainit(void);
void tafinit(void);
void hangup(int signo);

#ifdef WIN32
#define tflush()
#else
#define tflush() fflush(stdout)
#endif

#endif
