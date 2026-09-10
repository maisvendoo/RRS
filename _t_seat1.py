d = open(r"viewer\src\VehicleExterior.cpp", encoding="utf-8").read()
a = "        exit_dir.push_back(vsg::radians(ed));"
assert a in d, "exit_dir line"
b = a + """

        // Сиденье помощника (посадка по E, ТЗ ходьба): без <AssistantPos> -
        // типовое расположение напротив машиниста (левее и чуть назад)
        vsg::dvec3 asist = dp + vsg::dvec3(-1.7, -0.6, 0.0);
        QString AssistPos = "";
        if (cfg.getString(secNode, "AssistantPos", AssistPos))
        {
            std::istringstream ss(AssistPos.toStdString());
            ss >> asist.x >> asist.y >> asist.z;
        }
        assistant_pos.push_back(asist);"""
d = d.replace(a, b, 1)
open(r"viewer\src\VehicleExterior.cpp", "w", encoding="utf-8").write(d)

h = open(r"viewer\include\VehicleExterior.h", encoding="utf-8").read()
ha = ("    std::vector<vsg::dvec3>  exit_pos = {};\n"
      "    std::vector<double>  exit_dir = {};")
assert ha in h, "header block"
hb = ha + """

    /// Сиденье помощника в кабинах (посадка по E, ТЗ ходьба)
    std::vector<vsg::dvec3>  assistant_pos = {};"""
h = h.replace(ha, hb, 1)
oldc = "    exit_pos.clear();\n    exit_dir.clear();"
assert oldc in h
h = h.replace(oldc, oldc + "\n    assistant_pos.clear();")
open(r"viewer\include\VehicleExterior.h", "w", encoding="utf-8").write(h)
print("exterior ok")
