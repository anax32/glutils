namespace gl
{
/**
 * GLCamera class maintains a simple camera for rotating and zooming around the origin
 */
    class Camera
    {
    protected:
        // angles
        float		viewPos[3];				///< view position (for translations, a bit shit)
        float		viewAngle[4];			///< view angle (x,y,z + zoom)
        float		defaultViewPos[3];		///< position to return to on reset
        float		defaultViewAngle[4];	///< angle and zoom to return to on reset

    public:
        /**
         * constructor
         * sets the defaults to arbitrary values
         */
        Camera()
        {/*
          defaultViewPos[0] = 0.0f;
          defaultViewPos[1] = 0.0f;
          defaultViewPos[2] = 0.0f;
          defaultViewAngle[0] = 45.0f;
          defaultViewAngle[1] = 45.0f;
          defaultViewAngle[2] = 0.0f;
          defaultViewAngle[3] = -10.0f;
          */
            defaultViewPos[0] = -1.75f;
            defaultViewPos[1] = 0.0f;
            defaultViewPos[2] = -0.25f;
            defaultViewAngle[0] = 41.0f;
            defaultViewAngle[1] = 41.0f;
            defaultViewAngle[2] = 0.0f;
            defaultViewAngle[3] = -64.0f;

            reset();
        }
        /**
         * destructor
         * does nothing
         */
        virtual ~Camera()
        {}
        /**
         * set the position, angle and zoom to the values in the defaults
         */
        void reset()
        {
            viewPos[0] = defaultViewPos[0];
            viewPos[1] = defaultViewPos[1];
            viewPos[2] = defaultViewPos[2];
            viewAngle[0] = defaultViewAngle[0];
            viewAngle[1] = defaultViewAngle[1];
            viewAngle[2] = defaultViewAngle[2];
            viewAngle[3] = defaultViewAngle[3];
        }
        /**
         * set the values to use as the defaults
         * @param newDefault default values to use (3 floats, xyz)
         */
        void setDefaultViewPos(float *newDefault)
        {
            defaultViewPos[0] = newDefault[0];
            defaultViewPos[1] = newDefault[1];
            defaultViewPos[2] = newDefault[2];
        }
        /**
         * set the values to use as the defaults
         * @param newDefault default values to use (4 floats, xyz zoom)
         */
        void setDefaultViewAngle(float *newDefault)
        {
            defaultViewAngle[0] = newDefault[0];
            defaultViewAngle[1] = newDefault[1];
            defaultViewAngle[2] = newDefault[2];
            defaultViewAngle[3] = newDefault[3];
        }
        /**
         * glTranslate according to zoom
         * glRotate according to view angle
         */
        void rotate() const
        {
            // rotate to the camera angles
            glTranslatef(0.0f, 0.0f, viewAngle[3]);
            glRotatef(viewAngle[0], -1.0f, 0.0f, 0.0f);
            glRotatef(viewAngle[1], 0.0f, 1.0f, 0.0f);
            glRotatef(viewAngle[2], 0.0f, 0.0f, 1.0f);
        }
        /**
         * glTranslate to viewpos
         */
        void translate()
        {
            glTranslatef(viewPos[0], viewPos[1], viewPos[2]);
        }
        void calcRotation(float x, float y)
        {
            viewAngle[0] += y;
            viewAngle[1] += x;
        }
        void calcTranslation(float x, float y)
        {
            viewPos[0] += y;
            viewPos[2] -= x;
        }
        void calcZoom(int z, const float mn = -std::numeric_limits<float>::max(), const float mx = std::numeric_limits<float>::max())
        {
            if (z > 0)
            {
                viewAngle[3] = std::max(mn, std::min(mx, viewAngle[3] * 0.9f));
                //	viewAngle[3] *= 0.9f;
            }
            else
            {
                viewAngle[3] = std::max(mn, std::min(mx, viewAngle[3] * 1.1f));
                //	viewAngle[3] *= 1.1f;
            }
        }
    };
}