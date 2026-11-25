#include "collision_detection.h"
#include <cmath>
#include <random>

CollisionDetector::CollisionDetector(){}

void CollisionDetector::Detect(Player &player, EnemyManager &manager)
{
    if (player.fired)
    {
        // collision detection between player;s bullet and enemy
        for (unsigned int i = 0; i < manager.enemyCount - 1; i++)
        {
            PlayerAttacksEnemy(player, manager.enemies[i]);
        }
    }

    for (unsigned int i = 0; i < manager.enemyCount - 1; i++)
    {
        if (manager.enemies[i]->canDamage) 
        {
            // collision detection between enemy's laser and player
            EnemyAttacksPlayer(player, manager.enemies[i]);
        }
    }
}

void CollisionDetector::PlayerAttacksEnemy(Player &player, std::shared_ptr<Enemy> enemy)
{
    // every pellet is its own ray inside the weapon's spread cone
    const WeaponDef *w = player.currentWeapon();
    const int pellets = w ? w->pellets : 1;
    const float spread = w ? glm::radians(w->spreadDeg) : 0.0f;
    static std::mt19937 gen(12345);
    std::uniform_real_distribution<float> jitter(-1.0f, 1.0f);
    for (int p = 0; p < pellets; ++p)
    {
        glm::vec3 dir = player.Front;
        if (spread > 0)
            dir = glm::normalize(dir + (player.Right * jitter(gen) + player.Up * jitter(gen)) * std::tan(spread));
        Ray ray(player.Position, dir);
        if (enemy->boundingBox.intersect(ray, player.range) == true && enemy->getLifeState())
        {
            enemy->gotHit(); // kill enemy
            player.updateKills(); // update killcount by one
            break;
        }
    }
}

void CollisionDetector::EnemyAttacksPlayer(Player &player, std::shared_ptr<Enemy> enemy)
{
    // construct ray object with enemy position and laser direction
    Ray ray = Ray(enemy->position, enemy->laserDirection);

    // check for collision
    if (player.boundingBox.intersect(ray, enemy->range))
    {
        player.gotAttacked(Enemy::DAMAGE); // decrease player's health
        enemy->canDamage = false; // set to false to ensure that player's health only decreases once per hit
    }
}