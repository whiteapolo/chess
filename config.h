#ifndef CONFIG_H
#define CONFIG_H

#define DIM 			8
#define TILE_SIZE 		90

#define SCREEN_WIDTH 		TILE_SIZE*DIM
#define SCREEN_HEIGHT 		TILE_SIZE*DIM
#define MIN_SCREEN_WIDTH 	TILE_SIZE*DIM
#define MIN_SCREEN_HEIGHT 	TILE_SIZE*DIM
#define WINDOW_NAME 		"chess"
#define FPS 	   		60
#define FONT_PATH  		"assets/JetBrainsMono-Bold.ttf"
/* #define PIECES_PATH 		"assets/$.png" */
#define PIECES_PATH 		"assets/old/$.png"
#define SOUND_PATH 		"assets/$.mp3"
#define FONT_SIZE  		24

/* #define BG_COLOR     		CLITERAL(Color){0x42, 0x54, 0x5f, 0xff} */
#define BG_COLOR     		CLITERAL(Color){0x30, 0x2e, 0x2b, 0xff}
/* #define W_T_C        		CLITERAL(Color){0xc7, 0xc5, 0xa6, 0xff} */
#define W_T_C        		CLITERAL(Color){0xed, 0xd6, 0xb0, 0xff}
/* #define B_T_C        		CLITERAL(Color){0x6d, 0x7b, 0x7b, 0xff} */
#define B_T_C        		CLITERAL(Color){0xb8, 0x87, 0x62, 0xff}
#define S_T_C        		CLITERAL(Color){0xdb, 0xb5, 0x5a, 0x88}
#define PIECES_TINT  		CLITERAL(Color){0xeb, 0xe5, 0xcf, 0xff}
/* #define PIECES_TINT  		WHITE */

#define INACTIVE_C 		CLITERAL(Color){0x00, 0x00, 0x00, 0x30}

#endif
