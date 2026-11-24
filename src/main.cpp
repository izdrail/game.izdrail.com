/// === Shoot drones! === ///

#include "player.h"
#include "enemy.h"
#include "hud.h"
#include "glfw_setup.h"
#include "enemy_manager.h"
#include "collision_detection.h"
#include "text_renderer.h"

int main(int argc, char **argv)
{
    // ask user for the type of environment
    // options: --seed N, --env desert|forest|snow|night, --autostart
    std::string envType;
    bool autostart = false, hasSpawn = false;
    float spawnX = 0, spawnZ = 0, spawnYaw = -90, spawnPitch = 0;
    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "--seed" && i + 1 < argc) World::seed = std::stoull(argv[++i]);
        else if (a == "--env" && i + 1 < argc) envType = argv[++i];
        else if (a == "--autostart") autostart = true;
        else if (a == "--pitch" && i + 1 < argc) spawnPitch = std::stof(argv[++i]);
        else if (a == "--spawn" && i + 3 < argc) { spawnX = std::stof(argv[i + 1]); spawnZ = std::stof(argv[i + 2]); spawnYaw = std::stof(argv[i + 3]); hasSpawn = true; i += 3; }
    }
    if (envType.empty())
    {
        std::cout << "Choose the environment you want to play in (desert, forest, snow, night): ";
        std::cin >> envType;
    }
    std::cout << "World seed: " << World::seed << std::endl;

    // initialize and configure glwf, load OpenGL function pointers and create window
    GLFWwindow *window = setup("Drone Shooter", Player::SCR_HEIGHT, Player::SCR_WIDTH);

    // prepare game related objects
    Player player; 
    if (autostart) player.hasStarted = true;
    if (hasSpawn) player.setSpawn(spawnX, spawnZ, spawnYaw, spawnPitch);
    World world(envType); // will choose env. type randomly if input is invalid
    EnemyManager manager;
    CollisionDetector detector;
    TextRenderer text(ASSET_PATH "/resources/font/theboldfont.ttf", ASSET_PATH "/shaders/text.vert", ASSET_PATH "/shaders/text.frag");

    // timing
    float currentFrame;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // per-frame time logic
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        text.deltaTime = deltaTime;

        // title/start screen
        if (!player.hasStarted)
        {
            startingScreen(text, player, world);
            player.processKeyboardMouse(window, deltaTime); // pressing enter starts the game
        }
    
        // game started
        if (player.hasStarted && player.getLifeState())
        {
            // pass time variables to player and enemy manager
            player.currentFrame = currentFrame; 
            manager.currentTime = currentFrame;
            manager.deltaTime = deltaTime;

            // keyboard and mouse input
            player.processKeyboardMouse(window, deltaTime);

            // draw the world objects, the gun and the enemies
            world.Draw(player.GetViewMatrix(), player.getProjectionMatrix());
            player.controlPlayerRendering();
            manager.manage(player.Position, player.GetViewMatrix(), player.getProjectionMatrix());

            // handle collisions
            detector.Detect(player, manager);

            // HUD
            inGameScreen(text, player);
        }

        // ending screen
        if (player.hasStarted && (!player.getLifeState()))
        {
            manager.reset(); // reset enemies in case player restarts the game
            endingScreen(text, player, world);
            player.processKeyboardMouse(window, deltaTime); // pressing enter restarts the game
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}