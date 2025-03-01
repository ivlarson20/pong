/**
* Author: Isabelle Larson
* Assignment: Pong Clone
* Date due: 2025-3-01, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640 * 1.75,
              WINDOW_HEIGHT = 480 * 1.75;


constexpr float BG_RED     = 0.1922f,
                BG_BLUE    = 0.549f,
                BG_GREEN   = 0.9059f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex.glsl",
               F_SHADER_PATH[] = "shaders/fragment.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

const char ART_SPRITE_FILEPATH[] = "art.png";   // racket  1
constexpr char PAT_SPRITE_FILEPATH[] = "pat.png";   // racket 2
constexpr char BALL_SPRITE_FILEPATH[] = "ball.png";
constexpr char BACKGROUND_FILEPATH[] = "background.png";

constexpr glm::vec3 INIT_POS_ART = glm::vec3(4.2f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_PAT = glm::vec3(-4.2f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_SCALE_RACKET = glm::vec3(0.5f, 1.5f, 0.0f);
constexpr glm::vec3 INIT_SCALE_BACK = glm::vec3(0.5f, 0.5f, 0.0f);
constexpr glm::vec3 INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_SCALE_BALL = glm::vec3(0.415f, 0.49f, 0.0f);


constexpr float ROT_INCREMENT = 1.0f;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;
ShaderProgram g_shader_program;

glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix, g_background_matrix, g_art_matrix, g_pat_matrix, g_ball_matrix;  //ADD TO THIS WHEN YOU ADD A NEW SHAPE


float g_previous_ticks = 0.0f;

glm::vec3 g_translation_ball = glm::vec3(0.0f,0.0f,0.0f);

GLuint g_background_texture_id;
GLuint g_art_texture_id;
GLuint g_pat_texture_id;
GLuint g_ball_texture_id;





void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    stbi_set_flip_vertically_on_load(true);
    

    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
        
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    

    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Challengers Pong",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);


    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, WINDOW_WIDTH, WINDOW_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    g_background_texture_id = load_texture(BACKGROUND_FILEPATH);
    g_art_texture_id = load_texture(ART_SPRITE_FILEPATH);
    g_pat_texture_id = load_texture(PAT_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}

void update()
{
    // delta time calculation s
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    
    // transformations
    g_art_matrix = glm::mat4(1.0f);
    g_art_matrix = glm::translate(g_art_matrix, INIT_POS_ART);
    g_art_matrix = glm::scale(g_art_matrix, INIT_SCALE_RACKET);
    
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::scale(g_ball_matrix, INIT_SCALE_BALL);
    

   
    g_background_matrix = glm::mat4(1.0f);
    g_background_matrix = glm::scale(g_background_matrix, INIT_SCALE_BACK);
    
    g_pat_matrix = glm::mat4(1.0f);
    g_pat_matrix = glm::translate(g_pat_matrix, INIT_POS_PAT);
    g_pat_matrix = glm::scale(g_pat_matrix, INIT_SCALE_RACKET);
    
    
    
        

    
}

void draw_object(glm::mat4 object_g_model_matrix, GLuint object_texture_id)
{
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(g_shader_program.get_program_id());

    //draw_object(g_background_matrix, g_background_texture_id);
    
    draw_object(g_art_matrix, g_art_texture_id);
    draw_object(g_pat_matrix, g_pat_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);

    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
