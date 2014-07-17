/*

--------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl                                        
              
--------------------------------------------------------------------------------


  Tiny library with a couple of handy functions for opengl based applications.
  Sent changes to: https://gist.github.com/roxlu/7788294

  MATRIX
  -----------------------------------------------------------------------------------
  rx_perspective(45.0f, 4.0f/3.0f, 0.1, 10.0f, m);      - create a pespective matrix
  rx_translate(0.0f, 0.0f, -4.0f, m);                   - translate the given matrix
  rx_translation(0.0f, 0.0f, -4.0f, m);                 - set identity + translate a matrix
  rx_rotation(HALF_PI, 0.0f, 1.0f, 0.0, m);             - get a rotation matrix (angle axis based)
  rx_print_mat4x4(m);                                   - print the contents of a matrix

  VECTOR
  -----------------------------------------------------------------------------------
  rx_set_vec3(x, y, z, result);                         - set the values of the vector
  rx_multiply_vec3(a,b,result);                         - multiply two vectors
  rx_multiply_vec3(a, 10.0f, result);                   - multiply vector a by 10.0f
  rx_normalize_vec3(a, result);                         - normalize the given vector
  rx_cross_vec3(a, b, result);                          - get the cross product between two vectors
  rx_print_vec3(v);                                     - print the contents of the vector
  float length = rx_length_vec3(a,b);                   - get the length of the vector

  SHADER
  -----------------------------------------------------------------------------------
  vert = rx_create_shader(GL_VERTEX_SHADER, source_char_p);    - create a shader, pass type
  prog = rx_create_program(vert, frag);                        - create a problem - DOES NOT LINK


  IMAGES
  -----------------------------------------------------------------------------------
  rx_save_png("filename.png", pixels, 640, 480, 3);            - writes a png using lib png
  rx_load_png("filepath.png", &pix, width, height, nchannels)  - load the pixels, width, height and nchannels for the given filepath. make sure to delete pix (which is unsigned char*)

  UTILS
  -----------------------------------------------------------------------------------
  std::string path = rx_get_exe_path();                  - returns the path to the exe 
  std::string contents = rx_read_file("filepath.txt");   - returns the contents of the filepath.

 */

#ifndef ROXLU_TINYLIB_H
#define ROXLU_TINYLIB_H

#include <cmath>
#include <iostream>
#include <assert.h>
#include <iterator>
#include <algorithm>
#include <string>
#include <fstream>

#if defined(__APPLE__)
#  if !defined(__gl_h_)
#    include <OpenGL/gl3.h>
#    include <OpenGL/glext.h>
#  endif
#  include <libgen.h> /* dirname */
#  include <CoreFoundation/CFRunLoop.h>
#  include <mach/mach.h>
#  include <mach/mach_time.h>
#  include <mach-o/dyld.h> /* _NSGetExecutablePath */
#  include <sys/resource.h>
#  include <sys/sysctl.h>
#  include <sys/stat.h> /* stat() */
#  include <unistd.h>  /* sysconf */
#else 
#  include <GL/glx.h>
#endif

#include <png.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530717958647693
#endif

#ifndef M_TWO_PI
#define M_TWO_PI 6.28318530717958647693
#endif

#ifndef FOUR_PI
#define FOUR_PI 12.56637061435917295385
#endif

#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI/180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0/PI)
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(val,min,max) (MAX(MIN(val,max),min))
#endif

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

#ifndef DX
#define DX(i,j,w)((j)*(w))+(i)
#endif

static void rx_frustum(float l, float r, float b, float t, float n, float f, float* m) {
  m[1]  = 0.0f;
  m[2]  = 0.0f;
  m[3]  = 0.0f;
  m[4]  = 0.0f;
  m[6]  = 0.0f;
  m[7]  = 0.0f;
  m[12] = 0.0f;
  m[13] = 0.0f;

  m[0]  = 2.0f * n / (r-l);
  m[5]  = 2.0f * n / (t-b);
  m[8]  = (r+l) / (r-l);
  m[9]  = (t+b) / (t-b);
  m[10] = -(f+n) / (f-n);
  m[11] = -1.0f;
  m[14] = -2.0f * f * n / (f-n);
  m[15] = 0.0f;
}

static void rx_perspective(float fovDegrees, float aspect, float n, float f, float* m) {
  float hh = tanf(fovDegrees * DEG_TO_RAD * 0.5) * n;
  return rx_frustum(-hh*aspect, hh*aspect, -hh, hh, n, f, m);
}

static void rx_translate(float x, float y, float z, float* m) {
  m[12] = x;
  m[13] = y;
  m[14] = z;
}

static void rx_identity(float* m) {
  m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
  m[3] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
  m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
  m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

static void rx_translation(float x, float y, float z, float* m) {
  rx_identity(m);
  rx_translate(x, y, z, m);
}

static void rx_rotation(float rad, float x, float y, float z, float* m) {
  float s = sin(rad);
  float c = cos(rad);
  float magnitude = sqrt(x*x + y*y + z*z);
  float xm = x / magnitude;
  float ym = y / magnitude;
  float zm = z / magnitude;

  m[0] = c + (xm*xm) * (1-c);
  m[1] = ym*xm*(1-c)+zm*s;
  m[2] = zm*xm*(1-c) - ym*s;
  m[3] = 0;

  m[4] = ym*xm*(1-c)-zm*s;
  m[5] = c+(ym*ym)*(1-c);
  m[6] = zm*ym*(1-c) + xm*s;
  m[7] = 0.0f;

  m[8] =  zm*xm*(1-c)+ym*s;
  m[9] = ym*zm*(1-c)-xm*s;
  m[10] = c+zm*zm*(1-c);
  m[11] = 0.0f;
  
  m[12] = 0.0f;
  m[13] = 0.0f;
  m[14] = 0.0f;
  m[15] = 1.0f;
}

static void rx_print_mat4x4(float* m) {
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[0], m[4], m[8], m[12]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[1], m[5], m[9], m[13]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[2], m[6], m[10], m[14]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[3], m[7], m[11], m[15]);
  printf("\n");
}

static void rx_multiply_vec3(float* a, float* b, float* result) {
  result[0] = a[0] * b[0];
  result[1] = a[1] * b[1];
  result[2] = a[2] * b[2];
}

static void rx_multiply_vec3(float* a, float v, float* result) {
  result[0] = a[0] * v;
  result[1] = a[1] * v;
  result[2] = a[2] * v;
}

static float rx_length_vec3(float* a) {
  return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

static void rx_normalize_vec3(float* a, float* result) {
  float l = 1.0f / rx_length_vec3(a);
  rx_multiply_vec3(a, l, result);
}

static void rx_set_vec3(float x, float y, float z, float* result) {
  result[0] = x;
  result[1] = y;
  result[2] = z;
}

static void rx_cross_vec3(float* a, float* b, float* result) {
  result[0] = a[1] * b[2] - a[2] * b[1];
  result[1] = a[2] * b[0] - a[0] * b[2];
  result[2] = a[0] * b[1] - a[1] * b[0];
}

static void rx_print_vec3(float* v) {
  printf("x: %f, y: %f, z: %f\n", v[0], v[1], v[2]);
}

static void rx_print_shader_link_info(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  GLsizei nchars = 0;
  glGetProgramiv(shader, GL_LINK_STATUS, &status);
  if(!status) {
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetProgramInfoLog(shader, 2048, &nchars, error);
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

static void rx_print_shader_compile_info(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(!status) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetShaderInfoLog(shader, count, NULL, error);
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

static GLuint rx_create_program(GLuint vert, GLuint frag) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  return prog;
}

static GLuint rx_create_shader(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src,  NULL);
  glCompileShader(s);
  rx_print_shader_compile_info(s);
  return s;
}

// UTILS
// ---------------------------------------------------------------------------
#if defined(_WIN32) // rx_get_exe_path()
static std::string rx_get_exe_path() {
  char buffer[MAX_PATH];

  // Try to get the executable path with a buffer of MAX_PATH characters.
  DWORD result = ::GetModuleFileNameA(nullptr, buffer, static_cast<DWORD>(MAX_PATH));
  if(result == 0) {
    return "";
  }

  std::string::size_type pos = std::string(buffer).find_last_of( "\\/" );

  return std::string(buffer).substr(0, pos) +"\\";
}
#elif defined(__APPLE__) // rx_get_exe_path()
static std::string rx_get_exe_path() {
  char buffer[1024];
  uint32_t usize = sizeof(buffer);;

  int result;
  char* path;
  char* fullpath;

  result = _NSGetExecutablePath(buffer, &usize);
  if (result) {
    return "";
  }

  path = (char*)malloc(2 * PATH_MAX);
  fullpath = realpath(buffer, path);

  if (fullpath == NULL) {
    free(path);
    return "";
  }
  strncpy(buffer, fullpath, usize);

  const char* dn = dirname(buffer);
  usize = strlen(dn);
  std::string ret(dn, usize) ;
  ret.push_back('/');

  free(path);
  return ret;
}
#elif defined(__linux) // rx_get_exe_path()
static std::string rx_get_exe_path() {
  char buffer[MAX_PATH];
  size_t size = MAX_PATH;
  ssize_t n = readlink("/proc/self/exe", buffer, size - 1);
  if (n <= 0) {
    return "";
  }
  buffer[n] = '\0';


  const char* dn = dirname(buffer);
  size = strlen(dn);
  std::string ret(dn, size) ;
  ret.push_back('/');
  return ret;
}
#endif // rx_get_exe_path()

// write an w*h array of pixels to a png file
static bool rx_save_png(std::string filepath, unsigned char* pixels, int w, int h, int channels = 3) {

  if(!w || !h) {
    printf("error: cannot save png because the given width and height are invalid: %d x %d\n", w, h);
    return false;
  }

  if(!channels || channels > 4) {
    printf("error: cannot save png because the number of color channels is invalid: %d\n", channels);
    return false;
  }

  if(!pixels) {
    printf("error: cannot save png because we got an invalid pixels array: %p\n", pixels);
    return false;
  }

  if(!filepath.size()) {
    printf("error: cannot save png because the given filepath is invalid.\n");
    return false;
  }

  png_structp png_ptr; 
  png_infop info_ptr;

  FILE* fp = fopen(filepath.c_str(), "wb");
  if(!fp) {
    printf("error: canont save png because we cannot open the filepath: %s\n", filepath.c_str());
    fp = NULL;
    return false;
  }

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr == NULL) {
    printf("error: cannot save png because we cannot create a png write struct.\n");
    fclose(fp);
    fp = NULL;
    return false;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL) {
    printf("error: cannot save png brecause we cannot create a png info struct.\n");
    fclose(fp);
    fp = NULL;
    return false;
  }

  if(setjmp(png_jmpbuf(png_ptr))) {
    printf("error: cannot save png because we cannot set the jump pointer.\n");
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    fp = NULL;
    return false;
  }

  png_uint_32 color_type;
  switch(channels) {
    case 1: {
      color_type = PNG_COLOR_TYPE_GRAY; 
      break;
    }
    case 3: {
      color_type = PNG_COLOR_TYPE_RGB;
      break;
    }
    case 4: {
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
      break;
    }
    default: {
      printf("error: cannot save png because we cannot detect the color type based on the number of channels.\n");
      fclose(fp);
      fp = NULL;
      return false;
    }
  };

  png_set_IHDR(png_ptr, 
               info_ptr, 
               w, 
               h, 
               8, 
               color_type, 
               PNG_INTERLACE_NONE, 
               PNG_COMPRESSION_TYPE_DEFAULT, 
               PNG_FILTER_TYPE_DEFAULT);

  png_bytep* row_ptrs = new png_bytep[h];
  for(size_t j = 0; j < h; ++j) {
    row_ptrs[j] = pixels + (j * (w * channels));
  }
 
  png_init_io(png_ptr, fp);
  png_set_rows(png_ptr, info_ptr, row_ptrs);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  delete[] row_ptrs;

  fclose(fp);

  return true;

}

static bool rx_load_png(std::string filepath, 
                        unsigned char** pixels,
                        uint32_t& width,
                        uint32_t& height,
                        uint32_t& nchannels
)
{
  png_structp png_ptr;
  png_infop info_ptr; 

  FILE* fp = fopen(filepath.c_str(), "rb");
  if(!fp) {
    printf("Error: cannot load the png file: %s\n", filepath.c_str());
    fp = NULL;
    return false;
  }

  unsigned char sig[8];
  size_t r = 0;
  r = fread(sig, 1, 8, fp);
  if(r != 8) {
    printf("Error: invalid png signature (not enough bytes read) in: %s.\n", filepath.c_str());
    fclose(fp);
    fp = NULL;
    return  false;
  }

  if(!png_check_sig(sig, 8)) {
    printf("Error: invalid png signature (wrong siganture) in: %s.\n", filepath.c_str());
    fclose(fp);
    fp = NULL;
    return false;
  }
  
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    printf("Error: cannot create png read struct: %s\n", filepath.c_str());
    fclose(fp);
    fp = NULL;
    return false;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    printf("Error: cannot create png info struct for: %s\n", filepath.c_str());
    fclose(fp);
    fp = NULL;
    return false;
  }

  #if !defined(_WIN32)
  if(setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    fp = NULL;
    return false;
  }
  #endif

  // @TODO - add option to rescale to 8bit color info or 16bit
  // @TODO - add option to strip the alpha (not recommended in the example)
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  uint32_t stride = 0;
  uint32_t num_bytes = 0;
  uint32_t bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  uint32_t color_type = png_get_color_type(png_ptr, info_ptr);
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  nchannels = png_get_channels(png_ptr, info_ptr);
  
  if(width == 0 || height == 0) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    fp = NULL;
    return false;
  }

  // @TODO - add option to allow input colors/gray values to be not converted
  switch(color_type) {
    case PNG_COLOR_TYPE_PALETTE: {
      png_set_palette_to_rgb(png_ptr);
      nchannels = 3;
      break;
    }
    case PNG_COLOR_TYPE_GRAY: {
      if(bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
        bit_depth = 8;
      }
      break;
    }
    default:break;
  };
  
  // When transparency is set convert it to a full alpha channel
  if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
    nchannels += 1;
  }

  stride = width * bit_depth * nchannels / 8;  
  num_bytes = width * height * bit_depth * nchannels / 8;

  *pixels = new unsigned char[num_bytes];
  if(!pixels) {
    printf("Error: image is to big: %s\n", filepath.c_str());
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    fp = NULL;
    pixels = NULL;
    return false;
  }

  png_bytep* row_ptrs = new png_bytep[height];
  if(!row_ptrs) {
    printf("Error: image is to big: %s\n", filepath.c_str());
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    fp = NULL;
    delete[] *pixels;
    pixels = 0;
    return false;
  }

  for(size_t i = 0; i < height; ++i) {
    row_ptrs[i] = (png_bytep)(*pixels) +(i * stride);
  }

  png_read_image(png_ptr, row_ptrs);

  delete[] row_ptrs;
  row_ptrs = NULL;
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  fclose(fp);
  return true;
}

static std::string rx_read_file(std::string filepath) {
  std::ifstream ifs(filepath.c_str(), std::ios::in);
  if(!ifs.is_open()) {
    return "";
  }
  std::string str((std::istreambuf_iterator<char>(ifs)) , std::istreambuf_iterator<char>());
  return str;
}

#endif
