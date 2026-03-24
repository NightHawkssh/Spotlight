// Copyright (c) 2026 Spotlight Contributors

#include "plugin.h"
#include <QCoroGenerator>
#include <QProcess>
#include <QStandardPaths>
#include <albert/icon.h>
#include <albert/logging.h>
#include <albert/matcher.h>
#include <albert/standardrichitem.h>
#include <albert/systemutil.h>
ALBERT_LOGGING_CATEGORY("screencapture")
using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

namespace
{

struct ScreenshotOption
{
    QString id;
    QString name;
    QString description;
    // Returns {tool, args} for the given screenshot tool
    function<pair<QString, QStringList>(const QString &tool)> command;
};

static QString detectTool()
{
    // Try common screenshot tools in order of preference
    static const QStringList tools = {
        u"spectacle"_s,
        u"gnome-screenshot"_s,
        u"scrot"_s,
        u"maim"_s,
        u"xfce4-screenshooter"_s,
    };

    for (const auto &tool : tools)
    {
        auto path = QStandardPaths::findExecutable(tool);
        if (!path.isEmpty())
            return tool;
    }

    return {};
}

static pair<QString, QStringList> fullScreenCommand(const QString &tool)
{
    if (tool == u"spectacle"_s)
        return {tool, {u"-f"_s, u"-b"_s}};
    if (tool == u"gnome-screenshot"_s)
        return {tool, {}};
    if (tool == u"scrot"_s)
        return {tool, {}};
    if (tool == u"maim"_s)
        return {tool, {u"/tmp/screenshot.png"_s}};
    if (tool == u"xfce4-screenshooter"_s)
        return {tool, {u"-f"_s}};
    return {};
}

static pair<QString, QStringList> activeWindowCommand(const QString &tool)
{
    if (tool == u"spectacle"_s)
        return {tool, {u"-a"_s, u"-b"_s}};
    if (tool == u"gnome-screenshot"_s)
        return {tool, {u"-w"_s}};
    if (tool == u"scrot"_s)
        return {tool, {u"-u"_s}};
    if (tool == u"maim"_s)
        return {tool, {u"-i"_s, u"$(xdotool getactivewindow)"_s}};
    if (tool == u"xfce4-screenshooter"_s)
        return {tool, {u"-w"_s}};
    return {};
}

}  // namespace

Plugin::Plugin()
{
    screenshotTool_ = detectTool();
    if (screenshotTool_.isEmpty())
        WARN << u"No screenshot tool found on this system"_s;
    else
        INFO << u"Using screenshot tool:"_s << screenshotTool_;
}

QString Plugin::defaultTrigger() const { return u"ss "_s; }

QString Plugin::synopsis(const QString &) const { return u"[full|window]"_s; }

ItemGenerator Plugin::items(QueryContext &ctx)
{
    vector<shared_ptr<Item>> items;

    if (screenshotTool_.isEmpty())
    {
        auto item = StandardRichItem::make(
            u"notools"_s,
            tr("No screenshot tool found"),
            tr("Install spectacle, gnome-screenshot, scrot, maim, or xfce4-screenshooter"),
            [] { return Icon::grapheme(u"\u26a0\ufe0f"_s); }
        );
        item->setDetailMarkdown(
            u"## No Screenshot Tool Found\n\n"
            u"Please install one of the supported screenshot tools:\n\n"
            u"- **spectacle** (KDE)\n"
            u"- **gnome-screenshot** (GNOME)\n"
            u"- **scrot**\n"
            u"- **maim**\n"
            u"- **xfce4-screenshooter** (XFCE)\n"_s
        );
        items.push_back(move(item));
        co_yield items;
        co_return;
    }

    Matcher matcher(ctx.query(), {});
    auto tool = screenshotTool_;

    // Full screen option
    {
        auto name = tr("Full Screen");
        if (ctx.query().isEmpty() || matcher.match(name))
        {
            auto detail = u"## Full Screen Screenshot\n\n"
                          u"Capture the entire screen using **%1**.\n"_s
                              .arg(tool);

            auto item = StandardRichItem::make(
                u"full"_s,
                name,
                tr("Capture the entire screen using %1").arg(tool),
                [] { return Icon::grapheme(u"\U0001f5b5"_s); },
                vector<Action>{
                    {
                        u"take"_s, tr("Take screenshot"),
                        [tool]
                        {
                            auto [cmd, args] = fullScreenCommand(tool);
                            if (!cmd.isEmpty())
                                runDetachedProcess(QStringList{cmd} + args);
                        }
                    }
                }
            );

            item->setDetailMarkdown(detail);
            item->setMetadata({
                {u"Tool"_s, tool, {}},
                {u"Mode"_s, u"Full Screen"_s, {}}
            });

            items.push_back(move(item));
        }
    }

    // Active window option
    {
        auto name = tr("Active Window");
        if (ctx.query().isEmpty() || matcher.match(name))
        {
            auto detail = u"## Active Window Screenshot\n\n"
                          u"Capture the currently active window using **%1**.\n"_s
                              .arg(tool);

            auto item = StandardRichItem::make(
                u"window"_s,
                name,
                tr("Capture the active window using %1").arg(tool),
                [] { return Icon::grapheme(u"\U0001f5bc"_s); },
                vector<Action>{
                    {
                        u"take"_s, tr("Take screenshot"),
                        [tool]
                        {
                            auto [cmd, args] = activeWindowCommand(tool);
                            if (!cmd.isEmpty())
                                runDetachedProcess(QStringList{cmd} + args);
                        }
                    }
                }
            );

            item->setDetailMarkdown(detail);
            item->setMetadata({
                {u"Tool"_s, tool, {}},
                {u"Mode"_s, u"Active Window"_s, {}}
            });

            items.push_back(move(item));
        }
    }

    co_yield items;
}
