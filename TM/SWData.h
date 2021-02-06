/* SWData.h */
#ifndef SWDATA_H_INCLUDED
#define SWDATA_H_INCLUDED

typedef struct __attribute__((__packed__)) {
  uint8_t SWStat;
} SWData_t;
extern SWData_t SWData;

#define SWS_IDLE 1
#define SWS_SEQUENCE 2
#define SWS_SEQUENCE2 3
#define SWS_SEQUENCE3 4
#define SWS_SHUTDOWN 255

#endif
