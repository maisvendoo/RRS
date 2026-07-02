#ifndef EDITOR_BOX_SELECTION_STATE_H
#define EDITOR_BOX_SELECTION_STATE_H

#include "editor/states/EditorState.h"

class BoxSelectionState : public EditorState
{
public:
    virtual ~BoxSelectionState() override;

    virtual void fill_status_bar() const override;
};

#endif // EDITOR_BOX_SELECTION_STATE_H
