/*
    NET2 is a threaded, event based, network IO library for SDL.
    Copyright (C) 2002 Bob Pendleton

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation; either version 2.1
    of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    If you do not wish to comply with the terms of the LGPL please
    contact the author as other terms are available for a fee.
    
    Bob Pendleton
    Bob@Pendleton.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDLUtils.h"
#include "fastevents.h"

//----------------------------------------
//
// print utilities
//

void printSDLRect(const SDL_Rect *r)
{
  if (NULL == r)
  {
    return;
  }

  printf("\nSDL_Rect\n");
  printf("x= %d y= %d\n", r->x, r->y);
  printf("w= %d h= %d\n", r->w, r->h);

  fflush(NULL);
}

void printSDLColor(const SDL_Color *c)
{
  if (NULL == c)
  {
    return;
  }

  printf("\nSDL_Color\n");
  printf("R(%2x) G(%2x) B(%2x)\n", (Uint32)c->r, (Uint32)c->g, (Uint32)c->b);

  fflush(NULL);
}

void printSDLPalette(const SDL_Palette *p)
{
  int i = 0;

  if (NULL == p)
  {
    return;
  }

  printf("\nSDL_Palette\n");
  printf("ncolors %d\n", p->ncolors);

  for (i = 0; i < p->ncolors; i++)
  {
    SDL_Color *c = &p->colors[i];
    printf("R(%2x) G(%2x) B(%2x)\n", (Uint32)c->r, (Uint32)c->g, (Uint32)c->b);
  }

  fflush(NULL);
}

void printSDLPixelFormat(const SDL_PixelFormat *f)
{
  if (NULL == f)
  {
    return;
  }

  printf("\nSDL_PixelFormat\n");
  printf("bits/pixel  = %u\n", (Uint32)f->BitsPerPixel);
  printf("bytes/pixel = %u\n", (Uint32)f->BytesPerPixel);
  printf("MASK  R(%08x) G(%08x) B(%08x) A(%08x)\n", 
         (Uint32)f->Rmask, (Uint32)f->Gmask, (Uint32)f->Bmask, (Uint32)f->Amask);
  printf("SHIFT R(%8d) G(%8d) B(%8d) A(%8d)\n", 
         (Uint32)f->Rshift, (Uint32)f->Gshift, (Uint32)f->Bshift, (Uint32)f->Ashift);
  printf("LOSS  R(%8d) G(%8d) B(%8d) A(%8d)\n", 
         (Uint32)f->Rloss, (Uint32)f->Gloss, (Uint32)f->Bloss, (Uint32)f->Aloss);
  printf("ColorKey (%08x)\n", (Uint32)f->colorkey);
  printf("Surface Alpha %d\n", (Uint32)f->alpha);
  if (NULL != f->palette)
  {
    printSDLPalette(f->palette);
  }

  fflush(NULL);
}

void fillSDLPixelFormat(SDL_PixelFormat *f, Uint32 bitspp, Uint32 bytespp)
{
  if (NULL == f)
  {
    return;
  }
  memset(f, 0, sizeof(SDL_PixelFormat));
  f->BitsPerPixel = bitspp;
  f->BytesPerPixel = bytespp;

  fflush(NULL);
}

typedef struct
{
  Uint32 value;
  char *name;
} nameValue;

nameValue initFlags[] =
{
  {SDL_INIT_TIMER, "SDL_INIT_TIMER"},
  {SDL_INIT_AUDIO, "SDL_INIT_AUDIO"},
  {SDL_INIT_VIDEO, "SDL_INIT_VIDEO"},
  {SDL_INIT_CDROM, "SDL_INIT_CDROM"},
  {SDL_INIT_JOYSTICK, "SDL_INIT_JOYSTICK"},
  {SDL_INIT_NOPARACHUTE, "SDL_INIT_NOPARACHUTE"},
  {SDL_INIT_EVENTTHREAD, "SDL_INIT_EVENTTHREAD"},
  {SDL_INIT_EVERYTHING, "SDL_INIT_EVERYTHING"},
};
const int numInitFlags = sizeof(initFlags) / sizeof(nameValue);

void printSDLInitFlags(const Uint32 f)
{
  int i = 0;

  printf("\nSDL_InitFlags (%08x)\n", f);
  for (i = 0; i < numInitFlags; i++)
  {
    if (0 != (f & initFlags[i].value))
    {
      printf(" %08x %s\n", initFlags[i].value, initFlags[i].name);
    }
  }

  fflush(NULL);
}

nameValue surfaceFlags[] = 
{
  /* Available for SDL_CreateRGBSurface() or SDL_SetVideoMode() */
  {SDL_SWSURFACE, "SDL_SWSURFACE"},
  {SDL_HWSURFACE, "SDL_HWSURFACE"},
  {SDL_ASYNCBLIT, "SDL_ASYNCBLIT"},
  /* Available for SDL_SetVideoMode() */
  {SDL_ANYFORMAT, "SDL_ANYFORMAT"},
  {SDL_HWPALETTE, "SDL_HWPALETTE"},
  {SDL_DOUBLEBUF, "SDL_DOUBLEBUF"},
  {SDL_FULLSCREEN, "SDL_FULLSCREEN"},
  {SDL_OPENGL, "SDL_OPENGL"},
  {SDL_OPENGLBLIT, "SDL_OPENGLBLIT"},
  {SDL_RESIZABLE, "SDL_RESIZABLE"},
  {SDL_NOFRAME, "SDL_NOFRAME"},
  /* Used internally (read-only) */
  {SDL_HWACCEL, "SDL_HWACCEL"},
  {SDL_SRCCOLORKEY, "SDL_SRCCOLORKEY"},
  {SDL_RLEACCELOK, "SDL_RLEACCELOK"},
  {SDL_RLEACCEL, "SDL_RLEACCEL"},
  {SDL_SRCALPHA, "SDL_SRCALPHA"},
  {SDL_PREALLOC, "SDL_PREALLOC"},
};
const int numSurfaceFlags = sizeof(surfaceFlags) / sizeof(nameValue);

void printSDLSurfaceFlags(const Uint32 f)
{
  int i = 0;

  printf("\nSDL_SurfaceFlags (%08x)\n", f);
  // Weirdly, SDL_SWSURFACE == 0 you have to guess when you really have one

  if ((0 == (f & SDL_HWSURFACE)) &&
      (0 == (f & SDL_OPENGL)))
  {
    printf(" %08x %s\n", surfaceFlags[0].value, surfaceFlags[0].name);
  }

  for (i = 0; i < numSurfaceFlags; i++)
  {
    if (0 != (f & surfaceFlags[i].value))
    {
      printf(" %08x %s\n", surfaceFlags[i].value, surfaceFlags[i].name);
    }
  }

  fflush(NULL);
}

void printSDLSurface(const SDL_Surface *s)
{
  if (NULL == s)
  {
    return;
  }

  printf("\nSDL_Surface\n");
  printSDLSurfaceFlags(s->flags);
  printSDLPixelFormat(s->format);
  printf("w= %d h= %d\n", s->w, s->h);
  printf("pitch= %d\n", (Uint32)s->pitch);
  printSDLRect(&s->clip_rect);
  printf("refcount=%d\n", s->refcount);

  fflush(NULL);
}

void printSDLVideoInfo(const SDL_VideoInfo *vi)
{
  if (NULL == vi)
  {
    return;
  }

  printf("\nSDL_VideoInfo\n");
  printf("hw_available %d\n", vi->hw_available);
  printf("wm_available %d\n", vi->wm_available);
  printf("blit_hw      %d\n", vi->blit_hw);
  printf("blit_hw_CC   %d\n", vi->blit_hw_CC);
  printf("blit_hw_A    %d\n", vi->blit_hw_A);
  printf("blit_sw      %d\n", vi->blit_sw);
  printf("blit_sw_CC   %d\n", vi->blit_sw_CC);
  printf("blit_sw_A    %d\n", vi->blit_sw_A);
  printf("blit_fill    %d\n", vi->blit_fill);
  printf("video_mem    %d\n", vi->video_mem);
  printSDLPixelFormat(vi->vfmt);

  fflush(NULL);
}

nameValue appFlags[] =
{
  {SDL_APPMOUSEFOCUS, "SDL_APPMOUSEFOCUS"},
  {SDL_APPINPUTFOCUS, "SDL_APPINPUTFOCUS"},
  {SDL_APPACTIVE, "SDL_APPACTIVE"},
};
const int numAppFlags = sizeof(appFlags) / sizeof(nameValue);

nameValue modifiers[] =
{
  {KMOD_LSHIFT, "LSHIFT"},
  {KMOD_RSHIFT, "RSHIFT"},
  {KMOD_LCTRL, "LCTRL"},
  {KMOD_RCTRL, "RCTRL"},
  {KMOD_LALT, "LALT"},
  {KMOD_RALT, "RALT"},
  {KMOD_LMETA, "LMETA"},
  {KMOD_RMETA, "RMETA"},
  {KMOD_NUM, "NUM"},
  {KMOD_CAPS, "CAPS"},
  {KMOD_MODE, "MODE"},
};
const int numModifiers = sizeof(modifiers) / sizeof(nameValue);

void printSDLModifiers(SDLMod mod)
{
  int i = 0;

  if(!mod) 
  {
    printf(" (none)\n");
    return;
  }

  for (i = 0; i < numModifiers; i++)
  {
    if(0 != (mod & modifiers[i].value))
    {
      printf(" %s", modifiers[i].name);
    }
  }

  printf("\n");

  fflush(NULL);
}

void printSDLEvent(SDL_Event *e)
{
  SDL_keysym *sym = NULL;
  int i = 0;

  switch (e->type)
  {
  case SDL_ACTIVEEVENT:			/* Application loses/gains visibility */
    printf("SDL_ACTIVEEVENT\n");
    printf("    gain    =%d\n", (Uint32)e->active.gain);
    printf("    state   =%02x\n", (Uint32)e->active.state);
    for (i = 0; i < numAppFlags; i++)
    {
      if (0 != (appFlags[i].value & e->active.state))
      {
        printf("            =%02x,%s\n", appFlags[i].value, appFlags[i].name);
      }
    }
    break;

  case SDL_KEYDOWN:			/* Keys pressed */
    sym = &e->key.keysym;
    printf("SDL_KEYDOWN\n");
    printf("    which   =%02x\n", (Uint32)e->key.which);
    printf("    state   =%02x\n", (Uint32)e->key.state);
    printf("    scancode=%d\n", (Uint32)sym->scancode);
    if (0 != sym->sym)
    {
      printf("    sym     =%d,\"%c\",\"%s\"\n", 
             sym->sym, 
             sym->sym, 
             SDL_GetKeyName(sym->sym));
    }
    printf("    mod     =%04x ", (Uint32)sym->mod);
    printSDLModifiers(sym->mod);
    printf("    unicode =%04x ", (Uint32)sym->unicode);
    if (sym->unicode)
    {
      /* Is it a control-character? */
      if (sym->unicode < ' ') 
      {
        printf("(^%c)", sym->unicode+'@');
      }
      else
      {
#ifdef UNICODE
        printf("(%c)", sym->unicode);
#else
        /* This is a Latin-1 program, so only show 8-bits */
        if (!(sym->unicode & 0xFF00))
        {
          printf("(%c)", sym->unicode);
        }
#endif
      }
    }
    printf("\n");
    break;

  case SDL_KEYUP:			/* Keys released */
    sym = &e->key.keysym;
    printf("SDL_KEYUP\n");
    printf("    which   =%02x\n", (Uint32)e->key.which);
    printf("    state   =%02x\n", (Uint32)e->key.state);
    printf("    scancode=%d\n", (Uint32)sym->scancode);
    if (0 != sym->sym)
    {
      printf("    sym     =%d,\"%c\",\"%s\"\n", 
             sym->sym, 
             sym->sym, 
             SDL_GetKeyName(sym->sym));
    }
    printf("    mod     =%04x ", (Uint32)sym->mod);
    printSDLModifiers(sym->mod);
    printf("    unicode =%04x ", (Uint32)sym->unicode);
    if (sym->unicode)
    {
      /* Is it a control-character? */
      if (sym->unicode < ' ') 
      {
        printf("(^%c)", sym->unicode+'@');
      }
      else
      {
#ifdef UNICODE
        printf("(%c)", sym->unicode);
#else
        /* This is a Latin-1 program, so only show 8-bits */
        if (!(sym->unicode & 0xFF00))
        {
          printf("(%c)", sym->unicode);
        }
#endif
      }
    }
    printf("\n");
    break;

  case SDL_MOUSEMOTION:			/* Mouse moved */
    printf("SDL_MOUSEMOTION\n");
    printf("    which   =%d\n", (Uint32)e->motion.which);
    printf("    state   =%01x\n", (Uint32)e->motion.state);
    printf("    abs x,y =%d,%d\n", (Uint32)e->motion.x, (Uint32)e->motion.y);
    printf("    rel x,y =%d,%d\n", (Sint32)e->motion.xrel, (Sint32)e->motion.yrel);
    break;

  case SDL_MOUSEBUTTONDOWN:		/* Mouse button pressed */
    printf("SDL_MOUSEBUTTONDOWN\n");
    printf("    which   =%d\n", (Uint32)e->button.which);
    printf("    button  =%d\n", (Uint32)e->button.button);
    printf("    state   =%01x\n", (Uint32)e->button.state);
    printf("    abs x,y =%d,%d\n", (Uint32)e->motion.x, (Uint32)e->motion.y);
    break;

  case SDL_MOUSEBUTTONUP:		/* Mouse button released */
    printf("SDL_MOUSEBUTTONUP\n");
    printf("    which   =%d\n", (Uint32)e->button.which);
    printf("    button  =%d\n", (Uint32)e->button.button);
    printf("    state   =%01x\n", (Uint32)e->button.state);
    printf("    abs x,y =%d,%d\n", (Uint32)e->motion.x, (Uint32)e->motion.y);
    break;

  case SDL_JOYAXISMOTION:		/* Joystick axis motion */
    printf("SDL_JOYAXISMOTION\n");
    printf("    which   =%d\n", (Uint32)e->jaxis.which);
    printf("    axis    =%d\n", (Uint32)e->jaxis.axis);
    printf("    value   =%d\n", (Sint32)e->jaxis.value);
    break;

  case SDL_JOYBALLMOTION:		/* Joystick trackball motion */
    printf("SDL_JOYBALLMOTION\n");
    printf("    which   =%d\n", (Uint32)e->jball.which);
    printf("    ball    =%d\n", (Uint32)e->jball.ball);
    printf("    rel x,y =%d,%d\n", (Sint32)e->jball.xrel, (Sint32)e->jball.yrel);
    break;

  case SDL_JOYHATMOTION:		/* Joystick hat position change */
    printf("SDL_JOYHATMOTION\n");
    printf("    which   =%d\n", (Uint32)e->jhat.which);
    printf("    hat     =%d\n", (Uint32)e->jhat.hat);
    printf("    value   =%d\n", (Sint32)e->jhat.value);
    break;

  case SDL_JOYBUTTONDOWN:		/* Joystick button pressed */
    printf("SDL_JOYBUTTONDOWN\n");
    printf("    which   =%d\n", (Uint32)e->jbutton.which);
    printf("    button  =%d\n", (Uint32)e->jbutton.button);
    printf("    state   =%01x\n", (Uint32)e->jbutton.state);
    break;

  case SDL_JOYBUTTONUP:			/* Joystick button released */
    printf("SDL_JOYBUTTONUP\n");
    printf("    which   =%d\n", (Uint32)e->jbutton.which);
    printf("    button  =%d\n", (Uint32)e->jbutton.button);
    printf("    state   =%01x\n", (Uint32)e->jbutton.state);
    break;

  case SDL_QUIT:			/* User-requested quit */
    printf("SDL_QUIT\n");
    break;

  case SDL_SYSWMEVENT:			/* System specific event */
    printf("SDL_SYSWMEVENT\n");
    break;

  case SDL_VIDEORESIZE:			/* User resized video mode */
    printf("SDL_VIDEORESIZE\n");
    printf("    w,h     =%d,%d\n", (Sint32)e->resize.w, (Sint32)e->resize.h);
    break;

  case SDL_VIDEOEXPOSE:			/* Screen needs to be redrawn */
    printf("SDL_VIDEOEXPOSE\n");
    break;

  default:
    if ((SDL_USEREVENT <= e->type) && (e->type <= (SDL_NUMEVENTS - 1)))
    {
      printf("SDL_USEREVENT\n");
      printf("    code    =%d\n", e->user.code);
      printf("    data1   =%08x\n", (Uint32)e->user.data1);
      printf("    data2   =%08x\n", (Uint32)e->user.data2);
    }
    else
    {
      printf("Unknown event type %08x\n", e->type);
    }
    break;
  }

  fflush(NULL);
}

void printSDLOpenGLAttrs()
{
  int value;
  if (0 == SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_RED_SIZE     ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_GREEN_SIZE   ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_BLUE_SIZE    ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_ALPHA_SIZE   ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_BUFFER_SIZE  ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &value))
  {
    printf("%s=%3d\n", "SDL_GL_DOUBLEBUFFER ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_DEPTH_SIZE   ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_STENCIL_SIZE ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_ACCUM_RED_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_ACCUM_RED    ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_ACCUM_GREEN_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_ACCUM_GREEN  ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_ACCUM_BLUE_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_ACCUM_BLUE   ", value);
  }

  if (0 == SDL_GL_GetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, &value))
  {
    printf("%s=%3d\n", "SDL_GL_ACCUM_ALPHA  ", value);
  }

  fflush(NULL);
}

void printIPaddress(IPaddress *addr)
{
  printf("host=%d.%d.%d.%d\n", 
         ((addr->host >>  0) & 0xff),
         ((addr->host >>  8) & 0xff),
         ((addr->host >> 16) & 0xff),
         ((addr->host >> 24) & 0xff));

  printf("port=%4d\n", addr->port);

  fflush(NULL);
}

void printBytes(char *buf, int len)
{
  int width = 16;
  int i;
  int j;

  for (i = 0; i < len; i += width)
  {
    for (j = i; j < (i + width); j++)
    {
      printf("%02x ", buf[j]);
    }
    printf(" - ");

    for (j = i; j < (i + width); j++)
    {
      char c = buf[j];

      if ((' ' <= c) && (c < 127))
      {
        printf("%c", c);
      }
      else
      {
        printf(" ");
      }
    }
    printf("\n");
  }
}

void printUDPpacket(UDPpacket *packet)
{
  printf("channel   =%d\n", packet->channel);
  printf("len       =%d\n", packet->len);
  printf("maxlen    =%d\n", packet->maxlen);
  printf("status    =%d\n", packet->status);
  printIPaddress(&packet->address);
  printBytes(packet->data, packet->maxlen);
}
//----------------------------------------
//
// SDL helper functions
//

void mySDL_Quit()
{
  FE_Quit();
  SDLNet_Quit();
  SDL_Quit();
}

char *mySDL_Init(Uint32 flags)
{
  if (-1 == SDL_Init(flags))
  {
    return SDL_GetError();
  }

  if (-1 == SDLNet_Init())
  {
    return SDLNet_GetError();
  }

  if (-1 == FE_Init())
  {
    return FE_GetError();
  }

  return NULL;
}

void mySDLInitOrQuit(Uint32 flags)
{
  char *message = NULL;


  if (NULL != (message = mySDL_Init(flags)))
  {
    printf("Failed to initialize SDL error=%s\n", message);
    exit(1);
  }

  atexit(mySDL_Quit);
}

