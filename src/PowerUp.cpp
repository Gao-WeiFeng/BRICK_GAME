#include "PowerUp.h"
#include "Game.h"
#include <cstdlib>

PowerUp::PowerUp(float x, float y, PowerUpType type) 
    : type(type), position({x, y}), speed({0, 100.0f}) {} // 修复：使用speed

void PowerUp::Update(float dt) {
    position.y += speed.y * dt; // 修复：使用speed
}

void PowerUp::Draw() {
    Color color;
    switch(type) {
        case PowerUpType::PADDLE_EXTEND: color = YELLOW; break;
        case PowerUpType::MULTI_BALL: color = GREEN; break;
        case PowerUpType::SLOW_BALL: color = BLUE; break;
        default: color = WHITE; break;
    }
    DrawRectangle(position.x, position.y, 20, 20, color);
}

Rectangle PowerUp::GetRect() {
    return {position.x, position.y, 20, 20};
}

// 加长板效果实现
void PaddleExtendEffect::Apply(Game& game) {
    // 同时加长两个板
    game.GetPaddle1().Extend(extraWidth, duration);
    game.GetPaddle2().Extend(extraWidth, duration);
}

// 多球效果实现
void MultiBallEffect::Apply(Game& game) {
    auto& balls = game.GetBalls();
    if (balls.empty()) return;
    
    // 复制现有的球，改变方向
    Ball original = balls[0];
    for (int i = 0; i < extraBalls; i++) {
        Ball newBall = original;
        Vector2 speed = original.GetSpeed();
        newBall.Reset(original.GetPosition(), {speed.x + (rand()%20 - 10), speed.y - 50});
        balls.push_back(newBall);
    }
}

// 减速效果实现
void SlowBallEffect::Apply(Game& game) {
    game.ActivateSlowEffect(duration);
}

// 工厂函数实现
std::unique_ptr<PowerUpEffect> CreatePowerUp(PowerUpType type, float extraWidth, int extraBalls, float speedFactor, float duration) {
    switch(type) {
        case PowerUpType::PADDLE_EXTEND:
            return std::make_unique<PaddleExtendEffect>(extraWidth, duration);
        case PowerUpType::MULTI_BALL:
            return std::make_unique<MultiBallEffect>(extraBalls);
        case PowerUpType::SLOW_BALL:
            return std::make_unique<SlowBallEffect>(speedFactor, duration); // 修复：两个参数
        default:
            return nullptr;
    }
}