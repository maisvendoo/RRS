#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

class EditorState
{
public:
    virtual ~EditorState();

    virtual void fill_status_bar() const;
};

#endif // EDITOR_STATE_H
