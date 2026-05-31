#include "Game.h"
#include "json.hpp"
#include "PowerUp.h"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
using json = nlohmann::json;

// 总关卡数定义
const int Game::TOTAL_LEVELS = 3;

// Leaderboard的实现
Leaderboard::Leaderboard(const char* file) : count(0), filename(file) { Load(); }

void Leaderboard::Load() {
    FILE* f = fopen(filename, "r");
    if (f) {
        count = 0;
        while (count < MAX_ENTRIES && fscanf(f, "%31s %d %ld", entries[count].name, &entries[count].score, &entries[count].timestamp) == 3) {
            count++;
        }
        fclose(f);
    }
}

void Leaderboard::Save() {
    FILE* f = fopen(filename, "w");
    if (f) {
        for (int i = 0; i < count; i++) fprintf(f, "%s %d %ld\n", entries[i].name, entries[count].timestamp);
    }
}

int Leaderboard::AddScore(const char* name, int score) {
    if (count >= MAX_ENTRIES && score <= entries[count - 1].score) return 0;
    
    ScoreEntry newEntry;
    strncpy(newEntry.name, name, 31); newEntry.name[31] = '\0';
    newEntry.score = score; newEntry.timestamp = time(nullptr);
    
    int pos = 0;
    while (pos < count && entries[pos].score >= score) pos++;
    if (count < MAX_ENTRIES) count++;
    for (int i = count - 1; i > pos; i--) entries[i] = entries[i - 1];
    entries[pos] = newEntry;
    
    Save();
    return pos + 1;
}

bool Leaderboard::GetEntry(int rank, ScoreEntry& entry) {
    if (rank > 0 && rank <= count) { entry = entries[rank - 1]; return true; }
    return false;
}

int Leaderboard::GetCount() { return count; }
bool Leaderboard::CanEnter(int score) { return count < MAX_ENTRIES || score > entries[count - 1].score; }

// 带错误处理的JSON加载函数
json Game::LoadJSONWithFallback(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            TraceLog(LOG_WARNING, "文件不存在: %s，使用默认配置", path.c_str());
            return json();
        }
        json config;
        file >> config;
        return config;
    } catch (const json::parse_error& e) {
        TraceLog(LOG_ERROR, "JSON解析失败: %s", e.what());
        return json();
    }
}

// 加载指定关卡
void Game::LoadLevel(int level) {
    // 清空旧的砖块
    bricks.clear();
    
    // 重置球和板
    balls.clear();
    Ball newBall({(float)screenWidth/2, (float)screenHeight - 100}, {0.0f, 0.0f}, ballRadius);
    balls.push_back(newBall);
    paddle1 = Paddle(150, screenHeight - 50, paddleWidth, paddleHeight);
    paddle2 = Paddle(screenWidth - 150 - paddleWidth, screenHeight - 50, paddleWidth, paddleHeight);
    
    // 清空道具和粒子
    powerUps.clear();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particleActive[i] = false;
    }
    slowEffectTime = 0.0f;
    gameTime = 0.0f;
    
    // 加载关卡文件
    std::string filename = "level" + std::to_string(level) + ".json";
    json config = LoadJSONWithFallback(filename);
    bool loaded = false;
    
    if (!config.empty() && config.contains("bricks")) {
        auto& bricks_cfg = config["bricks"];
        if (bricks_cfg.contains("rows") && bricks_cfg.contains("cols") && bricks_cfg.contains("layout")) {
            int rows = bricks_cfg["rows"];
            int cols = bricks_cfg["cols"];
            float width = bricks_cfg.value("width", brickWidth);
            float height = bricks_cfg.value("height", brickHeight);
            auto layout = bricks_cfg["layout"];
            
            // 颜色映射
            std::map<int, Color> color_map;
            color_map[1] = RED;
            color_map[2] = ORANGE;
            color_map[3] = YELLOW;
            color_map[4] = GREEN;
            color_map[5] = BLUE;
            color_map[6] = PURPLE;
            
            // 如果有自定义color_map，覆盖默认
            if (bricks_cfg.contains("color_map")) {
                auto& cm = bricks_cfg["color_map"];
                for (auto& item : cm.items()) {
                    int type = std::stoi(item.key());
                    std::string color_name = item.value();
                    if (color_name == "red") color_map[type] = RED;
                    else if (color_name == "orange") color_map[type] = ORANGE;
                    else if (color_name == "yellow") color_map[type] = YELLOW;
                    else if (color_name == "green") color_map[type] = GREEN;
                    else if (color_name == "blue") color_map[type] = BLUE;
                    else if (color_name == "purple") color_map[type] = PURPLE;
                }
            }
            
            // 创建砖块
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    int brickType = layout[i][j];
                    if (brickType != 0) {
                        float x = 50 + j * width;
                        float y = 80 + i * height;
                        Color color = color_map.count(brickType) ? color_map[brickType] : RED;
                        bricks.emplace_back(x, y, width, height, color);
                    }
                }
            }
            loaded = true;
        }
    }
    
    // 如果加载失败，使用默认的硬编码布局
    if (!loaded) {
        TraceLog(LOG_WARNING, "加载关卡%d失败，使用默认布局", level);
        Color brickColors[] = {RED, ORANGE, YELLOW, GREEN, BLUE};
        for (int row = 0; row < brickRows; row++) {
            for (int col = 0; col < brickCols; col++) {
                bricks.emplace_back(50 + col * brickWidth, 80 + row * brickHeight, brickWidth, brickHeight, brickColors[row]);
            }
        }
    }
    
    winCount = (int)bricks.size();
    currentState = GameState::PLAYING;
}

// 保存存档
void Game::SaveSave() {
    // 网络模式下不保存存档
    if (isNetworked) return;
    try {
        json save;
        save["version"] = 1;
        save["current_level"] = currentLevel;
        save["score"] = score;
        save["lives"] = lives;
        save["slow_effect_remaining"] = slowEffectTime;
        
        std::ofstream file("save.json");
        file << save.dump(4); // 格式化输出，缩进4空格
        saveExists = true;
        TraceLog(LOG_INFO, "存档保存成功！");
    } catch (std::exception& e) {
        TraceLog(LOG_ERROR, "存档保存失败：%s", e.what());
    }
}

// 加载存档
bool Game::LoadSave() {
    try {
        std::ifstream file("save.json");
        if (!file.is_open()) {
            TraceLog(LOG_WARNING, "存档文件不存在");
            return false;
        }
        json save;
        file >> save;
        
        // 版本兼容处理
        int version = save.value("version", 0);
        if (version == 1) {
            currentLevel = save["current_level"];
            score = save["score"];
            lives = save["lives"];
            slowEffectTime = save.value("slow_effect_remaining", 0.0f);
        } else {
            // 旧版本存档，使用默认值
            TraceLog(LOG_WARNING, "存档版本不兼容，使用默认值");
            currentLevel = 1;
            score = 0;
            lives = initialLives;
        }
        
        // 加载对应的关卡
        LoadLevel(currentLevel);
        TraceLog(LOG_INFO, "存档加载成功！当前关卡：%d", currentLevel);
        return true;
    } catch (std::exception& e) {
        TraceLog(LOG_ERROR, "存档加载失败：%s", e.what());
        return false;
    }
}

// Game类的实现
Game::Game() :
    paddle1(150.0f, 550.0f, 100.0f, 15.0f),
    paddle2(550.0f, 550.0f, 100.0f, 15.0f),
    leaderboard("scores.txt"),
    currentState(GameState::MENU),
    slowEffectTime(0.0f),
    slowSpeedFactor(0.7f),
    isNetworked(false),
    isServer(false),
    netHost(nullptr),
    netPeer(nullptr),
    loadState(LoadState::IDLE),
    bricksColorChanged(false),
    currentLevel(1),
    saveExists(false)
{
    // 初始化粒子对象池
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particleActive[i] = false;
    }
    // 先给默认值，后面会从配置文件覆盖
    screenWidth = 800;
    screenHeight = 600;
    windowTitle = "Breakout";
    ballRadius = 10.0f;
    ballGravity = 0.08f;
    ballMaxSpeed = 15.0f;
    ballBounceForce = 0.5f;
    paddleWidth = 100.0f;
    paddleHeight = 15.0f;
    paddleSpeed = 18.0f;
    paddleBoostSpeed = 28.0f;
    brickRows = 5;
    brickCols = 8;
    brickWidth = 85.0f;
    brickHeight = 25.0f;
    initialLives = 3;
    lives = initialLives; // 初始化生命值，避免未初始化的随机值
    scorePerBrick = 10;
    timeMultiplierDecay = 0.05f;
    // 道具默认配置
    powerup_config[(int)PowerUpType::PADDLE_EXTEND] = {40, 2, 0.7f, 5, 0.3f};
    powerup_config[(int)PowerUpType::MULTI_BALL] = {40, 2, 0.7f, 0, 0.2f};
    powerup_config[(int)PowerUpType::SLOW_BALL] = {40, 2, 0.7f, 5, 0.25f};
}

void Game::InitChineseFont() {
    // 把所有用到的中文字符全部列出来，一个都不能漏！
    const char* text = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:打砖块游戏按C继续上次第一第二第三关主机客户端模式排行榜等待连接正在分数生命关卡时间秒空格发射暂停重开加载全部通关结束返回菜单板加宽生效减速多球";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(text, &codepointCount);
    const char* fontPaths[] = { "../fonts/NotoSansSC.otf", "/home/cly/Test/Breakout/fonts/NotoSansSC.otf", "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc", "/mnt/c/Windows/Fonts/msyh.ttc" };
    printf("正在尝试加载中文字体...\n");
    fontLoaded = false;
    for (int i = 0; i < 4; i++) {
        if (FileExists(fontPaths[i])) {
            printf("  找到字体文件: %s\n", fontPaths[i]);
            chineseFont = LoadFontEx(fontPaths[i], 48, codepoints, codepointCount);
            if (chineseFont.texture.id != 0) {
                printf("  字体加载成功！\n");
                fontLoaded = true;
                break;
            }
        }
    }
    if (!fontLoaded) {
        printf("警告: 字体加载失败，使用默认字体\n");
        chineseFont = GetFontDefault();
    }
    UnloadCodepoints(codepoints);
}

void Game::DrawChineseText(const char* text, int x, int y, int fontSize, Color color) {
    Vector2 pos = { (float)x, (float)y };
    DrawTextEx(chineseFont, text, pos, (float)fontSize, 2, color);
}

void Game::DrawChineseTextCentered(const char* text, int y, int fontSize, Color color) {
    Vector2 size = MeasureTextEx(chineseFont, text, (float)fontSize, 2);
    DrawChineseText(text, (screenWidth - (int)size.x) / 2, y, fontSize, color);
}

int Game::CalculateScore(int baseScore) {
    float multiplier = 5.0f - gameTime * timeMultiplierDecay;
    if (multiplier < 1.0f) multiplier = 1.0f;
    return (int)(baseScore * multiplier);
}

void Game::ResetGame() {
    // 重置游戏状态
    balls.clear();
    Ball newBall({(float)screenWidth/2, (float)screenHeight - 100}, {0.0f, 0.0f}, ballRadius);
    balls.push_back(newBall);
    // 清空道具和粒子
    powerUps.clear();
    // 清空粒子对象池
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particleActive[i] = false;
    }
    slowEffectTime = 0.0f;
    score = 0;
    lives = initialLives;
    gameTime = 0.0f;
    playerRank = 0;
    currentLevel = 1; // 重置关卡到第一关
    // 加载第一关
    LoadLevel(1);
    // 重置两个板
    paddle1 = Paddle(150, screenHeight - 50, paddleWidth, paddleHeight);
    paddle2 = Paddle(screenWidth - 150 - paddleWidth, screenHeight - 50, paddleWidth, paddleHeight);
}

void Game::LoadConfig(const std::string& path) {
    try {
        std::ifstream f(path);
        if (!f.is_open()) {
            printf("警告：无法打开配置文件，使用默认参数\n");
            return;
        }
        json config = json::parse(f);
        // 读取窗口配置
        if (config.contains("window")) {
            screenWidth = config["window"]["width"];
            screenHeight = config["window"]["height"];
            windowTitle = config["window"]["title"];
        }
        // 读取球的配置
        if (config.contains("ball")) {
            ballRadius = config["ball"]["radius"];
            ballGravity = config["ball"]["gravity"];
            ballMaxSpeed = config["ball"]["maxSpeed"];
            ballBounceForce = config["ball"]["bounceForce"];
        }
        // 读取板的配置
        if (config.contains("paddle")) {
            paddleWidth = config["paddle"]["width"];
            paddleHeight = config["paddle"]["height"];
            paddleSpeed = config["paddle"]["speed"];
            paddleBoostSpeed = config["paddle"]["boostSpeed"];
        }
        // 读取砖块的配置
        if (config.contains("bricks")) {
            brickRows = config["bricks"]["rows"];
            brickCols = config["bricks"]["cols"];
            brickWidth = config["bricks"]["width"];
            brickHeight = config["bricks"]["height"];
        }
        // 读取游戏的配置
        if (config.contains("game")) {
            initialLives = config["game"]["initialLives"];
            scorePerBrick = config["game"]["scorePerBrick"];
            timeMultiplierDecay = config["game"]["timeMultiplierDecay"];
        }
        // 读取道具的配置！
        if (config.contains("powerups")) {
            auto& pu = config["powerups"];
            if (pu.contains("paddle_extend")) {
                auto& p = pu["paddle_extend"];
                powerup_config[(int)PowerUpType::PADDLE_EXTEND].extra_width = p["extra_width"];
                powerup_config[(int)PowerUpType::PADDLE_EXTEND].duration = p["duration"];
                powerup_config[(int)PowerUpType::PADDLE_EXTEND].drop_rate = p["drop_rate"];
            }
            if (pu.contains("multi_ball")) {
                auto& p = pu["multi_ball"];
                powerup_config[(int)PowerUpType::MULTI_BALL].extra_balls = p["extra_balls"];
                powerup_config[(int)PowerUpType::MULTI_BALL].duration = p["duration"];
                powerup_config[(int)PowerUpType::MULTI_BALL].drop_rate = p["drop_rate"];
            }
            if (pu.contains("slow_ball")) {
                auto& p = pu["slow_ball"];
                powerup_config[(int)PowerUpType::SLOW_BALL].speed_factor = p["speed_factor"];
                powerup_config[(int)PowerUpType::SLOW_BALL].duration = p["duration"];
                powerup_config[(int)PowerUpType::SLOW_BALL].drop_rate = p["drop_rate"];
                slowSpeedFactor = p["speed_factor"];
            }
        }
        printf("配置文件加载成功！\n");
    } catch (std::exception& e) {
        printf("配置文件解析失败：%s，使用默认参数\n", e.what());
    }
}

void Game::Init() {
    // 初始化字体
    InitChineseFont();
    // 加载配置文件
    LoadConfig("../config.json");
    // 检测是否有存档
    saveExists = FileExists("save.json");
    // 重新初始化对象，用配置的参数
    balls.clear();
    Ball newBall({(float)screenWidth/2, (float)screenHeight - 100}, {0.0f, 0.0f}, ballRadius);
    balls.push_back(newBall);
    paddle1 = Paddle(150, screenHeight - 50, paddleWidth, paddleHeight);
    paddle2 = Paddle(screenWidth - 150 - paddleWidth, screenHeight - 50, paddleWidth, paddleHeight);
    // 关键修改：不调用ResetGame()，直接设置为主菜单状态
    currentState = GameState::MENU;
    SetTargetFPS(60);
}

bool Game::InitNetwork(bool isServer, const char* serverIp) {
    if (enet_initialize() != 0) {
        printf("ENet库初始化失败！\n");
        return false;
    }
    this->isNetworked = true;
    this->isServer = isServer;
    if (isServer) {
        // 主机：监听12345端口
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = 12345;
        netHost = enet_host_create(&address, 1, 2, 0, 0); // 最多1个客户端
        if (netHost == nullptr) {
            printf("创建ENet主机失败！\n");
            enet_deinitialize();
            return false;
        }
        printf("主机已启动，等待客户端连接...\n");
    } else {
        // 客户端：连接到指定主机
        netHost = enet_host_create(nullptr, 1, 2, 0, 0);
        if (netHost == nullptr) {
            printf("创建ENet客户端失败！\n");
            return false;
        }
        ENetAddress address;
        enet_address_set_host(&address, serverIp);
        address.port = 12345;
        enet_host_connect(netHost, &address, 2, 0);
        printf("正在连接主机...\n");
    }
    return true;
}

void Game::ShutdownNetwork() {
    if (isNetworked) {
        if (netHost) enet_host_destroy(netHost);
        enet_deinitialize();
        isNetworked = false;
    }
}

void Game::ProcessNetworkEvents() {
    if (!isNetworked || !netHost) return;
    
    ENetEvent event;
    while (enet_host_service(netHost, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("客户端已连接！游戏开始\n");
            ResetGame(); // 主机开始游戏
            currentState = GameState::PLAYING;
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("客户端已断开\n");
            ShutdownNetwork();
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            if (isServer) {
                // 主机收到客户端的板位置
                NetworkPaddlePosition pos;
                memcpy(&pos, event.packet->data, sizeof(pos));
                paddle1.GetRect().x = pos.x;
            } else {
                // 客户端收到主机的游戏状态
                NetworkGameState state;
                memcpy(&state, event.packet->data, sizeof(state));
                
                // 保存快照用于插值
                lastSnapshot = currentSnapshot;
                lastSnapshotTime = GetTime();
                
                currentSnapshot.state = state;
                currentSnapshot.timestamp = GetTime();
                
                // 连接成功后自动开始游戏
                if (currentState == GameState::MENU) {
                    printf("连接到主机成功！游戏开始\n");
                    ResetGame(); // 客户端也初始化砖块
                    currentState = GameState::PLAYING;
                }
            }
            enet_packet_destroy(event.packet);
        }
    }
}

void Game::SendGameState() {
    if (!isNetworked || !isServer || netPeer == nullptr) return;
    
    NetworkGameState state;
    state.ballX = balls[0].GetPosition().x;
    state.ballY = balls[0].GetPosition().y;
    state.ballSpeedX = balls[0].GetSpeed().x;
    state.ballSpeedY = balls[0].GetSpeed().y;
    state.paddle1X = paddle1.GetRect().x;
    state.paddle2X = paddle2.GetRect().x;
    state.score = score;
    state.lives = lives;
    state.gameTime = gameTime;
    
    // 打包砖块状态
    uint64_t brickMask = 0;
    for (int i = 0; i < bricks.size() && i < 40; i++) {
        if (bricks[i].IsActive()) {
            brickMask |= (1ULL << i);
        }
    }
    state.brickMask = brickMask;
    
    // 打包球状态
    for (int i = 0; i < 3; i++) {
        if (i < balls.size()) {
            state.balls[i].x = balls[i].GetPosition().x;
            state.balls[i].y = balls[i].GetPosition().y;
            state.balls[i].vx = balls[i].GetSpeed().x;
            state.balls[i].vy = balls[i].GetSpeed().y;
            state.balls[i].launched = balls[i].IsLaunched();
        } else {
            state.balls[i].launched = false;
        }
    }
    
    // 打包道具状态
    for (int i = 0; i < 3; i++) {
        if (i < powerUps.size()) {
            state.powerups[i].x = powerUps[i].position.x;
            state.powerups[i].y = powerUps[i].position.y;
            state.powerups[i].type = (int)powerUps[i].type;
            state.powerups[i].active = powerUps[i].position.y < screenHeight;
        } else {
            state.powerups[i].active = false;
        }
    }
    
    ENetPacket* packet = enet_packet_create(&state, sizeof(state), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(netPeer, 0, packet);
}

void Game::SendPaddlePosition(float x) {
    if (!isNetworked || isServer || netPeer == nullptr) return;
    NetworkPaddlePosition pos;
    pos.x = x;
    ENetPacket* packet = enet_packet_create(&pos, sizeof(pos), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(netPeer, 0, packet);
}

void Game::InterpolateState(float alpha) {
    // 更新客户端显示
    // 先清空本地的球，然后根据主机的状态重建
    if (!isNetworked || isServer) return;
    
    float t = (GetTime() - lastSnapshotTime);
    if (t > 0.1f) return; // 太久没更新就不插值了
    
    NetworkGameState& prev = lastSnapshot.state;
    NetworkGameState& curr = currentSnapshot.state;
    
    // 插值板位置
    paddle1.GetRect().x = prev.paddle1X + (curr.paddle1X - prev.paddle1X) * alpha;
    paddle2.GetRect().x = prev.paddle2X + (curr.paddle2X - prev.paddle2X) * alpha;
    
    // 插值球位置
    if (balls.empty()) balls.emplace_back(Vector2{0,0}, Vector2{0,0}, ballRadius);
    balls[0].Reset(
        Vector2{prev.ballX + (curr.ballX - prev.ballX) * alpha, prev.ballY + (curr.ballY - prev.ballY) * alpha},
        Vector2{curr.ballSpeedX, curr.ballSpeedY}
    );
    
    // 同步游戏状态
    score = curr.score;
    lives = curr.lives;
    gameTime = curr.gameTime;
    
    // 解析砖块状态bitmask，更新每个砖块的激活状态
    for (int i = 0; i < bricks.size() && i < 40; i++) {
        bool active = (curr.brickMask & (1ULL << i)) != 0;
        bricks[i].SetActive(active);
    }
}

void Game::Update() {
    float dt = GetFrameTime();
    // 处理网络事件
    ProcessNetworkEvents();
    
    // 全局按键处理
    if (IsKeyPressed(KEY_R)) ResetGame();
    if (IsKeyPressed(KEY_P)) {
        if (currentState == GameState::PLAYING) {
            currentState = GameState::PAUSED;
        } else if (currentState == GameState::PAUSED) {
            currentState = GameState::PLAYING;
        }
    }
    
    // 单人模式下支持自由切换关卡
    if (!isNetworked) {
        if (IsKeyPressed(KEY_ONE)) {
            currentLevel = 1;
            LoadLevel(1);
        }
        if (IsKeyPressed(KEY_TWO)) {
            currentLevel = 2;
            LoadLevel(2);
        }
        if (IsKeyPressed(KEY_THREE)) {
            currentLevel = 3;
            LoadLevel(3);
        }
        // L键：打开排行榜
        if (IsKeyPressed(KEY_L)) {
            currentState = GameState::LEADERBOARD;
        }
    }
    // 主菜单按键处理
    if (currentState == GameState::MENU) {
        // 关卡选择已经在全局按键处理过了，这里只处理网络和继续
        if (IsKeyPressed(KEY_C) && saveExists) {
            isNetworked = false;
            if (!LoadSave()) {
                // 加载失败，重置
                ResetGame();
            }
        }
        if (IsKeyPressed(KEY_H)) {
            // 主机模式
            if (InitNetwork(true)) {
                // 等待客户端连接
            }
        }
        if (IsKeyPressed(KEY_J)) {
            // 客户端模式
            InitNetwork(false, "127.0.0.1");
        }
        if (IsKeyPressed(KEY_L)) {
            // 主菜单打开排行榜
            currentState = GameState::LEADERBOARD;
        }
        return;
    }
    
    // 【核心修改】暂停时只保留输入处理，跳过所有游戏更新逻辑
    // 原来的错误判断：if (currentState != GameState::PLAYING && currentState != GameState::PAUSED) return;
    if (currentState != GameState::PLAYING) return;
    
    // 更新板的效果时间，直接调用Update，用户的函数自己会处理
    paddle1.Update(dt);
    paddle2.Update(dt);
    if (slowEffectTime > 0) {
        slowEffectTime -= dt;
    }
    
    // 只有单人模式和主机才运行下面的物理逻辑
    if (!isNetworked || isServer) {
        // 更新板
        // 左板（客户端的）：A/D 键，符合你的要求
        if (IsKeyDown(KEY_A)) {
            float speed = paddleSpeed;
            if (IsKeyDown(KEY_LEFT_SHIFT)) speed = paddleBoostSpeed;
            paddle1.MoveLeft(speed);
        }
        if (IsKeyDown(KEY_D)) {
            float speed = paddleSpeed;
            if (IsKeyDown(KEY_LEFT_SHIFT)) speed = paddleBoostSpeed;
            paddle1.MoveRight(speed);
        }
        // 右板（主机的）：左右方向键
        if (IsKeyDown(KEY_LEFT)) {
            float speed = paddleSpeed;
            if (IsKeyDown(KEY_RIGHT_SHIFT)) speed = paddleBoostSpeed;
            paddle2.MoveLeft(speed);
        }
        if (IsKeyDown(KEY_RIGHT)) {
            float speed = paddleSpeed;
            if (IsKeyDown(KEY_RIGHT_SHIFT)) speed = paddleBoostSpeed;
            paddle2.MoveRight(speed);
        }
        
        // 如果有网络，发送客户端的板位置
        if (isNetworked && !isServer) {
            SendPaddlePosition(paddle1.GetRect().x);
        }
        
        // 更新球
        bool allLaunched = true;
        for (auto& ball : balls) {
            if (!ball.IsLaunched()) {
                allLaunched = false;
                // 球跟着板走
                ball.ResetToPaddle(paddle1.GetRect().x + paddle1.GetRect().width/2, paddle1.GetRect().y);
                continue;
            }
            
            allLaunched = true;
            // 更新球的速度
            float speedFactor = 1.0f;
            if (slowEffectTime > 0) {
                speedFactor = slowSpeedFactor;
            }
            
            ball.Move(speedFactor);
            ball.ApplyGravity();
            
            // 球和边界碰撞
            ball.BounceEdge(screenWidth, screenHeight);
            
            // 球和板碰撞
            if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle1.GetRect())) {
                ball.BouncePaddle(paddle1.GetRect());
                ball.AddBounceForce(ballBounceForce);
            }
            if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddle2.GetRect())) {
                ball.BouncePaddle(paddle2.GetRect());
                ball.AddBounceForce(ballBounceForce);
            }
            
            // 球和砖块碰撞
            for (auto& brick : bricks) {
                if (brick.IsActive() && ball.CheckBrickCollision(brick.GetRect())) {
                    brick.SetActive(false);
                    winCount--;
                    // 加分
                    score += CalculateScore(scorePerBrick);
                    
                    // 掉落道具
                    if (GetRandomValue(0, 100) < powerup_config[(int)PowerUpType::PADDLE_EXTEND].drop_rate * 100) {
                        PowerUpType type = (PowerUpType)GetRandomValue(0, 2);
                        powerUps.emplace_back(brick.GetRect().x + brick.GetRect().width/2, brick.GetRect().y, type);
                    }
                    
                    // 粒子效果
                    for (int i = 0; i < 10; i++) {
                        for (int p = 0; p < MAX_PARTICLES; p++) {
                            if (!particleActive[p]) {
                                particles[p].pos.x = brick.GetRect().x + brick.GetRect().width/2;
                                particles[p].pos.y = brick.GetRect().y + brick.GetRect().height/2;
                                particles[p].vel.x = GetRandomValue(-200, 200) / 100.0f;
                                particles[p].vel.y = GetRandomValue(-200, 200) / 100.0f;
                                particles[p].life = 1.0f;
                                particles[p].color = brick.GetColor();
                                particleActive[p] = true;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            
            // 球掉出底部，生命减1
            if (ball.GetPosition().y > screenHeight) {
                // 直接标记为要移除，不需要SetActive
                ball.Reset(Vector2{0, screenHeight + 100}, Vector2{0,0});
            }
        }
        
        // 更新道具
        for (auto& powerup : powerUps) {
            if (powerup.position.y < screenHeight) {
                powerup.Update(dt);
                
                // 道具和板碰撞
                if (CheckCollisionCircleRec(powerup.position, 10, paddle1.GetRect())) {
                    // 应用道具效果
                    auto effect = CreatePowerUp(powerup.type, 
                        powerup_config[(int)powerup.type].extra_width,
                        powerup_config[(int)powerup.type].extra_balls,
                        powerup_config[(int)powerup.type].speed_factor,
                        powerup_config[(int)powerup.type].duration);
                    effect->Apply(*this);
                    powerup.position.y = screenHeight + 100; // 标记为失效
                }
                if (CheckCollisionCircleRec(powerup.position, 10, paddle2.GetRect())) {
                    // 应用道具效果
                    auto effect = CreatePowerUp(powerup.type, 
                        powerup_config[(int)powerup.type].extra_width,
                        powerup_config[(int)powerup.type].extra_balls,
                        powerup_config[(int)powerup.type].speed_factor,
                        powerup_config[(int)powerup.type].duration);
                    effect->Apply(*this);
                    powerup.position.y = screenHeight + 100; // 标记为失效
                }
            }
        }
        
        // 移除失效的道具
        powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(), 
            [this](PowerUp& p) { return p.position.y > screenHeight; }), powerUps.end());
        
        // 更新粒子
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (particleActive[i]) {
                particles[i].pos.x += particles[i].vel.x;
                particles[i].pos.y += particles[i].vel.y;
                particles[i].vel.y += 0.1f; // 重力
                particles[i].life -= dt;
                if (particles[i].life <= 0) {
                    particleActive[i] = false;
                }
            }
        }
        
        // 移除失效的球，通过位置判断
        balls.erase(std::remove_if(balls.begin(), balls.end(), 
            [this](Ball& b) { return b.GetPosition().y > screenHeight; }), balls.end());
        
        // 如果所有球都没了，生命减1
        if (balls.empty()) {
            lives--;
            if (lives <= 0) {
                currentState = GameState::GAMEOVER;
                if (leaderboard.CanEnter(score)) playerRank = leaderboard.AddScore("Player", score);
            } else {
                // 重生一个球
                Ball newBall({(float)screenWidth/2, (float)screenHeight - 100}, {0.0f, 0.0f}, ballRadius);
                balls.push_back(newBall);
            }
        }
        
        // 发射球
        if (IsKeyPressed(KEY_SPACE) && !allLaunched) {
            for (auto& ball : balls) {
                if (!ball.IsLaunched()) {
                    ball.Launch(paddle1.GetRect().x, paddle1.GetRect().width);
                }
            }
        }
        
        // 更新游戏时间
        gameTime += dt;
        
        // 胜利/关卡切换检测
        if (winCount <= 0) {
            if (currentLevel < TOTAL_LEVELS) {
                // 还有下一关，自动加载
                currentLevel++;
                LoadLevel(currentLevel);
                SaveSave(); // 自动保存进度
            } else {
                // 所有关卡通关，进入胜利
                currentState = GameState::VICTORY;
                if (leaderboard.CanEnter(score)) playerRank = leaderboard.AddScore("Player", score);
            }
        }
        
        // 网络模式下发送状态
        if (isNetworked && isServer) {
            SendGameState();
        }
    } else {
        // 客户端插值
        InterpolateState(dt / 0.05f); // 假设20fps的更新率
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(Color{30, 30, 40, 255});
    
    // 绘制边界
    DrawRectangle(0, 0, 5, screenHeight, GRAY);
    DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
    DrawRectangle(0, 0, screenWidth, 5, GRAY);
    
    // 根据状态绘制不同的内容
    switch(currentState) {
        case GameState::MENU:
            // 主菜单，只保留简单中文，没有英文
            DrawChineseTextCentered("打砖块游戏", screenHeight/2 - 100, 40, YELLOW);
            // 如果有存档，显示继续游戏选项
            if (saveExists) {
                DrawChineseTextCentered("按 C 继续上次游戏", screenHeight/2 - 70, 24, GREEN);
            }
            DrawChineseTextCentered("按 1 第一关", screenHeight/2 - 30, 20, WHITE);
            DrawChineseTextCentered("按 2 第二关", screenHeight/2, 20, WHITE);
            DrawChineseTextCentered("按 3 第三关", screenHeight/2 + 30, 20, WHITE);
            DrawChineseTextCentered("按 H 主机模式", screenHeight/2 + 60, 24, WHITE);
            DrawChineseTextCentered("按 J 客户端模式", screenHeight/2 + 90, 24, WHITE);
            DrawChineseTextCentered("按 L 排行榜", screenHeight/2 + 130, 20, Fade(WHITE, 0.6f));
            
            // 显示网络连接状态
            if (isNetworked) {
                if (isServer) {
                    DrawChineseTextCentered("等待客户端连接...", screenHeight/2 + 170, 20, GREEN);
                } else {
                    DrawChineseTextCentered("正在连接主机...", screenHeight/2 + 170, 20, BLUE);
                }
            }
            // 主菜单也显示帧率
            DrawFPS(screenWidth - 80, 30);
            break;
            
        case GameState::PLAYING:
        case GameState::PAUSED:
        {
            // 先绘制粒子，在最底层
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particleActive[i]) {
                    DrawCircleV({particles[i].pos.x, particles[i].pos.y}, 3, Fade(particles[i].color, particles[i].life));
                }
            }
            
            // 游戏中或者暂停，先绘制游戏元素
            for (auto& brick : bricks) brick.Draw();
            // 绘制两个板
            paddle1.Draw();
            paddle2.Draw();
            // 绘制所有的球
            for (auto& ball : balls) ball.Draw();
            // 绘制道具
            for (auto& powerup : powerUps) powerup.Draw();
            
            // 绘制UI，全部用中文，删掉所有原来的英文
            DrawChineseText("分数:", 20, 8, 24, WHITE);
            DrawText(TextFormat("%d", score), 80, 10, 24, YELLOW);
            DrawChineseText("生命:", 650, 8, 24, WHITE);
            DrawText(TextFormat("%d", lives), 710, 10, 24, lives > 1 ? GREEN : RED);
            // 显示当前关卡，放在最右边，不会和其他文字重叠
            DrawChineseText("关卡:", 350, 8, 24, WHITE);
            DrawText(TextFormat("%d/%d", currentLevel, TOTAL_LEVELS), 410, 10, 24, YELLOW);
            
            DrawChineseText("时间:", 20, 35, 20, Fade(WHITE, 0.8f));
            DrawText(TextFormat("%.1f秒", gameTime), 75, 37, 20, Fade(WHITE, 0.8f));
            float currentMultiplier = 5.0f - gameTime * timeMultiplierDecay;
            if (currentMultiplier < 1.0f) currentMultiplier = 1.0f;
            DrawText(TextFormat("x%.1f", currentMultiplier), 140, 37, 20, currentMultiplier > 2.0f ? GREEN : Fade(WHITE, 0.5f));
            
            // 网络模式提示
            if (isNetworked) {
                if (isServer) {
                    DrawChineseText("主机模式", 300, 8, 20, GREEN);
                } else {
                    DrawChineseText("客户端模式", 300, 8, 20, BLUE);
                }
            } else {
                DrawText("单人模式 - 左板A/D 右板←/→", 280, 8, 18, YELLOW);
            }
            
            // 显示当前激活的效果，全部用中文
            int effectY = 55;
            if (paddle1.GetRect().width > paddle1.GetOriginalWidth() ||
                paddle2.GetRect().width > paddle2.GetOriginalWidth()) {
                DrawChineseText("板加宽生效中!", 20, effectY, 16, ORANGE);
                effectY += 20;
            }
            if (slowEffectTime > 0) {
                DrawChineseText(TextFormat("减速生效中: %.1f秒", slowEffectTime), 20, effectY, 16, BLUE);
                effectY += 20;
            }
            if (balls.size() > 1) {
                DrawChineseText(TextFormat("多球: %d个", (int)balls.size()), 20, effectY, 16, GREEN);
            }
            
            bool hasUnlaunched = false;
            for (auto& ball : balls) {
                if (!ball.IsLaunched()) {
                    hasUnlaunched = true;
                    break;
                }
            }
            if (hasUnlaunched && (!isNetworked || isServer)) {
                DrawChineseTextCentered("按空格发射", 55, 20, YELLOW);
                // 删掉原来的英文PRESS SPACE，不会再重叠了
            }
            
            // 快捷键提示，放在关卡的右边，不会重叠，和按键完全匹配
            DrawChineseText("P-暂停 R-重开 L-排行榜", 450, 12, 18, Fade(WHITE, 0.6f));
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) DrawText(">>> BOOST <<<", 350, 575, 18, YELLOW);
            
            // 如果是暂停，覆盖暂停界面
            if (currentState == GameState::PAUSED) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
                DrawChineseTextCentered("暂停", screenHeight/2 - 40, 48, YELLOW);
                DrawChineseTextCentered("按 P 继续", screenHeight/2 + 30, 24, WHITE);
            }
            
            // 显示帧率，放在最右边，完全不重叠
            DrawFPS(screenWidth - 80, 30);
        }
        break;
        
        case GameState::GAMEOVER:
        case GameState::VICTORY:
            // 游戏结束或者胜利
            for (auto& brick : bricks) brick.Draw();
            paddle1.Draw();
            paddle2.Draw();
            for (auto& ball : balls) ball.Draw();
            
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.85f));
            
            if (currentState == GameState::VICTORY) {
                DrawChineseTextCentered("全部通关!", screenHeight/2 - 80, 48, GREEN);
                DrawText(TextFormat("FINAL SCORE: %d", score), screenWidth/2 - 100, screenHeight/2 - 30, 28, YELLOW);
            }
            else {
                DrawChineseTextCentered("游戏结束", screenHeight/2 - 80, 48, RED);
                DrawText(TextFormat("SCORE: %d", score), screenWidth/2 - 60, screenHeight/2 - 30, 28, YELLOW);
            }
            
            DrawChineseTextCentered("按 L 返回主菜单", screenHeight/2 + 100, 20, Fade(WHITE, 0.6f));
            // 游戏结束也显示帧率
            DrawFPS(screenWidth - 80, 30);
            break;
            
        case GameState::LEADERBOARD:
            // 排行榜界面，原有代码完全保留
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.85f));
            DrawChineseTextCentered("排行榜", screenHeight/2 - 200, 40, YELLOW);
            
            for (int i = 0; i < leaderboard.GetCount(); i++) {
                ScoreEntry entry;
                leaderboard.GetEntry(i+1, entry);
                char line[128];
                sprintf(line, "%d. %s - %d", i+1, entry.name, entry.score);
                DrawText(line, screenWidth/2 - 100, screenHeight/2 - 150 + i*30, 24, WHITE);
            }
            
            DrawChineseTextCentered("按 L 返回主菜单", screenHeight/2 + 100, 20, Fade(WHITE, 0.6f));
            // 排行榜也显示帧率
            DrawFPS(screenWidth - 80, 30);
            break;
    }
    
    EndDrawing();
}

void Game::Shutdown() {
    // 退出时自动保存当前游戏状态
    if (currentState == GameState::PLAYING || currentState == GameState::PAUSED) {
        SaveSave();
    }
}