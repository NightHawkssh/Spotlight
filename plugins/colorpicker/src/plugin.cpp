// Copyright (c) 2026 Spotlight Contributors

#include "plugin.h"
#include <QColor>
#include <QRegularExpression>
#include <albert/icon.h>
#include <albert/standardrichitem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

QString Plugin::defaultTrigger() const { return u"color "_s; }

QString Plugin::synopsis(const QString &) const { return u"<hex color>"_s; }

vector<RankItem> Plugin::rankItems(QueryContext &ctx)
{
    vector<RankItem> results;

    auto query = ctx.query().trimmed();
    if (query.isEmpty())
        return results;

    // Accept with or without # prefix
    QString hex = query;
    if (!hex.startsWith(u'#'))
        hex.prepend(u'#');

    // Validate hex color
    static QRegularExpression re(uR"(^#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})$)"_s);
    if (!re.match(hex).hasMatch())
        return results;

    QColor color(hex);
    if (!color.isValid())
        return results;

    int r = color.red();
    int g = color.green();
    int b = color.blue();

    // Normalize to 6-digit hex
    auto hexStr = u"#%1%2%3"_s
        .arg(r, 2, 16, u'0')
        .arg(g, 2, 16, u'0')
        .arg(b, 2, 16, u'0');
    hexStr = hexStr.toUpper();

    auto rgbStr = u"rgb(%1, %2, %3)"_s.arg(r).arg(g).arg(b);

    float h, s, l;
    color.getHslF(&h, &s, &l);
    int hDeg = qRound(static_cast<double>(h) * 360.0);
    int sPct = qRound(static_cast<double>(s) * 100.0);
    int lPct = qRound(static_cast<double>(l) * 100.0);
    auto hslStr = u"hsl(%1, %2%, %3%)"_s.arg(hDeg).arg(sPct).arg(lPct);

    // Build detail markdown
    auto detail = u"## Color: %1\n\n"
                  u"| Format | Value |\n"
                  u"|--------|-------|\n"
                  u"| **HEX** | `%1` |\n"
                  u"| **RGB** | `%2` |\n"
                  u"| **HSL** | `%3` |\n"
                  u"\n"
                  u"**Red:** %4 &nbsp; **Green:** %5 &nbsp; **Blue:** %6\n"_s
                      .arg(hexStr, rgbStr, hslStr)
                      .arg(r).arg(g).arg(b);

    auto item = StandardRichItem::make(
        u"color"_s,
        hexStr,
        u"%1 | %2"_s.arg(rgbStr, hslStr),
        [hexStr] { return Icon::grapheme(u"\U0001f3a8"_s); },
        vector<Action>{
            {
                u"hex"_s, tr("Copy HEX"),
                [hexStr] { setClipboardText(hexStr); }
            },
            {
                u"rgb"_s, tr("Copy RGB"),
                [rgbStr] { setClipboardText(rgbStr); }
            },
            {
                u"hsl"_s, tr("Copy HSL"),
                [hslStr] { setClipboardText(hslStr); }
            }
        }
    );

    item->setDetailMarkdown(detail);
    item->setMetadata({
        {u"HEX"_s, hexStr, {}},
        {u"RGB"_s, rgbStr, {}},
        {u"HSL"_s, hslStr, {}}
    });

    results.emplace_back(move(item), 1.0);
    return results;
}
