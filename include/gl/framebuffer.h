/*
* framebuffer management
*/
namespace gl
{
  namespace framebuffer
  {
    /*
    * dictionary-of-framebuffers type
    */
    typedef resource_set_t  framebuffer_set_t;
    const unsigned int      invalid_framebuffer_id = -1;

    /*
    * create a framebuffer object
    */
    unsigned int create ()
    {
      unsigned int  fboid;
      glGenFramebuffers (1, &fboid);
      return fboid;
    }
    void bind (const unsigned int fboid)
    {
      glBindFramebuffer (GL_FRAMEBUFFER, fboid);
    }
    void unbind ()
    {
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
    }
    void draw_buffers (const unsigned int numberOfBuffers)
    {
      GLenum      db[] = {GL_COLOR_ATTACHMENT0,
                          GL_COLOR_ATTACHMENT1,
                          GL_COLOR_ATTACHMENT2,
                          GL_COLOR_ATTACHMENT3,
                          GL_COLOR_ATTACHMENT4,
                          GL_COLOR_ATTACHMENT5,
                          GL_COLOR_ATTACHMENT6,
                          GL_COLOR_ATTACHMENT7,
                          GL_COLOR_ATTACHMENT8,
                          GL_COLOR_ATTACHMENT9,
                          GL_COLOR_ATTACHMENT10,
                          GL_COLOR_ATTACHMENT11,
                          GL_COLOR_ATTACHMENT12,
                          GL_COLOR_ATTACHMENT13,
                          GL_COLOR_ATTACHMENT14};

      glDrawBuffers (numberOfBuffers, db);
    }
    void attach_texture1D (const unsigned int texid)
    {
      glFramebufferTexture1D (GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_1D,
                              texid,
                              0);
      draw_buffers (1);
    }
    void attach_texture (const unsigned int texid, const unsigned int depid = 0)
    {
      // attach the target
      glFramebufferTexture2D (GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_TEXTURE_2D,
                              texid,
                              0);
      glFramebufferTexture2D (GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D,
                              depid,
                              0);
      draw_buffers (1);
    }
    void attach_texture (const unsigned int *targets, const unsigned int targetCount, const unsigned int depid = 0)
    {
      // attach the targets
      for (auto i=0; i<targetCount; i++)
      {
        glFramebufferTexture2D (GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0+i,
                                GL_TEXTURE_2D,
                                targets[i],
                                0);
      }

      glFramebufferTexture2D (GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_TEXTURE_2D,
                              depid,
                              0);

      draw_buffers (targetCount);
    }
    void attach_texture (const gl::texture::texture_set_t& texture_set)
    {
      unsigned int  tcnt = 0;

      for (auto& texname : texture_set)
      {
        if (texname.first.compare ("depth") == 0)
        {
          continue;
        }

        glFramebufferTexture2D (GL_FRAMEBUFFER,
                                GL_COLOR_ATTACHMENT0+tcnt,
                                GL_TEXTURE_2D,
                                texname.second,
                                0);
        tcnt++;
      }

      if (gl::has_resource (texture_set, "depth") == true)
      {
        glFramebufferTexture2D (GL_FRAMEBUFFER,
                                GL_DEPTH_ATTACHMENT,
                                GL_TEXTURE_2D,
                                gl::get_resource (texture_set, "depth").second,
                                0);
      }

      draw_buffers (tcnt);
    }
    /*
    * verify the currently bound framebuffer object
    * if glCheckFramebufferStatus is GL_FRAMEBUFFER_COMPLETE return true,
    * otherwise, write the error to stderr and return false.
    */
    bool verify()
    {
      switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
      {
        case GL_FRAMEBUFFER_COMPLETE:
        {
          /*OutputDebugString ("FrameBuffer complete\n\0");  */
          return true;
        }
        case GL_FRAMEBUFFER_UNDEFINED:
        {
          std::cerr << "ERR: frameBuffer undefined\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        {
          std::cerr << "ERR: frameBuffer incomplete\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        {
          std::cerr << "ERR: frameBuffer incomplete (missing)\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        {
          std::cerr << "ERR: frameBuffer incomplete (draw)\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        {
          std::cerr << "ERR: frameBuffer incomplete (read)\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_UNSUPPORTED:
        {
          std::cerr << "ERR: frameBuffer unsupported\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        {
          std::cerr << "ERR: frameBuffer incomplete (ms)\0" << std::endl;
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        {
          std::cerr << "ERR: frameBuffer incomplete (layers)\0" << std::endl;
          break;
        }
      }

      return false;
    }
    /*
    * call glDeleteFramebuffers with the framebuffer object id
    */
    void clean (unsigned int fboid)
    {
      glDeleteFramebuffers(1, &fboid);
    }
  }
}
