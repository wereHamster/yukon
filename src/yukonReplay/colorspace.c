
#include <yukonCore/yCore.h>

static int32_t RGB_Y_tab[256];
static int32_t B_U_tab[256];
static int32_t G_U_tab[256];
static int32_t G_V_tab[256];
static int32_t R_V_tab[256];


#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

/********** generic colorspace macro **********/


#define MAKE_COLORSPACE(NAME,SIZE,PIXELS,VPIXELS,FUNC,C1,C2,C3,C4) \
void	\
NAME(uint8_t * x_ptr, int x_stride,	\
				 uint8_t * y_ptr, uint8_t * u_ptr, uint8_t * v_ptr,	\
				 int y_stride, int uv_stride,	\
				 int width, int height, int vflip)	\
{	\
	int fixed_width = (width + 1) & ~1;				\
	int x_dif = x_stride - (SIZE)*fixed_width;		\
	int y_dif = y_stride - fixed_width;				\
	int uv_dif = uv_stride - (fixed_width / 2);		\
	int x, y;										\
	if (vflip) {								\
		x_ptr += (height - 1) * x_stride;			\
		x_dif = -(SIZE)*fixed_width - x_stride;		\
		x_stride = -x_stride;						\
	}												\
	for (y = 0; y < height; y+=(VPIXELS)) {			\
		FUNC##_ROW(SIZE,C1,C2,C3,C4);				\
		for (x = 0; x < fixed_width; x+=(PIXELS)) {	\
			FUNC(SIZE,C1,C2,C3,C4);				\
			x_ptr += (PIXELS)*(SIZE);				\
			y_ptr += (PIXELS);						\
			u_ptr += (PIXELS)/2;					\
			v_ptr += (PIXELS)/2;					\
		}											\
		x_ptr += x_dif + (VPIXELS-1)*x_stride;		\
		y_ptr += y_dif + (VPIXELS-1)*y_stride;		\
		u_ptr += uv_dif + ((VPIXELS/2)-1)*uv_stride;	\
		v_ptr += uv_dif + ((VPIXELS/2)-1)*uv_stride;	\
	}												\
}



/********** colorspace output (yv12_to_xxx) functions **********/

/* yuv -> rgb def's */

#define RGB_Y_OUT		1.164
#define B_U_OUT			2.018
#define Y_ADD_OUT		16

#define G_U_OUT			0.391
#define G_V_OUT			0.813
#define U_ADD_OUT		128

#define R_V_OUT			1.596
#define V_ADD_OUT		128

#define SCALEBITS_OUT	13
#define FIX_OUT(x)		((uint16_t) ((x) * (1L<<SCALEBITS_OUT) + 0.5))




/* rgb/rgbi output */

#define WRITE_RGB(SIZE,ROW,UV_ROW,C1,C2,C3,C4)	\
	rgb_y = RGB_Y_tab[ y_ptr[(ROW)*y_stride + 0] ];						\
	x_ptr[(ROW)*x_stride+(C3)] = MAX(0, MIN(255, (rgb_y + b_u##UV_ROW) >> SCALEBITS_OUT));	\
	x_ptr[(ROW)*x_stride+(C2)] = MAX(0, MIN(255, (rgb_y - g_uv##UV_ROW) >> SCALEBITS_OUT));	\
	x_ptr[(ROW)*x_stride+(C1)] = MAX(0, MIN(255, (rgb_y + r_v##UV_ROW) >> SCALEBITS_OUT));	\
	if ((SIZE)>3) x_ptr[(ROW)*x_stride+(C4)] = 0;									\
	rgb_y = RGB_Y_tab[ y_ptr[(ROW)*y_stride + 1] ];									\
	x_ptr[(ROW)*x_stride+(SIZE)+(C3)] = MAX(0, MIN(255, (rgb_y + b_u##UV_ROW) >> SCALEBITS_OUT));	\
	x_ptr[(ROW)*x_stride+(SIZE)+(C2)] = MAX(0, MIN(255, (rgb_y - g_uv##UV_ROW) >> SCALEBITS_OUT));	\
	x_ptr[(ROW)*x_stride+(SIZE)+(C1)] = MAX(0, MIN(255, (rgb_y + r_v##UV_ROW) >> SCALEBITS_OUT));	\
	if ((SIZE)>3) x_ptr[(ROW)*x_stride+(SIZE)+(C4)] = 0;


#define YV12_TO_RGB_ROW(SIZE,C1,C2,C3,C4) 	/* nothing */
#define YV12_TO_RGB(SIZE,C1,C2,C3,C4)				\
	int rgb_y;												\
	int b_u0 = B_U_tab[ u_ptr[0] ];							\
	int g_uv0 = G_U_tab[ u_ptr[0] ] + G_V_tab[ v_ptr[0] ];	\
	int r_v0 = R_V_tab[ v_ptr[0] ];							\
	WRITE_RGB(SIZE, 0, 0, C1,C2,C3,C4)						\
	WRITE_RGB(SIZE, 1, 0, C1,C2,C3,C4)

#define YV12_TO_RGBI_ROW(SIZE,C1,C2,C3,C4) 	/* nothing */
#define YV12_TO_RGBI(SIZE,C1,C2,C3,C4)				\
	int rgb_y;												\
	int b_u0 = B_U_tab[ u_ptr[0] ];							\
	int g_uv0 = G_U_tab[ u_ptr[0] ] + G_V_tab[ v_ptr[0] ];	\
	int r_v0 = R_V_tab[ v_ptr[0] ];							\
    int b_u1 = B_U_tab[ u_ptr[uv_stride] ];					\
	int g_uv1 = G_U_tab[ u_ptr[uv_stride] ] + G_V_tab[ v_ptr[uv_stride] ];	\
	int r_v1 = R_V_tab[ v_ptr[uv_stride] ];					\
	WRITE_RGB(SIZE, 0, 0, C1,C2,C3,C4)		\
	WRITE_RGB(SIZE, 1, 1, C1,C2,C3,C4)		\
	WRITE_RGB(SIZE, 2, 0, C1,C2,C3,C4)		\
	WRITE_RGB(SIZE, 3, 1, C1,C2,C3,C4)



MAKE_COLORSPACE(yv12_to_rgba_c,    4,2,2, YV12_TO_RGB,    0,1,2,3)


/* initialize rgb lookup tables */

void
colorspace_init(void)
{
	int32_t i;

	for (i = 0; i < 256; i++) {
		RGB_Y_tab[i] = FIX_OUT(RGB_Y_OUT) * (i - Y_ADD_OUT);
		B_U_tab[i] = FIX_OUT(B_U_OUT) * (i - U_ADD_OUT);
		G_U_tab[i] = FIX_OUT(G_U_OUT) * (i - U_ADD_OUT);
		G_V_tab[i] = FIX_OUT(G_V_OUT) * (i - V_ADD_OUT);
		R_V_tab[i] = FIX_OUT(R_V_OUT) * (i - V_ADD_OUT);
	}
}
