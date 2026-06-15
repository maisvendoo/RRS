#ifndef BASIC_EDITOR_STATE_H
#define BASIC_EDITOR_STATE_H

#include "editor/states/EditorState.h"

#include <string>

class BasicEditorState : public EditorState
{
public:
    explicit BasicEditorState(const std::string& route_dir);

    virtual ~BasicEditorState() override;

    virtual void fill_status_bar() const override;

private:
    const std::string& route_dir;
};

#endif // BASIC_EDITOR_STATE_H
