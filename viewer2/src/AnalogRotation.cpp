#include "AnalogRotation.h"

#include "CfgReader.h"
#include "ProcAnimation.h"

#include <iostream>
#include <vsg/maths/common.h>
#include <vsg/maths/mat4.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <algorithm>
#include <sstream>
#include <string>

AnalogRotation::AnalogRotation(vsg::MatrixTransform* transform)
    : ProcAnimation(transform)
    , matrix(transform->matrix)
    , translation(transform->matrix[3][0], transform->matrix[3][1], transform->matrix[3][2])
{
    std::cout << "Complex matrix = {\n";
    for (int i = 0; i < 4; ++i)
    {
        std::cout << "    ";
        for (int j = 0; j < 4; ++j)
        {
            std::cout << matrix[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "}\n";

    std::cout << "Translation: " << translation[0] << ' ' << translation[1] << ' ' << translation[2] << '\n';
    std::cout << "Rotation matrix = {\n";
    for (int i = 0; i < 4; ++i)
    {
        std::cout << "    ";
        for (int j = 0; j < 4; ++j)
        {
            std::cout << rotation[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "}\n";
    std::cout << "Scale matrix = {\n";
    for (int i = 0; i < 4; ++i)
    {
        std::cout << "    ";
        for (int j = 0; j < 4; ++j)
        {
            std::cout << scale[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "}\n\n";
}

void AnalogRotation::anim_step(float t, float dt)
{
    float delta = (pos - cur_pos);
    if (abs(delta) > 1e-5f)
    {
        cur_pos += delta * duration * dt;
        update();
    }
}

bool AnalogRotation::load_config(CfgReader& cfg)
{
    QString sec_name = "AnalogRotation";

    int tmp_int = 0;
    if (cfg.getInt(sec_name, "SignalID", tmp_int))
        signal_id = tmp_int;

    double tmp_dbl = 1.0;
    if (cfg.getDouble(sec_name, "Duration", tmp_dbl))
        duration = tmp_dbl;

    cfg.getBool(sec_name, "FixedSignal", is_fixed_signal);

    cfg.getBool(sec_name, "Infinity", infinity);

    QString tmp_qstr = "0.0 0.0 1.0";
    if (cfg.getString(sec_name, "Axis", tmp_qstr))
    {
        std::string tmp = tmp_qstr.toStdString();
        std::istringstream ss(tmp);
        ss >> axis.x >> axis.y >> axis.z;
    }

    update();
    return true;
}

void AnalogRotation::update()
{
    if (keypoints.empty())
    {
        return;
    }

    angle = interpolate(cur_pos);

    if (!infinity)
    {
        angle = std::clamp(angle, keypoints.front().value, keypoints.back().value);
    }

    vsg::dmat4 rotate = vsg::rotate(static_cast<double>(vsg::radians(angle)), axis);
    transform->matrix = matrix * rotate;
    // transform->matrix = vsg::translate(translation) * vsg::dmat4(1.0) * rotate;
    // transform->matrix = vsg::translate(translation) * rotate * vsg::dmat4(1.0);
    // transform->matrix = rotate * vsg::transpose(matrix);
}
