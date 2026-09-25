//------------------------------------------------------------------------------
//
//      osm-import: GPX треки
//
//------------------------------------------------------------------------------

#include "gpx-reader.h"

#include <QDomDocument>
#include <QFile>

namespace osm
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool parse_gpx(const std::string& path, std::vector<GpxTrack>& out,
               std::string* error)
{
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = "Can't open GPX file: " + path;
        }
        return false;
    }

    QDomDocument doc;
    QString parse_error = "";
    int line = 0;
    int column = 0;

    if (!doc.setContent(&file, &parse_error, &line, &column))
    {
        file.close();
        if (error != nullptr)
        {
            *error = "GPX parse error at " + std::to_string(line) + ":" +
                     std::to_string(column);
        }
        return false;
    }

    file.close();

    const QDomElement root = doc.documentElement();

    if (root.isNull() || root.tagName() != "gpx")
    {
        if (error != nullptr)
        {
            *error = "Root element is not <gpx>";
        }
        return false;
    }

    auto read_segment = [](const QDomElement& seg, GpxTrack& track)
    {
        QDomElement pt = seg.firstChildElement();

        while (!pt.isNull())
        {
            if (pt.tagName() == "trkpt" || pt.tagName() == "rtept")
            {
                GpxPoint p;
                p.lat = pt.attribute("lat").toDouble();
                p.lon = pt.attribute("lon").toDouble();

                const QDomElement ele = pt.firstChildElement("ele");
                if (!ele.isNull())
                {
                    p.ele = ele.text().toDouble();
                    p.has_ele = true;
                }

                track.points.push_back(p);
            }

            pt = pt.nextSiblingElement();
        }
    };

    // <trk><name/><trkseg><trkpt .../></trkseg></trk>
    QDomElement trk = root.firstChildElement("trk");

    while (!trk.isNull())
    {
        GpxTrack track;

        const QDomElement name = trk.firstChildElement("name");
        if (!name.isNull())
        {
            track.name = name.text().toStdString();
        }

        QDomElement seg = trk.firstChildElement("trkseg");

        while (!seg.isNull())
        {
            read_segment(seg, track);
            seg = seg.nextSiblingElement("trkseg");
        }

        if (!track.points.empty())
        {
            out.push_back(std::move(track));
        }

        trk = trk.nextSiblingElement("trk");
    }

    // <rte><rtept .../></rte>
    QDomElement rte = root.firstChildElement("rte");

    while (!rte.isNull())
    {
        GpxTrack track;

        const QDomElement name = rte.firstChildElement("name");
        if (!name.isNull())
        {
            track.name = name.text().toStdString();
        }

        read_segment(rte, track);

        if (!track.points.empty())
        {
            out.push_back(std::move(track));
        }

        rte = rte.nextSiblingElement("rte");
    }

    return !out.empty();
}

} // namespace osm
