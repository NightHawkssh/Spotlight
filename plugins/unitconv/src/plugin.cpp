// Copyright (c) 2026 Spotlight Contributors

#include "plugin.h"
#include <QRegularExpression>
#include <albert/icon.h>
#include <albert/standardrichitem.h>
#include <albert/systemutil.h>
#include <cmath>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

namespace
{

struct Conversion
{
    QString fromUnit;
    QString toUnit;
    QString category;
    QString formula;
    // function: input value -> output value
    function<double(double)> convert;
};

static const vector<Conversion> conversions = {
    // Length
    {u"km"_s, u"miles"_s, u"Length"_s, u"km * 0.621371"_s, [](double v){ return v * 0.621371; }},
    {u"miles"_s, u"km"_s, u"Length"_s, u"miles * 1.60934"_s, [](double v){ return v * 1.60934; }},
    {u"m"_s, u"ft"_s, u"Length"_s, u"m * 3.28084"_s, [](double v){ return v * 3.28084; }},
    {u"ft"_s, u"m"_s, u"Length"_s, u"ft * 0.3048"_s, [](double v){ return v * 0.3048; }},
    {u"cm"_s, u"in"_s, u"Length"_s, u"cm * 0.393701"_s, [](double v){ return v * 0.393701; }},
    {u"in"_s, u"cm"_s, u"Length"_s, u"in * 2.54"_s, [](double v){ return v * 2.54; }},
    {u"m"_s, u"yd"_s, u"Length"_s, u"m * 1.09361"_s, [](double v){ return v * 1.09361; }},
    {u"yd"_s, u"m"_s, u"Length"_s, u"yd * 0.9144"_s, [](double v){ return v * 0.9144; }},
    {u"mm"_s, u"in"_s, u"Length"_s, u"mm * 0.0393701"_s, [](double v){ return v * 0.0393701; }},
    {u"in"_s, u"mm"_s, u"Length"_s, u"in * 25.4"_s, [](double v){ return v * 25.4; }},

    // Weight
    {u"kg"_s, u"lbs"_s, u"Weight"_s, u"kg * 2.20462"_s, [](double v){ return v * 2.20462; }},
    {u"lbs"_s, u"kg"_s, u"Weight"_s, u"lbs * 0.453592"_s, [](double v){ return v * 0.453592; }},
    {u"g"_s, u"oz"_s, u"Weight"_s, u"g * 0.035274"_s, [](double v){ return v * 0.035274; }},
    {u"oz"_s, u"g"_s, u"Weight"_s, u"oz * 28.3495"_s, [](double v){ return v * 28.3495; }},
    {u"kg"_s, u"st"_s, u"Weight"_s, u"kg * 0.157473"_s, [](double v){ return v * 0.157473; }},
    {u"st"_s, u"kg"_s, u"Weight"_s, u"st * 6.35029"_s, [](double v){ return v * 6.35029; }},

    // Temperature
    {u"f"_s, u"c"_s, u"Temperature"_s, u"(F - 32) * 5/9"_s, [](double v){ return (v - 32.0) * 5.0 / 9.0; }},
    {u"c"_s, u"f"_s, u"Temperature"_s, u"C * 9/5 + 32"_s, [](double v){ return v * 9.0 / 5.0 + 32.0; }},
    {u"c"_s, u"k"_s, u"Temperature"_s, u"C + 273.15"_s, [](double v){ return v + 273.15; }},
    {u"k"_s, u"c"_s, u"Temperature"_s, u"K - 273.15"_s, [](double v){ return v - 273.15; }},
    {u"f"_s, u"k"_s, u"Temperature"_s, u"(F - 32) * 5/9 + 273.15"_s, [](double v){ return (v - 32.0) * 5.0 / 9.0 + 273.15; }},
    {u"k"_s, u"f"_s, u"Temperature"_s, u"(K - 273.15) * 9/5 + 32"_s, [](double v){ return (v - 273.15) * 9.0 / 5.0 + 32.0; }},

    // Data
    {u"bytes"_s, u"kb"_s, u"Data"_s, u"bytes / 1024"_s, [](double v){ return v / 1024.0; }},
    {u"kb"_s, u"bytes"_s, u"Data"_s, u"KB * 1024"_s, [](double v){ return v * 1024.0; }},
    {u"kb"_s, u"mb"_s, u"Data"_s, u"KB / 1024"_s, [](double v){ return v / 1024.0; }},
    {u"mb"_s, u"kb"_s, u"Data"_s, u"MB * 1024"_s, [](double v){ return v * 1024.0; }},
    {u"mb"_s, u"gb"_s, u"Data"_s, u"MB / 1024"_s, [](double v){ return v / 1024.0; }},
    {u"gb"_s, u"mb"_s, u"Data"_s, u"GB * 1024"_s, [](double v){ return v * 1024.0; }},
    {u"gb"_s, u"tb"_s, u"Data"_s, u"GB / 1024"_s, [](double v){ return v / 1024.0; }},
    {u"tb"_s, u"gb"_s, u"Data"_s, u"TB * 1024"_s, [](double v){ return v * 1024.0; }},
    {u"bytes"_s, u"mb"_s, u"Data"_s, u"bytes / 1048576"_s, [](double v){ return v / 1048576.0; }},
    {u"mb"_s, u"bytes"_s, u"Data"_s, u"MB * 1048576"_s, [](double v){ return v * 1048576.0; }},

    // Time
    {u"sec"_s, u"min"_s, u"Time"_s, u"sec / 60"_s, [](double v){ return v / 60.0; }},
    {u"min"_s, u"sec"_s, u"Time"_s, u"min * 60"_s, [](double v){ return v * 60.0; }},
    {u"min"_s, u"hr"_s, u"Time"_s, u"min / 60"_s, [](double v){ return v / 60.0; }},
    {u"hr"_s, u"min"_s, u"Time"_s, u"hr * 60"_s, [](double v){ return v * 60.0; }},
    {u"hr"_s, u"days"_s, u"Time"_s, u"hr / 24"_s, [](double v){ return v / 24.0; }},
    {u"days"_s, u"hr"_s, u"Time"_s, u"days * 24"_s, [](double v){ return v * 24.0; }},
    {u"days"_s, u"weeks"_s, u"Time"_s, u"days / 7"_s, [](double v){ return v / 7.0; }},
    {u"weeks"_s, u"days"_s, u"Time"_s, u"weeks * 7"_s, [](double v){ return v * 7.0; }},
};

}  // namespace

QString Plugin::defaultTrigger() const { return u"conv "_s; }

QString Plugin::synopsis(const QString &) const { return u"<value> <from> to <to>"_s; }

vector<RankItem> Plugin::rankItems(QueryContext &ctx)
{
    vector<RankItem> results;

    auto query = ctx.query().trimmed();
    if (query.isEmpty())
        return results;

    // Pattern: <number> <unit> to <unit>
    static QRegularExpression re(
        uR"(^(-?[\d.]+)\s*(\w+)\s+to\s+(\w+)$)"_s,
        QRegularExpression::CaseInsensitiveOption);

    auto match = re.match(query);
    if (!match.hasMatch())
        return results;

    bool ok = false;
    double value = match.captured(1).toDouble(&ok);
    if (!ok)
        return results;

    auto fromUnit = match.captured(2).toLower();
    auto toUnit = match.captured(3).toLower();

    for (const auto &conv : conversions)
    {
        if (conv.fromUnit.compare(fromUnit, Qt::CaseInsensitive) == 0
            && conv.toUnit.compare(toUnit, Qt::CaseInsensitive) == 0)
        {
            double result = conv.convert(value);

            // Format nicely: avoid unnecessary decimals
            auto resultStr = (result == floor(result) && abs(result) < 1e12)
                ? QString::number(static_cast<long long>(result))
                : QString::number(result, 'g', 10);

            auto valueStr = (value == floor(value) && abs(value) < 1e12)
                ? QString::number(static_cast<long long>(value))
                : QString::number(value, 'g', 10);

            auto text = u"%1 %2 = %3 %4"_s
                .arg(valueStr, conv.fromUnit, resultStr, conv.toUnit);

            auto detail = u"## Unit Conversion\n\n"
                          u"**Input:** %1 %2\n\n"
                          u"**Result:** %3 %4\n\n"
                          u"**Category:** %5\n\n"
                          u"**Formula:** `%6`\n"_s
                              .arg(valueStr, conv.fromUnit, resultStr, conv.toUnit,
                                   conv.category, conv.formula);

            auto copyText = u"%1 %2"_s.arg(resultStr, conv.toUnit);

            auto item = StandardRichItem::make(
                u"conv"_s,
                text,
                u"%1 | %2"_s.arg(conv.category, conv.formula),
                [] { return Icon::grapheme(u"\U0001f4d0"_s); },
                vector<Action>{
                    {
                        u"copy"_s, tr("Copy result"),
                        [copyText] { setClipboardText(copyText); }
                    },
                    {
                        u"copyv"_s, tr("Copy value only"),
                        [resultStr] { setClipboardText(resultStr); }
                    }
                }
            );

            item->setDetailMarkdown(detail);
            item->setMetadata({
                {u"Category"_s, conv.category, {}},
                {u"Formula"_s, conv.formula, {}}
            });

            results.emplace_back(move(item), 1.0);
            return results;
        }
    }

    return results;
}
