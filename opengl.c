#include "image.h"
#include "debug.h"

double opengl_init(int window_w, int window_h)
{
	double screen_ratio;

        screen_ratio = (double)window_w / (double)window_h;
        glEnable( GL_TEXTURE_2D ); // Need this to display a texture
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport( 0, 0, window_w, window_h );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

//      gluPerspective(45.0, WINDOW_SIZE_X/WINDOW_SIZE_Y, 1.0, 5000.0);
        glOrtho(0, window_w, window_h, 0, 1, -1);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        /* do the initial display */
        glClearColor(0, 0, 0, 0);
        glClear( GL_COLOR_BUFFER_BIT );
        glClearDepth(0.0f);

	return screen_ratio;
}

int opengl_init_texture(img_t * img)
{
        GLenum texture_format;
        GLint  nOfColors;

	if( img->surf->format == NULL ) {
		return -1;
	}

        if( img->init == 0) {
                // get the number of channels in the SDL surface
                nOfColors = img->surf->format->BytesPerPixel;
                if (nOfColors == 4)     // contains an alpha channel
                {
                        if (img->surf->format->Rmask == 0x000000ff)
                                texture_format = GL_RGBA;
                        else
                                texture_format = GL_BGRA;
                } else if (nOfColors == 3)     // no alpha channel
                {
                        if (img->surf->format->Rmask == 0x000000ff)
                                texture_format = GL_RGB;
                        else
                                texture_format = GL_BGR;
                } else {
                        printd(DEBUG_SDL,"Not a true color image\n");
                        return -1;
                }
                glEnable(GL_TEXTURE_2D);
                glGenTextures( 1, &(img->tex) );
                glBindTexture( GL_TEXTURE_2D, img->tex );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, img->surf->w, img->surf->h, 0,
                                texture_format, GL_UNSIGNED_BYTE, img->surf->pixels );
                img->init = 1;
        }

        return 0;
}

void opengl_delete_texture(img_t * img)
{
	glDeleteTextures( 1, &img->tex );
	SDL_FreeSurface(img->surf);
	free(img);
}

void opengl_blit(int pixel_ref_size, double x, double y, img_t * src, double size, double a)
{
        int w;
        int h;

        int px = (int)((double)pixel_ref_size * x);
        int py = (int)((double)pixel_ref_size * y);

        if( src->ratio > 1.0 ) {
                w = (int)((double)pixel_ref_size * size);
                h = (int)((double)pixel_ref_size * src->h * size);
        }
        else {
                w = (int)((double)pixel_ref_size * src->w * size);
                h = (int)((double)pixel_ref_size * size);
        }

        if( opengl_init_texture(src) != 0) {
		return;
	}

        // Bind the texture to which subsequent calls refer to
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, src->tex );
/*
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(0.5,0.5,0.0);
        glRotatef(a,0.0,0.0,1.0);
        glTranslatef(-0.5,-0.5,0.0);
*/

        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
//      glTranslatef(px,py, 0.0);
        glTranslatef(px,py, 0.0);
        glTranslatef(w/2,h/2, 0.0);
        glRotatef(a,0.0,0.0,1.0);

        glBegin( GL_QUADS );
        // Top-left vertex (corner)
        glTexCoord2i( 0, 0 );
//        glVertex3f( 0.0, 0.0, 0.0 );
        glVertex3f( -w/2, -h/2, 0.0 );

        // Bottom-left vertex (corner)
        glTexCoord2i( 0, 1 );
//        glVertex3f( 0.0, h, 0.0 );
        glVertex3f( -w/2, h/2, 0.0 );

        // Bottom-right vertex (corner)
        glTexCoord2i( 1, 1 );
//        glVertex3f( w, h, 0.0 );
        glVertex3f( w/2, h/2, 0.0 );

        // Top-right vertex (corner)
        glTexCoord2i( 1, 0 );
//        glVertex3f( w, 0, 0.0 );
        glVertex3f( w/2, -h/2, 0.0 );
        glEnd();
}

void opengl_clear_screen()
{
	glClearColor(0, 0, 0, 0);
	glClear( GL_COLOR_BUFFER_BIT );
	glClearDepth(0.0f);
}
