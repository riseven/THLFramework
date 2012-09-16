


#define THL_CLEAR_BACK  0x00000001


// API

// Inicialización ------------------------
int THL_Init(int, int, int, int flags);
void THL_Close(void);

// Video ---------------------------------
int THL_LoadImage(const char *file);
void THL_UnloadImage(int handler);
void THL_SetImageControl(int handler);
int THL_ImageWidth(int handler);
int THL_ImageHeight(int handler);
int THL_NewSprite( int ImgHandle, int x, int y, int z, int angle, int zoom, int region );
void THL_SetSpriteControl( int handler, int cx, int cy );
int THL_SpriteX( int handler );
int THL_SpriteY( int handler );
int THL_SpriteWidth( int handler );
int THL_SpriteHeight( int handler );
int THL_SpriteZ( int handler );
int THL_SpriteAngle( int handler );
int THL_SpriteScale( int handler );
int THL_SpriteRegion( int handler );
void THL_DeleteSprite(int handler);
void THL_MoveSprite( int handler, int x, int y);
void THL_MoveSpriteRel( int handler, int dx, int dy);
void THL_SetSpriteZ( int handler, int z);
void THL_ScaleSprite( int handler, int scale );
void THL_ScaleSpriteRel( int handler, int scale );
void THL_RotateSprite( int handler, int angle );
void THL_RotateSpriteRel( int handler, int angle );
void THL_SetSpriteRegion( int handler, int region );
void THL_SetSpriteControl( int handler, int cx, int cy );
void THL_DefineRegion( int region, int x, int y, int w, int h);
int THL_DrawScreen();

// Sonido --------------------------------
void THL_PlayMusic(const char *file, int loop);
int THL_isPlayingMusic();
void THL_StopMusic();
// Efectos de sonido

// Input ---------------------------------
int THL_Key(int code);
void THL_SetMouse( int x, int y);
int THL_MouseX();
int THL_MouseY();
int THL_MouseLeft();
int THL_MouseRight();
int THL_MouseMid();

// Texto ---------------------------------
int THL_LoadFont(const char *file);
void THL_UnloadFont(int handle);
int THL_Write(int FontHandler, int x, int y, const char *text);
void THL_Rewrite(int handler, int FontHandler, const char *text);


// Funciones internas
// ---------------------------------------
void _THL_DeleteSpriteList();
void _THL_DeleteSpriteEntry(struct _THL_Sprite *sprite);
void _THL_MusicDone();
void _THL_RotateScaleSprite(int handler);

