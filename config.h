#ifndef CONFIG_H
#define CONFIG_H

#define DIM 			8
#define TILE_SIZE 		90
#define PIECE_SIZE 		90

#define WINDOW_WIDTH 		TILE_SIZE*DIM
#define WINDOW_HEIGHT 		TILE_SIZE*DIM
#define MIN_WINDOW_WIDTH 	TILE_SIZE*DIM
#define MIN_WINDOW_HEIGHT 	TILE_SIZE*DIM
#define WINDOW_NAME 		"chess"
#define FPS 	   		60
/* #define PIECES_PATH 		"assets/$.png" */
#define PIECES_PATH 		"assets/old/$.png"
#define SOUND_PATH 		"assets/"
#define FONT_SIZE  		20

#define COLOR(c) CLITERAL(Color){(c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF}

#define THEME_2

#ifdef THEME_1
	#define BG_COLOR 	COLOR(0x302e2bff)
	#define W_T_C 		COLOR(0xEDD6B0FF)
	#define B_T_C        	COLOR(0xb88762ff)
	#define S_T_C     	COLOR(0xdbb55a88)
	#define PIECES_TINT 	COLOR(0xebe5cfff)
	#define INACTIVE_C 	COLOR(0x00000030)
#endif

#ifdef THEME_2
	#define BG_COLOR 	COLOR(0x302e2bff)
	#define W_T_C 		COLOR(0xc7c5a6ff)
	#define B_T_C        	COLOR(0x6d7b7bff)
	#define S_T_C     	COLOR(0xdbb55a88)
	#define PIECES_TINT 	COLOR(0xebe5cfff)
	#define INACTIVE_C 	COLOR(0x00000030)
#endif

#endif
