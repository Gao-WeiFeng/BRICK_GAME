#ifndef GAME_H
#define GAME_H
// ==============================================
// 第一步：在包含任何头文件之前，先处理Windows.h冲突
// ==============================================
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define NOGDI
#define NOUSER
#define NOKERNEL
#define NOSERVICE
#define NOMCX
#define NOIME
// 先包含Windows.h，然后立即取消所有冲突的宏
#include <windows.h>
// 关键：取消所有和Raylib冲突的宏！
#undef DrawText
#undef CreateWindow
#undef ShowWindow
#undef MessageBox
#undef GetMessage
#undef SendMessage
#undef PostMessage
#undef DefWindowProc
#undef RegisterClass
#undef LoadIcon
#undef LoadCursor
#undef CreateWindowEx
#undef GetWindowRect
#undef MoveWindow
#undef SetWindowText
#undef GetWindowText
#undef GetClientRect
#undef InvalidateRect
#undef UpdateWindow
#undef ShowCursor
#undef SetCursor
#undef GetCursorPos
#undef SetCursorPos
#undef GetKeyState
#undef GetAsyncKeyState
#undef GetKeyboardState
#undef GetMouseMovePointsEx
#undef GetSystemMetrics
#undef SystemParametersInfo
#undef GetDC
#undef ReleaseDC
#undef CreateCompatibleDC
#undef CreateCompatibleBitmap
#undef SelectObject
#undef DeleteObject
#undef BitBlt
#undef StretchBlt
#undef SetPixel
#undef GetPixel
#undef LineTo
#undef MoveToEx
#undef Rectangle
#undef Ellipse
#undef Polygon
#undef Polyline
#undef CreatePen
#undef CreateSolidBrush
#undef CreateHatchBrush
#undef SetBkColor
#undef SetTextColor
#undef SetBkMode
#undef DrawTextEx
#undef TextOut
#undef GetTextExtentPoint32
#undef CreateFont
#undef DeleteFont
#undef GetTextMetrics
#undef SetViewportOrgEx
#undef SetWindowOrgEx
#undef SetMapMode
#undef SetWorldTransform
#undef ModifyWorldTransform
#undef GetWorldTransform
#undef SaveDC
#undef RestoreDC
#undef BeginPaint
#undef EndPaint
#undef GetUpdateRect
#undef ValidateRect
#undef InvalidateRgn
#undef ValidateRgn
#undef CreateRgn
#undef CombineRgn
#undef FillRgn
#undef FrameRgn
#undef InvertRgn
#undef PaintRgn
#undef SelectClipRgn
#undef GetClipBox
#undef ExtSelectClipRgn
#undef GetRegionData
#undef CreateRectRgn
#undef CreateEllipticRgn
#undef CreatePolygonRgn
#undef CreatePolyPolygonRgn
#undef OffsetRgn
#undef GetRgnBox
#undef EqualRgn
#undef PtInRegion
#undef RectInRegion
#undef CombineRgn
#undef SetWindowRgn
#undef GetWindowRgn
#undef UpdateLayeredWindow
#undef SetLayeredWindowAttributes
#undef GetLayeredWindowAttributes
#undef AnimateWindow
#undef SetWindowPos
#undef BringWindowToTop
#undef SetForegroundWindow
#undef GetForegroundWindow
#undef SetActiveWindow
#undef GetFocus
#undef EnableWindow
#undef IsWindowEnabled
#undef IsWindowVisible
#undef ShowWindowAsync
#undef IsIconic
#undef IsZoomed
#undef CloseWindow
#undef OpenIcon
#undef DestroyWindow
#undef SetParent
#undef GetParent
#undef SetWindowLongPtr
#undef GetWindowLongPtr
#undef SetClassLongPtr
#undef GetClassLongPtr
#undef SetWindowLong
#undef GetWindowLong
#undef SetClassLong
#undef GetClassLong
#undef CallWindowProc
#undef SetWindowSubclass
#undef RemoveWindowSubclass
#undef DefSubclassProc
#undef GetSubclassWindowProc
#undef GetSubclassRefData
#undef SetSubclassRefData
#undef CreateDialogParam
#undef DialogBoxParam
#undef EndDialog
#undef GetDlgItem
#undef SetDlgItemText
#undef GetDlgItemText
#undef SetDlgItemInt
#undef GetDlgItemInt
#undef CheckDlgButton
#undef IsDlgButtonChecked
#undef SendDlgItemMessage
#undef CreateWindowEx
#undef RegisterClassEx
#undef UnregisterClass
#undef GetClassInfoEx
#undef GetModuleHandle
#undef LoadLibrary
#undef FreeLibrary
#undef GetProcAddress
#undef GetLastError
#undef SetLastError
#undef FormatMessage
#undef LocalAlloc
#undef LocalFree
#undef GlobalAlloc
#undef GlobalFree
#undef HeapCreate
#undef HeapDestroy
#undef HeapAlloc
#undef HeapFree
#undef HeapReAlloc
#undef HeapSize
#undef GetProcessHeap
#undef CreateThread
#undef CloseHandle
#undef WaitForSingleObject
#undef WaitForMultipleObjects
#undef SetEvent
#undef ResetEvent
#undef CreateEvent
#undef CreateMutex
#undef ReleaseMutex
#undef CreateSemaphore
#undef ReleaseSemaphore
#undef InitializeCriticalSection
#undef DeleteCriticalSection
#undef EnterCriticalSection
#undef LeaveCriticalSection
#undef TryEnterCriticalSection
#undef Sleep
#undef GetTickCount
#undef GetTickCount64
#undef QueryPerformanceCounter
#undef QueryPerformanceFrequency
#undef GetSystemTime
#undef GetLocalTime
#undef GetSystemTimeAsFileTime
#undef FileTimeToSystemTime
#undef SystemTimeToFileTime
#undef FileTimeToLocalFileTime
#undef CompareFileTime
#undef GetDateFormat
#undef GetTimeFormat
#undef GetNumberFormat
#undef GetCurrencyFormat
#undef LCMapString
#undef MultiByteToWideChar
#undef WideCharToMultiByte
#undef GetACP
#undef GetOEMCP
#undef GetConsoleCP
#undef GetConsoleOutputCP
#undef SetConsoleCP
#undef SetConsoleOutputCP
#undef AllocConsole
#undef FreeConsole
#undef AttachConsole
#undef GetStdHandle
#undef SetStdHandle
#undef WriteConsole
#undef ReadConsole
#undef WriteFile
#undef ReadFile
#undef CreateFile
#undef DeleteFile
#undef MoveFile
#undef CopyFile
#undef CreateDirectory
#undef RemoveDirectory
#undef GetFileAttributes
#undef SetFileAttributes
#undef GetTempPath
#undef GetTempFileName
#undef GetFullPathName
#undef PathFileExists
#undef PathIsDirectory
#undef PathIsFile
#undef PathCombine
#undef PathRemoveFileSpec
#undef PathStripPath
#undef PathFindExtension
#undef PathFindFileName
#undef PathRemoveExtension
#undef PathAppend
#undef PathCanonicalize
#undef PathRelativePathTo
#undef PathMakePretty
#undef PathQuoteSpaces
#undef PathUnquoteSpaces
#undef PathCompactPath
#undef PathCompactPathEx
#undef PathAddBackslash
#undef PathRemoveBackslash
#undef PathIsRelative
#undef PathIsUNC
#undef PathIsRoot
#undef PathSkipRoot
#undef PathStripToRoot
#undef PathGetDriveNumber
#undef PathBuildRoot
#undef PathIsSameRoot
#undef PathCommonPrefix
#undef PathMatchSpec
#undef PathMatchSpecEx
#undef PathIsContentType
#undef PathIsHTMLFile
#undef PathIsURL
#undef PathCreateFromURL
#undef PathURLCreateFromPath
#undef PathGetArgs
#undef PathRemoveArgs
#undef PathGetCommandLine
#undef PathSetCommandLine
// ==============================================
// 第二步：现在再包含Raylib和其他头文件
// ==============================================
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "PowerUp.h"
#include "json.hpp"
// 定义json类型，让头文件里也能识别
using json = nlohmann::json;
#include <vector>
#include <string>
#include <map>
#include <enet/enet.h>
// 多线程相关头文件
#include <thread>
#include <future>
#include <mutex>
#include <chrono>
// 游戏状态枚举，代替原来的一堆bool变量
enum class GameState {
    MENU,       // 主菜单
    PLAYING,    // 游戏中
    PAUSED,     // 暂停
    GAMEOVER,   // 游戏结束（失败）
    VICTORY,    // 胜利
    LEADERBOARD // 排行榜
};
// 网络消息类型
enum class NetworkMessageType {
    GAME_STATE,     // 主机→客户端：游戏状态更新
    PADDLE_POSITION // 客户端→主机：板位置更新
};
// 网络同步用的小球状态
struct NetworkBallState {
    float x, y;
    float vx, vy;
    bool launched;
};
// 网络同步用的道具状态
struct NetworkPowerUpState {
    float x, y;
    int type;
    bool active;
};
// 游戏状态序列化结构体（强制1字节对齐，确保跨平台一致）
#pragma pack(1)
struct NetworkGameState {
    float ballX, ballY;
    float ballSpeedX, ballSpeedY;
    float paddle1X; // 主机控制的板
    float paddle2X; // 客户端控制的板
    int score;
    int lives;
    int winCount;
    float gameTime;
    uint64_t brickMask; // 砖块状态bitmask，40个bit对应40个砖块的激活状态
    
    // 同步多球状态，最多支持3个球
    NetworkBallState balls[3];
    // 同步道具状态，最多支持3个道具
    NetworkPowerUpState powerups[3];
};
// 板位置消息
struct NetworkPaddlePosition {
    float x;
};
#pragma pack()
// 排行榜条目结构
struct ScoreEntry {
    char name[32];
    int score;
    time_t timestamp;
};
// 排行榜类
class Leaderboard {
private:
    static const int MAX_ENTRIES = 10;
    ScoreEntry entries[MAX_ENTRIES];
    int count;
    const char* filename;
public:
    Leaderboard(const char* file);
    void Load();
    void Save();
    int AddScore(const char* name, int score);
    bool GetEntry(int rank, ScoreEntry& entry);
    int GetCount();
    bool CanEnter(int score);
};
class Game {
private:
    // 游戏核心对象
    std::vector<Ball> balls;
    Paddle paddle1; // 主机控制的板
    Paddle paddle2; // 客户端控制的板
    std::vector<Brick> bricks;
    Leaderboard leaderboard;
    // 新加的：道具和粒子
    std::vector<PowerUp> powerUps;
    // 粒子对象池，预分配所有粒子，避免频繁内存分配
    static const int MAX_PARTICLES = 1000;
    Particle particles[MAX_PARTICLES];
    bool particleActive[MAX_PARTICLES];
    // 减速效果的剩余时间
    float slowEffectTime;
    float slowSpeedFactor;
    // 游戏状态
    GameState currentState;
    int score;
    int lives;
    int winCount;
    int playerRank;
    float gameTime;
    // 字体相关
    Font chineseFont;
    bool fontLoaded;
    // 配置参数
    int screenWidth;
    int screenHeight;
    std::string windowTitle;
    // 球的配置
    float ballRadius;
    float ballGravity;
    float ballMaxSpeed;
    float ballBounceForce;
    // 板的配置
    float paddleWidth;
    float paddleHeight;
    float paddleSpeed;
    float paddleBoostSpeed;
    // 砖块的配置
    int brickRows;
    int brickCols;
    float brickWidth;
    float brickHeight;
    // 游戏的配置
    int initialLives;
    int scorePerBrick;
    float timeMultiplierDecay;
    // 道具的配置，从JSON读取
    struct PowerUpConfig {
        float extra_width;
        float extra_balls;
        float speed_factor;
        float duration;
        float drop_rate;
    };
    PowerUpConfig powerup_config[3];
    // 网络相关
    ENetHost* netHost;
    ENetPeer* netPeer;
    bool isNetworked;  // 是否为网络模式
    bool isServer;     // 是否为主机
    // 状态插值（解决球瞬移问题）
    struct Snapshot {
        NetworkGameState state;
        double timestamp;
    };
    Snapshot lastSnapshot;
    Snapshot currentSnapshot;
    double lastSnapshotTime;
    // 异步加载相关成员
    enum class LoadState { IDLE, LOADING, DONE };
    LoadState loadState;
    std::future<void> loadFuture;
    std::mutex loadMutex;
    bool bricksColorChanged;
    // 关卡与存档系统
    int currentLevel;
    static const int TOTAL_LEVELS;
    bool saveExists; // 是否存在存档文件
    
    // 私有辅助方法
    void InitChineseFont();
    void DrawChineseText(const char* text, int x, int y, int fontSize, Color color);
    void DrawChineseTextCentered(const char* text, int y, int fontSize, Color color);
    int CalculateScore(int baseScore);
    void ResetGame();
    // 关卡与存档辅助方法
    void LoadLevel(int level);
    void SaveSave();
    bool LoadSave();
    json LoadJSONWithFallback(const std::string& path);
public:
    // 构造函数
    Game();
    // 游戏的核心流程方法
    void Init();
    void Update();
    void Draw();
    void Shutdown();
    // 加载配置文件
    void LoadConfig(const std::string& path);
    // 给效果类用的获取器
    Paddle& GetPaddle1() { return paddle1; }
    Paddle& GetPaddle2() { return paddle2; }
    std::vector<Ball>& GetBalls() { return balls; }
    void ActivateSlowEffect(float duration) { slowEffectTime = duration; }
    float GetSlowFactor() { return slowSpeedFactor; }
    int GetScreenWidth() { return screenWidth; }
    int GetScreenHeight() { return screenHeight; }
    // 网络初始化与关闭
    bool InitNetwork(bool isServer, const char* serverIp = "127.0.0.1");
    void ShutdownNetwork();
    // 网络事件处理
    void ProcessNetworkEvents();
    void SendGameState();
    void SendPaddlePosition(float x);
    // 状态插值平滑
    void InterpolateState(float alpha);
};
#endif