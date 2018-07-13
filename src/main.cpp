#include "progression.h"

#include <fstream>

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);


const int XDIM = 256;
const int YDIM = 256;
const int ZDIM = 109;

const float EPSILON = 0.00001;

GLuint LoadVolume(const std::string& file) {
    std::ifstream in(file, std::ios::binary);
    if (in.fail())
        return -1;

    // read data in
    GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
    in.read((char*)pData, XDIM*YDIM*ZDIM * sizeof(GLubyte));
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
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, XDIM, YDIM, ZDIM, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
    glGenerateMipmap(GL_TEXTURE_3D);
    delete[] pData;

    return volTex;
}

int main(int argc, char* argv[]) {
    PG::Window window("Volume Rendering by Raycasting", 640, 480);

    PG::Shader shader(
        "Volume Raycaster",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/volumeShader.vert",
        "C:/Users/ltyler/Documents/volume-rendering/shaders/volumeShader.frag"
    );
    shader.Enable();

    PG::Camera camera(
        PG::Transform(glm::vec3(0, 0, 5))
    );

    GLuint volTex = LoadVolume("C:/Users/ltyler/Documents/volume-rendering/data/head.raw");
    if (volTex == -1) {
        std::cout << "Failed to load volume" << std::endl;
        return 1;
    }

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.Enable();
    glBindVertexArray(cubeVAO);

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
        GL_CHECK_ERRORS

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 model = glm::mat4(1);

        glm::mat4 MVP = camera.GetP() * camera.GetV() * model;
        glUniformMatrix4fv(shader["MVP"], 1, GL_FALSE, glm::value_ptr(MVP));
        glUniform3fv(shader["camPos"], 1, glm::value_ptr(camera.transform.position));
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);


        window.EndFrame();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(2, cubeVBO);
    glDeleteTextures(1, &volTex);

    return 0;
}
