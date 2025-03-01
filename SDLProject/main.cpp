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

constexpr int WINDOW_WIDTH  = 1092,
              WINDOW_HEIGHT = 516;


constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

const char ART_SPRITE_FILEPATH[] = "art.png";   // racket  1
constexpr char PAT_SPRITE_FILEPATH[] = "pat.png";   // racket 2
constexpr char BALL_SPRITE_FILEPATH[] = "ball.png";
constexpr char BACKGROUND_FILEPATH[] = "background.png";
constexpr char WIN_FILEPATH[] = "win.png";

constexpr glm::vec3 INIT_POS_ART = glm::vec3(4.2f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_POS_PAT = glm::vec3(-4.2f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_SCALE_RACKET = glm::vec3(1.5f, 1.5f, 0.0f);
constexpr glm::vec3 INIT_SCALE_BACK = glm::vec3(10.0f, 5.0f, 0.0f);
constexpr glm::vec3 INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 INIT_SCALE_BALL = glm::vec3(0.60f, 0.60f, 0.0f);
constexpr glm::vec3 INIT_POS_WIN = glm::vec3(0.0f,0.0f,0.0f);
constexpr glm::vec3 INIT_SCALE_WIN = glm::vec3(6.52f, 1.48f, 0.0f);


//constexpr float ROT_INCREMENT = 1.0f;

AppStatus g_app_status = RUNNING;
SDL_Window* g_display_window;
ShaderProgram g_shader_program;

glm::mat4 g_view_matrix, g_model_matrix, g_projection_matrix, g_background_matrix, g_art_matrix, g_pat_matrix, g_ball_matrix, g_win_matrix;  //ADD TO THIS WHEN YOU ADD A NEW SHAPE


float g_previous_ticks = 0.0f;
bool is_playing = false;
bool did_win = false;

glm::vec3 g_translation_ball = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_art_movement = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_pat_movement = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_art_position = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_pat_position = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_ball_position = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_ball_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_init_velocity = glm::vec3(-2.0f, 1.5f, 0.0f);
glm::vec3 g_win_position = glm::vec3(0.0f,0.0f,0.0f);



float g_racket_speed = 0.5f;

GLuint g_background_texture_id;
GLuint g_art_texture_id;
GLuint g_pat_texture_id;
GLuint g_ball_texture_id;
GLuint g_win_texture_id;





void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    //stbi_set_flip_vertically_on_load(true);
    

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
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -2.36f, 2.36f, -1.0f, 1.0f);

    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    g_background_texture_id = load_texture(BACKGROUND_FILEPATH);
    g_art_texture_id = load_texture(ART_SPRITE_FILEPATH);
    g_pat_texture_id = load_texture(PAT_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_win_texture_id = load_texture(WIN_FILEPATH);
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_art_movement = glm::vec3(0.0f);
    g_pat_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_UP:
                        // move art up
                        g_art_movement.y = 1.0f;
                        break;
                    
                    case SDLK_DOWN:
                        // move art down
                        g_art_movement.y = -1.0f;
                        break;
                        
                    case SDLK_w:
                        // move pat up
                        g_pat_movement.y = 1.0f;
                        break;
                        
                    case SDLK_s:
                        // move pat down
                        g_pat_movement.y = -1.0f;
                        break;
                        
                    
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        if (!is_playing) {
                            g_ball_velocity = g_ball_init_velocity;
                            g_win_position = INIT_POS_WIN;
                            is_playing = true;
                            did_win = false;
                        }
                        break;

                        
                }

        
        }
    }
}

void update()
{
    // delta time calculation s
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    g_art_position += g_art_movement * g_racket_speed;
    g_pat_position += g_pat_movement * g_racket_speed;
    g_ball_position += g_ball_velocity * delta_time;

    // bound by top and bottom
    if (g_ball_position.y >= 2.36f - INIT_SCALE_BALL.y || g_ball_position.y <= -2.36f + INIT_SCALE_BALL.y) {
        g_ball_velocity.y = -g_ball_velocity.y;
    }
    
    // collide w the paddle
    float x_distance_art = fabs(g_art_position.x + INIT_POS_ART.x - INIT_POS_BALL.x - g_ball_position.x)-((INIT_SCALE_BALL.x + INIT_SCALE_RACKET.x) / 2.0f);
    float x_distance_pat = fabs(g_pat_position.x + INIT_POS_PAT.x - INIT_POS_BALL.x - g_ball_position.x)-((INIT_SCALE_BALL.x + INIT_SCALE_RACKET.x) / 2.0f);
    float y_distance_art = fabs(g_art_position.y + INIT_POS_ART.y - INIT_POS_BALL.y - g_ball_position.y)-((INIT_SCALE_BALL.y + INIT_SCALE_RACKET.y) / 2.0f);
    float y_distance_pat = fabs(g_pat_position.y + INIT_POS_PAT.y - INIT_POS_BALL.y - g_ball_position.y)-((INIT_SCALE_BALL.y + INIT_SCALE_RACKET.y) / 2.0f);
    
    if ((x_distance_art < 0.0f && y_distance_art < 0.0f) ||(x_distance_pat < 0.0f && y_distance_pat < 0.0f))
    {
        g_ball_velocity.x = -g_ball_velocity.x;
    }

    
    // someone wins - ADD THE MESSAGE HERE
    if (g_ball_position.x > 5.0f || g_ball_position.x < -5.0f) {
        g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f); // Reset ball
        g_ball_velocity = glm::vec3(0.0f, 0.0f, 0.0f); // Reset velocity
        is_playing = false;
        did_win = true;
    }
    
    
    // transformations
    //right racket, white if my sprites ever show up
    g_art_matrix = glm::mat4(1.0f);
    g_art_matrix = glm::translate(g_art_matrix, INIT_POS_ART);
    g_art_matrix = glm::translate(g_art_matrix, g_art_position);
    g_art_matrix = glm::scale(g_art_matrix, INIT_SCALE_RACKET);
    
    //left racket, black if my sprites ever show
    g_pat_matrix = glm::mat4(1.0f);
    g_pat_matrix = glm::translate(g_pat_matrix, INIT_POS_PAT);
    g_pat_matrix = glm::translate(g_pat_matrix, g_pat_position);
    g_pat_matrix = glm::scale(g_pat_matrix, INIT_SCALE_RACKET);
    
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);
    g_ball_matrix = glm::scale(g_ball_matrix, INIT_SCALE_BALL);
    

    g_background_matrix = glm::mat4(1.0f);
    g_background_matrix = glm::scale(g_background_matrix, INIT_SCALE_BACK);
    
    g_win_matrix = glm::mat4(1.0f);
    g_win_matrix = glm::translate(g_win_matrix, INIT_POS_WIN);
    g_win_matrix = glm::scale(g_win_matrix, INIT_SCALE_WIN);
    

    
    
    
        

    
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

    draw_object(g_background_matrix, g_background_texture_id);
    
    draw_object(g_art_matrix, g_art_texture_id);
    draw_object(g_pat_matrix, g_pat_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    
    if (did_win == true)
    {
        draw_object(g_win_matrix, g_win_texture_id);
    }


    
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
