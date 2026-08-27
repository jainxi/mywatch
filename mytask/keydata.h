#ifndef __KEYDATA_H__
#define __KEYDATA_H__
void key_tick(void);

#define KEY_HOLD				0x01
#define KEY_DOWN				0x02
#define KEY_UP					0x04
#define KEY_SINGLE				0x08
#define KEY_DOUBLE				0x10
#define KEY_LONG			     0x20
#define KEY_REPEAT		        0x40
#define r_raw					0x21
#define l_raw					0x41
#define KEY_PRESSED				1
#define KEY_UNPRESSED			0
#define KEY_TIME_DOUBLE			200
#define KEY_TIME_LONG			700
#define KEY_TIME_REPEAT			100
#endif
