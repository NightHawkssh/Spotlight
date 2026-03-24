// Copyright (c) 2026 Spotlight Contributors

#include "plugin.h"
#include <albert/icon.h>
#include <albert/matcher.h>
#include <albert/standardrichitem.h>
#include <albert/systemutil.h>
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

static QString toCodePoints(const QString &emoji)
{
    QStringList parts;
    int i = 0;
    while (i < emoji.size()) {
        uint cp;
        if (emoji[i].isHighSurrogate() && i + 1 < emoji.size()) {
            cp = QChar::surrogateToUcs4(emoji[i], emoji[i + 1]);
            i += 2;
        } else {
            cp = emoji[i].unicode();
            i += 1;
        }
        // Skip variation selectors (FE0F) for cleaner display
        if (cp != 0xFE0F)
            parts.append(u"U+%1"_s.arg(cp, 4, 16, u'0').toUpper());
    }
    return parts.join(u' ');
}

Plugin::Plugin()
{
    emojis_ = {
        {u"\U0001f600"_s, u"grinning face"_s},
        {u"\U0001f601"_s, u"beaming face with smiling eyes"_s},
        {u"\U0001f602"_s, u"face with tears of joy"_s},
        {u"\U0001f923"_s, u"rolling on the floor laughing"_s},
        {u"\U0001f603"_s, u"grinning face with big eyes"_s},
        {u"\U0001f604"_s, u"grinning face with smiling eyes"_s},
        {u"\U0001f605"_s, u"grinning face with sweat"_s},
        {u"\U0001f606"_s, u"grinning squinting face"_s},
        {u"\U0001f609"_s, u"winking face"_s},
        {u"\U0001f60a"_s, u"smiling face with smiling eyes"_s},
        {u"\U0001f60b"_s, u"face savoring food"_s},
        {u"\U0001f60e"_s, u"smiling face with sunglasses"_s},
        {u"\U0001f60d"_s, u"smiling face with heart eyes"_s},
        {u"\U0001f618"_s, u"face blowing a kiss"_s},
        {u"\U0001f617"_s, u"kissing face"_s},
        {u"\U0001f619"_s, u"kissing face with smiling eyes"_s},
        {u"\U0001f61a"_s, u"kissing face with closed eyes"_s},
        {u"\U0001f642"_s, u"slightly smiling face"_s},
        {u"\U0001f917"_s, u"hugging face"_s},
        {u"\U0001f914"_s, u"thinking face"_s},
        {u"\U0001f610"_s, u"neutral face"_s},
        {u"\U0001f611"_s, u"expressionless face"_s},
        {u"\U0001f636"_s, u"face without mouth"_s},
        {u"\U0001f644"_s, u"face with rolling eyes"_s},
        {u"\U0001f60f"_s, u"smirking face"_s},
        {u"\U0001f623"_s, u"persevering face"_s},
        {u"\U0001f625"_s, u"sad but relieved face"_s},
        {u"\U0001f62e"_s, u"face with open mouth"_s},
        {u"\U0001f910"_s, u"zipper mouth face"_s},
        {u"\U0001f62f"_s, u"hushed face"_s},
        {u"\U0001f62a"_s, u"sleepy face"_s},
        {u"\U0001f62b"_s, u"tired face"_s},
        {u"\U0001f634"_s, u"sleeping face"_s},
        {u"\U0001f60c"_s, u"relieved face"_s},
        {u"\U0001f61b"_s, u"face with tongue"_s},
        {u"\U0001f61c"_s, u"winking face with tongue"_s},
        {u"\U0001f61d"_s, u"squinting face with tongue"_s},
        {u"\U0001f924"_s, u"drooling face"_s},
        {u"\U0001f612"_s, u"unamused face"_s},
        {u"\U0001f613"_s, u"downcast face with sweat"_s},
        {u"\U0001f614"_s, u"pensive face"_s},
        {u"\U0001f615"_s, u"confused face"_s},
        {u"\U0001f643"_s, u"upside down face"_s},
        {u"\U0001f911"_s, u"money mouth face"_s},
        {u"\U0001f632"_s, u"astonished face"_s},
        {u"\U0001f641"_s, u"slightly frowning face"_s},
        {u"\U0001f616"_s, u"confounded face"_s},
        {u"\U0001f61e"_s, u"disappointed face"_s},
        {u"\U0001f61f"_s, u"worried face"_s},
        {u"\U0001f624"_s, u"face with steam from nose"_s},
        {u"\U0001f622"_s, u"crying face"_s},
        {u"\U0001f62d"_s, u"loudly crying face"_s},
        {u"\U0001f626"_s, u"frowning face with open mouth"_s},
        {u"\U0001f627"_s, u"anguished face"_s},
        {u"\U0001f628"_s, u"fearful face"_s},
        {u"\U0001f629"_s, u"weary face"_s},
        {u"\U0001f62c"_s, u"grimacing face"_s},
        {u"\U0001f630"_s, u"anxious face with sweat"_s},
        {u"\U0001f631"_s, u"face screaming in fear"_s},
        {u"\U0001f633"_s, u"flushed face"_s},
        {u"\U0001f635"_s, u"dizzy face"_s},
        {u"\U0001f621"_s, u"pouting face"_s},
        {u"\U0001f620"_s, u"angry face"_s},
        {u"\U0001f637"_s, u"face with medical mask"_s},
        {u"\U0001f912"_s, u"face with thermometer"_s},
        {u"\U0001f915"_s, u"face with head bandage"_s},
        {u"\U0001f922"_s, u"nauseated face"_s},
        {u"\U0001f927"_s, u"sneezing face"_s},
        {u"\U0001f608"_s, u"smiling face with horns"_s},
        {u"\U0001f47f"_s, u"angry face with horns"_s},
        {u"\U0001f480"_s, u"skull"_s},
        {u"\U0001f47b"_s, u"ghost"_s},
        {u"\U0001f47d"_s, u"alien"_s},
        {u"\U0001f916"_s, u"robot"_s},
        {u"\U0001f4a9"_s, u"pile of poo"_s},
        {u"\u2764\ufe0f"_s, u"red heart"_s},
        {u"\U0001f49b"_s, u"yellow heart"_s},
        {u"\U0001f49a"_s, u"green heart"_s},
        {u"\U0001f499"_s, u"blue heart"_s},
        {u"\U0001f49c"_s, u"purple heart"_s},
        {u"\U0001f494"_s, u"broken heart"_s},
        {u"\U0001f495"_s, u"two hearts"_s},
        {u"\U0001f525"_s, u"fire"_s},
        {u"\u2b50"_s, u"star"_s},
        {u"\U0001f31f"_s, u"glowing star"_s},
        {u"\U0001f4a5"_s, u"collision"_s},
        {u"\U0001f44d"_s, u"thumbs up"_s},
        {u"\U0001f44e"_s, u"thumbs down"_s},
        {u"\U0001f44f"_s, u"clapping hands"_s},
        {u"\U0001f64f"_s, u"folded hands"_s},
        {u"\U0001f4aa"_s, u"flexed biceps"_s},
        {u"\U0001f44b"_s, u"waving hand"_s},
        {u"\u270c\ufe0f"_s, u"victory hand"_s},
        {u"\U0001f44c"_s, u"ok hand"_s},
        {u"\U0001f918"_s, u"sign of the horns"_s},
        {u"\u261d\ufe0f"_s, u"index pointing up"_s},
        {u"\U0001f449"_s, u"backhand index pointing right"_s},
        {u"\U0001f448"_s, u"backhand index pointing left"_s},
        {u"\U0001f446"_s, u"backhand index pointing up"_s},
        {u"\U0001f447"_s, u"backhand index pointing down"_s},
        {u"\u2705"_s, u"check mark"_s},
        {u"\u274c"_s, u"cross mark"_s},
    };
}

QString Plugin::defaultTrigger() const { return u"emoji "_s; }

QString Plugin::synopsis(const QString &) const { return u"<search>"_s; }

vector<RankItem> Plugin::rankItems(QueryContext &ctx)
{
    vector<RankItem> results;

    Matcher matcher(ctx);

    for (const auto &entry : emojis_)
    {
        if (!ctx.isValid())
            return results;

        if (auto m = matcher.match(entry.name); m)
        {
            auto codePoints = toCodePoints(entry.emoji);

            auto detail = u"## %1 %2\n\n"
                          u"**Code point:** `%3`\n"_s
                              .arg(entry.emoji, entry.name, codePoints);

            auto item = StandardRichItem::make(
                entry.name,
                u"%1  %2"_s.arg(entry.emoji, entry.name),
                codePoints,
                [e = entry.emoji] { return Icon::grapheme(e); },
                vector<Action>{
                    {
                        u"copy"_s, tr("Copy emoji"),
                        [e = entry.emoji] { setClipboardText(e); }
                    },
                    {
                        u"cp"_s, tr("Copy code point"),
                        [codePoints] { setClipboardText(codePoints); }
                    }
                }
            );

            item->setDetailMarkdown(detail);

            results.emplace_back(move(item), m);
        }
    }

    return results;
}
