#include "Shader.hpp"

namespace MB
{
    char * GL_SHADER::TEST_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        void main(){
            gl_Position = vec4(pos * 1.0, 1.0);
        }
    );

    char * GL_SHADER::TEST_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;

        void main()
        {
            color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    );




    char * GL_SHADER::YUV_VIDEO_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;

        void main(){
            outCoor = coor;
            gl_Position = vec4(pos * 1.0, 1.0);
        }
    );

    char * GL_SHADER::YUV_VIDEO_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        in vec3 outCoor;
        uniform sampler2D numberTex;

        void main()
        {
            vec2 TexCoords = vec2(outCoor.x, 1.0 - outCoor.y);
            color = texture(numberTex, TexCoords);
        }
    );

    char * GL_SHADER::TEXT_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;
        uniform mat4 mat;

        void main(){
            outCoor = coor;
            gl_Position = mat * vec4(pos * 1.0, 1.0);
        }
    );

    char * GL_SHADER::TEXT_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        in vec3 outCoor;
        uniform sampler2D charTex;

        uniform float color_r;
        uniform float color_g;
        uniform float color_b;

        void main(){
            vec2 TexCoords = vec2(outCoor.x, 1.0 - outCoor.y);
            float r = texture(charTex, TexCoords).r;

            color = vec4(color_r, color_g, color_b, r);
        }
    );

    char * GL_SHADER::POINT_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;
        uniform mat4 mat;

        void main(){
            outCoor = coor;
            gl_Position = mat * vec4(pos * 1.0, 1.0);
        }
    );

    char * GL_SHADER::POINT_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        in vec3 outCoor;
        void main(){
            color = vec4(outCoor, 1.0);
        }
    );


    char * GL_SHADER::SINGLE_TEXTURE_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;
        void main(){
            outCoor = coor;
            gl_Position = vec4(pos, 1.0);
        }
    );

    char * GL_SHADER::SINGLE_TEXTURE_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        uniform sampler2D imageTex;
        in vec3 outCoor;
        void main(){
            vec2 TexCoords = vec2(outCoor.x, 1.0 - outCoor.y);
            vec3 rgb = texture(imageTex, TexCoords).rgb;
            color = vec4(rgb, 1.0);
        }
    );


    char * GL_SHADER::FRAME_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        uniform mat4 mvp;

        out vec3 outCoor;

        void main()
        {
            outCoor = coor;
            gl_Position = mvp * vec4(pos, 1.0);
        }
    );

    char * GL_SHADER::FRAME_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        uniform sampler2D y;
        uniform sampler2D u;
        uniform sampler2D v;
        in vec3 outCoor;
        void main(){
            vec2 t = vec2(outCoor.x, 1.0 - outCoor.y);

            vec3 yuv;
            vec3 rgb;

            yuv.x = texture(y, t).r;
            yuv.y = texture(u, t).r - 0.5;
            yuv.z = texture(v, t).r - 0.5;

            rgb = mat3( 1,       1,         1,
                        0,       -0.39465,  2.03211,
                        1.13983, -0.58060,  0) * yuv;

            color = vec4(rgb, 1.0);
        }
    );


    char * GL_SHADER::YUV_2_TEXTURE_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;

        void main()
        {
            outCoor = coor;
            gl_Position = vec4(pos, 1.0);
        }
    );

    char * GL_SHADER::YUV_2_TEXTURE_FRAGMENT_SHADER = (char *)SHADER(

        out vec4 color;

        uniform sampler2D y;
        uniform sampler2D u;
        uniform sampler2D v;

        uniform int colorRange;

        in vec3 outCoor;
        void main(){
            vec2 t = vec2(outCoor.x, outCoor.y);

            vec3 yuv;
            vec3 rgb;

            yuv.x = texture(y, t).r;
            yuv.y = texture(u, t).r - 0.5;
            yuv.z = texture(v, t).r - 0.5;

            mat3 kBt709VideoRangeYUV2RGBMatrix = mat3(  1.164, 1.164, 1.164,
                                                        0.0, -0.213, 2.112,
                                                        1.793, -0.533, 0.0);

            mat3 kBt709FullRangeYUV2RGBMatrix  = mat3(  1.164,  1.164,  1.164,
                                                        0.0,   -0.213,  2.112,
                                                        1.793, -0.533,  0.0);

            mat3 kBt601VideoRangeYUV2RGBMatrix = mat3(  1.164, 1.164, 1.164,
                                                        0.0, -0.392, 2.017,
                                                        1.596, -0.813, 0.0);

            mat3 kBt601FullRangeYUV2RGBMatrix = mat3(   1.0, 1.0, 1.0,
                                                        0.0, -0.343, 1.765,
                                                        1.4, -0.711, 0.0);

            if(colorRange == 1){
                rgb = kBt601VideoRangeYUV2RGBMatrix * yuv;
            }
            if(colorRange == 2){

                rgb = kBt601FullRangeYUV2RGBMatrix * yuv;
            }

            color = vec4(rgb, 1.0);
        }
    );



    char * GL_SHADER::MVP_TEXTURE_VERTEX_SHADER = (char *)SHADER(
        layout (location = 0) in vec3 pos;
        layout (location = 1) in vec3 coor;

        out vec3 outCoor;

        uniform mat4 mvp;

        void main(){
            outCoor = coor;
            gl_Position = mvp * vec4(pos, 1.0);
        }
    );

    char * GL_SHADER::MVP_TEXTURE_FRAGMENT_SHADER = (char *)SHADER(
        out vec4 color;
        uniform sampler2D imageTex;
        in vec3 outCoor;
        void main(){
            vec2 TexCoords = vec2(outCoor.x, 1.0 - outCoor.y);
            vec3 rgb = texture(imageTex, TexCoords).rgb;
            color = texture(imageTex, TexCoords);
        }
    );
}