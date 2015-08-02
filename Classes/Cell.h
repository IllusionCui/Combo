//
//  Cell.h
//  IGGameC
//
//  Created by IllusionCui on 15/7/30.
//
//

#ifndef __IGGameC__Cell__
#define __IGGameC__Cell__

#include "common.h"

enum class CellStatus {
    normal,
    selected
};

class Cell : public Node {
public:
    static Cell * create(int row, int col);
    
    virtual void setContentSize(const Size& contentSize);
    
    void setValue(int value);
    
    bool isSelectable();
    bool canBeMoveTo();
protected:
    Cell();
    ~Cell();
    
    virtual bool init(int row, int col);
    
    void updateShow();
private:
    CC_SYNTHESIZE_READONLY(int, m_row, Row);
    CC_SYNTHESIZE_READONLY(int, m_col, Col);
    CC_SYNTHESIZE(CellStatus, m_status, Status);
    CC_SYNTHESIZE_READONLY(int, m_value, Value);
    Label * m_label;
};

#endif /* defined(__IGGameC__Cell__) */
