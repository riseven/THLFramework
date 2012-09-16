
#include <stdio.h>
#include <stdlib.h>
#include "sdl_keysym.h"
#include "thl.h"

int main()
{
    int fuente;
    int texto;
    int imagen ;
    int sp ;

    // Iniciamos el THLFramework
    if (THL_Init(640,480,24,THL_CLEAR_BACK))
    {
        exit(0);
    }

    fuente = THL_LoadFont("fuente.png");
    texto = THL_Write(fuente, 20, 20, "Hola Edu");
    THL_DefineRegion(1, 30, 10,  100, 200 );
    THL_SetSpriteControl( texto, 0, 0); 
    imagen = THL_LoadImage("prueba.jpg");
    sp = THL_NewSprite(imagen, 0,0,0,0,101,0);

    while(1) {
        if ( THL_Key(SDLK_z))
            THL_ScaleSpriteRel(texto, 1);

        if ( THL_Key(SDLK_x))
            THL_ScaleSpriteRel(texto, -1);

        if ( THL_Key(SDLK_RIGHT))
            THL_MoveSpriteRel(texto, 5, 0);

        if ( THL_Key(SDLK_LEFT))
            THL_MoveSpriteRel(texto, -5, 0);

        if ( THL_Key(SDLK_UP))
            THL_MoveSpriteRel(texto, -0, -5);

        if ( THL_Key(SDLK_DOWN))
            THL_MoveSpriteRel(texto, 0, 5);

        if ( THL_Key(SDLK_a))
        {
            THL_RotateSpriteRel(texto, 1);
        }

        if ( THL_Key(SDLK_s))
            THL_RotateSpriteRel(texto, -1);

        if ( THL_MouseLeft() )
            THL_SetSpriteRegion(texto, 0);

        if ( THL_MouseRight() )
            THL_SetSpriteRegion(texto, 1);

        if ( THL_MouseLeft() )
            THL_Rewrite(texto, fuente, "Adios Edu");
        
        THL_DrawScreen();
    }

    // Cerramos el THLFramework
    THL_Close();
}