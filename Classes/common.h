//
//  common.h
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#ifndef IGGameC_common_h
#define IGGameC_common_h

#include <Util.h>

const string defaultTTF = "fonts/default.ttf";

enum class GameType {
    autoRecover,   // 自动恢复
    recoverByLine   // 旁消
};

typedef struct Config {
    // 数据先关
    int rows;
    int cols;
    GameType type;
    vector<int> initWeights;
    
    
    // 布局相关
    float gridLineWidthPercent;
    float cheerWidthPercent;
    float topHeightPercent;
    float buttomHeightPercent;
} Config;

typedef struct GameData {
    int score;      // 分数
    bool newHighScore;
    int combo;      // 连线数
    bool newHighCombo;
} GameDate;

#endif
