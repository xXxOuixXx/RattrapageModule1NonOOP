#pragma warning(disable : 4996)
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>

#define TILEMAP_WIDTH 30
#define TILEMAP_HEIGHT 26
#define TILE_SIZE_PX 50

class Tilemap
{

public:
    char cells[TILEMAP_WIDTH * TILEMAP_HEIGHT] = { 0 };


    bool InBounds(sf::Vector2i coords);
    void Save();
    void Load();
    sf::Vector2i PosToCoords(sf::Vector2f world_position);
    bool TileAt(sf::Vector2i tile_coord);
    bool IsSolid(sf::Vector2i tile_coord);
    bool IsTrap(sf::Vector2i tile_coord);
    bool IsDoor(sf::Vector2i tile_coord);
    bool IsKey(sf::Vector2i tile_coord);
};

bool Tilemap::IsSolid(sf::Vector2i tile_coord)
{
    if (InBounds(tile_coord))
    {
        return cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] == 1;
    }
    return false;
}

bool Tilemap::IsTrap(sf::Vector2i tile_coord)
{
    if (InBounds(tile_coord))
    {
        return cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] == 2;
    }
    return false;
}

bool Tilemap::IsDoor(sf::Vector2i tile_coord)
{
    if (InBounds(tile_coord))
    {
        return cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] == 3;
    }
    return false;
}

bool Tilemap::IsKey(sf::Vector2i tile_coord)
{
    if (InBounds(tile_coord))
    {
        return cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] == 4;
    }
    return false;
}

sf::Vector2i Tilemap::PosToCoords(sf::Vector2f world_position)
{
    return sf::Vector2i(static_cast<int>(world_position.x / TILE_SIZE_PX), static_cast<int>(world_position.y / TILE_SIZE_PX));
}

bool Tilemap::InBounds(sf::Vector2i coords)
{
    return coords.x >= 0 && coords.x < TILEMAP_WIDTH && coords.y >= 0 && coords.y < TILEMAP_HEIGHT;
}

void Tilemap::Save()
{
    FILE* f = fopen("level.data", "wb");
    if (f)
    {
        fwrite(cells, sizeof(cells), 1, f);
        fclose(f);
        std::cout << "Tilemap saved successfully." << std::endl;
    }
    else
    {
        std::cout << "Error saving tilemap." << std::endl;
    }
}

void Tilemap::Load()
{
    FILE* f = nullptr;
    if (fopen_s(&f, "level.data", "rb") == 0)
    {
        fread(cells, sizeof(cells), 1, f);
        fclose(f);
        std::cout << "Tilemap loaded successfully." << std::endl;
    }
    else
    {
        std::cout << "The file level.data does not exist. A new tilemap will be created." << std::endl;
    }
}

int main()
{
    // Game parameters
    const int window_width = 800;
    const int window_height = 600;
    const int initial_lives = 3;


    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Dungeon Crawler");

    // Loading textures
    sf::Texture player_texture;
    if (!player_texture.loadFromFile("player.png")) {

        return 1;
    }

    sf::Texture key_texture;
    if (!key_texture.loadFromFile("clee.png")) {

        return 1;
    }

    sf::Texture door_texture;
    if (!door_texture.loadFromFile("door.png")) {

        return 1;
    }

    sf::Texture block_texture;
    if (!block_texture.loadFromFile("mur.png")) {

        return 1;
    }

    sf::Texture trap_texture;
    if (!trap_texture.loadFromFile("traps.png")) {

        return 1;
    }

    // Initialization of the player and game elements
    sf::RectangleShape player(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    player.setTexture(&player_texture);

    sf::RectangleShape key(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    key.setTexture(&key_texture);
    key.setPosition(sf::Vector2f(21 * TILE_SIZE_PX, 7 * TILE_SIZE_PX));

    sf::RectangleShape door(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    door.setTexture(&door_texture);
    door.setPosition(sf::Vector2f(8 * TILE_SIZE_PX, 8 * TILE_SIZE_PX));

    sf::RectangleShape block(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    block.setTexture(&block_texture);
    block.setPosition(sf::Vector2f(20 * TILE_SIZE_PX, 9 * TILE_SIZE_PX)); // Position of the block that removes a life

    sf::RectangleShape trap(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    trap.setTexture(&trap_texture);
    trap.setPosition(sf::Vector2f(20 * TILE_SIZE_PX, 9 * TILE_SIZE_PX)); // Position of the trap

    sf::Texture victory_texture;
    if (!victory_texture.loadFromFile("victory.png")) {

        return 1;
    }

    sf::Texture defeat_texture;
    if (!defeat_texture.loadFromFile("defeat.png")) {

        return 1;
    }
    int remaining_lives = initial_lives;

    // Initial position of the player
    sf::Vector2f initial_position = sf::Vector2f(23 * TILE_SIZE_PX, 23 * TILE_SIZE_PX);
    player.setPosition(initial_position);

    // Initialization of the view
    sf::View view(sf::FloatRect(50, 50, 1300, 1000));
    view.setViewport(sf::FloatRect(0, 0, 1.3f, 1.3f));

    // Initialization of the Tilemap
    Tilemap tilemap;
    tilemap.Load(); // Load the tilemap

    // Editor mode (set to true to place blocks)
    bool editor_mode = false;

    // Variable pour suivre si le joueur a la clé
    bool hasKey = false;

    // Variables for editor modes
    bool trap_mode = false;
    bool door_mode = false;
    bool key_mode = false;
    bool remove_mode = false;

    // Main loop
    while (window.isOpen() && remaining_lives > 0)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            sf::Vector2i mouse_position_window = sf::Mouse::getPosition(window);
            sf::Vector2f mouse_position_world = window.mapPixelToCoords(mouse_position_window, view); // Prend en compte la vue 

            sf::Vector2i mouse_position_tile = tilemap.PosToCoords(mouse_position_world);
            std::cout << "Mouse Position (Tilemap): (" << mouse_position_tile.x << ", " << mouse_position_tile.y << ")" << std::endl;
            sf::Vector2i trapTileCoord = tilemap.PosToCoords(trap.getPosition());
            if (event.type == sf::Event::Closed)
                window.close();

            if (!editor_mode && event.type == sf::Event::KeyPressed)
            {
                sf::Vector2f next_position = player.getPosition();

                if (event.key.code == sf::Keyboard::Up && player.getPosition().y > 0)
                    next_position.y -= TILE_SIZE_PX;
                else if (event.key.code == sf::Keyboard::Down && player.getPosition().y < TILEMAP_HEIGHT * TILE_SIZE_PX - TILE_SIZE_PX)
                    next_position.y += TILE_SIZE_PX;
                else if (event.key.code == sf::Keyboard::Left && player.getPosition().x > 0)
                    next_position.x -= TILE_SIZE_PX;
                else if (event.key.code == sf::Keyboard::Right && player.getPosition().x < 1200 - TILE_SIZE_PX)
                    next_position.x += TILE_SIZE_PX;

                // Check if solid
                sf::Vector2i next_tile_coord = tilemap.PosToCoords(next_position);
                sf::FloatRect next_player_bounds(next_position, player.getSize());

                bool collision_x = false;
                bool collision_y = false;

                // Check horizontal collision
                if (tilemap.IsSolid(sf::Vector2i(next_tile_coord.x, tilemap.PosToCoords(player.getPosition()).y)))
                {
                    collision_x = true;
                }

                // Check vertical collision
                if (tilemap.IsSolid(sf::Vector2i(tilemap.PosToCoords(player.getPosition()).x, next_tile_coord.y)))
                {
                    collision_y = true;
                }


                if (!collision_x)
                {
                    player.setPosition(sf::Vector2f(next_position.x, player.getPosition().y));
                }

                if (!collision_y)
                {
                    player.setPosition(sf::Vector2f(player.getPosition().x, next_position.y));
                }

                sf::Vector2i trapTileCoord = tilemap.PosToCoords(trap.getPosition());

                if (tilemap.IsTrap(trapTileCoord))
                {
                    // The player stepped on a trap
                    remaining_lives--;
                    player.setPosition(initial_position);

                    if (remaining_lives <= 0)
                    {
                        sf::Sprite defeatSprite(defeat_texture);
                        window.draw(defeatSprite);
                        window.display();
                        sf::sleep(sf::seconds(2)); // Pause pendant 2 secondes
                        window.close();
                    }
                }
            }

            // Toggle editor mode with the E key
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::E)
            {
                editor_mode = !editor_mode;
                trap_mode = false;
                door_mode = false;
                key_mode = false;
                remove_mode = false;
            }

            // Trap mode with the T key
            if (editor_mode && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T)
            {
                trap_mode = !trap_mode;
                door_mode = false;
                key_mode = false;
                remove_mode = false;
            }

            // Door mode with the D key
            if (editor_mode && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::D)
            {
                trap_mode = false;
                door_mode = !door_mode;
                key_mode = false;
                remove_mode = false;
            }

            // Key mode with the K key
            if (editor_mode && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::K)
            {
                trap_mode = false;
                door_mode = false;
                key_mode = !key_mode;
                remove_mode = false;
            }

            // Remove mode with the R key
            if (editor_mode && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
            {
                trap_mode = false;
                door_mode = false;
                key_mode = false;
                remove_mode = !remove_mode;
            }

            // Place or remove a tile in editor mode
            if (editor_mode && sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
                sf::Vector2f world_position = window.mapPixelToCoords(mouse_position);
                sf::Vector2i tile_coord = tilemap.PosToCoords(world_position);

                // Check if the case coordinates are valid
                if (tilemap.InBounds(tile_coord))
                {
                    // Place or remove a tile based on the active mode
                    if (trap_mode)
                        tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] = 2; // Trap
                    else if (door_mode)
                        tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] = 3; // Door
                    else if (key_mode)
                        tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] = 4; // Key
                    else if (remove_mode)
                        tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] = 0; // Enleve
                    else
                        tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x] = !tilemap.cells[tile_coord.y * TILEMAP_WIDTH + tile_coord.x];
                }
            }
        }

        // Check if the player is on a key tile
        if (player.getGlobalBounds().intersects(key.getGlobalBounds()))
        {
            door.setPosition(sf::Vector2f(10 * TILE_SIZE_PX, 1 * TILE_SIZE_PX));
            hasKey = true;
        }

        // Check if the player is on a door tile and has the key
        if (hasKey && player.getGlobalBounds().intersects(door.getGlobalBounds()))
        {

            // Display victory
            sf::Sprite victorySprite(victory_texture);
            window.draw(victorySprite);
            window.display();
            sf::sleep(sf::seconds(2));
            window.close();
        }

        // Check player is on a block tile
        if (player.getGlobalBounds().intersects(block.getGlobalBounds()))
        {
            //  stepped on a block that removes a life
            remaining_lives--;
            player.setPosition(initial_position);

            if (remaining_lives <= 0)
            {
                // Game over
                std::cout << "Defeat, you have run out of lives." << std::endl;
                window.close();
            }
        }

        view.setCenter(player.getPosition());
        window.setView(view);

        window.clear();

        sf::RectangleShape background(sf::Vector2f(TILEMAP_WIDTH * TILE_SIZE_PX, TILEMAP_HEIGHT * TILE_SIZE_PX));
        background.setFillColor(sf::Color(30, 30, 30));
        window.draw(background);

        window.draw(player);
        window.draw(key);
        window.draw(door);

        // Draw solid blocks 
        for (int i = 0; i < TILEMAP_WIDTH; ++i)
        {
            for (int j = 0; j < TILEMAP_HEIGHT; ++j)
            {
                if (tilemap.cells[j * TILEMAP_WIDTH + i])
                {
                    sf::RectangleShape solid_block(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
                    solid_block.setTexture(&block_texture);
                    solid_block.setPosition(sf::Vector2f(i * TILE_SIZE_PX, j * TILE_SIZE_PX));
                    window.draw(solid_block);
                }
            }
        }

        window.draw(block);
        window.draw(trap);

        window.display();

        sf::sleep(sf::milliseconds(50));
    }

    // Save the tilemap
    tilemap.Save();

    return 0;
}