#pragma once
#include "raylib.h"
#include <vector>

struct EnemyRay {
    Vector2 dir;
    float length;
    Color color;
    bool active;
    bool isOuter;
    float currentRelAngle;
    float targetRelAngle;
};

enum class AttackType {
    CIRCLE,
    IN_FRONT
};

enum class AttackPhase {
    CHARGING,
    FIRING
};

class Enemy {
public:
    Vector2 position;
    AttackType attackType;
    float attackDuration;
    float attackTimer;
    float attackCooldown;
    float attackDistance;
    
    std::vector<float> angles;
    std::vector<EnemyRay> rays;

    float circleRadius;
    float expandingRadius;
    bool attacking;
    Vector2 circlePos;

    AttackPhase attackPhase;
    float innerAngleDeg;
    float outerAngleDeg;
    float angularSpeed;
    float flashTimer;
    bool flashOn;
    float lockedAngle;
    
    float movementSpeed;
    float attackRange;

    Enemy(Vector2 pos, AttackType type, float duration,
        float minCooldown, float maxCooldown, float distance, float moveSpeed, float range,
        float innerAngle = 15.0f, float outerAngle = 45.0f, 
        std::vector<float> ang = {});

    void Update(float dt, Vector2 playerPos, float playerDistance);
    void Draw();

private:
    void StartAttack(Vector2 playerPos);
    void UpdateInFrontAttack(float dt, Vector2 playerPos);
    void EndAttack();
};