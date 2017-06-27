/*
 * data buffers
 */

#include <vector>

namespace gl
{
	namespace buffer
	{
		const unsigned int		invalid_buffer_id = -1;

		unsigned int create ()
		{
			unsigned int	boid;
			glGenBuffers (1, &boid);
			return boid;
		}
		bool is_buffer (const unsigned int& boid)
		{
			return (glIsBuffer (boid) == GL_TRUE);
		}
		void clean (unsigned int& boid)
		{
			glDeleteBuffers (1, &boid);
			boid = 0;
		}

		void state ()
		{
			GLint	i;
			char	b[256]={0};

			glGetIntegerv (GL_ARRAY_BUFFER_BINDING, &i);				snprintf (b, 256, "%s:%i\n\0", "GL_ARRAY_BUFFER_BINDING\0", i);				std::cerr << b << std::endl;
			glGetIntegerv (GL_ATOMIC_COUNTER_BUFFER_BINDING, &i);		snprintf (b, 256, "%s:%i\n\0", "GL_ATOMIC_COUNTER_BUFFER_BINDING\0", i);	std::cerr << b << std::endl;
		//	glGetIntegerv (GL_COPY_READ_BUFFER_BINDING, &i);			snprintf (b, 256, "%s:%i\n\0", GL_ARRAY_BUFFER_BINDING, i);	OutputDebugString (b);
		//	glGetIntegerv (GL_COPY_WRITE_BUFFER_BINDING, &i);			snprintf (b, 256, "%s:%i\n\0", GL_ARRAY_BUFFER_BINDING, i);	OutputDebugString (b);
		    glGetIntegerv (GL_DRAW_INDIRECT_BUFFER_BINDING, &i);		snprintf (b, 256, "%s:%i\n\0", "GL_DRAW_INDIRECT_BUFFER_BINDING\0", i);		std::cerr << b << std::endl;
		    glGetIntegerv (GL_DISPATCH_INDIRECT_BUFFER_BINDING, &i);	snprintf (b, 256, "%s:%i\n\0", "GL_DISPATCH_INDIRECT_BUFFER_BINDING\0", i);	std::cerr << b << std::endl;
		    glGetIntegerv (GL_ELEMENT_ARRAY_BUFFER_BINDING, &i);		snprintf (b, 256, "%s:%i\n\0", "GL_ELEMENT_ARRAY_BUFFER_BINDING\0", i);		std::cerr << b << std::endl;
		    glGetIntegerv (GL_PIXEL_PACK_BUFFER_BINDING, &i);			snprintf (b, 256, "%s:%i\n\0", "GL_PIXEL_PACK_BUFFER_BINDING\0", i);		std::cerr << b << std::endl;
		    glGetIntegerv (GL_PIXEL_UNPACK_BUFFER_BINDING, &i);			snprintf (b, 256, "%s:%i\n\0", "GL_PIXEL_UNPACK_BUFFER_BINDING\0", i);		std::cerr << b << std::endl;
		//	glGetIntegerv (GL_SHADER_STORAGE_BUFFER_BINDING, &i);		snprintf (b, 256, "%s:%i\0", GL_ARRAY_BUFFER_BINDING, i);	OutputDebugString (b);
		//	glGetIntegerv (GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &i);	snprintf (b, 256, "%s:%i\0", GL_ARRAY_BUFFER_BINDING, i);	OutputDebugString (b);
		//	glGetIntegerv (GL_UNIFORM_BUFFER_BINDING, &i);			snprintf (b, 256, "%s:%i\0", GL_ARRAY_BUFFER_BINDING, i);	OutputDebugString (b);
		}
		/*
		 * glBindBuffer binds a buffer object to the specified buffer binding point. Calling glBindBuffer with
		 *      target set to one of the accepted symbolic constants and buffer set to the name
		 *      of a buffer object binds that buffer object name to the target. If no buffer object with name buffer
		 *      exists, one is created with that name. When a buffer object is bound to a target, the previous binding for that
		 *      target is automatically broken.
		 * Any glVertexAttribPointer stuff must be done AFTER the GL_ARRAY_BUFFER target is bound.
		 * If the buffer is changed, or even rebound to the same buffer, vertex attributes must be relinked or a driver crash occurs.
		 *
		 *	GL_ARRAY_BUFFER						/// data arrays
		 *	GL_COPY_READ_BUFFER
		 *	GL_COPY_WRITE_BUFFER
		 *	GL_DRAW_INDIRECT_BUFFER				/// for glDrawArraysIndirect and glDrawElementsIndirect 
		 *	GL_DISPATCH_INDIRECT_BUFFER			/// for glDispatchComputeIndirect 
		 *	GL_ELEMENT_ARRAY_BUFFER				/// use this buffer as a source of indices for drawing commands
		 *	GL_PIXEL_PACK_BUFFER				/// bind here to read texture object data into buffers
		 *	GL_PIXEL_UNPACK_BUFFER				/// bind here to read buffer data into texture objects
		 *	GL_QUERY_BUFFER						/// for glGetQueryObject commands...
		 *	GL_TEXTURE_BUFFER					/// texture object	(alter texture stuff?)

		 *	GL_UNIFORM_BUFFER					/// Do not affect the bindings, use glBindBufferBase and glBindBufferRange instead
		 *	GL_ATOMIC_COUNTER_BUFFER			/// FIXME: instead, add dumb parameters to bindBuffer and put a conditional
		 *	GL_SHADER_STORAGE_BUFFER			/// over the target, so we can use the same syntax for all binds, and let GLUtils
		 *	GL_TRANSFORM_FEEDBACK_BUFFER		/// handle the bullshit.
		 */
		void bind (const unsigned int boid, const GLenum target = GL_ARRAY_BUFFER, const GLuint offset = 0)
		{
			switch (target)
			{
			case GL_ARRAY_BUFFER:
			case GL_ELEMENT_ARRAY_BUFFER:
			case GL_DRAW_INDIRECT_BUFFER:
			case GL_COPY_READ_BUFFER:
			case GL_COPY_WRITE_BUFFER:
				glBindBuffer (target, boid);
				break;
			case GL_ATOMIC_COUNTER_BUFFER:
			case GL_TRANSFORM_FEEDBACK_BUFFER:
			case GL_UNIFORM_BUFFER:
			case GL_SHADER_STORAGE_BUFFER:
				glBindBufferBase (target, offset, boid);
				break;
			case GL_TEXTURE_1D:
			case GL_TEXTURE_2D:
			case GL_TEXTURE_3D:
			case GL_TEXTURE_CUBE_MAP:
				glActiveTexture (GL_TEXTURE0 + offset);
				glBindTexture (target, boid);
				glActiveTexture (GL_TEXTURE0);
				break;
			default:
				std::cerr << "ERR: Unknown target for bindBuffer ()\0" << std::endl;
				break;	// error
			}
		}
		void unbind (const GLenum target = GL_ARRAY_BUFFER)
		{
			bind (0, target);
		}
		/**
		 * @param count number of elements in the set NOT size in bytes.
		 * @param data pointer to the data for uploading.
		 * @param target buffer target to upload on.
		 * @param usage hint to gl about how the buffer will be used.
		 * @returns true if the data was uploaded, false otherwise.b
		 */
		template<class T>
		void set_content (const size_t count, const T* data, const GLenum target = GL_ARRAY_BUFFER, const GLenum usage = GL_STATIC_DRAW)
		{
			glBufferData (target, count * sizeof (T), data, usage);
		}
		template<class T>
		void set_content (const std::vector<T>& vector, const GLenum target = GL_ARRAY_BUFFER, const GLenum usage = GL_STATIC_DRAW)
		{
			glBufferData (target, vector.size () * sizeof (T), &(vector[0]), usage);
		}
		void reserve (const size_t cb, const GLenum target = GL_ARRAY_BUFFER, const GLenum usage = GL_STATIC_DRAW)
		{
			glBufferData (target, cb, NULL, usage);
		}
		/**
		 * @param ptr a pointer to a pointer which will give access to the gl buffer
		 */
		template<class T>
		void ptr_map (T** ptr, const GLenum target = GL_ARRAY_BUFFER, const GLenum access = GL_READ_WRITE)
		{
			*ptr = reinterpret_cast<T*>(glMapBuffer (target, access));
		}
		/**
		 * unmaps the currently bound buffer
		 */
		void ptr_unmap (const GLenum target = GL_ARRAY_BUFFER)
		{
			glUnmapBuffer (target);
		}
		/**
		 * @param target buffer to query, default is GL_ARRAY_BUFFER
		 * @returns the size in bytes of the buffer currently bound to the specified target.
		 */
		GLsizei size (GLenum target = GL_ARRAY_BUFFER)
		{
			GLsizei cb = 0;
			glGetBufferParameteriv (target, GL_BUFFER_SIZE, &cb);
			return cb;
		}
		/**
		 * attempts to reserve (via bufferReserve ()) increasing amounts of memory until glGetError () returns
		 * something other than GL_NO_ERROR.
		 * The previous value is taken to be the largest amount of data that can be stored in that buffer.
		 * Not practical.
		 */
		GLsizei max_size (GLenum target = GL_ARRAY_BUFFER)
		{
			const GLsizei	incr = 1024*1024;	// 1Mb
			GLsizei			lastss, curss;
			unsigned int	bd;

			bd = create ();
			bind (bd, target);

			// test the routine
			for (lastss=0,curss=incr;;curss+=incr)
			{
				// attempt the upload
				reserve (curss, target);

				// test the error
				switch (glGetError ())
				{
					case GL_INVALID_ENUM:
					case GL_INVALID_VALUE:
					case GL_INVALID_OPERATION:
					case GL_OUT_OF_MEMORY:			{break;}
					case GL_NO_ERROR:
					default:						{lastss = curss; break;}
				}

				if (lastss != curss)				{break;}
			}

			clean (bd);

			return lastss;
		}
	}
}