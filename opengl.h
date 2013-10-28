double opengl_init(int, int);
int    opengl_init_texture(pic_t * pic);
void   opengl_delete_texture(pic_t * pic);
void   opengl_blit(int pixel_ref_size, double x, double y, pic_t * src, double size, double a);
void   opengl_clear_screen();
