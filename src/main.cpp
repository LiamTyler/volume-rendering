#include "progression.h"

#include <fstream>

const float EPSILON = 0.00001;

const int SW = 640;
const int SH = 480;

GLuint LoadVolume(const std::string& file, int xdim, int ydim, int zdim) {
    std::ifstream in(file, std::ios::binary);
    if (in.fail())
        return -1;

    // read data in
    GLubyte* pData = new GLubyte[xdim*ydim*zdim];
    in.read((char*)pData, xdim*ydim*zdim * sizeof(GLubyte));
    in.close();

    // create the 3D texture
    GLuint volTex;
    glGenTextures(1, &volTex);
    glBindTexture(GL_TEXTURE_3D, volTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, xdim, ydim, zdim, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
    glGenerateMipmap(GL_TEXTURE_3D);
    delete[] pData;

    return volTex;
}

int main(int argc, char* argv[]) {
    PG::Window window("Volume Rendering by Raycasting", SW, SH);

    PG::Shader shader(
        "Volume Raycaster",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/volumeShader.vert",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/volumeShader.frag"
    );
    shader.Enable();

    PG::Camera camera(
        PG::Transform(glm::vec3(2, -.2, 2),
            glm::vec3(0, glm::radians(45.0f), 0))
    );

    /*GLuint volTex = LoadVolume("C:/Users/ltyler/Documents/volume-rendering/data/head.raw", 256, 256, 109);
    if (volTex == -1) {
        std::cout << "Failed to load volume" << std::endl;
        return 1;
    }*/

    // unit cube vertices
    glm::vec3 cubeVerts[8] = {
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(-0.5f, 0.5f, 0.5f)
    };
    // unit cube indices
    GLushort cubeIndices[36] = {
        0,5,4,
        5,0,1,
        3,7,6,
        3,6,2,
        7,4,6,
        6,4,5,
        2,1,3,
        3,1,0,
        3,0,7,
        7,0,4,
        6,5,2,
        2,5,1
    };
    GLuint cubeVAO;
    GLuint cubeVBO[2];  // [0] = verts, [1] = indices
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(2, cubeVBO);
    glBindVertexArray(cubeVAO);

    // setup verts
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), &cubeVerts[0].x, GL_STATIC_DRAW);
    glEnableVertexAttribArray(shader["vertex"]);
    glVertexAttribPointer(shader["vertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

    // setup indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

    
    // Setup the texture to hold the exit points for the rays
    GLuint exitPointFBO;
    glGenFramebuffers(1, &exitPointFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, exitPointFBO);

    GLuint exitPointTex;
    glGenTextures(1, &exitPointTex);
    glBindTexture(GL_TEXTURE_2D, exitPointTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SW, SH, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, exitPointTex, 0);
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer incomplete!" << std::endl;
        return EXIT_FAILURE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    static const GLfloat quadData[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };
    

    PG::Shader texShader(
        "Volume Raycaster",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/texShader.vert",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/texShader.frag"
    );
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(texShader["vertex"]);
    glVertexAttribPointer(texShader["vertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        window.StartFrame();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_p:
                    break;
                case SDLK_SPACE:
                    break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    break;
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
                float dx = (float)-event.motion.xrel;
                float dy = (float)-event.motion.yrel;
            }
        }

        float dt = window.GetDT();

       
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glBindFramebuffer(GL_FRAMEBUFFER, exitPointFBO);
        glViewport(0, 0, SW, SH);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Enable();
        glBindVertexArray(cubeVAO);


        glm::mat4 model = glm::mat4(1);

        glm::mat4 MVP = camera.GetP() * camera.GetV() * model;
        glUniformMatrix4fv(shader["MVP"], 1, GL_FALSE, glm::value_ptr(MVP));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

        glDisable(GL_CULL_FACE);



        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SW, SH);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texShader.Enable();

        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, exitPointTex);
        glUniform1i(texShader["tex"], 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);




        window.EndFrame();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(2, cubeVBO);
    //glDeleteTextures(1, &volTex);

    return 0;
}
