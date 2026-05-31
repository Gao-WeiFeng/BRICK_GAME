#ifndef POWERUP_H
#define POWERUP_H

#include "raylib.h"
#include <memory>

// 道具类型
enum class PowerUpType {
    PADDLE_EXTEND,
    MULTI_BALL,
    SLOW_BALL
};

// 前向声明Game类
class Game;

// 道具效果基类
class PowerUpEffect {
public:
    virtual ~PowerUpEffect() = default;
    virtual void Apply(Game& game) = 0;
};

// 道具类
class PowerUp {
public:
    PowerUpType type;
    Vector2 position;
    Vector2 speed; // 修复：使用speed而不是velocity

    PowerUp(float x, float y, PowerUpType type);
    void Update(float dt);
    void Draw();
    Rectangle GetRect();
};

// 加长板效果
class PaddleExtendEffect : public PowerUpEffect {
private:
    float extraWidth;
    float duration;
public:
    PaddleExtendEffect(float w, float d) : extraWidth(w), duration(d) {}
    void Apply(Game& game) override;
};

// 多球效果
class MultiBallEffect : public PowerUpEffect {
private:
    int extraBalls;
public:
    MultiBallEffect(int n) : extraBalls(n) {}
    void Apply(Game& game) override;
};

// 减速效果
class SlowBallEffect : public PowerUpEffect {
private:
    float speedFactor;
    float duration;
public:
    SlowBallEffect(float f, float d) : speedFactor(f), duration(d) {} // 修复：两个参数
    void Apply(Game& game) override;
};

// 粒子结构体
struct Particle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float life;
    float maxLife;
};

// 工厂函数
std::unique_ptr<PowerUpEffect> CreatePowerUp(PowerUpType type, float extraWidth, int extraBalls, float speedFactor, float duration);

#endif