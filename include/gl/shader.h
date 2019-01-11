namespace gl
{
  /*
   * shader management
   */
  namespace shader
  {
    #define MAX_DEFAULT_STRING_LENGTH  (256)

    /*
    * shader_t is a collection of GL_PROGRAM, GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc objects
    */
    typedef resource_set_t        shader_t;
    const unsigned int            invalid_shader_id = -1;
    /*
    * definition_t is a list of shader types and filenames containing the source for a shader of that type
    */
    typedef std::map<std::string, std::string>  definition_t;
    /*
    * pipeline_t is a definition and the compiled objects associated with that definition
    */
    typedef std::pair<definition_t, shader_t>   pipeline_t;
    /*
    * dictionary-of-pipelines type
    */
    typedef std::map<std::string, pipeline_t>   pipeline_set_t;

    typedef enum  ShaderTypeEnum
    {
      VERTEX_SHADER = GL_VERTEX_SHADER,
      GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
      FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
      SHADER_PROGRAM = GL_PROGRAM,
      COMPUTE_SHADER = GL_COMPUTE_SHADER,
      TESS_CTRL_SHADER = GL_TESS_CONTROL_SHADER,
      TESS_EVAL_SHADER = GL_TESS_EVALUATION_SHADER
    } ShaderType;
    const char*    ShaderTypeName[] =
    {
      "vertex\0",
      "geometry\0",
      "fragment\0",
      "program\0",
      "compute\0",
      "tesselation_control\0",
      "tesselation_eval\0"
    };

    /*
    * convert string names to gl enum types
    */
    const GLenum shader_type_enum (const std::string& type)
    {
      if (type.compare ("program\0") == 0)
      {
        return SHADER_PROGRAM;
      }
      else if (type.compare ("vertex\0") == 0)
      {
        return VERTEX_SHADER;
      }
      else if (type.compare ("geometry\0") == 0)
      {
        return GEOMETRY_SHADER;
      }
      else if (type.compare ("fragment\0") == 0)
      {
        return FRAGMENT_SHADER;
      }
      else if (type.compare ("compute") == 0)
      {
        return COMPUTE_SHADER;
      }
      else if (type.compare ("tesselation_control") == 0)
      {
        return TESS_CTRL_SHADER;
      }
      else if (type.compare ("tesselation_eval") == 0)
      {
        return TESS_EVAL_SHADER;
      }
      else
      {
        return 0;
      }
    }
    /**
    * wrapper for glGetProgramiv and glGetShaderiv
    * @param shader shader profile to interogate
    * @param type shader type to get (if a shader param), or SHADER_PROGRAM for program info.
    * @param param parameter for which to retrieve the value.
    * @return the program variable for the parameter (result of glGetProgramiv)
    */
    unsigned int get_info (const shader_t& shader,
                           const std::string& type,
                           GLenum param)
    {
      GLint p;
      auto  enum_type = shader_type_enum (type);

      switch (enum_type)
      {
        case VERTEX_SHADER:
        case GEOMETRY_SHADER:
        case FRAGMENT_SHADER:
        case COMPUTE_SHADER:
        case TESS_CTRL_SHADER:
        case TESS_EVAL_SHADER:
        {
          glGetShaderiv (gl::get_resource (shader, type).second,
                         param,
                         &p);
          break;
        }
        case SHADER_PROGRAM:
        {
          glGetProgramiv (gl::get_resource (shader, "program").second,
                          param,
                          &p);
          break;
        }
      }

      return p;
    }
    /**
    * prints the log associated with a shader to fout.
    * @param shader ShaderProfile object describing the shader to inspect
    * @param shaderType shader element to return the log for (vertex, geom, etc)
    * @param fout filestream to print output on
    */
    void get_log (const shader_t& shader, const std::string& type, const char *prefix = NULL)
    {
      GLsizei slen = 0;
      int     log_len = 0;
      size_t  tag_len = 0;
      char    *log = NULL;
      auto    enum_type = shader_type_enum (type);

      // get the length of the log file
      log_len = get_info (shader, type, GL_INFO_LOG_LENGTH);

      // if there is no file stream or no log, exit
      if (log_len < 1)
      {
        return;
      }

      if (prefix != NULL)
      {
        tag_len = strlen (prefix) + 2;
      }
      else
      {
        tag_len = type.size () + 2;
      }

      // allocate storage
      log = new char[log_len];
      memset (log, 0, log_len * sizeof (char));

      // copy the log into the allocated storage space
      switch (enum_type)
      {
        case VERTEX_SHADER:
        case GEOMETRY_SHADER:
        case FRAGMENT_SHADER:
        case COMPUTE_SHADER:
        case TESS_CTRL_SHADER:
        case TESS_EVAL_SHADER:
        {
          glGetShaderInfoLog (gl::get_resource (shader, type).second, log_len, &slen, log);
          break;
        }
        case SHADER_PROGRAM:
        {
          glGetProgramInfoLog (gl::get_resource (shader, type).second, log_len, &slen, log);
          break;
        }
        default:
        {
          return;
        }
      }

      // write the log to the stream
      std::cout << log << std::endl;

      // cleanup
      delete[] log;
    }
    /*
    * verify a shader by calling glLinkProgram and glValidateProgram.
    * returns true if GL_LINE_STATUS and GL_VALIDATE_STATUS == GL_TRUE, false otherwise
    */
    bool verify (shader_t& shader)
    {
      GLint  valid, link;

      glLinkProgram (shader["program\0"]);
      glGetProgramiv (shader["program\0"], GL_LINK_STATUS, &link);

      if (link != GL_TRUE)
      {
        get_log (shader, "program\0", "PROGRAM\0");
        return false;
      }

      glValidateProgram (shader["program\0"]);
      glGetProgramiv (shader["program\0"], GL_VALIDATE_STATUS, &valid);

      if (valid != GL_TRUE)
      {
        get_log (shader, "program\0", "PROGRAM\0");
        return false;
      }

      return true;
    }
    /**
    * @returns true if the shader program has been validated, false otherwise.
    */
    bool is_valid (const shader_t& shader)
    {
      return ((glIsProgram (gl::get_resource (shader, "program").second) == GL_TRUE) &&
              (get_info (shader, "program", GL_VALIDATE_STATUS) == GL_TRUE));
    }

    /*
    * fill a shader_t by loading the source files in the definition_t
    */
    void create (const definition_t& definition, shader_t& shader)
    {
      shader["program\0"] = glCreateProgram ();

      for (auto d : definition)
      {
        if (d.second.length() == 0)
        {
          return;
        }

        auto  type = d.first;
        int   status = 0;

        auto gl_type = shader_type_enum (type);

        if (gl_type == GL_NONE)
        {
          continue;
        }

        shader[type] = glCreateShader (gl_type);

        auto buf = d.second.c_str ();

        glShaderSource (gl::get_resource (shader, type).second,
                        1,
                        (const GLchar**)&buf,
                        NULL);
        glCompileShader (gl::get_resource (shader, type).second);
        //delete[] buf;

        // check it compiled
        glGetShaderiv (gl::get_resource (shader, type).second,
                       GL_COMPILE_STATUS,
                       &status);

        get_log (shader, type);

        if (status == GL_FALSE)
        {
          std::cout << "ERR: Did not compile '" << type << "'" << std::endl;
          glDeleteShader (gl::get_resource (shader, type).second);
          shader[type] = 0;
        }

        if (glIsShader (gl::get_resource (shader, type).second) == GL_TRUE)
        {
          glAttachShader (gl::get_resource (shader, "program\0").second,
                          gl::get_resource (shader, type).second);
        }
      }

      // validate the program
      verify (shader);
    }
    /*
    * load the definition in pipeline_t into the shader in pipeline_t
    */
    void create (pipeline_t& p)
    {
      create (p.first, p.second);
    }
    /*
    * load all the pipeline_ts in pipeline_set_t
    */
    void create (pipeline_set_t& p)
    {
      for (auto& d : p)
      {
        create (d.second);
      }
    }
    /*
    * delete the shaders and programs associated with the shader_t
    */
    void clean (shader_t& shader)
    {
      if (is_valid (shader) == true)
      {
        GLsizei      prc;   // count
        unsigned int  prn;  // name

        // this convoluted way to remove attached shaders is so we don't rely on
        // a fixed number of shaders, and we don't cause glErrors to accumulate
        // by passing null values to glDetachShader
        for (glGetAttachedShaders (gl::get_resource (shader, "program").second, 1, &prc, &prn);
             prc>0;
             glGetAttachedShaders (gl::get_resource (shader, "program").second, 1, &prc, &prn))
        {
          glDetachShader (gl::get_resource (shader, "program").second, prn);
        }
      }

      for (auto type : ShaderTypeName)
      {
        if (gl::has_resource (shader, type) == false)
        {
          continue;
        }

        auto res = gl::get_resource (shader, type).second;

        if (glIsShader (res) == GL_TRUE)
        {
          glDeleteShader (res);
        }
        else if (glIsProgram (res) == GL_TRUE)
        {
          glDeleteProgram (res);
        }

        shader[type] = 0;
      }
    }
    void clean (pipeline_t& pipeline)
    {
      clean (pipeline.second);
    }
    void clean (pipeline_set_t& pipeline_set)
    {
      for (auto d : pipeline_set)
      {
        clean (d.second);
      }
    }

    /**
    * reads a file into a buffer. if buflen is 0,
    * the size of the file (in bytes) is returned in this parameter.
    * @param filename name of the file to read from disk
    * @param buf the buffer to contain the contents; this may be NULL only if buflen is zero
    * @param buflen the size of the buffer in bytes. If this is 0, the required size is stored here.
    * @returns the number of bytes read from the file, or zero if no read was performed.
    */
    size_t read_file (const char *filename, char *buf, size_t& buflen)
    {
      FILE    *f = NULL;
      size_t  strt, len, rb;

      if (filename == NULL)
      {
        return 0;
      }

      // read the programs from file
      f = fopen (filename, "rb\0");

      if (f == NULL)
      {
        return 0;
      }

      // get the file length in bytes
      strt = ftell (f);
      fseek (f, 0, SEEK_END);
      len  = ftell (f) - strt;
      fseek (f, 0, SEEK_SET);

      if (len == 0)
      {
        fclose (f);
        return 0;
      }

      // allocate storage
      if (len > buflen)
      {
        buflen = len;
        fclose (f);
        return 0;
      }

      memset (buf, 0, buflen*sizeof (char));

      // read the program into storage space
      rb = fread (buf, sizeof (char), len, f);

      // close the file
      fclose (f);

      // return the number of bytes read from the file
      return rb;
    }

    std::string read_source_file(const char *filename)
    {
        size_t len = 0;

        read_file(filename, nullptr, len);

        if (len == 0)
        {
            return std::string("");
        }

        char* buf = new char[len+1];
        std::fill(buf, buf + len + 1, 0);
        read_file(filename, buf, len);
        std::string s(buf);
        delete[] buf;
        return s;
    }

    /**
    * @returns true if the shader program linked correctly, false otherwise
    */
    bool link_status (const shader_t& shader)
    {
      return (get_info (shader, "program", GL_LINK_STATUS) == GL_TRUE);
    }
    /**
    * link a shader and return the link status.
    * @param shader shader profile with input shaders and program variable.
    * @returns the link status of the program after calling glLinkProgram ()
    */
    bool link (const shader_t& shader)  
    {
      glLinkProgram (gl::get_resource (shader, "program").second);
      return link_status (shader);
    }
    /**
    * Makes a program described by the shader profile active.
    * @param shader shader profile to make active by calling glUseProgram ()
    */
    void bind (const shader_t& shader)
    {
      glUseProgram (gl::get_resource (shader, "program").second);
    }
    void bind (const pipeline_t& pipeline)
    {
      bind (pipeline.second);
    }
    /**
    * Sets the current program to 0 (default functionality)
    */
    void unbind ()
    {
      glUseProgram (0);
    }
    unsigned int program_interface_variable (const shader_t& shader,
                                             const GLenum programInterface,
                                             const GLenum parameter)
    {
      GLint  p;
      glGetProgramInterfaceiv (gl::get_resource (shader, "program").second,
                               programInterface,
                               parameter,
                               &p);
      return p;
    }
    unsigned int program_active_resource_count (const shader_t& shader,
                                                const GLenum programInterface)
    {
      return program_interface_variable (shader,
                                         programInterface,
                                         GL_ACTIVE_RESOURCES);
    }
    void display_program_resource (const shader_t& shader,
                                   const GLenum programInterface,
                                   const GLuint resourceIndex)
    {
      // The length of the longest name string for programInterface​>,
      // including a null terminator, can be queried by calling
      // glGetProgramInterface with a pname​ of GL_MAX_NAME_LENGTH.
      char      name[MAX_DEFAULT_STRING_LENGTH];
      GLsizei   cb = MAX_DEFAULT_STRING_LENGTH;
      GLsizei   wb = 0;

      // get the name
      glGetProgramResourceName (gl::get_resource (shader, "program").second,
                                programInterface,
                                resourceIndex,
                                cb,
                                &wb,
                                name);

      unsigned int  pcnt;
      GLenum        props[16] = {GL_NONE};
      GLint         buf[16] = {0};

      // NB: If we attempt to fetch properties which don't belong to the given interface, ogl fails loudly, not silently.
      switch (programInterface)
      {
        case GL_UNIFORM:
        {
          props[0] = GL_TYPE;
          props[1] = GL_OFFSET;
          props[2] = GL_BLOCK_INDEX;
          props[3] = GL_ARRAY_STRIDE;
          props[4] = GL_MATRIX_STRIDE;
          props[5] = GL_IS_ROW_MAJOR;
          props[6] = GL_LOCATION;
          pcnt = 7;
          break;
        }
        case GL_PROGRAM_INPUT:
        case GL_PROGRAM_OUTPUT:
        case GL_TRANSFORM_FEEDBACK_VARYING:
        {
          props[0] = GL_TYPE;
          props[1] = GL_LOCATION;
          pcnt = 2;
          break;
        }
        case GL_BUFFER_VARIABLE:
        {
          props[0] = GL_TYPE;
          props[1] = GL_ARRAY_SIZE;
          props[2] = GL_OFFSET;
          props[3] = GL_BLOCK_INDEX;
          props[4] = GL_ARRAY_STRIDE;
          props[5] = GL_MATRIX_STRIDE;
          props[6] = GL_IS_ROW_MAJOR;
          props[7] = GL_TOP_LEVEL_ARRAY_SIZE;
          props[8] = GL_TOP_LEVEL_ARRAY_STRIDE;
          pcnt = 9;
          break;
        }
        case GL_SHADER_STORAGE_BLOCK:
        {
          props[0] = GL_BUFFER_BINDING;
          props[1] = GL_BUFFER_DATA_SIZE;
          props[2] = GL_NUM_ACTIVE_VARIABLES;
          props[3] = GL_ACTIVE_VARIABLES;
          pcnt = 4;
          break;
        }
  //      case GL_VERTEX_SUBROUTINE:
  //      case GL_TESS_CONTROL_SUBROUTINE:
  //      case GL_TESS_EVALUATION_SUBROUTINE:
  //      case GL_GEOMETRY_SUBROUTINE:
  //      case GL_FRAGMENT_SUBROUTINE:
  //      case GL_COMPUTE_SUBROUTINE:
        default:
        {
          pcnt = 0;
          break;
        }
      }

      if (pcnt > 0)
      {
        // get the info
        glGetProgramResourceiv (gl::get_resource (shader, "program").second,
                                programInterface,
                                resourceIndex,
                                pcnt, props,
                                sizeof (buf)/sizeof (buf[0]),
                                NULL, buf);
      }
/*
      switch (programInterface)
      {
      case GL_UNIFORM:            {Note::notemsgn ("  [%-10s][ % 4i][    ]\t%s\0", nameForType (buf[0]), buf[6], name); break;}
      case GL_PROGRAM_INPUT:
      case GL_PROGRAM_OUTPUT:
      case GL_TRANSFORM_FEEDBACK_VARYING:    {Note::notemsgn ("  [%-10s][ % 4i][    ]\t%s\0", nameForType (buf[0]), buf[1], name); break;}
      case GL_BUFFER_VARIABLE:        {Note::notemsgn ("  [%-10s][%02i+%02i]\t%s\0", nameForType (buf[0]), buf[3], buf[2], name); break;}
      case GL_SHADER_STORAGE_BLOCK:      {Note::notemsgn ("  [% 10i][ % 4i][    ]\t%s\0", buf[0], buf[1], name); break;}
      default:                {Note::notemsgn ("  %s\0", name); break;}    
      }
*/
    }
    void display_program_uniform_info (const shader_t& shader, const unsigned int index)
    {
      display_program_resource (shader, GL_UNIFORM, index);
    }
    void display_program_input_info (const shader_t& shader, const unsigned int index)
    {
      display_program_resource (shader, GL_PROGRAM_INPUT, index);
    }
    void display_program_output_info (const shader_t& shader, const unsigned int index)
    {
      display_program_resource (shader, GL_PROGRAM_OUTPUT, index);
    }
    void display_program_buffer_info (const shader_t& shader, const unsigned int index)
    {
      display_program_resource (shader, GL_BUFFER_VARIABLE, index);
    }
    void display_program_storage_info (const shader_t& shader, const unsigned int index)
    {
      display_program_resource (shader, GL_SHADER_STORAGE_BLOCK, index);
    }
    /**
    * returns the number of active uniform variables in a shader program
    * @param shader shader profile with compiled program
    * @returns number of active uniforms in the program.
    */
    unsigned int program_uniform_count (const shader_t& shader)
    {
      return program_active_resource_count (shader, GL_UNIFORM);
    }
    unsigned int program_input_count (const shader_t& shader)
    {
      return program_active_resource_count (shader, GL_PROGRAM_INPUT);
    }
    unsigned int program_output_count (const shader_t& shader)
    {
      return program_active_resource_count (shader, GL_PROGRAM_OUTPUT);
    }
    unsigned int program_buffer_count (const shader_t& shader)
    {
      return program_active_resource_count (shader, GL_BUFFER_VARIABLE);
    }
    unsigned int program_storage_count (const shader_t& shader)
    {
      return program_active_resource_count (shader, GL_SHADER_STORAGE_BLOCK);
    }
    void display_program_info (const shader_t& shdr)
    {
      unsigned int  uc, i, j;

      GLenum    infblk[] = {GL_UNIFORM,
                            GL_UNIFORM_BLOCK,
                            GL_ATOMIC_COUNTER_BUFFER,
                            GL_PROGRAM_INPUT,
                            GL_PROGRAM_OUTPUT,
                            GL_TRANSFORM_FEEDBACK_VARYING,
                            GL_TRANSFORM_FEEDBACK_BUFFER,
                            GL_BUFFER_VARIABLE,
                            GL_SHADER_STORAGE_BLOCK,
                            GL_VERTEX_SUBROUTINE,
                            GL_GEOMETRY_SUBROUTINE,
                            GL_FRAGMENT_SUBROUTINE,
                            GL_TESS_CONTROL_SUBROUTINE,
                            GL_TESS_EVALUATION_SUBROUTINE,
                            GL_COMPUTE_SUBROUTINE,
                            GL_VERTEX_SUBROUTINE_UNIFORM,
                            GL_GEOMETRY_SUBROUTINE_UNIFORM,
                            GL_FRAGMENT_SUBROUTINE_UNIFORM,
                            GL_TESS_CONTROL_SUBROUTINE_UNIFORM,
                            GL_TESS_EVALUATION_SUBROUTINE_UNIFORM,
                            GL_COMPUTE_SUBROUTINE_UNIFORM};

      const char*  infnms[] = {"uniforms\0",
                               "uniform blocks\0",
                               "atomic buffers\0",
                               "inputs\0",
                               "outputs\0",
                               "feedback varyings\0",
                               "feedback buffer\0",
                               "buffers\0",
                               "storage block\0",
                               "vertex func\0",
                               "geometry func\0",
                               "fragment func\0",
                               "tess control func\0",
                               "tess eval func\0",
                               "compute func\0",
                               "vertex func uniforms\0",
                               "geometry func uniforms\0",
                               "fragment func uniforms\0",
                               "tess control func uniforms\0",
                               "tess eval func uniforms\0",
                               "compute func uniforms\0"};

      if (is_valid (shdr) == false)
      {
        std::cerr << "Program not valid" << std::endl;
        return;
      }

      for (j=0;j<sizeof (infblk)/sizeof (infblk[0]); j++)
      {
        if ((uc = program_active_resource_count (shdr, infblk[j])) == 0)
        {
          continue;
        }

        std::cout << infnms[j] << uc << std::endl;

        for (i=0;i<uc;i++)
        {
          display_program_resource (shdr, infblk[j], i);
        }
      }
    }
    unsigned int program_uniform_location (const shader_t& shader, const char *name)
    {
      return glGetProgramResourceLocation (gl::get_resource (shader, "program").second, GL_UNIFORM, name);
    }
    unsigned int program_uniform_index (const shader_t& shader, const char *name)
    {
      return glGetProgramResourceIndex (gl::get_resource (shader, "program").second, GL_UNIFORM, name);
    }
    void program_uniform_info (const shader_t& shader,
                               const unsigned int activeUniformIndex,
                               GLchar *nameBuffer,
                               const GLsizei nameBufferLen,
                               GLsizei& writtenBytes,
                               GLenum& dataType)
    {
      GLenum  props[1] = {GL_TYPE};
      GLint   buf[1] = {0};

      // get the name!
      if (nameBuffer != NULL)
      {
        glGetProgramResourceName (gl::get_resource (shader, "program").second,
                                  GL_UNIFORM,
                                  activeUniformIndex,
                                  nameBufferLen,
                                  &writtenBytes,
                                  nameBuffer);
      }

      // get the info
      glGetProgramResourceiv (gl::get_resource (shader, "program").second,
                              GL_UNIFORM,
                              activeUniformIndex,
                              sizeof (props)/sizeof (props[0]),
                              props,
                              sizeof (buf)/sizeof (buf[0]),
                              NULL,
                              buf);

      dataType = buf[0];
    }
    void program_storage_info (const shader_t& shader,
                               const unsigned int resourceIndex,
                               GLchar *nameBuffer,
                               const GLsizei nameBufferLen,
                               GLsizei& writtenBytes,
                               GLuint& blockBinding)
    {
      // get the name
      glGetProgramResourceName (gl::get_resource (shader, "program").second,
                                GL_SHADER_STORAGE_BLOCK,
                                resourceIndex,
                                nameBufferLen,
                                &writtenBytes,
                                nameBuffer);

      // get the properties
      GLenum  props[4] = {GL_BUFFER_BINDING,
                          GL_BUFFER_DATA_SIZE,
                          GL_NUM_ACTIVE_VARIABLES,
                          GL_ACTIVE_VARIABLES};
      GLint   buf[4] = {0};

      glGetProgramResourceiv (gl::get_resource (shader, "program").second,
                              GL_SHADER_STORAGE_BLOCK,
                              resourceIndex,
                              sizeof (props)/sizeof(props[0]),
                              props,
                              sizeof (buf)/sizeof (buf[0]),
                              NULL,
                              buf);

      // write out the properties
      blockBinding = buf[0];
    }
  }
}
