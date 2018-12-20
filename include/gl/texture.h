#include <functional>
#include <vector>

/*
* texture management
*/
namespace gl
{
  namespace texture
  {
    /*
     * dictionary-of-textures type
     */
    typedef resource_set_t  texture_set_t;
    const unsigned int      invalid_texture_id = -1;

    /*
     * create a 2d texture
     * if internal format is a GL_DEPTH* type a GL_DEPTH_COMPONENT image is create
     * otherwise, a GL_RGBA image is created
     */
    unsigned int create (const size2d size,
                         const GLenum internal_format = GL_RGBA32F)
    {
      unsigned int texid = 0;

      glGenTextures (1, &texid);
      glBindTexture (GL_TEXTURE_2D, texid);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      switch (internal_format)
      {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        {
          glTexImage2D (GL_TEXTURE_2D,
                        0,
                        internal_format,
                        std::get<0>(size),
                        std::get<1>(size),
                        0,
                        GL_DEPTH_COMPONENT,
                        GL_FLOAT,
                        NULL);
          break;
        }
        default:
        {
          glTexImage2D (GL_TEXTURE_2D,
                        0,
                        internal_format,
                        std::get<0>(size),
                        std::get<1>(size),
                        0,
                        GL_RGBA,
                        GL_FLOAT,
                        NULL);
          break;
        }
      }

      return texid;
    }
    /*
     * create a 3d texture
     */
    unsigned int create (const size3d size,
                         const GLenum internal_format = GL_RGBA32F)
    {
      unsigned int texid = 0;

      glGenTextures (1, &texid);
      glBindTexture (GL_TEXTURE_3D, texid);
      glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      //  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
      glTexImage3D (GL_TEXTURE_3D,
                    0,
                    internal_format,
                    std::get<0>(size),
                    std::get<1>(size),
                    std::get<2>(size),
                    0,
                    GL_RGBA,
                    GL_FLOAT,
                    NULL);

      return texid;
    }

    bool is_texture (const unsigned int tex_id)
    {
      return (glIsTexture (tex_id) == GL_TRUE);
    }

    typedef std::function <bool(const int, const int, const int, const unsigned char*)>    write_fn_t;

    bool default_write_fn(const int w, const int h, const int d, const unsigned char* img_buf)
    {
      auto N = w*h*d;
      auto mn = *std::min_element(img_buf, img_buf + N);
      auto mx = *std::max_element(img_buf, img_buf + N);

      fprintf(stdout, "received image: (%ix%ix%i) %i -> %i", w, h, d, mn, mx);

      return true;
    }

    /*
     * write a texture object to a callback function
     * NB: this is only valid for GL_TEXTURE_2D targets at the moment
     * write_fn signature should be (const int, const int, const int, const unsigned char*)->bool
     * (as given above in the write_fn_t type)
     * The texture data will be extracted, converted to four channel byte-encoded RGBA and
     * passed to the write_fn callback.
     * Memory location used to store the image data will be deleted after write_fn returns.
     */
    void write (const unsigned int texid, write_fn_t write_fn)
    {
      int    w, h;

      glGetError ();

      glBindTexture (GL_TEXTURE_2D, texid);
      glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
      glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

      if (glGetError() != GL_NONE)
      {
        return;
      }
      else if ((w == 0) || (h == 0))
      {
        return;
      }

      float         *img_f = new float[w*h*4];
      unsigned char *img_ub = new unsigned char[w*h*4];

      glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, img_f);

      // convert the float to bytes
      std::transform (img_f + (0), img_f + (w*h*4),
                      img_ub, [](const float f){return static_cast<unsigned int>(f*255.0f);});

      if (write_fn (w, h, 4, img_ub) == false)
      {
        std::cerr << "ERR: Could not output texture '" << texid << "' (" << w << ", " << h << ")" << std::endl;
      }

      // cleanup
      delete[] img_ub;
      delete[] img_f;
    }
    /*
     * call glDeleteTextures for a texture object id
     */
    void clean (unsigned int texid)
    {
      glDeleteTextures (1, &texid);
    }
    /*
     * calls clean on all the textures in a texture_set object
     */
    void clean (texture_set_t& texture_set)
    {
      for (auto& tex_id : texture_set)
      {
        clean (tex_id.second);
      }
    }
    /**
     * bind a texture object to the atu
     */
    void bind (unsigned int texid, GLenum target = GL_TEXTURE_2D)
    {
      glBindTexture (target, texid);
    }
    /**
     * bind object 0 to the atu
     */
    void unbind (GLenum target = GL_TEXTURE_2D)
    {
      glBindTexture (target, 0);
    }
    /**
     * Wrapper function for glGetTexLevelParameteriv.
     * @param parameter glenum of the texture parameter to query
     * @return integer value of the texture parameter.
     */
    unsigned int parameter (GLenum parameter,
                            GLenum target = GL_TEXTURE_2D,
                            GLint level = 0)
    {
      GLint  p;

      switch (parameter)
      {
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        {
          if (target != GL_TEXTURE_2D)
          {
            p = GL_NONE;
          }
          else
          {
            glGetTexParameteriv (target, GL_TEXTURE_MIN_FILTER, &p);
          }
          break;
        }
        default:
        {
          glGetTexLevelParameteriv (target, level, parameter, &p);
          break;
        }
      }

      return p;
    }
    /**
     * @return the width of the currently bound texture
     */
    unsigned int width (GLenum target = GL_TEXTURE_2D, GLint level = 0)
    {
      return parameter (GL_TEXTURE_WIDTH, target, level);
    }
    /**
     * @return the height of the currently bound texture
     */
    unsigned int height (GLenum target = GL_TEXTURE_2D, GLint level = 0)
    {
      return parameter (GL_TEXTURE_HEIGHT, target, level);
    }
    /**
     * @return the internal format of the currently bound texture
     */
    unsigned int format (GLenum target = GL_TEXTURE_2D, GLint level = 0)
    {
      return parameter (GL_TEXTURE_INTERNAL_FORMAT, target, level);
    }
    /**
     * @returns the number of components for an opengl image format.
     */
    unsigned int format_component_count (const unsigned int format)
    {
      switch (format)
      {
        case GL_LUMINANCE:
        case GL_ALPHA:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_DEPTH_COMPONENT:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        {
          return 1;
        }
        case GL_LUMINANCE_ALPHA:
        case GL_RG:
        case GL_RG_INTEGER:
        case GL_RG8:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        {
          return 2;
        }
        case GL_RGB:
        {
          return 3;
        }
        case GL_RGBA:
        {
          return 4;
        }
        case GL_RGBA32F:
        {
          return 4;
        }
        default:
        {
          std::cerr << "ERR: unknown texture format\0";
          return 0;
        }
      }
    }
    unsigned int target_for_type (const unsigned int type)
    {
      switch (type)
      {
        case GL_TEXTURE_1D:
        {
          return GL_TEXTURE_1D;
        }
        case GL_TEXTURE_2D:
        {
          return GL_TEXTURE_2D;
        }
        case GL_TEXTURE_3D:
        {
          return GL_TEXTURE_3D;
        }
        case GL_DEPTH_COMPONENT:
        {
          return GL_TEXTURE_2D;
        }
        case GL_TEXTURE_2D_ARRAY:
        {
          return GL_TEXTURE_2D_ARRAY;
        }
        default:
        {
          std::cerr << "ERR: Unknown texture type\0";
          return GL_NONE;
        }
      }
    }
    unsigned int buffer_size (const unsigned int w, const unsigned int h, const unsigned int d)
    {
      return ((w<8)?8:w) * ((h<8)?8:h) * d;
    }
    /**
     * Produces mipmaps for the currently bound texture by setting the texture parameters
     * to GL_LINEAR_MIPMAP_LINEAR for GL_TEXTURE_MIN_FILTER and calling
     * glGenerateMipmapEXT ().
     */
    void mipmap (void)
    {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glGenerateMipmap (GL_TEXTURE_2D);
    }
    bool has_mipmaps (GLenum target = GL_TEXTURE_2D)
    {
      return (parameter (GL_TEXTURE_MIN_FILTER, target) == GL_LINEAR_MIPMAP_LINEAR);
    }
    bool set_content (const unsigned int tex_id,
                      const char *image,
                      const unsigned int w,
                      const unsigned int h,
                      const unsigned int format = GL_RGBA,
                      const unsigned int internal_format = GL_RGBA,
                      const bool flip = true,
                      GLenum target = GL_TEXTURE_2D)
    {
      auto data_format = GL_NONE;

      switch (format_component_count (format))
      {
        case 1:
        {
          data_format = GL_LUMINANCE;
          break;
        }
        case 3:
        {
          data_format = GL_RGB;
          break;
        }
        case 4:
        {
          data_format = GL_RGBA;
          break;
        }
      }

    //  glGenTextures (1, &textureID);
      glBindTexture (target, tex_id);
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);

      switch (target)
      {
        case GL_TEXTURE_1D:
        {
          glTexImage1D (target,
                        0,
                        internal_format,
                        h,
                        0,
                        data_format,
                        GL_UNSIGNED_BYTE,
                        image);
          break;
        }
        case GL_TEXTURE_2D:
        {
          glTexImage2D (target,
                        0,
                        internal_format,
                        w,
                        h,
                        0,
                        data_format,
                        GL_UNSIGNED_BYTE,
                        image);
          break;
        }
      }

      return true;
    }
	template<typename T>
	bool set_content(const unsigned int tex_id,
					 const std::vector<T>& values,
					 const size2d size,
					 const unsigned int format = GL_LUMINANCE,
					 const unsigned int internal_format = GL_LUMINANCE,
					 GLenum target = GL_TEXTURE_2D)
	{
		glBindTexture(target, tex_id);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);

		switch (target)
		{
			case GL_TEXTURE_2D:
			{
				glTexImage2D(target,
					0,
					internal_format,
					std::get<0> (size),
					std::get<1> (size),
					0,
					format,
					GL_FLOAT,
					values.data());
				break;
			}
			default:
			{
				return false;
			}
		}

		return true;
	}
  }
}
