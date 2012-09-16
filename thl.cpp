

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "thl.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SFont.h"


#define PIOVER2   1.57079632
#define PIOVER180 0.01745329

int THL_Flags;

SDL_Surface *THL_Screen ;

#define MAX_IMAGES      10000
SDL_Surface *THL_Surfaces[MAX_IMAGES] ; // Un array dinamico... cuando tenga tiempo ;)
struct _THL_ImageInfo
{
    int cx, cy;
} THL_ImageInfo[MAX_IMAGES];



struct _THL_Sprite
{
    int spHandler ;
    int imgHandler ;
    int x ;
    int y ;
    int z ;
    int cx ;
    int cy ;
    int angle ;
    int scale ;
    int region ;
    // Surface propia
    int _redraw ;
    int _angle ;
    int _scale ;
    int _cx ;
    int _cy ;
    SDL_Surface *surface;
    struct _THL_Sprite *next ;
};
struct _THL_Sprite *THL_Sprites ;

#define MAX_SPRITES     10000
struct _THL_Sprite *THL_SpritesIndex[MAX_SPRITES];

#define NUM_REGIONS     32
SDL_Rect THL_Regions[NUM_REGIONS] ;

Mix_Music *THL_Music;
int _THL_isPlayingMusic = 0 ;


Uint8 *_THL_Keystate = NULL;

#define MAX_FONTS       10000
SFont_Font *THL_Fonts[MAX_FONTS] ;
int THL_FontImage[MAX_FONTS] ;

//===============================================================================
//=                                INICIALIZACION                               =
//===============================================================================

int THL_Init(int Width, int Height, int Depth, int flags)
{
    int i ; // Contador para bucles

    THL_Flags = flags ;

    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) == -1 )
    { 
        printf("No se pudo inicializar SDL: %s.\n", SDL_GetError());
        exit(-1);
    }

    // Inicializamos el modo grafico
    if ((THL_Screen = SDL_SetVideoMode(Width, Height, Depth, SDL_HWSURFACE|SDL_DOUBLEBUF)) == NULL )
    {
        if ((THL_Screen = SDL_SetVideoMode(Width, Height, Depth, SDL_SWSURFACE)) == NULL )
        {
            return -1 ;
        }
    }

    // Inicializamos el mixer
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);


    // Inicializamos el vector de surfaces
    for ( i = 0 ; i < MAX_IMAGES ; i++ )
    {
        THL_Surfaces[i] = NULL ;
    }

    THL_Sprites = NULL ;

    // Inicializamos el vector de spriteshandlers
    for ( i = 0 ; i < MAX_SPRITES ; i++ )
    {
        THL_SpritesIndex[i] = NULL ;
    }

    // Inicializamos las regiones
    for ( i = 0 ; i < NUM_REGIONS ; i++ )
    {
        THL_Regions[i].x = 0 ;
        THL_Regions[i].y = 0 ;
        THL_Regions[i].w = THL_Screen->w ;
        THL_Regions[i].h = THL_Screen->h ;
    }



    THL_Music = NULL ;

    // Inicializamos el vector de fuentes
    for ( i = 0 ; i < MAX_FONTS ; i++ )
    {
        THL_Fonts[i] = NULL ;
    }

    return 0 ;
}

void THL_Close(void)
{
    Mix_CloseAudio();

    SDL_Quit();

    // Eliminamos la lista de sprites
    _THL_DeleteSpriteList();
}

//===============================================================================
//=                                     VIDEO                                   =
//===============================================================================

int THL_LoadImage(const char *file)
{
    int handler; // Handle de la imagen que se devuelve

    // Buscamos un hueco libre en el array de imagenes
    for ( handler = 0 ; handler < MAX_IMAGES ; handler++ )
    {
        if ( THL_Surfaces[handler] == NULL )
        {
            // Cargamos el grafico en esta posicion
            THL_Surfaces[handler] = IMG_Load(file) ;

            // Si no se ha cargado, devolvemos -1
            if ( THL_Surfaces[handler] == NULL )
            {
                printf("No se pudo cargar el fichero %s: %s.\n", file, IMG_GetError());
                return -1 ;
            }

            THL_ImageInfo[handler].cx = THL_Surfaces[handler]->w / 2 ;
            THL_ImageInfo[handler].cy = THL_Surfaces[handler]->h / 2 ;

            return handler ;
        }
    }

    // No se ha encontrado espacio en el array
    printf("No se pueden cargar mas imagenes.\n");
    return -1;
}

void THL_UnloadImage(int handler)
{
    struct _THL_Sprite *actual, *siguiente ;
    
    // Primero se borran todos los sprites de esta imagen
    actual = THL_Sprites ;
    while (actual)
    {
        siguiente = actual->next ;
        if ( actual->imgHandler == handler )
        {
            _THL_DeleteSpriteEntry( actual );
        }
        actual = siguiente ;
    }

    // Ahora se libera la imagen
    SDL_FreeSurface(THL_Surfaces[handler]);
    THL_Surfaces[handler] = NULL ;
}

void THL_SetImageControl( int handler, int cx, int cy )
{
    THL_ImageInfo[handler].cx = cx ;
    THL_ImageInfo[handler].cy = cy ;
}

int THL_ImageWidth( int handler )
{
    if ( THL_Surfaces[handler] == NULL )
        return 0;

    return THL_Surfaces[handler]->w ;
}

int THL_ImageHeight( int handler )
{
    if ( THL_Surfaces[handler] == NULL )
        return 0;

    return THL_Surfaces[handler]->h ;
}


int THL_NewSprite( int ImgHandler, int x, int y, int z, int angle, int zoom, int region )
{
    struct _THL_Sprite *actual, *nuevo, *anterior ;
    int salir ;
    int handler ;

    nuevo = (struct _THL_Sprite *) malloc( sizeof(struct _THL_Sprite) );
    nuevo->imgHandler = ImgHandler ;
    nuevo->x = x ;
    nuevo->y = y ;
    nuevo->z = z ;
    nuevo->cx = THL_ImageInfo[ImgHandler].cx ;
    nuevo->cy = THL_ImageInfo[ImgHandler].cy ;
    nuevo->angle = angle ;
    nuevo->scale = zoom ;
    nuevo->region = region ;
    nuevo->_angle = 0 ;
    nuevo->_scale = 100 ;
    nuevo->_cx = THL_ImageInfo[ImgHandler].cx ;
    nuevo->_cy = THL_ImageInfo[ImgHandler].cy ;
    nuevo->_redraw = 0 ;
    nuevo->surface = NULL ;
    if ( angle != 0 || zoom != 100) nuevo->_redraw = 1 ;

    actual = THL_Sprites ;
    anterior = NULL ;
    salir = 0 ;
    
    while( !salir )
    {
        if ( !actual )
        {
            nuevo->next = NULL ;
            if ( anterior == NULL )
            {
                THL_Sprites = nuevo ;
            }else
            {
                anterior->next = nuevo ;
            }
            salir = 1 ;
        }else if (actual->z >= z)
        {
            nuevo->next = actual ;
            if ( anterior == NULL )
            {
                THL_Sprites = nuevo ;
            }else
            {
                anterior->next = nuevo ;
            }
            salir = 1 ;
        }else
        {
            anterior = actual ;
            actual = actual->next ;
        }
    }

    // Buscamos un handler libre
    for ( handler = 0 ; handler < MAX_SPRITES ; handler++ )
    {
        if ( THL_SpritesIndex[handler] == NULL )
        {
            THL_SpritesIndex[handler] = nuevo ;
            nuevo->spHandler = handler ;
            return handler ;
        }
    }

    printf("No se pueden crear mas sprites.\n");
    return -1 ;
}

void THL_SetSpriteControl(int handler, int cx, int cy)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return ;

    THL_SpritesIndex[handler]->cx = cx ;
    THL_SpritesIndex[handler]->cy = cy ;
    THL_SpritesIndex[handler]->_cx = cx ;
    THL_SpritesIndex[handler]->_cy = cy ;
    THL_SpritesIndex[handler]->_redraw = 1 ;
}

int THL_SpriteX(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->x ;
}

int THL_SpriteY(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->y ;
}

int THL_SpriteWidth(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_Surfaces[THL_SpritesIndex[handler]->imgHandler]->w ;
}

int THL_SpriteHeight(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_Surfaces[THL_SpritesIndex[handler]->imgHandler]->h ;
}

int THL_SpriteZ(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->z ;
}

int THL_SpriteAngle(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->angle ;
}

int THL_SpriteScale(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->scale ;
}

int THL_SpriteRegion(int handler)
{
    if ( THL_SpritesIndex[handler] == NULL )
        return -1;
    return THL_SpritesIndex[handler]->region ;
}

void THL_DeleteSprite(int handler)
{
    if (handler != -1)
    {
        if ( THL_SpritesIndex[handler] == NULL )
            return ;
        _THL_DeleteSpriteEntry( THL_SpritesIndex[handler] );
    }else
    {
        _THL_DeleteSpriteList();
    }
}

void THL_MoveSprite( int handler, int x, int y)
{
    THL_SpritesIndex[handler]->x = x ;
    THL_SpritesIndex[handler]->y = y ;
}

void THL_MoveSpriteRel( int handler, int dx, int dy)
{
    THL_SpritesIndex[handler]->x += dx ;
    THL_SpritesIndex[handler]->y += dy ;
}

void THL_SetSpriteZ( int handler, int z )
{
    struct _THL_Sprite *actual, *cortado, *anterior;
    int salir ;

    if ( THL_SpritesIndex[handler] == NULL )
        return ;
    
    cortado = THL_SpritesIndex[handler];
    
    // Primero cortamos el sprite
    actual = THL_Sprites ;
    anterior = NULL ;
    while(1)
    {
        if ( !actual )
            return ;

        if ( actual == cortado )
        {
            if ( anterior == NULL )
                THL_Sprites = cortado->next ;
            else
                anterior->next = cortado->next ;
            break;
        }

        anterior = actual ;
        actual = actual->next ;
    }

    // Y ahora lo reinsertamos
    cortado->z = z ;
    actual = THL_Sprites ;
    anterior = NULL ;
    salir = 0 ;
    
    while( !salir )
    {
        if ( !actual )
        {
            cortado->next = NULL ;
            if ( anterior == NULL )
            {
                THL_Sprites = cortado ;
            }else
            {
                anterior->next = cortado ;
            }
            salir = 1 ;
        }else if (actual->z >= z)
        {
            cortado->next = actual ;
            if ( anterior == NULL )
            {
                THL_Sprites = cortado ;
            }else
            {
                anterior->next = cortado ;
            }
            salir = 1 ;
        }else
        {
            anterior = actual ;
            actual = actual->next ;
        }
    }

    return ;
}


void THL_ScaleSprite( int handler, int scale )
{
    if ( scale < 0 ) scale = 0 ;
    THL_SpritesIndex[handler]->scale = scale ;
    THL_SpritesIndex[handler]->_redraw = 1 ;
}

void THL_ScaleSpriteRel( int handler, int scale )
{
    int sc = THL_SpritesIndex[handler]->scale ;
    sc += scale ;
    if ( sc < 0 ) sc = 0 ;
    THL_SpritesIndex[handler]->scale = sc ;
    THL_SpritesIndex[handler]->_redraw = 1 ;
}

void THL_RotateSprite( int handler, int angle )
{
    while (angle>359) angle-= 360;
    while (angle<0) angle+=360;
    THL_SpritesIndex[handler]->angle = angle ;
    THL_SpritesIndex[handler]->_redraw = 1 ;
}

void THL_RotateSpriteRel( int handler, int angle )
{
    int deg = THL_SpritesIndex[handler]->angle ;
    deg += angle;
    while(deg>359) deg-=360;
    while(deg<0) deg+=360 ;
    THL_SpritesIndex[handler]->angle = deg ;
    THL_SpritesIndex[handler]->_redraw = 1 ;
}

void THL_SetSpriteRegion( int handler, int region )
{
    if ( THL_SpritesIndex[handler] == NULL )
        return ;

    if ( region < 0 || region >= NUM_REGIONS )
        return ;

    THL_SpritesIndex[handler]->region = region ;
}

void THL_DefineRegion( int region, int x, int y, int w, int h)
{
    if ( region < 0 || region >= NUM_REGIONS )
        return ;

    THL_Regions[region].x = x ;
    THL_Regions[region].y = y ;
    THL_Regions[region].w = w ;
    THL_Regions[region].h = h ;
}

int THL_DrawScreen()
{
    struct _THL_Sprite *actual;
    SDL_Rect rect ;
    SDL_Event event;
    int actualiza_keys = 0 ;

    // Primero comprobamos los eventos
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            actualiza_keys = 1 ;
            break ;
        case SDL_QUIT:  
            THL_Close();
            exit(0);
            break;
        }
    }

    if ( actualiza_keys )
    {
        _THL_Keystate = SDL_GetKeyState(NULL);
    }

    // Comprobamos si hay que borrar el fondo
    if ( THL_Flags & THL_CLEAR_BACK )
    {
        SDL_FillRect(THL_Screen, NULL, SDL_MapRGBA(THL_Screen->format, 0x00, 0x00, 0x00, 0x00));
    }

    actual = THL_Sprites ;
    while ( actual )
    {
        // Comprobamos si hay que redibujar el sprite
        if ( actual->_redraw )
        {
            if ( actual->surface != NULL )
                SDL_FreeSurface(actual->surface);
            actual->surface = NULL ;

            _THL_RotateScaleSprite( actual->spHandler );
        }

        rect.x = actual->x - actual->cx ;
        rect.y = actual->y - actual->cy ;

        // Establecemos la region
        SDL_SetClipRect(THL_Screen, &THL_Regions[ actual->region ]);

        if ( actual->surface == NULL )
            SDL_BlitSurface( THL_Surfaces[ actual->imgHandler ], NULL, THL_Screen, &rect );
        else
            SDL_BlitSurface( actual->surface, NULL, THL_Screen, &rect );
        actual = actual->next ;
    }

    SDL_Flip(THL_Screen);
    return 0;
}

//===============================================================================
//=                                    SONIDO                                   =
//===============================================================================

void THL_PlayMusic(const char *file, int loop)
{
    if ( THL_Music != NULL )
    {
        Mix_HaltMusic();
        Mix_FreeMusic(THL_Music);
        THL_Music = NULL ;
    }

    THL_Music=Mix_LoadMUS(file);
    if(!THL_Music) 
    {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        return ;
    }

    Mix_PlayMusic(THL_Music, loop);  
    Mix_HookMusicFinished(_THL_MusicDone);
    _THL_isPlayingMusic = 1 ;
}

int THL_isPlayingMusic()
{
    return _THL_isPlayingMusic;
}

void THL_StopMusic()
{
    Mix_HaltMusic();
    Mix_FreeMusic(THL_Music);
    THL_Music = NULL ;
    _THL_isPlayingMusic = 0 ;
}

//===============================================================================
//=                                     INPUT                                   =
//===============================================================================

int THL_Key(int code)
{
    if ( _THL_Keystate == NULL )
        return 0 ;

    return _THL_Keystate[code] ;
}

void THL_SetMouse( int x, int y)
{
    SDL_WarpMouse( x, y );
}

int THL_MouseX()
{
    int x ;
    SDL_GetMouseState(&x, NULL) ;
    return x ;
}

int THL_MouseY()
{
    int y ;
    SDL_GetMouseState(NULL, &y) ;
    return y ;
}

int THL_MouseLeft()
{
    return SDL_GetMouseState(NULL,NULL) & SDL_BUTTON(1) ;
}

int THL_MouseRight()
{
    return SDL_GetMouseState(NULL,NULL) & SDL_BUTTON(3) ;
}

int THL_MouseMid()
{
    return SDL_GetMouseState(NULL,NULL) & SDL_BUTTON(2) ;
}

//===============================================================================
//=                                     TEXTO                                   =
//===============================================================================

int THL_LoadFont(const char *file)
{
    int tempHandler, handler ;
    tempHandler = THL_LoadImage(file) ;
    
    // Buscamos una fuente libre
    for ( handler = 0 ; handler < MAX_FONTS ; handler++ )
    {
        if ( THL_Fonts[handler] == NULL )
        {
            THL_FontImage[handler] = tempHandler ;
            THL_Fonts[handler] = SFont_InitFont( THL_Surfaces[tempHandler] ) ;
            return handler ;
        }
    }

    printf("No se pueden cargar mas fuentes.\n");
    THL_UnloadImage(tempHandler);
    return -1 ;
}

void THL_UnloadFont(int handler)
{
    SFont_FreeFont(THL_Fonts[handler]);
    THL_Fonts[handler]=NULL;
}

int THL_Write(int FontHandler, int x, int y, const char *text)
{
    int img ;
    // Creamos una imagen para el texto
    for ( img = 0 ; img < MAX_IMAGES ; img++ )
    {
        if ( THL_Surfaces[img] == NULL )
        {
            THL_Surfaces[img] = SDL_CreateRGBSurface(SDL_HWSURFACE, SFont_TextWidth(THL_Fonts[FontHandler], text), SFont_TextHeight(THL_Fonts[FontHandler]), THL_Screen->format->BitsPerPixel, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
            THL_Surfaces[img] = SDL_DisplayFormat(THL_Surfaces[img]) ;
            SDL_SetColorKey(THL_Surfaces[img], SDL_SRCCOLORKEY, THL_Fonts[FontHandler]->Surface->format->colorkey);
            SFont_Write( THL_Surfaces[img], THL_Fonts[FontHandler], 0, 0, text) ;
            THL_ImageInfo[img].cx = THL_Surfaces[img]->w / 2 ;
            THL_ImageInfo[img].cy = THL_Surfaces[img]->h / 2 ;
            // Ahora creamos el sprite
            return THL_NewSprite(img, x, y, 0, 0, 100, 0);
        }
    }
    printf("No se pueden crear mas imagenes.\n");    
    return 0 ;
}

void THL_Rewrite(int handler, int FontHandler, const char *text)
{
    int img = THL_SpritesIndex[handler]->imgHandler;
    // Liberamos la imagen anterior. No se puede usar THL_UnloadImage porque eliminiaria
    // todos los sprites de esa imagen
    SDL_FreeSurface(THL_Surfaces[img]);    

    // Creamos una imagen para el texto
    THL_Surfaces[img] = SDL_CreateRGBSurface(SDL_HWSURFACE, SFont_TextWidth(THL_Fonts[FontHandler], text), SFont_TextHeight(THL_Fonts[FontHandler]), THL_Screen->format->BitsPerPixel, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    THL_Surfaces[img] = SDL_DisplayFormat(THL_Surfaces[img]) ;
    SDL_SetColorKey(THL_Surfaces[img], SDL_SRCCOLORKEY, THL_Fonts[FontHandler]->Surface->format->colorkey);
    SFont_Write( THL_Surfaces[img], THL_Fonts[FontHandler], 0, 0, text) ;
    
    if ( THL_SpritesIndex[handler]->surface != NULL )
    {
        THL_SpritesIndex[handler]->_redraw = 1 ;
    }

    return;
}


//===============================================================================
//=                             FUNCIONES INTERNAS                              =
//===============================================================================


void _THL_DeleteSpriteList()
{
    struct _THL_Sprite *actual, *siguiente ;
    
    actual = THL_Sprites ;

    while ( actual != NULL )
    {
        siguiente = actual->next ;
        SDL_FreeSurface(actual->surface);
        free(actual);
        actual = siguiente ;
    }

    THL_Sprites = NULL ;
}

void _THL_DeleteSpriteEntry(struct _THL_Sprite *sprite)
{
    struct _THL_Sprite *actual ;

    if ( THL_Sprites == sprite )
    {
        THL_Sprites = sprite->next ;
    }else
    {
        actual = THL_Sprites ;
        while (actual)
        {
            if ( actual->next == sprite )
            {
                actual->next = sprite->next ;
            }
        }
    }
    SDL_FreeSurface(sprite->surface);
    THL_SpritesIndex[sprite->spHandler] = NULL ;
    free(sprite);
}

void _THL_MusicDone()
{
    Mix_HaltMusic();
    Mix_FreeMusic(THL_Music);
    THL_Music = NULL ;
    _THL_isPlayingMusic = 0 ;
}

double _THL_DegToRad(int deg)
{
    return deg*PIOVER180;
}

int _THL_RadToDeg(double rad)
{
    return (int)(rad/PIOVER180);
}


void _THL_RotateScaleSprite(int handler)
{
    int deg = THL_SpritesIndex[handler]->angle ;
    int scale = THL_SpritesIndex[handler]->scale ;

    while(deg>360) deg-=360;
    while(deg<0) deg+=360;

    SDL_Surface *nuevo, *ant, *temp ;
    double rad = _THL_DegToRad(deg);

    // Realizamos los calculos de escala
    double sc = scale/100.0 ;
    double smod = 100.0/scale ;


    ant = THL_Surfaces[THL_SpritesIndex[handler]->imgHandler] ;

    int n_ancho = (ant->w * fabs(cos( rad )) + ant->h * fabs(sin( rad ))) *sc;
    int n_alto  = (ant->w * fabs(sin( rad )) + ant->h * fabs(cos( rad ))) *sc;

    // Primero creamos el nuevo surface
    temp = SDL_CreateRGBSurface(SDL_HWSURFACE, n_ancho, n_alto, ant->format->BitsPerPixel, ant->format->Rmask, ant->format->Gmask, ant->format->Bmask, ant->format->Amask);
    nuevo = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    SDL_SetColorKey(nuevo, SDL_SRCCOLORKEY, ant->format->colorkey );
    SDL_FillRect(nuevo, NULL, nuevo->format->colorkey);

    

    double acx = THL_SpritesIndex[handler]->_cx ;
    double acy = THL_SpritesIndex[handler]->_cy ;

    

    double ncx = (acx/ant->w) * n_ancho ;
    double ncy = (acy/ant->h) * n_alto ;

    double dacx = acx - ant->w/2 ;
    double dacy = acy - ant->h/2 ;

    double adist = sqrt( pow(dacx, 2.0) + pow(dacy, 2.0));
    double ndist = (adist * scale)/100.0 ;
    
    if ( dacx == 0 ) dacx = 0.0000000001 ;

    double alpha = atan(dacy/dacx)+rad ;
    if ( dacx < 0 ) alpha += PIOVER180 ;

    THL_SpritesIndex[handler]->cx = nuevo->w/2 + ndist*cos(alpha) ;
    THL_SpritesIndex[handler]->cy = nuevo->h/2 - ndist*sin(alpha) ;

    double ty0, nx0, ny0, xdx, xdy, ydx, ydy ;

    // Ahora Calculamos la coordenada n(0,0) en a
    if ( deg >= 0 && deg < 90 )
    {
        ty0 = ant->w * sin( rad ) ;

        nx0 = ty0 * sin( rad ) ;
        ny0 = -ty0 * cos( rad ) ;

    }else if ( deg >= 90 && deg < 180 )
    {
        ty0 = ant->h * sin( rad-PIOVER2 );

        nx0 = ant->w + ty0*cos( rad-PIOVER2 );
        ny0 = ty0 * sin( rad-PIOVER2 );
    }else if ( deg >= 180 && deg < 270 )
    {
        ty0 = ant->w * sin( rad - 2*PIOVER2 );

        nx0 = ant->w - ty0 * sin( rad - 2*PIOVER2 );
        ny0 = ant->h + ty0 * cos( rad - 2*PIOVER2 );
    }else
    {
        ty0 = ant->h * sin( rad - 3*PIOVER2 );

        nx0 = -ty0 * cos( rad - 3*PIOVER2 );
        ny0 = ant->h - ty0*sin( rad - 3*PIOVER2);
    }

    xdx = cos(rad) ;
    xdy = sin(rad) ;

    ydx = -sin(rad) ;
    ydy = cos(rad) ;
       



    int x,y;
    double brx,bry,rx,ry;

    if (SDL_LockSurface(nuevo))
    {
        SDL_FreeSurface(nuevo);
        return ;
    }

    if (SDL_LockSurface(ant))
    {
        SDL_UnlockSurface(nuevo);
        SDL_FreeSurface(nuevo);
        return;
    }



    for ( y = 0 , brx = nx0, bry = ny0 ; y < nuevo->h ; y++, brx+=ydx*smod, bry+=ydy*smod )
    {
        for ( x = 0 , rx = brx, ry = bry ; x < nuevo->w ; x++, rx+=xdx*smod, ry+=xdy*smod )
        {
            if ( rx >= 0 && rx < ant->w && ry >= 0 && ry < ant->h )
            {

                Uint8  *abits, *nbits;
                Uint32 Bpp;

   
                Bpp = ant->format->BytesPerPixel;
                abits = ((Uint8 *)ant->pixels)+((int)ry)*ant->pitch+((int)rx)*Bpp;
                nbits = ((Uint8 *)nuevo->pixels)+y*nuevo->pitch+x*Bpp;
                
                switch(Bpp) 
                {
                    case 1:
                        *nbits = *abits;
                        break;
                    case 2:
                        *((Uint16 *)nbits) = *((Uint16 *)abits);
                        break;
                    case 3: 
                        *((nbits)+nuevo->format->Rshift/8) = *((abits)+ant->format->Rshift/8);
                        *((nbits)+nuevo->format->Gshift/8) = *((abits)+ant->format->Gshift/8);
                        *((nbits)+nuevo->format->Bshift/8) = *((abits)+ant->format->Bshift/8);
                        break;
                    case 4:
                        *((Uint32 *)nbits) = *((Uint32 *)abits);
                        break;
                }
                /*
                (Uint32 *)(((Uint8 *)nuevo->pixels)+y*nuevo->pitch+x*nuevo->format->BytesPerPixel)
                for ( int b = 0 ; b < nuevo->format->BytesPerPixel ; b++ )
                {
                    ((Uint8 *)nuevo->pixels)[y*nuevo->pitch + x*nuevo->format->BytesPerPixel + b] = ((Uint8 *)ant->pixels)[((int)ry)*ant->pitch + ((int)rx)*ant->format->BytesPerPixel + b];
                }
                //((Uint32 *)(nuevo->pixels+(y*nuevo->pitch + x*nuevo->format->BytesPerPixel))) = 0x000000ff ;//((Uint8 *)ant->pixels)[ry*nuevo->pitch + rx*nuevo->format->BytesPerPixel] ;
                */
            }else
            {
                /*
                Uint8  *bits;
                Uint32 Bpp;
   
                Bpp = nuevo->format->BytesPerPixel;
                bits = ((Uint8 *)nuevo->pixels)+y*nuevo->pitch+x*Bpp;



                switch(Bpp) 
                {
                    case 1:
                        *bits = nuevo->format->colorkey;
                        break;
                    case 2:
                        *((Uint16 *)bits) = nuevo->format->colorkey;
                        break;
                    case 3: 
                        *((bits)+0) = (nuevo->format->colorkey&0x0000FF);
                        *((bits)+1) = (nuevo->format->colorkey&0x00FF00)>>8;
                        *((bits)+2) = (nuevo->format->colorkey&0xFF0000)>>16;
                        break;
                    case 4:
                        *((Uint32 *)bits) = nuevo->format->colorkey;
                        break;
                }
                */
                //*((Uint32 *)(((Uint8 *)nuevo->pixels)+y*nuevo->pitch + x*nuevo->format->BytesPerPixel)) = nuevo->format->colorkey ;
                //((Uint8 *)nuevo->pixels)[y*nuevo->pitch + x*nuevo->format->BytesPerPixel] = 0x000000ff ;
            }
        }
    }

    SDL_UnlockSurface(ant);
    SDL_UnlockSurface(nuevo);

    THL_SpritesIndex[handler]->surface = nuevo ;
    THL_SpritesIndex[handler]->_angle = deg ;
    THL_SpritesIndex[handler]->_scale = scale ;
    THL_SpritesIndex[handler]->_redraw = 0 ;   
}














