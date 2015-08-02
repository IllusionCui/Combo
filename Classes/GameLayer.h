//
//  GameLayer.h
//  Combo
//
//  Created by IllusionCui on 15/8/1.
//
//

#ifndef __Combo__GameLayer__
#define __Combo__GameLayer__

#include "common.h"

class SingleScene;
class Cell;

class GameLayer : public Layer {
public:
    static GameLayer * create(SingleScene * scene);
    
    void startGame();
    
    bool onTouchBegan(Touch* touch, Event* event);
    void onTouchMove(Touch* touch, Event* event);
    void onTouchEnd(Touch* touch, Event* event);
protected:
    GameLayer();
    ~GameLayer();
    
    virtual bool init(SingleScene * scene);
    
    void initData();
    void resetData();
    
    void initBg();
    
    void initCells();
    void startCells();
    int getIndexOfCellsByPos(Vec2 pos);
    
    void initUI();
    void updateUI();
    
    void selectCell(Cell * cell);
    void resetSelect();
    
private:
    CC_SYNTHESIZE(GameDate *, m_data, GameDate);
    
    SingleScene * m_scene;
    float m_gridSize;
    Vec2 m_gridStartPos;
    
    Vector<Cell *> m_cells;
    Vector<Cell *> m_currSelectCells;
    Vector<DrawNode *> m_currSelectLines;
    
    Label * m_scoreLabel;
    Label * m_comboLabel;
};

#endif /* defined(__Combo__GameLayer__) */
