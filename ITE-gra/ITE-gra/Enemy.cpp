#include "Enemy.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>

#define BASE_ANGULAR_SPEED 100.0f * DEG2RAD

Enemy::Enemy(Vector2 pos, AttackType type, float duration,
    float minCooldown, float maxCooldown, float distance, float moveSpeed, float range,
    float innerAngle, float outerAngle,
    std::vector<float> ang )
    : position(pos), attackType(type), attackDuration(duration),
    attacking(false), attackDistance(distance), angles(ang),
    innerAngleDeg(innerAngle), outerAngleDeg(outerAngle),
    lockedAngle(0.0f), movementSpeed(moveSpeed/60), attackRange(range)
{
    attackCooldown = (rand() % (int)((maxCooldown - minCooldown) * 1000) + int(minCooldown * 1000)) / 1000.0f;
    attackTimer = 0.0f;
    circleRadius = distance;
    expandingRadius = 0.0f;

    attackPhase = AttackPhase::CHARGING;

    // Angular speed is inversely proportional to attack duration
    angularSpeed = BASE_ANGULAR_SPEED / attackDuration;

    flashTimer = 0.0f;
    flashOn = true;
}


void Enemy::Update(float dt, Vector2 playerPos, float playerDist) {
    if (!attacking) {
        attackCooldown -= dt;
        if (attackCooldown <= 0 && attackRange >= playerDist) StartAttack(playerPos);
    }
    else {
        attackTimer += dt;
        if (attackType == AttackType::CIRCLE) {
            expandingRadius += circleRadius / attackDuration * dt;
        }
        else if (attackType == AttackType::IN_FRONT) {
            UpdateInFrontAttack(dt, playerPos);
        }
        if (attackTimer >= attackDuration) EndAttack();
    }
   
        if (attackRange < playerDist) {

            if (abs(playerPos.x - position.x) >= 1.0)
            {
                if (position.x > playerPos.x) { position.x = position.x - (movementSpeed * 1.33); }
                else { position.x = position.x + (movementSpeed * 1.33); }
            }
            if (abs(playerPos.y - position.y) >= 1.0)
            {
                if (position.y > playerPos.y) { position.y = position.y - movementSpeed; }
                else { position.y = position.y + movementSpeed; }
            }

        }
    

}




void Enemy::Draw() {
    if (attackType == AttackType::CIRCLE && attacking) {
        DrawCircleV(circlePos, circleRadius, GRAY);
        DrawCircleV(circlePos, expandingRadius, RED);
    }
    else if (attackType == AttackType::IN_FRONT && attacking) {
        for (auto& r : rays) {
            if (!r.active) continue;

            Color c = r.color;
            if (attackPhase == AttackPhase::FIRING && r.isOuter && !flashOn) {
                c = BLANK;
            }

            Vector2 end = { position.x + r.dir.x * r.length, position.y + r.dir.y * r.length };
            DrawLineV(position, end, c);
        }
    }
    DrawCircleV(position, 10, BLUE);
}

void Enemy::StartAttack(Vector2 playerPos) {
    attacking = true;
    attackTimer = 0.0f;
    if (attackType == AttackType::CIRCLE) {
        circlePos = playerPos;
        expandingRadius = 0.0f;
    }
    else if (attackType == AttackType::IN_FRONT) {
        rays.clear();
        attackPhase = AttackPhase::CHARGING;
        flashTimer = 0.0f;
        flashOn = true;

        float innerRad = innerAngleDeg * DEG2RAD;
        float outerRad = outerAngleDeg * DEG2RAD;

        float angles[] = { -outerRad, -innerRad, innerRad, outerRad };
        Color colors[] = { GRAY, RED, RED, GRAY };
        bool isOuter[] = { true, false, false, true };

        Vector2 toPlayer = Vector2Normalize(Vector2Subtract(playerPos, position));
        float baseAngle = atan2f(toPlayer.y, toPlayer.x);
        lockedAngle = baseAngle;

        for (int i = 0; i < 4; i++) {
            EnemyRay r;
            r.isOuter = isOuter[i];
            r.color = colors[i];
            r.currentRelAngle = angles[i];
            r.targetRelAngle = (r.isOuter) ? (i == 0 ? -innerRad : innerRad) : r.currentRelAngle;
            r.length = attackDistance;
            r.active = true;

            float actualAngle = baseAngle + r.currentRelAngle;
            r.dir = { cosf(actualAngle), sinf(actualAngle) };
            rays.push_back(r);
        }
    }
}

void Enemy::UpdateInFrontAttack(float dt, Vector2 playerPos) {
    float baseAngle;

    if (attackPhase == AttackPhase::CHARGING) {
        Vector2 toPlayer = Vector2Normalize(Vector2Subtract(playerPos, position));
        baseAngle = atan2f(toPlayer.y, toPlayer.x);
        lockedAngle = baseAngle;
    }
    else {
        baseAngle = lockedAngle;
    }

    bool allOuterAtTarget = true;

    for (auto& r : rays) {
        if (!r.active) continue;

        if (attackPhase == AttackPhase::CHARGING) {
            if (r.isOuter) {
                if (fabs(r.currentRelAngle - r.targetRelAngle) > 0.01f) {
                    float step = angularSpeed * dt;
                    if (r.currentRelAngle > r.targetRelAngle) {
                        r.currentRelAngle -= step;
                        if (r.currentRelAngle < r.targetRelAngle) r.currentRelAngle = r.targetRelAngle;
                    }
                    else {
                        r.currentRelAngle += step;
                        if (r.currentRelAngle > r.targetRelAngle) r.currentRelAngle = r.targetRelAngle;
                    }
                    allOuterAtTarget = false;
                }
            }
        }
        else if (attackPhase == AttackPhase::FIRING) {
            if (!r.isOuter) {
                r.active = false;
            }
            else {
                if (fabs(r.currentRelAngle - r.targetRelAngle) > 0.01f) {
                    float step = angularSpeed * dt * 1.5f;
                    if (r.currentRelAngle > r.targetRelAngle) {
                        r.currentRelAngle -= step;
                        if (r.currentRelAngle < r.targetRelAngle) r.currentRelAngle = r.targetRelAngle;
                    }
                    else {
                        r.currentRelAngle += step;
                        if (r.currentRelAngle > r.targetRelAngle) r.currentRelAngle = r.targetRelAngle;
                    }
                }

                flashTimer += dt;
                if (flashTimer > 0.1f) {
                    flashTimer = 0.0f;
                    flashOn = !flashOn;
                }
            }
        }

        float actualAngle = baseAngle + r.currentRelAngle;
        r.dir = { cosf(actualAngle), sinf(actualAngle) };
    }

    if (attackPhase == AttackPhase::CHARGING && allOuterAtTarget) {
        attackPhase = AttackPhase::FIRING;
        for (auto& r : rays) {
            if (r.isOuter) {
                r.targetRelAngle = 0.0f;
            }
        }
    }
}

void Enemy::EndAttack() {
    attacking = false;
    attackCooldown = (rand() % 3000 + 2000) / 1000.0f;
    rays.clear();
    expandingRadius = 0.0f;
}